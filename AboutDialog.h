#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

class CAboutDialog : public CDialog {
public:
	CAboutDialog(CWnd* pParent = NULL);

protected:
	//{{AFX_MSG(CAboutDialog)
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
