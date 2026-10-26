
#include "stdafx.h"
#include "spacemonger.h"
#include "AboutDialog.h"
#include "Lang.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CAboutDialog::CAboutDialog(CWnd* pParent)
	: CDialog(IDD_ABOUT, pParent)
{
}

BEGIN_MESSAGE_MAP(CAboutDialog, CDialog)
	//{{AFX_MSG_MAP(CAboutDialog)
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CAboutDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetWindowText(CurLang->about_spacemonger);
	SetDlgItemText(IDC_STATIC_FREEWARE, CurLang->freeware);
	SetDlgItemText(IDC_STATIC_WARRANTY, CurLang->warranty);
	SetDlgItemText(IDC_STATIC_EMAIL, CurLang->email);
	SetDlgItemText(IDOK, CurLang->ok);
	Theme_ApplyToWindow(m_hWnd);

	return TRUE;
}

HBRUSH CAboutDialog::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
	HBRUSH brush = Theme_OnCtlColor(pDC->m_hDC, pWnd->m_hWnd, nCtlColor);
	return brush != NULL ? brush : CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

BOOL CAboutDialog::OnEraseBkgnd(CDC *pDC)
{
	if (!Theme_IsDark()) return CDialog::OnEraseBkgnd(pDC);
	CRect rect;
	GetClientRect(&rect);
	pDC->FillRect(&rect, CBrush::FromHandle(Theme_Brush(COLOR_WINDOW)));
	return 1;
}
