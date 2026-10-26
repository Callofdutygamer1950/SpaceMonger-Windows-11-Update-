
#include "stdafx.h"
#include "spacemonger.h"
#include "FolderTree.h"
#include "FolderView.h"
#include "LargestDialog.h"
#include "Lang.h"

#include <algorithm>

BEGIN_MESSAGE_MAP(CLargestDialog, CDialog)
	ON_BN_CLICKED(IDC_GOTO, OnGoTo)
	ON_BN_CLICKED(IDC_EXPLORE_FILE, OnExplore)
	ON_BN_CLICKED(IDC_RECYCLE_FILE, OnRecycle)
	ON_BN_CLICKED(IDC_EXCEPTION, OnException)
	ON_BN_CLICKED(IDC_SHOW_EXCEPTIONS, OnShowExceptions)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LARGEST_LIST, OnItemChanged)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_LARGEST_LIST, OnItemActivate)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

CLargestDialog::CLargestDialog()
{
	m_showexceptions = 0;
}

CLargestDialog::~CLargestDialog()
{
}

void CLargestDialog::ShowWindowFor(CWnd *parent)
{
	if (theApp.m_largest != NULL && ::IsWindow(theApp.m_largest->m_hWnd)) {
		theApp.m_largest->Refresh();
		theApp.m_largest->ShowWindow(SW_SHOW);
		theApp.m_largest->SetActiveWindow();
		return;
	}
	CLargestDialog *dialog = new CLargestDialog;
	if (!dialog->Create(IDD_LARGEST, parent)) {
		delete dialog;
		return;
	}
	theApp.m_largest = dialog;
	dialog->ShowWindow(SW_SHOW);
}

BOOL CLargestDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(CurLang->ex->largesttitle);
	SetDlgItemText(IDC_GOTO, CurLang->ex->gotofile);
	SetDlgItemText(IDC_EXPLORE_FILE, CurLang->showinexplorer);
	SetDlgItemText(IDC_RECYCLE_FILE, CurLang->del);
	SetDlgItemText(IDC_SHOW_EXCEPTIONS, CurLang->ex->showexceptions);
	SetDlgItemText(IDCANCEL, CurLang->ex->close);

	CListCtrl *list = (CListCtrl *)GetDlgItem(IDC_LARGEST_LIST);
	list->SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER|LVS_EX_LABELTIP);
	CRect widths(110, 50, 175, 70);		// In dialog units
	MapDialogRect(&widths);
	list->InsertColumn(0, CurLang->ex->colname, LVCFMT_LEFT, widths.left);
	list->InsertColumn(1, CurLang->ex->colsize, LVCFMT_RIGHT, widths.top);
	list->InsertColumn(2, CurLang->ex->colfolder, LVCFMT_LEFT, widths.right);
	list->InsertColumn(3, CurLang->ex->colmodified, LVCFMT_LEFT, widths.bottom);
	// Let the Folder column take whatever width is left, so there's no
	// sideways scrolling.
	CRect client;
	list->GetClientRect(&client);
	int left = client.Width() - widths.left - widths.top - widths.bottom
		- GetSystemMetricsForDpi(SM_CXVSCROLL, Dpi_ForWindow(m_hWnd));
	if (left > 50) list->SetColumnWidth(2, left);

	Theme_ApplyToWindow(m_hWnd);
	Refresh();
	return TRUE;
}

void CLargestDialog::OnOK()
{
	OnGoTo();		// Enter in the list
}

void CLargestDialog::OnCancel()
{
	DestroyWindow();
}

void CLargestDialog::PostNcDestroy()
{
	if (theApp.m_largest == this) theApp.m_largest = NULL;
	delete this;
}

HBRUSH CLargestDialog::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
	HBRUSH brush = Theme_OnCtlColor(pDC->m_hDC, pWnd->m_hWnd, nCtlColor);
	return brush != NULL ? brush : CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CLargestDialog::OnEraseBkgnd(CDC *pDC)
{
	if (!Theme_IsDark()) return CDialog::OnEraseBkgnd(pDC);
	CRect rect;
	GetClientRect(&rect);
	pDC->FillRect(&rect, CBrush::FromHandle(Theme_Brush(COLOR_WINDOW)));
	return 1;
}

typedef std::pair<ui64, std::pair<CFolder *, unsigned int>> HeapEntry;

static bool HeapGreater(const HeapEntry &a, const HeapEntry &b)
{
	return a.first > b.first;
}

// Keeps the biggest files in a min-heap, so the smallest of them is on top
// and easy to replace.  Exceptions are only checked for files big enough to
// make the list, so most files never need their full path built.
void CLargestDialog::CollectLargest(CFolder *folder, std::vector<HeapEntry> &heap)
{
	CFolderTree *ft = (CFolderTree *)theApp.m_document;
	for (unsigned int i = 0; i < folder->cur; i++) {
		if (folder->children[i] != NULL) {
			CollectLargest(folder->children[i], heap);
			continue;
		}
		ui64 size = folder->sizes[i];
		if (size == 0 || IsSpecialName(folder->names[i])
			|| (folder->attribs[i] & ENTRY_CLOUD_ONLY))
			continue;
		if (heap.size() >= MaxFiles && size <= heap.front().first)
			continue;
		if (!theApp.m_settings.exceptions.empty()
			&& theApp.IsException(ft->GetFullPath(folder, i)))
			continue;

		heap.push_back(HeapEntry(size, std::make_pair(folder, i)));
		std::push_heap(heap.begin(), heap.end(), HeapGreater);
		if (heap.size() > MaxFiles) {
			std::pop_heap(heap.begin(), heap.end(), HeapGreater);
			heap.pop_back();
		}
	}
}

static CString FormatDate(ui64 date)
{
	if (date == 0) return CString();
	FILETIME local;
	SYSTEMTIME st;
	FileTimeToLocalFileTime((FILETIME *)&date, &local);
	FileTimeToSystemTime(&local, &st);
	wchar_t datebuf[64] = L"", timebuf[32] = L"";
	GetDateFormatEx(LOCALE_NAME_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, datebuf, 64, NULL);
	GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, timebuf, 32);
	return CString(datebuf) + L" " + timebuf;
}

static void SplitPath(const CString &path, CString &folder, CString &name)
{
	int slash = path.ReverseFind(L'\\');
	folder = (slash >= 0) ? path.Left(slash + 1) : CString();
	name = (slash >= 0) ? path.Mid(slash + 1) : path;
}

void CLargestDialog::Refresh(void)
{
	if (!::IsWindow(m_hWnd)) return;

	CFolderTree *ft = (CFolderTree *)theApp.m_document;
	m_items.clear();

	if (m_showexceptions) {
		for (size_t i = 0; i < theApp.m_settings.exceptions.size(); i++) {
			Item item;
			item.path = theApp.m_settings.exceptions[i];
			SplitPath(item.path, item.folder, item.name);
			CFolder *folder;
			unsigned int index;
			item.found = (ft != NULL && ft->FindPath(item.path, folder, index));
			item.size = item.found ? folder->sizes[index] : 0;
			item.time = item.found ? folder->times[index] : 0;
			m_items.push_back(item);
		}
		SetDlgItemText(IDC_LARGEST_INTRO, CurLang->ex->exceptionsintro);
		SetDlgItemText(IDC_EXCEPTION, CurLang->ex->removeexception);
	}
	else {
		std::vector<HeapEntry> heap;
		if (ft != NULL && ft->GetRoot() != NULL)
			CollectLargest(ft->GetRoot(), heap);
		std::sort(heap.begin(), heap.end(), HeapGreater);
		for (size_t i = 0; i < heap.size(); i++) {
			Item item;
			CFolder *folder = heap[i].second.first;
			unsigned int index = heap[i].second.second;
			item.path = ft->GetFullPath(folder, index);
			SplitPath(item.path, item.folder, item.name);
			item.size = folder->sizes[index];
			item.time = folder->times[index];
			item.found = 1;
			m_items.push_back(item);
		}
		CString intro;
		intro.Format(CurLang->ex->largestintro, ft != NULL ? (LPCWSTR)ft->m_path : L"");
		SetDlgItemText(IDC_LARGEST_INTRO, intro);
		SetDlgItemText(IDC_EXCEPTION, CurLang->ex->addexception);
	}

	CListCtrl *list = (CListCtrl *)GetDlgItem(IDC_LARGEST_LIST);
	list->SetRedraw(FALSE);
	list->DeleteAllItems();
	for (size_t i = 0; i < m_items.size(); i++) {
		const Item &item = m_items[i];
		int row = list->InsertItem((int)i, item.name);
		list->SetItemText(row, 1, item.found ? FormatSizeText(item.size) : CString(L"-"));
		list->SetItemText(row, 2, item.folder);
		list->SetItemText(row, 3, FormatDate(item.time));
	}
	if (m_items.empty()) list->InsertItem(0, CurLang->ex->nofiles);
	list->SetRedraw(TRUE);
	list->Invalidate();
	UpdateButtons();
}

int CLargestDialog::GetSelected(void)
{
	CListCtrl *list = (CListCtrl *)GetDlgItem(IDC_LARGEST_LIST);
	int row = list->GetNextItem(-1, LVNI_SELECTED);
	return (row >= 0 && row < (int)m_items.size()) ? row : -1;
}

void CLargestDialog::UpdateButtons(void)
{
	int row = GetSelected();
	BOOL found = (row >= 0 && m_items[row].found);
	GetDlgItem(IDC_GOTO)->EnableWindow(found);
	GetDlgItem(IDC_EXPLORE_FILE)->EnableWindow(found);
	GetDlgItem(IDC_RECYCLE_FILE)->EnableWindow(found && !theApp.m_settings.disable_delete);
	GetDlgItem(IDC_EXCEPTION)->EnableWindow(row >= 0);
	((CButton *)GetDlgItem(IDC_SHOW_EXCEPTIONS))->SetCheck(m_showexceptions);
}

BOOL CLargestDialog::SelectInView(int row)
{
	if (row < 0 || !m_items[row].found) return 0;
	CFolderTree *ft = (CFolderTree *)theApp.m_document;
	CFolder *folder;
	unsigned int index;
	if (ft == NULL || !ft->FindPath(m_items[row].path, folder, index)) return 0;
	((CFolderView *)theApp.m_view)->ShowFile(folder, index);
	return 1;
}

void CLargestDialog::OnItemChanged(NMHDR *nmhdr, LRESULT *result)
{
	NMLISTVIEW *nm = (NMLISTVIEW *)nmhdr;
	if ((nm->uChanged & LVIF_STATE) && (nm->uNewState & LVIS_SELECTED) && !(nm->uOldState & LVIS_SELECTED))
		SelectInView(nm->iItem);	// Clicking a file shows it in the picture
	UpdateButtons();
	*result = 0;
}

void CLargestDialog::OnItemActivate(NMHDR *nmhdr, LRESULT *result)
{
	OnGoTo();
	*result = 0;
}

void CLargestDialog::OnGoTo()
{
	if (SelectInView(GetSelected()))
		theApp.m_mainframe->SetActiveWindow();
}

void CLargestDialog::OnExplore()
{
	int row = GetSelected();
	if (row >= 0) ShowInExplorer(m_items[row].path);
}

void CLargestDialog::OnRecycle()
{
	int row = GetSelected();
	if (row < 0 || !SelectInView(row)) return;
	theApp.OnFileDelete();		// Asks first
	Refresh();
}

void CLargestDialog::OnException()
{
	int row = GetSelected();
	if (row < 0) return;
	if (m_showexceptions) theApp.RemoveException(m_items[row].path);
	else theApp.AddException(m_items[row].path);
	Refresh();
}

void CLargestDialog::OnShowExceptions()
{
	m_showexceptions = ((CButton *)GetDlgItem(IDC_SHOW_EXCEPTIONS))->GetCheck();
	Refresh();
}
