
#include "StdAfx.h"
#include "SpaceMonger.h"
#include "Lang.h"
#include "MainFrm.h"
#include "FolderTree.h"
#include "FolderView.h"
#include "DriveDialog.h"
#include "SetupDlg.h"
#include "TipWnd.h"
#include "AboutDialog.h"
#include "LargestDialog.h"
#include "FileTypes.h"

/////////////////////////////////////////////////////////////////////////////
// The primary CSpaceMonger object

CSpaceMonger theApp;

/////////////////////////////////////////////////////////////////////////////
// CApp

BEGIN_MESSAGE_MAP(CSpaceMonger, CWinApp)
	//{{AFX_MSG_MAP(CSpaceMonger)
	ON_COMMAND(ID_APP_ABOUT, OnAbout)
	ON_COMMAND(ID_APP_EXIT, OnAppExit)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_OPEN_FOLDER, OnFileOpenFolder)
	ON_COMMAND(ID_FILE_REFRESH, OnFileRefresh)
	ON_COMMAND(ID_FILE_RUN, OnFileRun)
	ON_COMMAND(ID_FILE_EXPLORE, OnFileExplore)
	ON_COMMAND(ID_FILE_DELETE, OnFileDelete)
	ON_COMMAND(ID_FILE_PROPERTIES, OnFileProperties)
	ON_COMMAND(ID_VIEW_ZOOM_FULL, OnZoomFull)
	ON_COMMAND(ID_VIEW_ZOOM_IN, OnZoomIn)
	ON_COMMAND(ID_VIEW_ZOOM_OUT, OnZoomOut)
	ON_COMMAND(ID_VIEW_FREE, OnShowFree)
	ON_COMMAND(ID_SETTINGS, OnSettings)
	ON_COMMAND(ID_VIEW_LARGEST, OnViewLargest)
	ON_COMMAND(ID_FILE_EXPORT, OnFileExport)
	ON_COMMAND(ID_FILE_RUNASADMIN, OnFileRunAsAdmin)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI_RANGE(100, 41000, OnIgnoreUpdate)
END_MESSAGE_MAP()

CSpaceMonger::CSpaceMonger()
{
	m_document = NULL;
	m_view = NULL;
	m_mainframe = NULL;
	m_largest = NULL;
}

CSpaceMonger::~CSpaceMonger()
{
}

BOOL CSpaceMonger::InitInstance()
{
	int i;

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE);

	INITCOMMONCONTROLSEX icc;
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_WIN95_CLASSES|ICC_STANDARD_CLASSES;
	InitCommonControlsEx(&icc);

	InitTipWnd(AfxGetInstanceHandle());

	// Load in the settings from the Registry.  If there were no settings,
	// we load defaults.  Once we've got them, save them back, which sounds
	// odd, but it will end up creating the necessary entries if they don't
	// exist.
	m_settings.Load();
	m_settings.Save();

	Theme_Init();
	Theme_Update(m_settings.theme);

	int showcmd = m_settings.showcmd;
	RECT rect = m_settings.rect;
	if (!m_settings.save_pos) {
		showcmd = SW_SHOWNORMAL;
		rect.top = rect.left = CW_USEDEFAULT;
		rect.right = rect.bottom = 0;	// This is actually CW_USEDEFAULT because of a bug
										// in CFrameWnd::Create()
	}

	// Create the main window
	m_mainframe = new CMainFrame;
	m_mainframe->CFrameWnd::Create(NULL, L"SpaceMonger", WS_OVERLAPPEDWINDOW, rect);
	m_pMainWnd = m_mainframe;

	// Create the one and only document
	m_document = new CFolderTree;
	m_document->Create();

	// Create the main document view
	m_view = new CFolderView;
	m_view->Create(m_mainframe, 1);
	m_view->SetDocument(m_document);
	m_view->SetFocus();

	// Load in the animations
	static int anim_name[4] = {
		IDB_SCAN1, IDB_SCAN2, IDB_SCAN3, IDB_SCAN4,
	};
	for (i = 0; i < 4; i++)
		m_scan_animation[i] = (HBITMAP)LoadImage(m_hInstance,
			MAKEINTRESOURCE(anim_name[i]), IMAGE_BITMAP, 128, 48,
			LR_LOADTRANSPARENT|LR_LOADMAP3DCOLORS);

	m_mainframe->ShowWindow(showcmd);
	m_mainframe->UpdateWindow();
	m_mainframe->RecalcLayout();

	// "SpaceMonger.exe C:\Some\Folder" starts by scanning that folder.
	if (__argc > 1 && __wargv[1][0] != L'\0') {
		// "C:\" in quotes arrives as C:" (the backslash escapes the quote),
		// so clean that up.
		CString path = __wargv[1];
		path.Trim();
		path.TrimRight(L'"');
		if (path.GetLength() == 2 && path[1] == L':') path += L"\\";
		OpenPath(path);
	}

	return(TRUE);
}

int CSpaceMonger::ExitInstance()
{
	m_settings.Save();

	// m_pMainWnd is auto-deleted by the framework
	delete m_view;
	delete m_document;

	CoUninitialize();

	return(CWinApp::ExitInstance());
}

void CSpaceMonger::OnAppExit(void)
{
	CWinApp::OnAppExit();
}

void CSpaceMonger::ApplyTheme(void)
{
	Theme_Update(m_settings.theme);
	if (m_mainframe != NULL) {
		Theme_ApplyToWindow(m_mainframe->m_hWnd);
		((CMainFrame *)m_mainframe)->m_toolbar.Reload();
	}
	if (m_view != NULL) m_view->OnUpdate(m_document);
}

void CSpaceMonger::OpenPath(const CString &path)
{
	ScanPath(path);
}

void CSpaceMonger::OnFileOpen(void)
{
	CDriveDialog drvdialog(m_mainframe);
	if (drvdialog.DoModal() != IDOK || drvdialog.m_path.IsEmpty()) return;
	OpenPath(drvdialog.m_path);
}

BOOL PickFolder(HWND owner, CString &path)
{
	IFileOpenDialog *dialog = NULL;
	if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&dialog))))
		return 0;

	BOOL result = 0;
	DWORD options = 0;
	dialog->GetOptions(&options);
	dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
	dialog->SetTitle(CurLang->choosefolder);
	if (SUCCEEDED(dialog->Show(owner))) {
		IShellItem *item = NULL;
		if (SUCCEEDED(dialog->GetResult(&item))) {
			PWSTR name = NULL;
			if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &name))) {
				path = name;
				CoTaskMemFree(name);
				result = 1;
			}
			item->Release();
		}
	}
	dialog->Release();
	return result;
}

void CSpaceMonger::OnFileOpenFolder(void)
{
	CString path;
	if (PickFolder(m_mainframe->m_hWnd, path))
		OpenPath(path);
}

void CSpaceMonger::OnFileRefresh(void)
{
	CFolderTree *ft = (CFolderTree *)m_document;
	ScanPath(ft->m_path);
}

// Scans a drive or folder.  If the scan is cancelled, whatever was showing
// before stays exactly as it was.
void CSpaceMonger::ScanPath(CString path)
{
	CFolderTree *ft = (CFolderTree *)m_document;
	if (path.IsEmpty()) return;

	wchar_t fullpath[32768];
	DWORD len = GetFullPathName(path, 32768, fullpath, NULL);
	DWORD attrib = (len > 0 && len < 32768) ? GetFileAttributes(MakeLongPath(fullpath)) : INVALID_FILE_ATTRIBUTES;
	if (attrib == INVALID_FILE_ATTRIBUTES || !(attrib & FILE_ATTRIBUTE_DIRECTORY)) {
		CString message;
		message.Format(CurLang->ex->openfailed, (LPCWSTR)path);
		AfxMessageBox(message, MB_OK|MB_ICONEXCLAMATION);
		return;
	}

	m_mainframe->EnableWindow(0);

	if (ft->LoadTree(path, 1, m_mainframe)) {
		// Only throw away the old picture once the new scan is done, so
		// that cancelling a rescan leaves the old one in place.
		m_view->SetDocument(NULL);
		m_view->SetDocument(m_document);
		ft->FreeOldTree();			// The view has let go of it now
		ft->UpdateAllViews();
		ScanChanged();
	}
	m_mainframe->EnableWindow(1);
	m_mainframe->SetActiveWindow();
}

void CSpaceMonger::ScanChanged(void)
{
	((CMainFrame *)m_mainframe)->m_toolbar.RefreshSearch();
	if (m_largest != NULL) m_largest->Refresh();
}

void CSpaceMonger::OnFileRun(void)
{
	CFolderView *fv = (CFolderView *)m_view;
	CFolderTree *ft = (CFolderTree *)m_document;

	if (fv == NULL || ft == NULL || fv->selected == NULL) return;

	CString title, path;
	if (ft != NULL) title += ft->m_path;
	fv->BuildTitleReverse(fv->selected->source, title);
	path = title;
	title += fv->selected->source->names[fv->selected->index];

	ShellExecute(m_mainframe->m_hWnd, NULL, title, NULL, path, SW_SHOWDEFAULT);
}

void CSpaceMonger::OnFileExplore(void)
{
	CFolderView *fv = (CFolderView *)m_view;
	if (fv == NULL || fv->selected == NULL) return;

	ShowInExplorer(fv->GetFullPath(fv->selected));
}

void ShowInExplorer(const CString &path)
{
	PIDLIST_ABSOLUTE pidl = NULL;
	if (SUCCEEDED(SHParseDisplayName(path, NULL, &pidl, 0, NULL))) {
		SHOpenFolderAndSelectItems(pidl, 0, NULL, 0);
		CoTaskMemFree(pidl);
	}
}

// Moves a file or folder to the Recycle Bin.  (If it's too big for the
// Recycle Bin, Windows itself asks before deleting it permanently.)
static BOOL RecyclePath(HWND owner, const CString &path)
{
	IFileOperation *op = NULL;
	if (FAILED(CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_ALL, IID_PPV_ARGS(&op))))
		return 0;

	BOOL result = 0;
	IShellItem *item = NULL;
	op->SetOwnerWindow(owner);
	op->SetOperationFlags(FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOFX_RECYCLEONDELETE
		| FOF_WANTNUKEWARNING | FOFX_ADDUNDORECORD);
	if (SUCCEEDED(SHCreateItemFromParsingName(path, NULL, IID_PPV_ARGS(&item)))) {
		if (SUCCEEDED(op->DeleteItem(item, NULL)) && SUCCEEDED(op->PerformOperations())) {
			BOOL aborted = 0;
			op->GetAnyOperationsAborted(&aborted);
			result = !aborted && GetFileAttributes(MakeLongPath(path)) == INVALID_FILE_ATTRIBUTES;
		}
		item->Release();
	}
	op->Release();
	return result;
}

// Hard links are counted once, on whichever name the scan found first.  If
// that name is recycled, move the size to another name of the same file.
static BOOL GiveSizeToOtherLink(CFolder *folder, ui64 fileid, ui64 size)
{
	if (folder == NULL) return 0;
	for (unsigned int i = 0; i < folder->cur; i++) {
		if (folder->children[i] != NULL) {
			if (GiveSizeToOtherLink(folder->children[i], fileid, size)) return 1;
		}
		else if (folder->fileids[i] == fileid && folder->sizes[i] == 0) {
			folder->sizes[i] = size;
			folder->size_self += size;
			for (CFolder *f = folder; f->parent != NULL; f = f->parent) {
				f->parent->sizes[f->parentindex] += size;
				f->parent->size_children += size;
			}
			return 1;
		}
	}
	return 0;
}

void CSpaceMonger::OnFileDelete(void)
{
	CFolderView *fv = (CFolderView *)m_view;
	CFolderTree *ft = (CFolderTree *)m_document;

	if (fv == NULL || ft == NULL || fv->selected == NULL) return;
	if (m_settings.disable_delete) return;

	CDisplayFolder *selected = fv->selected;
	CString title = fv->GetFullPath(selected);
	BOOL isfolder = (selected->source->children[selected->index] != NULL);

	CString question;
	question.Format(CurLang->recycleconfirm, (LPCWSTR)title);
	if (AfxMessageBox(question, MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2) != IDYES)
		return;

	if (!RecyclePath(m_mainframe->m_hWnd, title)) {
		AfxMessageBox(CurLang->recyclefailed, MB_OK|MB_ICONEXCLAMATION);
		return;
	}

	CFolder *folder = selected->source;
	unsigned int index = selected->index;
	ui64 fileid = isfolder ? 0 : folder->fileids[index];
	if (isfolder) {
		delete folder->children[index];
		folder->children[index] = NULL;
	}
	ui64 subsize = folder->sizes[index];
	if (isfolder) folder->size_children -= subsize;
	else folder->size_self -= subsize;
	CFolder *parent = folder->parent;
	while (parent != NULL) {
		parent->size_children -= subsize;
		parent->sizes[folder->parentindex] -= subsize;
		folder = parent;
		parent = parent->parent;
	}
	selected->source->sizes[index] = 0;
	selected->source->fileids[index] = 0;

	// If the file had other names (hard links), its space now belongs to one of them.
	if (fileid != 0 && subsize != 0) GiveSizeToOtherLink(ft->GetRoot(), fileid, subsize);


	// Rebuild the picture first: it may point into the folder just deleted.
	fv->OnUpdate(m_document);
	ScanChanged();
	if (m_settings.auto_rescan)
		OnFileRefresh();
}

void CSpaceMonger::OnZoomIn(void)
{
	((CFolderView *)m_view)->ZoomIn(((CFolderView *)m_view)->selected);
}

void CSpaceMonger::OnZoomOut(void)
{
	((CFolderView *)m_view)->ZoomOut();
}

void CSpaceMonger::OnZoomFull(void)
{
	((CFolderView *)m_view)->ZoomFull();
}

void CSpaceMonger::OnShowFree(void)
{
	CFolderView *fv = (CFolderView *)m_view;
	CMainFrame *mf = (CMainFrame *)m_mainframe;
	if (fv == NULL || mf == NULL) return;

	BOOL showfreespace = !fv->showfreespace;
	mf->m_toolbar.EnableState(CMainToolBar::ViewFree, showfreespace);
	fv->ShowFreeSpace(showfreespace);
}

void CSpaceMonger::OnSettings(void)
{
	BOOL oldskip = m_settings.skip_cloud_only;
	CSettingsDialog settingsdialog(m_mainframe);
	if (settingsdialog.DoModal() == IDOK) {
		m_settings.Save();
		ApplyTheme();
		if (m_view != NULL && m_document != NULL) {
			((CFolderView *)m_view)->UpdateTitleBar();
		}
		((CMainFrame *)m_mainframe)->m_toolbar.Reload();
		((CMainFrame *)m_mainframe)->RecalcLayout(0);
		if (m_largest != NULL) {
			// Easiest way to pick up a new language: open it again.
			m_largest->DestroyWindow();
			CLargestDialog::ShowWindowFor(m_mainframe);
		}

		// The cloud-files option changes what a scan finds, so rescan.
		CFolderTree *ft = (CFolderTree *)m_document;
		if (oldskip != m_settings.skip_cloud_only && ft != NULL && ft->GetRoot() != NULL)
			OnFileRefresh();
	}
}

void CSpaceMonger::OnFileProperties(void)
{
	CFolderView *fv = (CFolderView *)m_view;
	CFolderTree *ft = (CFolderTree *)m_document;

	if (fv == NULL || ft == NULL || fv->selected == NULL) return;

	CString path = fv->GetFullPath(fv->selected);

	SHELLEXECUTEINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(si);
	si.hwnd = fv->GetSafeHwnd();
	si.nShow = SW_SHOW;
	si.fMask  = SEE_MASK_INVOKEIDLIST;
	si.lpVerb = L"properties";
	si.lpFile = (LPCTSTR)path;
	ShellExecuteEx(&si);
}

void CSpaceMonger::OnViewLargest(void)
{
	CFolderTree *ft = (CFolderTree *)m_document;
	if (ft == NULL || ft->GetRoot() == NULL) return;
	CLargestDialog::ShowWindowFor(m_mainframe);
}

BOOL CSpaceMonger::IsElevated(void)
{
	static int elevated = -1;
	if (elevated < 0) {
		elevated = 0;
		HANDLE token;
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
			TOKEN_ELEVATION info;
			DWORD size;
			if (GetTokenInformation(token, TokenElevation, &info, sizeof(info), &size))
				elevated = info.TokenIsElevated ? 1 : 0;
			CloseHandle(token);
		}
	}
	return elevated;
}

// Starts a second copy of SpaceMonger as administrator, scanning the same
// place, and closes this one.  Windows asks the user first.
void CSpaceMonger::OnFileRunAsAdmin(void)
{
	CFolderTree *ft = (CFolderTree *)m_document;
	if (ft == NULL || ft->m_path.IsEmpty()) return;

	wchar_t exe[MAX_PATH];
	GetModuleFileName(NULL, exe, MAX_PATH);
	// A trailing backslash would escape the closing quote, so double it.
	CString path = ft->m_path;
	if (path.Right(1) == L"\\") path += L"\\";
	CString params = L"\"" + path + L"\"";

	SHELLEXECUTEINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(si);
	si.hwnd = m_mainframe->m_hWnd;
	si.lpVerb = L"runas";
	si.lpFile = exe;
	si.lpParameters = params;
	si.nShow = SW_SHOWNORMAL;
	if (ShellExecuteEx(&si))
		m_mainframe->PostMessage(WM_CLOSE);
}

BOOL CSpaceMonger::IsException(const CString &path)
{
	for (size_t i = 0; i < m_settings.exceptions.size(); i++)
		if (!m_settings.exceptions[i].CompareNoCase(path)) return 1;
	return 0;
}

void CSpaceMonger::AddException(const CString &path)
{
	if (IsException(path)) return;
	m_settings.exceptions.push_back(path);
	m_settings.Save();
}

void CSpaceMonger::RemoveException(const CString &path)
{
	for (size_t i = 0; i < m_settings.exceptions.size(); i++) {
		if (!m_settings.exceptions[i].CompareNoCase(path)) {
			m_settings.exceptions.erase(m_settings.exceptions.begin() + i);
			break;
		}
	}
	m_settings.Save();
}

//////////////////////////////////////////////////////////////////////////////
//  Export to a spreadsheet (CSV)

class CCsvWriter {
public:
	CCsvWriter(HANDLE file) : m_file(file), m_ok(1) {
		m_buffer.reserve(1 << 20);
		m_buffer.append("\xEF\xBB\xBF");		// UTF-8 marker, so Excel reads accents right
	}
	~CCsvWriter() { Flush(); }

	void Field(const CString &text, BOOL last = 0) {
		BOOL quote = text.FindOneOf(L",\"\r\n") >= 0;
		CString value = text;
		if (quote) {
			value.Replace(L"\"", L"\"\"");
			value = L"\"" + value + L"\"";
		}
		int len = WideCharToMultiByte(CP_UTF8, 0, value, value.GetLength(), NULL, 0, NULL, NULL);
		size_t at = m_buffer.size();
		m_buffer.resize(at + len);
		WideCharToMultiByte(CP_UTF8, 0, value, value.GetLength(), &m_buffer[at], len, NULL, NULL);
		m_buffer.append(last ? "\r\n" : ",");
		if (m_buffer.size() > (1 << 20) - 4096) Flush();
	}
	BOOL Flush(void) {
		DWORD written;
		if (!m_buffer.empty() && !WriteFile(m_file, m_buffer.data(), (DWORD)m_buffer.size(), &written, NULL))
			m_ok = 0;
		m_buffer.clear();
		return m_ok;
	}
	BOOL Ok(void) const { return m_ok; }

private:
	HANDLE m_file;
	std::string m_buffer;
	BOOL m_ok;
};

static ui64 CountRows(CFolder *folder)
{
	ui64 rows = 0;
	for (unsigned int i = 0; i < folder->cur; i++) {
		if (IsSpecialName(folder->names[i])) continue;
		rows++;
		if (folder->children[i] != NULL) rows += CountRows(folder->children[i]);
	}
	return rows;
}

static CString FormatIsoDate(ui64 date)
{
	if (date == 0) return CString();
	FILETIME local;
	SYSTEMTIME st;
	FileTimeToLocalFileTime((FILETIME *)&date, &local);
	FileTimeToSystemTime(&local, &st);
	CString result;
	result.Format(L"%04d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond);
	return result;
}

static void WriteRows(CCsvWriter &csv, CFolder *folder, const CString &prefix)
{
	for (unsigned int i = 0; i < folder->cur && csv.Ok(); i++) {
		if (IsSpecialName(folder->names[i])) continue;
		CString path = prefix + folder->names[i];
		BOOL isfolder = (folder->children[i] != NULL);
		CString number;
		csv.Field(path);
		csv.Field(isfolder ? L"Folder" : L"File");
		csv.Field(isfolder ? CString() : CString(CurLang->ex->typenames[GetFileType(folder->names[i])]));
		number.Format(L"%I64u", folder->sizes[i]);
		csv.Field(number);
		csv.Field(FormatSizeText(folder->sizes[i]));
		number.Format(L"%I64u", folder->actualsizes[i]);
		csv.Field(number);
		csv.Field((folder->attribs[i] & ENTRY_CLOUD_ONLY) ? L"Yes" : L"");
		csv.Field(FormatIsoDate(folder->times[i]), 1);
		if (isfolder) WriteRows(csv, folder->children[i], path + L"\\");
	}
}

// Saves everything in the folder being viewed (and below it) as a CSV file.
void CSpaceMonger::OnFileExport(void)
{
	CFolderView *fv = (CFolderView *)m_view;
	CFolderTree *ft = (CFolderTree *)m_document;
	if (fv == NULL || ft == NULL || fv->GetViewRoot() == NULL) return;

	CFolder *root = fv->GetViewRoot();
	CString rootpath = ft->m_path;
	if (root->parent != NULL) {
		rootpath = ft->GetFullPath(root->parent, root->parentindex) + L"\\";
	}

	ui64 rows = CountRows(root);
	if (rows > 1048575) {
		CString question;
		question.Format(CurLang->ex->exportmany, (LPCWSTR)FormatCountText(rows));
		if (AfxMessageBox(question, MB_YESNO|MB_ICONQUESTION) != IDYES) return;
	}

	// Suggest a name like "SpaceMonger - C drive.csv" or "SpaceMonger - Downloads.csv".
	CString name = rootpath;
	name.TrimRight(L'\\');
	int slash = name.ReverseFind(L'\\');
	if (slash >= 0) name = name.Mid(slash + 1);
	if (name.GetLength() == 2 && name[1] == L':') name = CString(name[0]) + L" drive";
	name.Replace(L':', L'_');
	name = L"SpaceMonger - " + name + L".csv";

	IFileSaveDialog *dialog = NULL;
	if (FAILED(CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog))))
		return;
	COMDLG_FILTERSPEC filter[] = { { L"CSV (Comma delimited)", L"*.csv" } };
	dialog->SetFileTypes(1, filter);
	dialog->SetDefaultExtension(L"csv");
	dialog->SetFileName(name);
	dialog->SetTitle(CurLang->ex->exporttitle);
	CString filename;
	if (SUCCEEDED(dialog->Show(m_mainframe->m_hWnd))) {
		IShellItem *item = NULL;
		if (SUCCEEDED(dialog->GetResult(&item))) {
			PWSTR path = NULL;
			if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
				filename = path;
				CoTaskMemFree(path);
			}
			item->Release();
		}
	}
	dialog->Release();
	if (filename.IsEmpty()) return;

	CWaitCursor wait;
	HANDLE file = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	BOOL ok = (file != INVALID_HANDLE_VALUE);
	if (ok) {
		CCsvWriter csv(file);
		csv.Field(L"Path");
		csv.Field(L"Type");
		csv.Field(L"Category");
		csv.Field(L"Size on disk (bytes)");
		csv.Field(L"Size on disk");
		csv.Field(L"File size (bytes)");
		csv.Field(L"Online-only");
		csv.Field(L"Modified", 1);
		WriteRows(csv, root, rootpath);
		ok = csv.Flush();
		CloseHandle(file);
	}
	if (!ok) {
		AfxMessageBox(CurLang->ex->exportfailed, MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	CString done;
	done.Format(CurLang->ex->exportdone, (LPCWSTR)FormatCountText(rows), (LPCWSTR)filename);
	AfxMessageBox(done, MB_OK|MB_ICONINFORMATION);
}

void CSpaceMonger::OnAbout()
{
	CAboutDialog about(m_mainframe);
	about.DoModal();
}

void CSpaceMonger::OnIgnoreUpdate(CCmdUI *ui)
{
	GeneralIgnoreUpdate(ui);
}

void GeneralIgnoreUpdate(CCmdUI *ui)
{
	CFolderView *fv = (CFolderView *)theApp.m_view;
	if (fv == NULL) return;
	CDisplayFolder *cur = fv->selected;
	int zoom = fv->zoomlevel;

	if (ui->m_nID == ID_FILE_DELETE) {
		if (theApp.m_settings.disable_delete || cur == NULL)
			ui->Enable(0);
	}
	else if (ui->m_nID == ID_FILE_RUN || ui->m_nID == ID_FILE_EXPLORE) {
		if (cur == NULL) ui->Enable(0);
	}
	else if (ui->m_nID == ID_FILE_PROPERTIES) {
		if (cur == NULL) ui->Enable(0);
	}
	else if (ui->m_nID == ID_FILE_REFRESH) {
		CFolderTree *ft = (CFolderTree *)theApp.m_document;
		if (ft == NULL || ft->GetRoot() == NULL) ui->Enable(0);
	}
	else if (ui->m_nID == ID_VIEW_FREE) {
		CFolderTree *ft = (CFolderTree *)theApp.m_document;
		ui->SetCheck(fv->showfreespace);
		if (ft == NULL || ft->GetRoot() == NULL) ui->Enable(0);
	}
	else if (ui->m_nID == ID_VIEW_ZOOM_IN) {
		if (cur == NULL || (cur->flags & 1) == 0)
			ui->Enable(0);
	}
	else if (ui->m_nID == ID_VIEW_ZOOM_OUT || ui->m_nID == ID_VIEW_ZOOM_FULL) {
		if (zoom == 0) ui->Enable(0);
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Application Settings
//

static const wchar_t *SettingsKey = L"Software\\65\\SpaceMonger\\1.3";

void CCurrentSettings::Reset(void)
{
	density = 0;
	file_color = 0;
	folder_color = 0;
	auto_rescan = 0;
	animated_zoom = 1;
	disable_delete = 0;
	bias = 0;
	show_name_tips = 1;
	nametip_delay = 125;
	show_info_tips = 1;
	rollover_box = 0;
	infotip_flags = TIP_DATE|TIP_SIZE|TIP_ICON;
	infotip_delay = 250;
	rect.left = rect.right = rect.top = rect.bottom = CW_USEDEFAULT;
	showcmd = SW_SHOWNORMAL;
	save_pos = 0;
	skip_cloud_only = 0;
	theme = THEME_SYSTEM;
	exceptions.clear();
	lang[0] = L'u';
	lang[1] = L's';
	lang[2] = L'\0';
	lang[3] = L'\0';
}

static void ReadDword(HKEY key, const wchar_t *name, void *value)
{
	DWORD size = sizeof(DWORD), type;
	DWORD data;
	if (RegQueryValueEx(key, name, NULL, &type, (BYTE *)&data, &size) == ERROR_SUCCESS
		&& type == REG_DWORD)
		*(DWORD *)value = data;
}

static void WriteDword(HKEY key, const wchar_t *name, const void *value)
{
	RegSetValueEx(key, name, 0, REG_DWORD, (const BYTE *)value, sizeof(DWORD));
}

void CCurrentSettings::Load(void)
{
	HKEY key;
	Reset();
	if (RegOpenKeyEx(HKEY_CURRENT_USER, SettingsKey, 0, KEY_READ, &key) != ERROR_SUCCESS)
		return;

	ReadDword(key, L"density", &density);
	ReadDword(key, L"file_color", &file_color);
	ReadDword(key, L"folder_color", &folder_color);
	ReadDword(key, L"auto_rescan", &auto_rescan);
	ReadDword(key, L"animated_zoom", &animated_zoom);
	ReadDword(key, L"disable_delete", &disable_delete);
	ReadDword(key, L"save_pos", &save_pos);
	ReadDword(key, L"bias", &bias);
	ReadDword(key, L"show_name_tips", &show_name_tips);
	ReadDword(key, L"show_rollover_box", &rollover_box);
	ReadDword(key, L"show_info_tips", &show_info_tips);
	ReadDword(key, L"nametip_delay", &nametip_delay);
	ReadDword(key, L"infotip_flags", &infotip_flags);
	ReadDword(key, L"infotip_delay", &infotip_delay);
	ReadDword(key, L"window_rect.left", &rect.left);
	ReadDword(key, L"window_rect.top", &rect.top);
	ReadDword(key, L"window_rect.right", &rect.right);
	ReadDword(key, L"window_rect.bottom", &rect.bottom);
	ReadDword(key, L"window_showcmd", &showcmd);
	ReadDword(key, L"skip_cloud_only", &skip_cloud_only);
	ReadDword(key, L"theme", &theme);
	exceptions.clear();
	DWORD listsize = 0, listtype;
	if (RegQueryValueEx(key, L"exceptions", NULL, &listtype, NULL, &listsize) == ERROR_SUCCESS
		&& listtype == REG_MULTI_SZ && listsize > 0) {
		std::vector<wchar_t> list(listsize / sizeof(wchar_t) + 2, L'\0');
		if (RegQueryValueEx(key, L"exceptions", NULL, &listtype, (BYTE *)list.data(), &listsize) == ERROR_SUCCESS) {
			for (const wchar_t *p = list.data(); *p != L'\0'; p += wcslen(p) + 1)
				exceptions.push_back(p);
		}
	}
	DWORD size = sizeof(lang) - sizeof(wchar_t), type;
	wchar_t langbuf[4] = { 0 };
	if (RegQueryValueEx(key, L"lang", NULL, &type, (BYTE *)langbuf, &size) == ERROR_SUCCESS
		&& type == REG_SZ)
		memcpy(lang, langbuf, sizeof(lang));
	RegCloseKey(key);

	if (density < -3) density = -3;
	if (density > 3) density = 3;

	if (file_color < 0 || file_color > FILE_COLOR_BY_TYPE) file_color = 0;
	if (folder_color < 0 || folder_color > 11) folder_color = 0;

	auto_rescan = !!auto_rescan;
	animated_zoom = !!animated_zoom;
	disable_delete = !!disable_delete;
	save_pos = !!save_pos;
	skip_cloud_only = !!skip_cloud_only;
	if (theme < THEME_SYSTEM || theme > THEME_DARK) theme = THEME_SYSTEM;

	rollover_box = !!rollover_box;

	show_name_tips = !!show_name_tips;
	if (nametip_delay < 0) nametip_delay = 0;
	if (nametip_delay > 30000) nametip_delay = 30000;

	show_info_tips = !!show_info_tips;
	infotip_flags &= TIP_PATH|TIP_NAME|TIP_ICON|TIP_DATE|TIP_SIZE|TIP_ATTRIB;
	if (infotip_delay < 0) infotip_delay = 0;
	if (infotip_delay > 30000) infotip_delay = 30000;

	if (bias < -20) bias = -20;
	if (bias > 20) bias = 20;

	// Ensure that the window's provided rectangle is sensible and has at
	// least some of its area on one of the monitors.
	if (rect.top > rect.bottom) {
		LONG temp = rect.top;
		rect.top = rect.bottom;
		rect.bottom = temp;
	}
	if (rect.left > rect.right) {
		LONG temp = rect.left;
		rect.left = rect.right;
		rect.right = temp;
	}
	if (rect.right - rect.left < 200 || rect.bottom - rect.top < 150
		|| MonitorFromRect(&rect, MONITOR_DEFAULTTONULL) == NULL) {
		rect.left = rect.right = rect.top = rect.bottom = CW_USEDEFAULT;
	}

	// Ensure that the window's placement is comprehensible.
	if (showcmd != SW_SHOWMINIMIZED && showcmd != SW_SHOWMAXIMIZED
		&& showcmd != SW_SHOWNORMAL)
		showcmd = SW_SHOWNORMAL;
	if (showcmd == SW_SHOWMINIMIZED) showcmd = SW_SHOWNORMAL;

	// Find the language in the list of languages.  If it's not found,
	// try US English.
	lang[0] = towlower(lang[0]);
	lang[1] = towlower(lang[1]);
	lang[2] = L'\0';
	lang[3] = L'\0';
	int i;
	CurLang = NULL;
	for (i = 0; Langs[i].name != NULL; i++) {
		if (lang[0] == Langs[i].abbrev[0] && lang[1] == Langs[i].abbrev[1]) {
			CurLang = Langs[i].text;
			break;
		}
	}
	if (CurLang == NULL) {
		CurLang = Langs[0].text;
		lang[0] = Langs[0].abbrev[0];
		lang[1] = Langs[0].abbrev[1];
		lang[2] = L'\0';
		lang[3] = L'\0';
	}
}

void CCurrentSettings::Save(void)
{
	HKEY key;
	if (RegCreateKeyEx(HKEY_CURRENT_USER, SettingsKey, 0, NULL, 0, KEY_WRITE, NULL, &key, NULL)
			!= ERROR_SUCCESS)
		return;
	WriteDword(key, L"density", &density);
	WriteDword(key, L"file_color", &file_color);
	WriteDword(key, L"folder_color", &folder_color);
	WriteDword(key, L"auto_rescan", &auto_rescan);
	WriteDword(key, L"animated_zoom", &animated_zoom);
	WriteDword(key, L"disable_delete", &disable_delete);
	WriteDword(key, L"save_pos", &save_pos);
	WriteDword(key, L"bias", &bias);
	WriteDword(key, L"show_name_tips", &show_name_tips);
	WriteDword(key, L"show_rollover_box", &rollover_box);
	WriteDword(key, L"show_info_tips", &show_info_tips);
	WriteDword(key, L"nametip_delay", &nametip_delay);
	WriteDword(key, L"infotip_flags", &infotip_flags);
	WriteDword(key, L"infotip_delay", &infotip_delay);
	WriteDword(key, L"window_rect.left", &rect.left);
	WriteDword(key, L"window_rect.top", &rect.top);
	WriteDword(key, L"window_rect.right", &rect.right);
	WriteDword(key, L"window_rect.bottom", &rect.bottom);
	WriteDword(key, L"window_showcmd", &showcmd);
	WriteDword(key, L"skip_cloud_only", &skip_cloud_only);
	WriteDword(key, L"theme", &theme);
	std::vector<wchar_t> list;
	for (size_t i = 0; i < exceptions.size(); i++) {
		list.insert(list.end(), (LPCWSTR)exceptions[i], (LPCWSTR)exceptions[i] + exceptions[i].GetLength());
		list.push_back(L'\0');
	}
	list.push_back(L'\0');
	if (list.size() < 2) list.push_back(L'\0');
	RegSetValueEx(key, L"exceptions", 0, REG_MULTI_SZ, (const BYTE *)list.data(),
		(DWORD)(list.size() * sizeof(wchar_t)));
	RegSetValueEx(key, L"lang", 0, REG_SZ, (const BYTE *)lang, 3 * sizeof(wchar_t));
	RegCloseKey(key);
}
