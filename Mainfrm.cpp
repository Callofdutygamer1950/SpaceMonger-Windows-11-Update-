
#include "StdAfx.h"
#include "SpaceMonger.h"
#include "FolderView.h"
#include "AboutDialog.h"
#include "Lang.h"
#include "FileTypes.h"

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SYSCOMMAND()
	ON_WM_ERASEBKGND()
	ON_WM_ACTIVATEAPP()
	ON_WM_CLOSE()
	ON_WM_WINDOWPOSCHANGED()
	ON_WM_SHOWWINDOW()
	ON_WM_SETTINGCHANGE()
	ON_MESSAGE(WM_DPICHANGED, OnDpiChanged)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI_RANGE(100, 41000, OnIgnoreUpdate)
END_MESSAGE_MAP()

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	CFrameWnd::PreCreateWindow(cs);

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;

	return(1);
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1) return(-1);

	HINSTANCE hinst = AfxFindResourceHandle(MAKEINTRESOURCE(ID_SPACEMONGER), RT_GROUP_ICON);
	HICON bigicon = (HICON)::LoadImage(hinst, MAKEINTRESOURCE(ID_SPACEMONGER), IMAGE_ICON,
		GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
	HICON smallicon = (HICON)::LoadImage(hinst, MAKEINTRESOURCE(ID_SPACEMONGER), IMAGE_ICON,
		GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	SetIcon(bigicon, 1);
	SetIcon(smallicon, 0);

	if (!m_toolbar.Create(this)) return(-1);
	if (!m_legend.Create(this)) return(-1);

	CMenu *menu = GetSystemMenu(FALSE);
	menu->AppendMenu(MF_SEPARATOR);
	menu->AppendMenu(MF_STRING, ID_APP_ABOUT, CString(L"&")
		+ CurLang->about_spacemonger + L"...");

	SavePlacement();
	Theme_ApplyToWindow(m_hWnd);

	RecalcLayout(0);

	return(0);
}

void CMainFrame::SavePlacement(void)
{
	WINDOWPLACEMENT w;
	w.length = sizeof(WINDOWPLACEMENT);
	::GetWindowPlacement(m_hWnd, &w);
	theApp.m_settings.rect.left = w.rcNormalPosition.left;
	theApp.m_settings.rect.top = w.rcNormalPosition.top;
	theApp.m_settings.rect.right = w.rcNormalPosition.right;
	theApp.m_settings.rect.bottom = w.rcNormalPosition.bottom;
	theApp.m_settings.showcmd = w.showCmd;
}

void CMainFrame::OnDestroy(void)
{
	SavePlacement();
	CFrameWnd::OnDestroy();
}

void CMainFrame::OnActivateApp(BOOL bActive, DWORD dwThreadID)
{
	CFrameWnd::OnActivateApp(bActive, dwThreadID);
	if (!bActive) {
		CSpaceMonger *monger = (CSpaceMonger *)AfxGetApp();
		if (monger != NULL && monger->m_view != NULL) {
			CFolderView *view = (CFolderView *)monger->m_view;
			view->SelectFolder(NULL);
		}
	}
}

void CMainFrame::RecalcLayout(BOOL bNotify)
{
	CRect rectBorder;
	int numwnds, wndnum, width;
	CWnd *wnd, *first, *last;

	if (m_bInRecalcLayout || !IsWindow(m_hWnd)
		|| (wnd = GetWindow(GW_CHILD)) == NULL) return;

	m_bInRecalcLayout = TRUE;

	first = wnd->GetWindow(GW_HWNDFIRST);
	last = wnd->GetWindow(GW_HWNDLAST);

	GetClientRect(&rectBorder);
	rectBorder.OffsetRect(-rectBorder.left, -rectBorder.top);
	width = rectBorder.right - rectBorder.left;

	// Position our toolbar
	int toolheight = m_toolbar.GetHeight();
	m_toolbar.SetWindowPos(NULL, 0, 0, width, toolheight,
		SWP_NOZORDER|SWP_NOACTIVATE);
	rectBorder.top += toolheight;

	// And the file-type legend, along the bottom, when it's needed
	if (m_legend.ShouldShow()) {
		int legendheight = m_legend.GetHeight();
		rectBorder.bottom -= legendheight;
		m_legend.SetWindowPos(NULL, 0, rectBorder.bottom, width, legendheight,
			SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW);
		m_legend.Invalidate(FALSE);
	}
	else m_legend.ShowWindow(SW_HIDE);

	// First count how many windows need to be rearranged
	numwnds = 0;
	if (first != NULL) {
		wnd = first;
		while (1) {
			if (wnd->m_hWnd != m_toolbar.m_hWnd && wnd->m_hWnd != m_legend.m_hWnd)
				numwnds++;
			if (wnd == last) break;
			wnd = wnd->GetWindow(GW_HWNDNEXT);
		}
	}

	// Now lay them all out in a horizontal band (cheap & easy)
	if (first != NULL && numwnds > 0) {
		wndnum = 0, wnd = first;
		while (1) {
			if (wnd->m_hWnd != m_toolbar.m_hWnd && wnd->m_hWnd != m_legend.m_hWnd) {
				wnd->SetWindowPos(NULL, rectBorder.left + (width * wndnum) / numwnds, rectBorder.top,
					(width * (wndnum+1)) / numwnds - (width * wndnum) / numwnds,
					rectBorder.bottom - rectBorder.top,
					SWP_NOZORDER);
				if (++wndnum == numwnds) break;
			}
			wnd = wnd->GetWindow(GW_HWNDNEXT);
		}
	}

	m_bInRecalcLayout = FALSE;
}

void CMainFrame::OnSysCommand(UINT nid, LPARAM lparam)
{
	if (nid == ID_APP_ABOUT) {
		CAboutDialog about;
		about.DoModal();
	}
	else CFrameWnd::OnSysCommand(nid, lparam);
}

BOOL CMainFrame::OnEraseBkgnd(CDC* pDC)
{
	return(-1);
}

void CMainFrame::OnClose()
{
	CFrameWnd::OnClose();
}

void CMainFrame::OnIgnoreUpdate(CCmdUI *ui)
{
	GeneralIgnoreUpdate(ui);
}

void CMainFrame::OnWindowPosChanged(WINDOWPOS *wp)
{
	CFrameWnd::OnWindowPosChanged(wp);
	SavePlacement();
}

void CMainFrame::OnShowWindow(BOOL bShow, UINT status)
{
	CFrameWnd::OnShowWindow(bShow, status);
	SavePlacement();
}

// Windows tells us when the user switches between light and dark mode.
void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CFrameWnd::OnSettingChange(uFlags, lpszSection);
	if (lpszSection != NULL && !wcscmp(lpszSection, L"ImmersiveColorSet")
		&& theApp.m_settings.theme == THEME_SYSTEM)
		theApp.ApplyTheme();
}

// Moving to a monitor with a different scale factor.
LRESULT CMainFrame::OnDpiChanged(WPARAM wParam, LPARAM lParam)
{
	RECT *suggested = (RECT *)lParam;
	SetWindowPos(NULL, suggested->left, suggested->top,
		suggested->right - suggested->left, suggested->bottom - suggested->top,
		SWP_NOZORDER|SWP_NOACTIVATE);

	m_toolbar.Reload();
	if (theApp.m_view != NULL) ((CFolderView *)theApp.m_view)->UpdateMetrics();
	RecalcLayout(0);
	if (theApp.m_view != NULL) theApp.m_view->OnUpdate(theApp.m_document);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CMainToolBar, CWnd)

BEGIN_MESSAGE_MAP(CMainToolBar, CWnd)
	//{{AFX_MSG_MAP(CMainToolBar)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_EN_CHANGE(IDC_SEARCH, OnSearchChange)
	ON_EN_SETFOCUS(IDC_SEARCH, OnSearchFocus)
	ON_EN_KILLFOCUS(IDC_SEARCH, OnSearchFocus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CMainToolBar::CMainToolBar()
{
	// Glyphs are from the Segoe Fluent Icons / Segoe MDL2 Assets fonts.
	// "label" indexes CurLang->toolbarnames.  When the window is narrow,
	// labels are dropped in "priority" order, lowest first.
	static const struct { UINT id; wchar_t glyph; int label; BOOL sep; int priority; } buttons[] = {
		{ ID_FILE_OPEN,			0xE838, 0, 0, 3 },		// FolderOpen
		{ ID_FILE_REFRESH,		0xE72C, 1, 1, 3 },		// Refresh
		{ ID_VIEW_ZOOM_FULL,	0xE9A6, 2, 0, 1 },		// FitPage
		{ ID_VIEW_ZOOM_IN,		0xE8A3, 3, 0, 1 },		// ZoomIn
		{ ID_VIEW_ZOOM_OUT,		0xE71F, 4, 1, 1 },		// ZoomOut
		{ ID_VIEW_FREE,			0xEDA2, 5, 0, 3 },		// HardDrive
		{ ID_VIEW_LARGEST,		0xE8FD, 10, 1, 3 },		// BulletedList
		{ ID_FILE_RUN,			0xE768, 6, 0, 2 },		// Play
		{ ID_FILE_DELETE,		0xE74D, 7, 1, 2 },		// Delete
		{ ID_SETTINGS,			0xE713, 8, 1, 3 },		// Setting
		{ ID_APP_ABOUT,			0xE946, 9, 0, 2 },		// Info
	};
	for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
		Button button;
		button.id = buttons[i].id;
		button.glyph = buttons[i].glyph;
		button.label = buttons[i].label;
		button.separator_after = buttons[i].sep;
		button.priority = buttons[i].priority;
		button.showlabel = 1;
		button.enabled = (button.id == ID_FILE_OPEN || button.id == ID_SETTINGS
			|| button.id == ID_APP_ABOUT);
		button.checked = 0;
		m_buttons.push_back(button);
	}
	m_height = 32;
	m_hot = m_pressed = -1;
	m_tracking = 0;
	m_tips = NULL;
}

CMainToolBar::~CMainToolBar()
{
}

int CMainToolBar::Create(CWnd *parent)
{
	if (!CWnd::Create(AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW, ::LoadCursor(NULL, IDC_ARROW)),
		L"", WS_CHILD|WS_VISIBLE, CRect(0, 0, 100, 32), parent, AFX_IDW_TOOLBAR))
		return 0;

	m_search.Create(WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL, CRect(0, 0, 10, 10), this, IDC_SEARCH);
	m_tips = ::CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP|TTS_ALWAYSTIP|TTS_NOPREFIX,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, m_hWnd, NULL, AfxGetInstanceHandle(), NULL);

	Reload();
	EnableState(ViewFree, 1);

	return 1;
}

void CMainToolBar::OnDestroy()
{
	if (m_tips != NULL) ::DestroyWindow(m_tips);
	m_tips = NULL;
	m_iconfont.DeleteObject();
	m_textfont.DeleteObject();
	CWnd::OnDestroy();
}

static int CALLBACK FontExistsProc(const LOGFONT *, const TEXTMETRIC *, DWORD, LPARAM found)
{
	*(BOOL *)found = 1;
	return 0;
}

static CString GetIconFontName(void)
{
	static const wchar_t *names[] = { L"Segoe Fluent Icons", L"Segoe MDL2 Assets" };
	HDC dc = ::GetDC(NULL);
	CString result = names[1];
	for (int i = 0; i < 2; i++) {
		LOGFONT lf;
		memset(&lf, 0, sizeof(lf));
		lf.lfCharSet = DEFAULT_CHARSET;
		wcscpy_s(lf.lfFaceName, names[i]);
		BOOL found = 0;
		EnumFontFamiliesEx(dc, &lf, FontExistsProc, (LPARAM)&found, 0);
		if (found) { result = names[i]; break; }
	}
	::ReleaseDC(NULL, dc);
	return result;
}

void CMainToolBar::Reload(void)
{
	int dpi = Dpi_ForWindow(m_hWnd);

	m_iconfont.DeleteObject();
	m_textfont.DeleteObject();
	m_iconfont.CreateFont(-Dpi_Scale(16, dpi), 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, GetIconFontName());
	m_textfont.Attach(Dpi_CreateUIFont(dpi, 9));

	m_height = Dpi_Scale(36, dpi);

	m_fieldbrush.DeleteObject();
	m_fieldbrush.CreateSolidBrush(Theme_IsDark() ? RGB(0x2D, 0x2D, 0x2D) : RGB(0xFF, 0xFF, 0xFF));
	if (m_search.m_hWnd != NULL) {
		m_search.SetFont(&m_textfont);
		m_search.SendMessage(EM_SETCUEBANNER, TRUE, (LPARAM)CurLang->ex->searchcue);
	}

	Layout();
	if (m_hWnd != NULL) Invalidate();
}

void CMainToolBar::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	Layout();
	Invalidate(FALSE);
}

HBRUSH CMainToolBar::OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor)
{
	pDC->SetTextColor(Theme_IsDark() ? RGB(0xFF, 0xFF, 0xFF) : RGB(0x1A, 0x1A, 0x1A));
	pDC->SetBkColor(Theme_IsDark() ? RGB(0x2D, 0x2D, 0x2D) : RGB(0xFF, 0xFF, 0xFF));
	return (HBRUSH)m_fieldbrush.m_hObject;
}

// One tooltip per button, showing its name.  TTF_SUBCLASS lets the tooltip
// watch the mouse by itself.
void CMainToolBar::UpdateTooltips(void)
{
	if (m_tips == NULL) return;
	TOOLINFO ti;
	memset(&ti, 0, sizeof(ti));		// lpszText must be NULL, or Windows copies the text into it
	ti.cbSize = sizeof(ti);
	while (::SendMessage(m_tips, TTM_ENUMTOOLS, 0, (LPARAM)&ti))
		::SendMessage(m_tips, TTM_DELTOOL, 0, (LPARAM)&ti);
	for (size_t i = 0; i < m_buttons.size(); i++) {
		CString label = CurLang->toolbarnames[m_buttons[i].label];
		memset(&ti, 0, sizeof(ti));
		ti.cbSize = sizeof(ti);
		ti.uFlags = TTF_SUBCLASS;
		ti.hwnd = m_hWnd;
		ti.uId = i + 1;
		ti.rect = m_buttons[i].rect;
		ti.lpszText = (LPWSTR)(LPCWSTR)label;
		::SendMessage(m_tips, TTM_ADDTOOL, 0, (LPARAM)&ti);
	}
	SetWindowTheme(m_tips, Theme_IsDark() ? L"DarkMode_Explorer" : NULL, NULL);
}

void CMainToolBar::OnSearchFocus()
{
	Invalidate(FALSE);		// The box's border shows when it has focus
}

void CMainToolBar::OnSearchChange()
{
	// Wait until the user stops typing for a moment.
	SetTimer(1, 300, NULL);
}

void CMainToolBar::OnTimer(UINT_PTR id)
{
	if (id == 1) {
		KillTimer(1);
		ApplySearch();
	}
	else CWnd::OnTimer(id);
}

void CMainToolBar::ApplySearch(void)
{
	CFolderView *view = (CFolderView *)theApp.m_view;
	if (view == NULL) return;

	CString text;
	m_search.GetWindowText(text);
	ui64 matches = 0, matchsize = 0;
	view->SetSearch(text, matches, matchsize);

	text.Trim();
	CString result;
	if (text.IsEmpty() || !view->IsAnythingOpen()) result.Empty();
	else if (matches == 0) result = CurLang->ex->searchnone;
	else result.Format(matches == 1 ? CurLang->ex->searchmatch : CurLang->ex->searchmatches,
		(LPCWSTR)FormatCountText(matches), (LPCWSTR)FormatSizeText(matchsize));
	CMainFrame *frame = (CMainFrame *)GetParent();
	frame->m_legend.SetSearchResult(result);
	frame->RecalcLayout(0);
}

void CMainToolBar::RefreshSearch(void)
{
	CString text;
	if (m_search.m_hWnd != NULL) m_search.GetWindowText(text);
	if (!text.IsEmpty()) ApplySearch();
}

void CMainToolBar::Layout(void)
{
	if (m_hWnd == NULL) return;

	int dpi = Dpi_ForWindow(m_hWnd);
	CClientDC dc(this);
	CFont *oldfont = dc.SelectObject(&m_textfont);
	CRect client;
	GetClientRect(&client);

	// Lay out the buttons with labels if they fit next to a usable search
	// field; otherwise, use icons only.
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	int top = Dpi_Scale(3, dpi), bottom = m_height - Dpi_Scale(3, dpi);
	int margin = Dpi_Scale(8, dpi), minbox = Dpi_Scale(140, dpi), maxbox = Dpi_Scale(260, dpi);
	int x = 0;
	for (int level = 0; level <= 3; level++) {
		x = Dpi_Scale(4, dpi);
		for (size_t i = 0; i < m_buttons.size(); i++) {
			Button &button = m_buttons[i];
			button.showlabel = (button.priority > level);
			int width = Dpi_Scale(7 + 16 + 7, dpi);
			if (button.showlabel)
				width += Dpi_Scale(5, dpi) + dc.GetTextExtent(CurLang->toolbarnames[button.label]).cx;
			button.rect.SetRect(x, top, x + width, bottom);
			x += width + Dpi_Scale(1, dpi);
			if (button.separator_after) x += Dpi_Scale(7, dpi);
		}
		if (client.right - margin - (x + margin) >= minbox) break;
	}

	// The search field takes the space left at the right end.
	// On a very narrow window the buttons come first, so the search field
	// shrinks into the space left, or hides if there's no room at all.
	int boxwidth = client.right - margin - (x + margin);
	if (boxwidth > maxbox) boxwidth = maxbox;
	BOOL showsearch = (boxwidth >= Dpi_Scale(90, dpi));
	if (!showsearch) boxwidth = 0;
	if (m_search.m_hWnd != NULL) m_search.ShowWindow(showsearch ? SW_SHOW : SW_HIDE);
	int boxheight = tm.tmHeight + Dpi_Scale(12, dpi);
	int boxtop = (m_height - boxheight) / 2;
	m_searchbox.SetRect(client.right - margin - boxwidth, boxtop, client.right - margin, boxtop + boxheight);
	UpdateTooltips();
	if (m_search.m_hWnd != NULL) {
		int left = m_searchbox.left + Dpi_Scale(8 + 16 + 6, dpi);
		m_search.SetWindowPos(NULL, left, m_searchbox.top + (boxheight - tm.tmHeight) / 2,
			m_searchbox.right - Dpi_Scale(8, dpi) - left, tm.tmHeight, SWP_NOZORDER|SWP_NOACTIVATE);
	}
	dc.SelectObject(oldfont);
}

int CMainToolBar::HitTest(CPoint point)
{
	for (size_t i = 0; i < m_buttons.size(); i++)
		if (m_buttons[i].rect.PtInRect(point)) return (int)i;
	return -1;
}

BOOL CMainToolBar::OnEraseBkgnd(CDC *pDC)
{
	return 1;
}

static void FillRoundRect(CDC *dc, const CRect &rect, int radius, COLORREF fill, COLORREF border)
{
	CBrush brush(fill);
	CPen pen(PS_SOLID, 1, border);
	CBrush *oldbrush = dc->SelectObject(&brush);
	CPen *oldpen = dc->SelectObject(&pen);
	dc->RoundRect(&rect, CPoint(radius, radius));
	dc->SelectObject(oldbrush);
	dc->SelectObject(oldpen);
}

void CMainToolBar::OnPaint()
{
	CPaintDC paintdc(this);
	CRect client;
	GetClientRect(&client);
	if (client.IsRectEmpty()) return;

	CDC dc;
	dc.CreateCompatibleDC(&paintdc);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(&paintdc, client.Width(), client.Height());
	CBitmap *oldbitmap = dc.SelectObject(&bitmap);

	int dpi = Dpi_ForWindow(m_hWnd);
	BOOL dark = Theme_IsDark();
	COLORREF background = dark ? RGB(0x20, 0x20, 0x20) : RGB(0xF3, 0xF3, 0xF3);
	COLORREF text = dark ? RGB(0xFF, 0xFF, 0xFF) : RGB(0x1A, 0x1A, 0x1A);
	COLORREF disabled = dark ? RGB(0x78, 0x78, 0x78) : RGB(0xA0, 0xA0, 0xA0);
	COLORREF hot = dark ? RGB(0x38, 0x38, 0x38) : RGB(0xE0, 0xE0, 0xE0);
	COLORREF pressed = dark ? RGB(0x2C, 0x2C, 0x2C) : RGB(0xD0, 0xD0, 0xD0);
	COLORREF checked = dark ? RGB(0x40, 0x40, 0x40) : RGB(0xD8, 0xD8, 0xD8);
	COLORREF line = dark ? RGB(0x3A, 0x3A, 0x3A) : RGB(0xD6, 0xD6, 0xD6);

	dc.FillSolidRect(&client, background);
	dc.FillSolidRect(0, client.bottom - 1, client.Width(), 1, line);
	dc.SetBkMode(TRANSPARENT);

	int radius = Dpi_Scale(8, dpi);
	for (size_t i = 0; i < m_buttons.size(); i++) {
		const Button &button = m_buttons[i];
		CRect rect = button.rect;

		if (button.enabled && (int)i == m_pressed && (int)i == m_hot)
			FillRoundRect(&dc, rect, radius, pressed, pressed);
		else if (button.enabled && (int)i == m_hot)
			FillRoundRect(&dc, rect, radius, hot, hot);
		else if (button.checked)
			FillRoundRect(&dc, rect, radius, checked, checked);

		dc.SetTextColor(button.enabled ? text : disabled);

		CRect iconrect(rect.left + Dpi_Scale(7, dpi), rect.top,
			rect.left + Dpi_Scale(7 + 16, dpi), rect.bottom);
		CFont *oldfont = dc.SelectObject(&m_iconfont);
		wchar_t glyph[2] = { button.glyph, 0 };
		dc.DrawText(glyph, 1, &iconrect, DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);

		dc.SelectObject(&m_textfont);
		if (button.showlabel) {
			CRect textrect(iconrect.right + Dpi_Scale(5, dpi), rect.top, rect.right, rect.bottom);
			dc.DrawText(CurLang->toolbarnames[button.label], -1, &textrect,
				DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);
		}
		dc.SelectObject(oldfont);

		if (button.separator_after) {
			int sx = rect.right + Dpi_Scale(1 + 3, dpi);
			dc.FillSolidRect(sx, rect.top + Dpi_Scale(6, dpi), Dpi_Scale(1, dpi),
				rect.Height() - Dpi_Scale(12, dpi), line);
		}
	}

	// The search box, with a magnifying glass, and the match count beside it.
	COLORREF field = dark ? RGB(0x2D, 0x2D, 0x2D) : RGB(0xFF, 0xFF, 0xFF);
	COLORREF fieldline = (GetFocus() == &m_search) ? (dark ? RGB(0x99, 0xEB, 0xFF) : RGB(0x00, 0x5F, 0xB8))
		: (dark ? RGB(0x45, 0x45, 0x45) : RGB(0xC8, 0xC8, 0xC8));
	if (m_searchbox.Width() > 0)
		FillRoundRect(&dc, m_searchbox, Dpi_Scale(8, dpi), field, fieldline);
	dc.SetTextColor(dark ? RGB(0xC8, 0xC8, 0xC8) : RGB(0x60, 0x60, 0x60));
	CFont *oldfont = dc.SelectObject(&m_iconfont);
	CRect glyphrect(m_searchbox.left + Dpi_Scale(8, dpi), m_searchbox.top,
		m_searchbox.left + Dpi_Scale(8 + 16, dpi), m_searchbox.bottom);
	if (m_searchbox.Width() > 0)
		dc.DrawText(L"\xE721", 1, &glyphrect, DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);
	dc.SelectObject(oldfont);

	paintdc.BitBlt(0, 0, client.Width(), client.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(oldbitmap);
}

void CMainToolBar::OnMouseMove(UINT flags, CPoint point)
{
	if (!m_tracking) {
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_hWnd;
		tme.dwHoverTime = 0;
		m_tracking = TrackMouseEvent(&tme);
	}
	int hot = HitTest(point);
	if (hot != m_hot) {
		m_hot = hot;
		Invalidate(FALSE);
	}
}

LRESULT CMainToolBar::OnMouseLeave(WPARAM, LPARAM)
{
	m_tracking = 0;
	if (m_hot != -1) {
		m_hot = -1;
		Invalidate(FALSE);
	}
	return 0;
}

void CMainToolBar::OnLButtonDown(UINT flags, CPoint point)
{
	m_pressed = HitTest(point);
	if (m_pressed >= 0 && !m_buttons[m_pressed].enabled) m_pressed = -1;
	if (m_pressed >= 0) SetCapture();
	Invalidate(FALSE);
}

void CMainToolBar::OnLButtonUp(UINT flags, CPoint point)
{
	int pressed = m_pressed;
	m_pressed = -1;
	if (GetCapture() == this) ReleaseCapture();
	Invalidate(FALSE);
	if (pressed >= 0 && HitTest(point) == pressed && m_buttons[pressed].enabled)
		GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(m_buttons[pressed].id, BN_CLICKED), 0);
}

void CMainToolBar::EnableState(int state, BOOL enabled)
{
	switch (state) {
	case CMainToolBar::ViewFree:
		for (size_t i = 0; i < m_buttons.size(); i++)
			if (m_buttons[i].id == ID_VIEW_FREE) m_buttons[i].checked = enabled;
		if (m_hWnd != NULL) Invalidate(FALSE);
		break;
	default:
		break;
	}
}

void CMainToolBar::UpdateButtonsForView(CFolderView *view)
{
	CSpaceMonger *app = (CSpaceMonger *)AfxGetApp();
	BOOL changed = 0;
	for (size_t i = 0; i < m_buttons.size(); i++) {
		Button &button = m_buttons[i];
		BOOL enabled = 1;
		switch (button.id) {
		case ID_FILE_RUN:
			enabled = view->IsAnythingOpen() && view->IsAnythingSelected();
			break;
		case ID_FILE_DELETE:
			enabled = !app->m_settings.disable_delete
				&& view->IsAnythingOpen() && view->IsAnythingSelected();
			break;
		case ID_FILE_REFRESH:
		case ID_VIEW_FREE:
		case ID_VIEW_LARGEST:
			enabled = view->IsAnythingOpen();
			break;
		case ID_VIEW_ZOOM_FULL:
		case ID_VIEW_ZOOM_OUT:
			enabled = view->IsAnythingOpen() && !view->IsZoomFull();
			break;
		case ID_VIEW_ZOOM_IN:
			enabled = view->IsAnythingOpen() && view->IsAnythingSelected()
				&& view->IsSelectedAFolder();
			break;
		}
		if (button.enabled != enabled) button.enabled = enabled, changed = 1;
	}
	if (changed && m_hWnd != NULL) Invalidate(FALSE);
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CLegendBar, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL CLegendBar::Create(CWnd *parent)
{
	return CWnd::Create(AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW, ::LoadCursor(NULL, IDC_ARROW)),
		L"", WS_CHILD, CRect(0, 0, 100, 24), parent, AFX_IDW_STATUS_BAR);
}

BOOL CLegendBar::ShouldShow(void)
{
	return theApp.m_settings.file_color == FILE_COLOR_BY_TYPE || !m_searchresult.IsEmpty();
}

void CLegendBar::SetSearchResult(const CString &text)
{
	m_searchresult = text;
	if (m_hWnd != NULL) Invalidate(FALSE);
}

int CLegendBar::GetHeight(void)
{
	return Dpi_Scale(28, Dpi_ForWindow(m_hWnd));
}

BOOL CLegendBar::OnEraseBkgnd(CDC *pDC)
{
	return 1;
}

void CLegendBar::OnPaint()
{
	CPaintDC paintdc(this);
	CRect client;
	GetClientRect(&client);
	if (client.IsRectEmpty()) return;

	CDC dc;
	dc.CreateCompatibleDC(&paintdc);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(&paintdc, client.Width(), client.Height());
	CBitmap *oldbitmap = dc.SelectObject(&bitmap);

	int dpi = Dpi_ForWindow(m_hWnd);
	BOOL dark = Theme_IsDark();
	dc.FillSolidRect(&client, dark ? RGB(0x20, 0x20, 0x20) : RGB(0xF3, 0xF3, 0xF3));
	dc.FillSolidRect(0, 0, client.Width(), 1, dark ? RGB(0x3A, 0x3A, 0x3A) : RGB(0xD6, 0xD6, 0xD6));

	CFont font;
	font.Attach(Dpi_CreateUIFont(dpi, 9));
	CFont *oldfont = dc.SelectObject(&font);
	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(dark ? RGB(0xFF, 0xFF, 0xFF) : RGB(0x1A, 0x1A, 0x1A));

	// The search result, at the right end.
	int right = client.right;
	if (!m_searchresult.IsEmpty()) {
		CRect textrect(0, 0, client.right - Dpi_Scale(12, dpi), client.bottom);
		dc.DrawText(m_searchresult, &textrect, DT_RIGHT|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);
		right = textrect.right - dc.GetTextExtent(m_searchresult).cx - Dpi_Scale(12, dpi);
	}

	// Same colors as the boxes in the view, so the legend matches them.
	int x = Dpi_Scale(10, dpi), swatch = Dpi_Scale(14, dpi);
	for (int type = 0; type < FT_COUNT && theApp.m_settings.file_color == FILE_COLOR_BY_TYPE; type++) {
		COLORREF color = GetFileTypeColor(type, 0);
		if (dark) color = RGB(GetRValue(color) * 9 / 20, GetGValue(color) * 9 / 20, GetBValue(color) * 9 / 20);
		int sy = (client.Height() - swatch) / 2;
		dc.FillSolidRect(x, sy, swatch, swatch, RGB(0, 0, 0));
		dc.FillSolidRect(x + 1, sy + 1, swatch - 2, swatch - 2, color);
		x += swatch + Dpi_Scale(6, dpi);
		CString name = CurLang->ex->typenames[type];
		CRect textrect(x, 0, right, client.bottom);
		dc.DrawText(name, &textrect, DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);
		x += dc.GetTextExtent(name).cx + Dpi_Scale(18, dpi);
	}

	dc.SelectObject(oldfont);
	paintdc.BitBlt(0, 0, client.Width(), client.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(oldbitmap);
}
