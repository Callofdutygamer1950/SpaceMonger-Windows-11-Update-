
#ifndef DRIVEDIALOG_H
#define DRIVEDIALOG_H

class CDriveDialog : public CDialog {
public:
	CDriveDialog(CWnd* pParent = NULL);

	CString m_path;			// The drive or folder the user picked

	//{{AFX_VIRTUAL(CDriveDialog)
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_VIRTUAL

protected:
	//{{AFX_MSG(CDriveDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnDriveDblClk(NMHDR *pNotifyStruct, LRESULT *result);
	afx_msg void OnBrowse();
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	BOOL ChooseSelected(void);

	HIMAGELIST m_imagelist;
};

#endif
