
#include "stdafx.h"
#include "spacemonger.h"
#include "FolderTree.h"
#include "Lang.h"

#include <ppl.h>
#include <thread>
#include <algorithm>
#include <numeric>
#include <unordered_set>

//////////////////////////////////////////////////////////////////////////////

CString MakeLongPath(const CString &path)
{
	if (path.Left(4) == L"\\\\?\\") return path;
	if (path.Left(2) == L"\\\\") return L"\\\\?\\UNC\\" + path.Mid(2);
	return L"\\\\?\\" + path;
}

CScanState::CScanState()
	: numfiles(0), numfolders(0), filespace(0), cancelled(false), done(false), lastpathtick(0)
{
	clustersize = 4096;
	skipcloudonly = 0;
	hardlinks = 0;
}

void CScanState::SetPath(const CString &path)
{
	std::lock_guard<std::mutex> lock(pathlock);
	curpath = path;
}

CString CScanState::GetPath(void)
{
	std::lock_guard<std::mutex> lock(pathlock);
	return curpath;
}

//////////////////////////////////////////////////////////////////////////////

CFolderTree::CFolderTree()
{
	root = cur = NULL;
	freespace = usedspace = totalspace = 0;
	clustersize = 4096;
	numfiles = numfolders = filespace = 0;
	hardlinks = 0;
	m_path = L"";
}

CFolderTree::~CFolderTree()
{
	if (root != NULL) delete root;
}

BOOL CFolderTree::IsVolumeRoot(void) const
{
	wchar_t volume[MAX_PATH];
	if (m_path.IsEmpty() || !GetVolumePathName(m_path, volume, MAX_PATH)) return 1;
	return CString(volume).CompareNoCase(m_path) == 0;
}

// Runs the message loop for the scanning dialog until the background
// scan finishes.  Returns FALSE if the application is quitting.
static BOOL PumpWhileScanning(CFolderTree *tree, CFolderDialog *dialog, CScanState *state)
{
	BOOL quitting = 0;
	MSG msg;
	while (!state->done) {
		::MsgWaitForMultipleObjects(0, NULL, FALSE, 100, QS_ALLINPUT);
		while (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				state->cancelled = true;
				quitting = 1;
				break;
			}
			if (!::IsWindow(dialog->m_hWnd) || !::IsDialogMessage(dialog->m_hWnd, &msg)) {
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}
		if (dialog->m_hWnd != NULL && dialog->state != NULL)
			dialog->UpdateDisplay(tree, state);
	}
	if (quitting) ::PostQuitMessage(0);
	return !quitting;
}

BOOL CFolderTree::LoadTree(const CString &path, BOOL includespace, CWnd *modalwin)
{
	if (path == L"") return 1;

	// Make the pathname canonical and complete, with a trailing backslash.
	wchar_t fullpath[32768];
	DWORD len = GetFullPathName(path, 32768, fullpath, NULL);
	CString scanpath = (len > 0 && len < 32768) ? CString(fullpath) : path;
	if (scanpath.Right(1) != L"\\") scanpath += L"\\";

	m_path = scanpath;
	GetSpace(scanpath);

	BOOL wholedrive = IsVolumeRoot();

	CScanState state;
	state.clustersize = clustersize;
	state.skipcloudonly = theApp.m_settings.skip_cloud_only;
	state.hardlinks = hardlinks;
	state.SetPath(scanpath);

	CFolderDialog dialog;
	dialog.Create(IDD_SCAN_DIALOG, modalwin);
	dialog.state = &state;
	dialog.Reset(wholedrive);

	CFolder *newroot = new CFolder;

	std::thread worker([&]() {
		newroot->LoadFolder(&state, scanpath);
		if (!state.cancelled) {
			if (state.hardlinks) {
				// Find the file IDs that appear more than once, then keep only
				// the first copy of each hard-linked file.
				std::vector<ui64> ids;
				newroot->CollectFileIds(ids);
				std::sort(ids.begin(), ids.end());
				std::vector<ui64> dups;
				for (size_t i = 1; i < ids.size(); i++)
					if (ids[i] == ids[i-1] && (dups.empty() || dups.back() != ids[i]))
						dups.push_back(ids[i]);
				ids.clear();
				ids.shrink_to_fit();
				if (!dups.empty()) {
					std::unordered_set<ui64> seen;
					newroot->RemoveDuplicateLinks(dups, seen);
				}
			}
			newroot->ComputeSizes();
		}
		state.done = true;
		if (::IsWindow(dialog.m_hWnd)) ::PostMessage(dialog.m_hWnd, WM_NULL, 0, 0);
	});

	BOOL keepgoing = PumpWhileScanning(this, &dialog, &state);
	worker.join();

	if (state.cancelled || !keepgoing) {
		delete newroot;
		dialog.state = NULL;
		if (::IsWindow(dialog.m_hWnd)) dialog.DestroyWindow();
		return 0;
	}

	if (root != NULL) delete root;
	root = cur = newroot;
	filespace = state.filespace;
	numfiles = state.numfiles;
	numfolders = state.numfolders;

	dialog.ForcedUpdate(this, &state);
	if (includespace && wholedrive) {
		root->AddFile(this, L"<", 1, freespace, freespace, 0);
		root->Finalize();
	}

	// Leave the finished dialog up very briefly so the user can see the totals.
	ULONGLONG start = GetTickCount64();
	MSG msg;
	while (GetTickCount64() - start < 400) {
		while (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		Sleep(10);
	}
	dialog.state = NULL;
	if (::IsWindow(dialog.m_hWnd)) dialog.DestroyWindow();
	if (modalwin != NULL)
		modalwin->SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	return 1;
}

void CFolderTree::GetSpace(const CString &path)
{
	DWORD SecPerClus = 8, BytesPerSec = 512, ClusPerDisk = 0, FreeClus = 0;
	ULARGE_INTEGER avail, total, totalfree;

	wchar_t volume[MAX_PATH];
	if (!GetVolumePathName(path, volume, MAX_PATH))
		wcsncpy_s(volume, path, _TRUNCATE);

	// First, compute the cluster size (which will be needed later)
	if (GetDiskFreeSpace(volume, &SecPerClus, &BytesPerSec, &FreeClus, &ClusPerDisk))
		clustersize = (ui64)BytesPerSec * (ui64)SecPerClus;
	else clustersize = 4096;
	if (clustersize == 0) clustersize = 4096;

	if (GetDiskFreeSpaceEx(path, &avail, &total, &totalfree)) {
		freespace = totalfree.QuadPart;
		totalspace = total.QuadPart;
	}
	else {
		freespace = clustersize * (ui64)FreeClus;
		totalspace = clustersize * (ui64)ClusPerDisk;
	}
	usedspace = totalspace - freespace;

	// Only NTFS-like volumes have hard links (and stable file IDs).
	DWORD flags = 0;
	hardlinks = GetVolumeInformation(volume, NULL, 0, NULL, NULL, &flags, NULL, 0)
		&& (flags & FILE_SUPPORTS_HARD_LINKS) != 0;
}

CFolder *CFolderTree::GetRoot(void)
{
	return(root);
}

CFolder *CFolderTree::SetCur(const CString &path)
{
	return(cur);
}

CFolder *CFolderTree::GetCur(void)
{
	return(cur);
}

CFolder *CFolderTree::Down(unsigned int index)
{
	if (index < cur->cur) {
		CFolder *newfolder = cur->children[index];
		if (newfolder != NULL) {
			cur = newfolder;
			return(cur);
		}
	}
	return(NULL);
}

CFolder *CFolderTree::Up(void)
{
	if (cur != root) {
		cur = cur->parent;
		return(cur);
	}
	return(NULL);
}

//////////////////////////////////////////////////////////////////////////////

static wchar_t *cpp_strdup(const wchar_t *string, int stringlen)
{
	wchar_t *newstr = (wchar_t *)malloc((stringlen + 1) * sizeof(wchar_t));
	if (newstr == NULL) return(NULL);
	memcpy(newstr, string, stringlen * sizeof(wchar_t));
	newstr[stringlen] = L'\0';
	return(newstr);
}

CFolder::CFolder()
{
	cur = 0;
	max = 2;
	names = (wchar_t **)malloc(sizeof(wchar_t *) * max);
	sizes = (ui64 *)malloc(sizeof(ui64) * max);
	actualsizes = (ui64 *)malloc(sizeof(ui64) * max);
	children = (CFolder **)malloc(sizeof(CFolder *) * max);
	times = (ui64 *)malloc(sizeof(ui64) * max);
	fileids = (ui64 *)malloc(sizeof(ui64) * max);
	size_self = size_children = 0;
	parent = NULL;
	parentindex = 0;
}

CFolder::~CFolder()
{
	unsigned int i;

	for (i = 0; i < cur; i++) {
		if (names[i] != NULL) free(names[i]);
		if (children[i] != NULL) delete children[i];
	}

	free(names);
	free(children);
	free(sizes);
	free(actualsizes);
	free(times);
	free(fileids);

	cur = max = 0;
	size_self = size_children = 0;
	parent = NULL;
	parentindex = 0;
}

void CFolder::MoreEntries(void)
{
	unsigned int newmax = max * 2;
	names = (wchar_t **)realloc(names, sizeof(wchar_t *) * newmax);
	sizes = (ui64 *)realloc(sizes, sizeof(ui64) * newmax);
	actualsizes = (ui64 *)realloc(actualsizes, sizeof(ui64) * newmax);
	times = (ui64 *)realloc(times, sizeof(ui64) * newmax);
	children = (CFolder **)realloc(children, sizeof(CFolder *) * newmax);
	fileids = (ui64 *)realloc(fileids, sizeof(ui64) * newmax);
	max = newmax;
}

void CFolder::AddFile(CFolderTree *tree, const wchar_t *name, ui32 namelen, ui64 size,
	ui64 actual_size, ui64 time, ui64 fileid)
{
	if (cur >= max) MoreEntries();

	names[cur] = cpp_strdup(name, namelen);
	actualsizes[cur] = actual_size;
	times[cur] = time;
	fileids[cur] = fileid;
	size_self += (sizes[cur] = size);
	children[cur] = NULL;
	cur++;
	if (tree != NULL) tree->filespace += size;
}

void CFolder::AddFolder(CFolderTree *tree, const wchar_t *name, ui32 namelen, CFolder *folder,
	ui64 time)
{
	if (cur >= max) MoreEntries();

	names[cur] = cpp_strdup(name, namelen);
	size_children += (sizes[cur] = folder->SizeTotal());
	actualsizes[cur] = sizes[cur];
	times[cur] = time;
	fileids[cur] = 0;
	children[cur] = folder;
	folder->parent = this;
	folder->parentindex = cur;
	cur++;
}

// Finalize
//   Sorts the directory by size, largest first.

void CFolder::Finalize(void)
{
	unsigned int i;

	if (cur > 1) {
		std::vector<unsigned int> order(cur);
		std::iota(order.begin(), order.end(), 0);
		std::stable_sort(order.begin(), order.end(),
			[this](unsigned int a, unsigned int b) { return sizes[a] > sizes[b]; });

		std::vector<wchar_t *> n(cur);
		std::vector<CFolder *> c(cur);
		std::vector<ui64> s(cur), a(cur), t(cur), f(cur);
		for (i = 0; i < cur; i++) {
			unsigned int j = order[i];
			n[i] = names[j], c[i] = children[j], s[i] = sizes[j];
			a[i] = actualsizes[j], t[i] = times[j], f[i] = fileids[j];
		}
		for (i = 0; i < cur; i++) {
			names[i] = n[i], children[i] = c[i], sizes[i] = s[i];
			actualsizes[i] = a[i], times[i] = t[i], fileids[i] = f[i];
		}
	}

	for (i = 0; i < cur; i++)
		if (children[i] != NULL) children[i]->parentindex = i;
}

// LoadFolder
//   Reads one folder, then reads its subfolders in parallel.  "path" must
//   end in a backslash.  Runs on the scanning threads, so it must only touch
//   this folder and the (thread-safe) scan state.

struct CPendingFolder {
	CFolder *folder;
	CString name;
	ui64 time;
};

static const DWORD ScanBufferSize = 64 * 1024;

void CFolder::LoadFolder(CScanState *state, const CString &path)
{
	if (state->cancelled) return;

	ULONGLONG tick = GetTickCount64();
	if (tick - state->lastpathtick > 100) {
		state->lastpathtick = tick;
		state->SetPath(path);
	}

	HANDLE handle = CreateFile(MakeLongPath(path), FILE_LIST_DIRECTORY,
		FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (handle == INVALID_HANDLE_VALUE) return;	// Access denied, etc.

	// Each thread reuses one buffer; it's only used before we recurse.
	thread_local std::vector<BYTE> buffer;
	if (buffer.size() < ScanBufferSize) buffer.resize(ScanBufferSize);

	const ui64 clustermask = state->clustersize - 1;
	std::vector<CPendingFolder> subfolders;

	while (!state->cancelled
		&& GetFileInformationByHandleEx(handle, FileIdBothDirectoryInfo, buffer.data(), ScanBufferSize)) {
		FILE_ID_BOTH_DIR_INFO *info = (FILE_ID_BOTH_DIR_INFO *)buffer.data();
		for (;;) {
			const wchar_t *name = info->FileName;
			ui32 namelen = info->FileNameLength / sizeof(wchar_t);
			DWORD attrib = info->FileAttributes;

			BOOL dots = (namelen == 1 && name[0] == L'.')
				|| (namelen == 2 && name[0] == L'.' && name[1] == L'.');

			// Ignore junction points, mount points, and symbolic links, so
			// that nothing gets counted twice.  For reparse points, the
			// directory listing puts the reparse tag in EaSize.
			BOOL skip = dots;
			if (!skip && (attrib & FILE_ATTRIBUTE_REPARSE_POINT)) {
				ULONG tag = info->EaSize;
				if (tag == IO_REPARSE_TAG_MOUNT_POINT || tag == IO_REPARSE_TAG_SYMLINK)
					skip = 1;
			}

			// "Online-only" cloud files (OneDrive, etc.) take no space on
			// this disk.  Folders that have never been opened locally are
			// skipped too, since listing them would download their contents.
			BOOL cloudonly = (attrib & (FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS
				| FILE_ATTRIBUTE_RECALL_ON_OPEN | FILE_ATTRIBUTE_OFFLINE)) != 0;
			if (!skip && cloudonly && state->skipcloudonly)
				skip = 1;

			if (!skip) {
				ui64 time = (ui64)info->LastWriteTime.QuadPart;
				if (attrib & FILE_ATTRIBUTE_DIRECTORY) {
					state->numfolders++;
					CPendingFolder pending;
					pending.folder = new CFolder;
					pending.name = CString(name, namelen);
					pending.time = time;
					subfolders.push_back(pending);
				}
				else {
					state->numfiles++;
					ui64 actualsize = (ui64)info->EndOfFile.QuadPart;
					ui64 size;
					if (cloudonly) {
						// Not on disk; show how big it would be if downloaded.
						size = (actualsize + clustermask) & ~clustermask;
					}
					else size = (ui64)info->AllocationSize.QuadPart;
					ui64 fileid = state->hardlinks ? (ui64)info->FileId.QuadPart : 0;
					AddFile(NULL, name, namelen, size, actualsize, time, fileid);
					state->filespace += size;
				}
			}

			if (info->NextEntryOffset == 0) break;
			info = (FILE_ID_BOTH_DIR_INFO *)((BYTE *)info + info->NextEntryOffset);
		}
	}
	CloseHandle(handle);

	// Now scan the subfolders, in parallel.
	if (subfolders.size() == 1) {
		subfolders[0].folder->LoadFolder(state, path + subfolders[0].name + L"\\");
	}
	else if (subfolders.size() > 1) {
		concurrency::task_group tasks;
		for (size_t i = 0; i < subfolders.size(); i++) {
			CFolder *folder = subfolders[i].folder;
			CString subpath = path + subfolders[i].name + L"\\";
			tasks.run([folder, state, subpath]() { folder->LoadFolder(state, subpath); });
		}
		tasks.wait();
	}

	for (size_t i = 0; i < subfolders.size(); i++) {
		AddFolder(NULL, subfolders[i].name, subfolders[i].name.GetLength(),
			subfolders[i].folder, subfolders[i].time);
	}
}

void CFolder::CollectFileIds(std::vector<ui64> &ids)
{
	for (unsigned int i = 0; i < cur; i++) {
		if (children[i] != NULL) children[i]->CollectFileIds(ids);
		else if (fileids[i] != 0) ids.push_back(fileids[i]);
	}
}

// Hard links are one file with several names.  Keep the size on the first
// name we find, and count the others as taking no space.
void CFolder::RemoveDuplicateLinks(const std::vector<ui64> &dups, std::unordered_set<ui64> &seen)
{
	for (unsigned int i = 0; i < cur; i++) {
		if (children[i] != NULL) {
			children[i]->RemoveDuplicateLinks(dups, seen);
			continue;
		}
		if (fileids[i] == 0 || !std::binary_search(dups.begin(), dups.end(), fileids[i]))
			continue;
		if (!seen.insert(fileids[i]).second) {
			size_self -= sizes[i];
			sizes[i] = 0;
		}
	}
}

// Adds up the folder sizes from the bottom up, and sorts every folder.
void CFolder::ComputeSizes(void)
{
	size_children = 0;
	for (unsigned int i = 0; i < cur; i++) {
		if (children[i] != NULL) {
			children[i]->ComputeSizes();
			children[i]->parent = this;
			sizes[i] = actualsizes[i] = children[i]->SizeTotal();
			size_children += sizes[i];
		}
	}
	Finalize();
}

void CFolder::DelFile(unsigned int index)
{
}

void CFolder::RenameFile(unsigned int index, const CString &name)
{
}

unsigned int CFolder::FindFile(const CString &name)
{
	return((unsigned int)-1);
}

//////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CFolderDialog, CDialog)

BEGIN_MESSAGE_MAP(CFolderDialog, CDialog)
	//{{AFX_MSG_MAP(CFolderDialog)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CFolderDialog::CFolderDialog()
{
	state = NULL;
	wholedrive = 1;
	last_tick = 0;
	cur_frame = 0;
}

CFolderDialog::~CFolderDialog()
{
}

void CFolderDialog::OnCancel(void)
{
	if (state != NULL) state->cancelled = true;
	GetDlgItem(IDCANCEL)->EnableWindow(0);
}

HBRUSH CFolderDialog::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
	HBRUSH brush = Theme_OnCtlColor(pDC->m_hDC, pWnd->m_hWnd, nCtlColor);
	return brush != NULL ? brush : CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CFolderDialog::OnEraseBkgnd(CDC *pDC)
{
	if (!Theme_IsDark()) return CDialog::OnEraseBkgnd(pDC);
	CRect rect;
	GetClientRect(&rect);
	pDC->FillRect(&rect, CBrush::FromHandle(Theme_Brush(COLOR_WINDOW)));
	return 1;
}

void CFolderDialog::OnPaint()
{
	CPaintDC dc(this);
	DrawAnimation(&dc);
}

void CFolderDialog::DrawAnimation(CDC *dc)
{
	int dpi = Dpi_ForWindow(m_hWnd);
	CDC srcdc;
	srcdc.CreateCompatibleDC(dc);
	HGDIOBJ old = ::SelectObject(srcdc.m_hDC, theApp.m_scan_animation[(cur_frame >> 1) & 3]);
	CRect rect;
	GetClientRect(&rect);
	int w = Dpi_Scale(128, dpi), h = Dpi_Scale(48, dpi);
	dc->SetStretchBltMode(COLORONCOLOR);
	int x = Dpi_Scale(8, dpi), y = rect.bottom - Dpi_Scale(48 + 8 + 12, dpi);
	if (Theme_IsDark()) {
		// The bitmap's background was mapped to the (light) window color
		// when it was loaded; let the dark dialog show through instead.
		dc->FillSolidRect(x, y, w, h, Theme_Color(COLOR_WINDOW));
		::TransparentBlt(dc->m_hDC, x, y, w, h, srcdc.m_hDC, 0, 0, 128, 48,
			GetSysColor(COLOR_WINDOW));
	}
	else dc->StretchBlt(x, y, w, h, &srcdc, 0, 0, 128, 48, SRCCOPY);
	::SelectObject(srcdc.m_hDC, old);
}

static CString FormatCount(ui64 value)
{
	CString result;
	result.Format(L"%I64u", value);
	for (int pos = result.GetLength() - 3; pos > 0; pos -= 3)
		result.Insert(pos, CurLang->digitcomma);
	return result;
}

void CFolderDialog::Reset(BOOL drive)
{
	if (!IsWindow(m_hWnd)) return;

	wholedrive = drive;
	last_tick = 0;
	cur_frame = 0;

	Theme_ApplyToWindow(m_hWnd);
	SetWindowText(wholedrive ? CurLang->scanning : CurLang->scanningfolder);
	SetDlgItemText(IDC_STATIC_FILESFOUND, CurLang->files_found);
	SetDlgItemText(IDC_STATIC_FOLDERSFOUND, CurLang->folders_found);
	SetDlgItemText(IDCANCEL, CurLang->cancel);
	SetDlgItemText(IDC_PATH, L"");
	SetDlgItemText(IDC_FILES, L"0");
	SetDlgItemText(IDC_FOLDERS, L"0");
	CProgressCtrl *progress = (CProgressCtrl *)GetDlgItem(IDC_LOAD_PROGRESS);
	if (progress != NULL) {
		if (wholedrive) {
			progress->SetRange(0, 4096);
			progress->SetPos(0);
		}
		else {
			// We can't know how big a folder is before we scan it.
			progress->ModifyStyle(0, PBS_MARQUEE);
			progress->SetMarquee(TRUE, 30);
		}
	}
	ShowWindow(SW_SHOW);
	UpdateWindow();
}

void CFolderDialog::UpdateDisplay(CFolderTree *tree, CScanState *st)
{
	// Guarantee that we don't update more often than 1/10 of a second
	ULONGLONG tick = ::GetTickCount64();
	if (tick < last_tick + 100) return;
	last_tick = tick;

	if (!IsWindow(m_hWnd)) return;

	SetDlgItemText(IDC_PATH, st->GetPath());
	SetDlgItemText(IDC_FILES, FormatCount(st->numfiles));
	SetDlgItemText(IDC_FOLDERS, FormatCount(st->numfolders));
	CProgressCtrl *progress = (CProgressCtrl *)GetDlgItem(IDC_LOAD_PROGRESS);
	if (progress != NULL && wholedrive) {
		ui64 filespace = st->filespace / 16384;
		ui64 usedspace = tree->usedspace / 16384;
		if (usedspace <= 0) usedspace = 1;
		if (filespace > usedspace) filespace = usedspace;
		progress->SetPos((int)(filespace * 4096 / usedspace));
	}

	cur_frame = (cur_frame + 1) & 7;
	if (!(cur_frame & 1)) {
		CClientDC dc(this);
		DrawAnimation(&dc);
	}
}

void CFolderDialog::ForcedUpdate(CFolderTree *tree, CScanState *st)
{
	if (!IsWindow(m_hWnd)) return;

	SetDlgItemText(IDC_PATH, tree->m_path);
	SetDlgItemText(IDC_FILES, FormatCount(st->numfiles));
	SetDlgItemText(IDC_FOLDERS, FormatCount(st->numfolders));
	GetDlgItem(IDCANCEL)->EnableWindow(0);
	CProgressCtrl *progress = (CProgressCtrl *)GetDlgItem(IDC_LOAD_PROGRESS);
	if (progress != NULL) {
		progress->SetMarquee(FALSE, 0);
		progress->ModifyStyle(PBS_MARQUEE, 0);
		progress->SetRange(0, 4096);
		progress->SetPos(4096);
	}
}
