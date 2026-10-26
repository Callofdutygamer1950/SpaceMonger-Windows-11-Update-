
#ifndef LARGESTDIALOG_H
#define LARGESTDIALOG_H

// The "Largest Files" window.  It stays open beside the main window (it's
// modeless), so you can click through files and see them in the picture.
class CLargestDialog : public CDialog {
public:
	CLargestDialog();
	virtual ~CLargestDialog();

	static void ShowWindowFor(CWnd *parent);	// Opens it, or brings it forward
	void Refresh(void);							// The scan changed

	enum { MaxFiles = 50 };

protected:
	struct Item {
		CString path, name, folder;
		ui64 size;
		ui64 time;
		BOOL found;				// Still in the current scan
	};

	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	virtual void PostNcDestroy();

	afx_msg void OnGoTo();
	afx_msg void OnExplore();
	afx_msg void OnRecycle();
	afx_msg void OnException();
	afx_msg void OnShowExceptions();
	afx_msg void OnItemChanged(NMHDR *nmhdr, LRESULT *result);
	afx_msg void OnItemActivate(NMHDR *nmhdr, LRESULT *result);
	afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	DECLARE_MESSAGE_MAP()

	void UpdateButtons(void);
	int GetSelected(void);
	BOOL SelectInView(int item);
	void CollectLargest(CFolder *folder, std::vector<std::pair<ui64, std::pair<CFolder *, unsigned int>>> &heap);

	std::vector<Item> m_items;
	BOOL m_showexceptions;
};

#endif
