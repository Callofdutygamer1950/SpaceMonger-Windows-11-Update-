#include "stdafx.h"
#include "spacemonger.h"
#include "DriveDialog.h"
#include "Lang.h"

CDriveDialog::CDriveDialog(CWnd* pParent)
	: CDialog(IDD_OPEN_DIALOG, pParent)
{
	m_imagelist = NULL;
}

BEGIN_MESSAGE_MAP(CDriveDialog, CDialog)
	//{{AFX_MSG_MAP(CDriveDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_DRIVE_LIST, OnDriveDblClk)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CDriveDialog::ChooseSelected(void)
{
	CListCtrl *lc = (CListCtrl *)GetDlgItem(IDC_DRIVE_LIST);
	if (lc == NULL || lc->GetSelectedCount() != 1) return 0;

	int item = lc->GetNextItem(-1, LVNI_SELECTED);
	if (item < 0) return 0;
	m_path.Format(L"%c:\\", (wchar_t)(L'A' + lc->GetItemData(item)));
	EndDialog(IDOK);
	return 1;
}

void CDriveDialog::OnDriveDblClk(NMHDR *pNotifyStruct, LRESULT *result)
{
	ChooseSelected();
	*result = 0;
}

void CDriveDialog::OnOK()
{
	ChooseSelected();
}

void CDriveDialog::OnCancel()
{
	m_path.Empty();
	EndDialog(IDCANCEL);
}

void CDriveDialog::OnBrowse()
{
	CString path;
	if (PickFolder(m_hWnd, path)) {
		m_path = path;
		EndDialog(IDOK);
	}
}

void CDriveDialog::OnDestroy()
{
	// The list control doesn't own the image list (no LVS_SHAREIMAGELISTS
	// semantics for LVSIL_NORMAL), so we free it ourselves.
	CListCtrl *lc = (CListCtrl *)GetDlgItem(IDC_DRIVE_LIST);
	if (lc != NULL) ListView_SetImageList(lc->m_hWnd, NULL, LVSIL_NORMAL);
	if (m_imagelist != NULL) ImageList_Destroy(m_imagelist);
	m_imagelist = NULL;
	CDialog::OnDestroy();
}

HBRUSH CDriveDialog::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
	HBRUSH brush = Theme_OnCtlColor(pDC->m_hDC, pWnd->m_hWnd, nCtlColor);
	return brush != NULL ? brush : CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CDriveDialog::OnEraseBkgnd(CDC *pDC)
{
	if (!Theme_IsDark()) return CDialog::OnEraseBkgnd(pDC);
	CRect rect;
	GetClientRect(&rect);
	pDC->FillRect(&rect, CBrush::FromHandle(Theme_Brush(COLOR_WINDOW)));
	return 1;
}

BOOL CDriveDialog::OnInitDialog()
{
	CWaitCursor wait;

	CDialog::OnInitDialog();

	SetWindowText(CurLang->selectdrive);
	SetDlgItemText(IDOK, CurLang->ok);
	SetDlgItemText(IDCANCEL, CurLang->cancel);
	SetDlgItemText(IDC_BROWSE, CurLang->browsefolder);

	CListCtrl *lc = (CListCtrl *)GetDlgItem(IDC_DRIVE_LIST);
	if (lc == NULL) return TRUE;

	int iconsize = GetSystemMetricsForDpi(SM_CXICON, Dpi_ForWindow(m_hWnd));
	m_imagelist = ImageList_Create(iconsize, iconsize, ILC_COLOR32|ILC_MASK, 0, 26);
	ListView_SetImageList(lc->m_hWnd, m_imagelist, LVSIL_NORMAL);

	lc->InsertColumn(0, L"Name");
	DWORD drives = GetLogicalDrives();
	int index = 0;
	for (int i = 0; i < 26; i++) {
		if (!(drives & (1 << i))) continue;

		wchar_t name[4] = { (wchar_t)(L'A' + i), L':', L'\\', L'\0' };
		UINT type = GetDriveType(name);
		if (type == DRIVE_UNKNOWN || type == DRIVE_NO_ROOT_DIR) continue;

		SHFILEINFO fileinfo;
		memset(&fileinfo, 0, sizeof(SHFILEINFO));
		SHGetFileInfo(name, 0, &fileinfo, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME|SHGFI_ICON);

		int image = -1;
		if (fileinfo.hIcon != NULL) {
			image = ImageList_AddIcon(m_imagelist, fileinfo.hIcon);
			DestroyIcon(fileinfo.hIcon);
		}

		LVITEM item;
		memset(&item, 0, sizeof(item));
		item.mask = LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
		item.iItem = index++;
		item.pszText = fileinfo.szDisplayName[0] ? fileinfo.szDisplayName : name;
		item.iImage = image;
		item.lParam = i;
		lc->InsertItem(&item);
	}
	if (lc->GetItemCount() > 0)
		lc->SetItemState(0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);

	Theme_ApplyToWindow(m_hWnd);
	lc->SetFocus();
	return FALSE;
}
