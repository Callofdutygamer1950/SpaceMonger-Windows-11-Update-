
#ifndef CMAINFRAME_H
#define CMAINFRAME_H

// The toolbar is drawn by hand (icons come from the Windows icon font), so
// that it stays sharp at any DPI and follows the light/dark theme.
class CMainToolBar : public CWnd {
public:
	CMainToolBar();
	virtual ~CMainToolBar();
	DECLARE_DYNCREATE(CMainToolBar)

	virtual int Create(CWnd *parent);

	enum {
		NoState = 0,
		ViewFree,
	};

	virtual void EnableState(int state, BOOL enabled);
	void UpdateButtonsForView(class CFolderView *view);

	void Reload(void);			// Language, theme, or DPI changed
	int GetHeight(void) const { return m_height; }

protected:
	struct Button {
		UINT id;
		wchar_t glyph;
		int label;				// Index into CurLang->toolbarnames
		BOOL separator_after;
		BOOL enabled, checked;
		CRect rect;
	};

	void Layout(void);
	int HitTest(CPoint point);

	//{{AFX_MSG(CMainToolBar)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnMouseMove(UINT flags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM, LPARAM);
	afx_msg void OnLButtonDown(UINT flags, CPoint point);
	afx_msg void OnLButtonUp(UINT flags, CPoint point);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	std::vector<Button> m_buttons;
	CFont m_iconfont, m_textfont;
	int m_height;
	int m_hot, m_pressed;
	BOOL m_tracking;
};

class CMainFrame : public CFrameWnd {
public:
	CMainFrame();
	virtual ~CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)
	BOOL PreCreateWindow(CREATESTRUCT& cs);

	virtual void RecalcLayout(BOOL bNotify);

protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy(void);
	afx_msg void OnSysCommand(UINT nid, LPARAM lparam);
	afx_msg void OnClose();
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnWindowPosChanged(WINDOWPOS *wp);
	afx_msg void OnShowWindow(BOOL bShow, UINT status);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg LRESULT OnDpiChanged(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	afx_msg void OnIgnoreUpdate(CCmdUI *ui);
	DECLARE_MESSAGE_MAP()

	void SavePlacement(void);

public:
	CMainToolBar m_toolbar;
};

#endif
