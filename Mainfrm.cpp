
#include "StdAfx.h"
#include "SpaceMonger.h"
#include "FolderView.h"
#include "AboutDialog.h"
#include "Lang.h"

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

	// First count how many windows need to be rearranged
	numwnds = 0;
	if (first != NULL) {
		wnd = first;
		while (1) {
			if (wnd->m_hWnd != m_toolbar.m_hWnd)
				numwnds++;
			if (wnd == last) break;
			wnd = wnd->GetWindow(GW_HWNDNEXT);
		}
	}

	// Now lay them all out in a horizontal band (cheap & easy)
	if (first != NULL && numwnds > 0) {
		wndnum = 0, wnd = first;
		while (1) {
			if (wnd->m_hWnd != m_toolbar.m_hWnd) {
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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CMainToolBar::CMainToolBar()
{
	// Glyphs are from the Segoe Fluent Icons / Segoe MDL2 Assets fonts.
	static const struct { UINT id; wchar_t glyph; BOOL sep; } buttons[] = {
		{ ID_FILE_OPEN,			0xE838, 0 },	// FolderOpen
		{ ID_FILE_REFRESH,		0xE72C, 1 },	// Refresh
		{ ID_VIEW_ZOOM_FULL,	0xE9A6, 0 },	// FitPage
		{ ID_VIEW_ZOOM_IN,		0xE8A3, 0 },	// ZoomIn
		{ ID_VIEW_ZOOM_OUT,		0xE71F, 1 },	// ZoomOut
		{ ID_VIEW_FREE,			0xEDA2, 1 },	// HardDrive
		{ ID_FILE_RUN,			0xE768, 0 },	// Play
		{ ID_FILE_DELETE,		0xE74D, 1 },	// Delete
		{ ID_SETTINGS,			0xE713, 1 },	// Setting
		{ ID_APP_ABOUT,			0xE946, 0 },	// Info
	};
	for (int i = 0; i < sizeof(buttons) / sizeof(buttons[0]); i++) {
		Button button;
		button.id = buttons[i].id;
		button.glyph = buttons[i].glyph;
		button.label = i;
		button.separator_after = buttons[i].sep;
		button.enabled = (button.id == ID_FILE_OPEN || button.id == ID_SETTINGS
			|| button.id == ID_APP_ABOUT);
		button.checked = 0;
		m_buttons.push_back(button);
	}
	m_height = 32;
	m_hot = m_pressed = -1;
	m_tracking = 0;
}

CMainToolBar::~CMainToolBar()
{
}

int CMainToolBar::Create(CWnd *parent)
{
	if (!CWnd::Create(AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW, ::LoadCursor(NULL, IDC_ARROW)),
		L"", WS_CHILD|WS_VISIBLE, CRect(0, 0, 100, 32), parent, AFX_IDW_TOOLBAR))
		return 0;

	Reload();
	EnableState(ViewFree, 1);

	return 1;
}

void CMainToolBar::OnDestroy()
{
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
	Layout();
	if (m_hWnd != NULL) Invalidate();
}

void CMainToolBar::Layout(void)
{
	if (m_hWnd == NULL) return;

	int dpi = Dpi_ForWindow(m_hWnd);
	CClientDC dc(this);
	CFont *oldfont = dc.SelectObject(&m_textfont);

	int x = Dpi_Scale(4, dpi);
	int top = Dpi_Scale(3, dpi), bottom = m_height - Dpi_Scale(3, dpi);
	for (size_t i = 0; i < m_buttons.size(); i++) {
		Button &button = m_buttons[i];
		CSize textsize = dc.GetTextExtent(CurLang->toolbarnames[button.label]);
		int width = Dpi_Scale(8 + 16 + 6 + 8, dpi) + textsize.cx;
		button.rect.SetRect(x, top, x + width, bottom);
		x += width + Dpi_Scale(2, dpi);
		if (button.separator_after) x += Dpi_Scale(9, dpi);
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

		CRect iconrect(rect.left + Dpi_Scale(8, dpi), rect.top,
			rect.left + Dpi_Scale(8 + 16, dpi), rect.bottom);
		CFont *oldfont = dc.SelectObject(&m_iconfont);
		wchar_t glyph[2] = { button.glyph, 0 };
		dc.DrawText(glyph, 1, &iconrect, DT_CENTER|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);

		CRect textrect(iconrect.right + Dpi_Scale(6, dpi), rect.top, rect.right, rect.bottom);
		dc.SelectObject(&m_textfont);
		dc.DrawText(CurLang->toolbarnames[button.label], -1, &textrect,
			DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);
		dc.SelectObject(oldfont);

		if (button.separator_after) {
			int sx = rect.right + Dpi_Scale(2 + 4, dpi);
			dc.FillSolidRect(sx, rect.top + Dpi_Scale(6, dpi), Dpi_Scale(1, dpi),
				rect.Height() - Dpi_Scale(12, dpi), line);
		}
	}

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
