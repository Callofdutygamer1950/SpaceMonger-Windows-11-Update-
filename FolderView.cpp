#include "stdafx.h"
#include "spacemonger.h"
#include "FolderView.h"
#include "MainFrm.h"
#include "TipWnd.h"
#include "Lang.h"

IMPLEMENT_DYNCREATE(CFolderView, CFreeView)

static COLORREF BoxColors[] = {
	PALETTERGB(0xFF, 0x7F, 0x7F),
	PALETTERGB(0xFF, 0xBF, 0x7F),
	PALETTERGB(0xFF, 0xFF, 0x00),
	PALETTERGB(0x7F, 0xFF, 0x7F),
	PALETTERGB(0x7F, 0xFF, 0xFF),
	PALETTERGB(0xBF, 0xBF, 0xFF),
	PALETTERGB(0xBF, 0xBF, 0xBF),
	PALETTERGB(0xFF, 0x7F, 0xFF),

	PALETTERGB(0xFF, 0xBF, 0xBF),
	PALETTERGB(0xFF, 0xDF, 0xBF),
	PALETTERGB(0xFF, 0xFF, 0xBF),
	PALETTERGB(0xBF, 0xFF, 0xBF),
	PALETTERGB(0xDF, 0xFF, 0xFF),
	PALETTERGB(0xDF, 0xDF, 0xFF),
	PALETTERGB(0xDF, 0xDF, 0xDF),
	PALETTERGB(0xFF, 0xBF, 0xFF),

	PALETTERGB(0xBF, 0x7F, 0x7F),
	PALETTERGB(0xBF, 0x9F, 0x5F),
	PALETTERGB(0xBF, 0xBF, 0x3F),
	PALETTERGB(0x7F, 0xBF, 0x7F),
	PALETTERGB(0x7F, 0xBF, 0xBF),
	PALETTERGB(0x9F, 0x9F, 0xFF),
	PALETTERGB(0x9F, 0x9F, 0x9F),
	PALETTERGB(0xBF, 0x7F, 0xBF),

	PALETTERGB(0x00, 0x00, 0x00),
	PALETTERGB(0xFF, 0xFF, 0xFF),
};

// These colors are used for specifically-chosen folder colors.
static COLORREF FixedColors[] = {
	PALETTERGB(0xFF, 0xFF, 0xFF),
	PALETTERGB(0xBF, 0xBF, 0xBF),
	PALETTERGB(0x7F, 0x7F, 0x7F),

	PALETTERGB(0xFF, 0x7F, 0x7F),
	PALETTERGB(0xFF, 0xBF, 0x7F),
	PALETTERGB(0xFF, 0xFF, 0x00),
	PALETTERGB(0x7F, 0xFF, 0x7F),
	PALETTERGB(0x7F, 0xFF, 0xFF),
	PALETTERGB(0xBF, 0xBF, 0xFF),
	PALETTERGB(0xFF, 0x7F, 0xFF),

	PALETTERGB(0xFF, 0xFF, 0xFF),
	PALETTERGB(0xFF, 0xFF, 0xFF),
	PALETTERGB(0xBF, 0xBF, 0xBF),

	PALETTERGB(0xFF, 0x9F, 0x9F),
	PALETTERGB(0xFF, 0xDF, 0xBF),
	PALETTERGB(0xFF, 0xFF, 0xBF),
	PALETTERGB(0xBF, 0xFF, 0xBF),
	PALETTERGB(0xDF, 0xFF, 0xFF),
	PALETTERGB(0xDF, 0xDF, 0xFF),
	PALETTERGB(0xFF, 0xBF, 0xFF),

	PALETTERGB(0xBF, 0xBF, 0xBF),
	PALETTERGB(0x7F, 0x7F, 0x7F),
	PALETTERGB(0x3F, 0x3F, 0x3F),

	PALETTERGB(0xBF, 0x7F, 0x7F),
	PALETTERGB(0xBF, 0x9F, 0x9F),
	PALETTERGB(0xBF, 0xBF, 0x3F),
	PALETTERGB(0x7F, 0xBF, 0x7F),
	PALETTERGB(0x7F, 0xBF, 0xBF),
	PALETTERGB(0x9F, 0x9F, 0xFF),
	PALETTERGB(0xBF, 0x7F, 0xBF),
};

// In dark mode, the boxes use deeper versions of the same colors, with
// light text, so the view isn't a wall of bright pastels.
static COLORREF ThemeBoxColor(COLORREF color)
{
	if (!Theme_IsDark()) return color;
	return RGB(GetRValue(color) * 9 / 20, GetGValue(color) * 9 / 20, GetBValue(color) * 9 / 20);
}

static COLORREF ThemeBoxTextColor(void)
{
	return Theme_IsDark() ? RGB(0xEB, 0xEB, 0xEB) : RGB(0, 0, 0);
}

static COLORREF ThemeSelectedColor(void)
{
	return Theme_IsDark() ? RGB(0xE6, 0xE6, 0xE6) : RGB(0, 0, 0);
}

static COLORREF ThemeSelectedTextColor(void)
{
	return Theme_IsDark() ? RGB(0, 0, 0) : RGB(0xFF, 0xFF, 0xFF);
}

CFolderView::CFolderView()
{
	rootfolder = NULL;
	selected = NULL;
	displayfolders = displayend = NULL;
	zoomlevel = 0;
	showfreespace = 1;
	lastcur = NULL;
	m_dpi = 96;
	m_border = 3;
	m_line = 12;
	vmin = hmin = 0;
}

CFolderView::~CFolderView()
{
	ClearDisplayFolders();
}

BEGIN_MESSAGE_MAP(CFolderView, CFreeView)
	//{{AFX_MSG_MAP(CFolderView)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONUP()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI_RANGE(100, 41000, OnIgnoreUpdate)
END_MESSAGE_MAP()

BOOL CFolderView::OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	if (::IsWindow(m_infotipwnd.m_hWnd)) {
		MSG msg;
		msg.message = message;
		msg.wParam = wParam;
		msg.lParam = lParam;
		msg.hwnd = m_hWnd;
		m_infotipwnd.ReflectMessage(&msg);
	}
	if (::IsWindow(m_nametipwnd.m_hWnd)) {
		MSG msg;
		msg.message = message;
		msg.wParam = wParam;
		msg.lParam = lParam;
		msg.hwnd = m_hWnd;
		m_nametipwnd.ReflectMessage(&msg);
	}
	return CFreeView::OnWndMsg(message, wParam, lParam, pResult);
}

int CFolderView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFreeView::OnCreate(lpCreateStruct) == -1)
		return -1;
	if (!m_infotipwnd.CreateEz(this, 64, 64)) return -1;
	if (!m_nametipwnd.CreateEz(this, 64, 64)) return -1;

	int i;
	LOGPALETTE *logpalette = (LOGPALETTE *)malloc(sizeof(LOGPALETTE) + 26 * sizeof(PALETTEENTRY));
	logpalette->palVersion = 0x300;
	logpalette->palNumEntries = 26;
	for (i = 0; i < 26; i++) {
		logpalette->palPalEntry[i].peFlags = 0;
		logpalette->palPalEntry[i].peRed = GetRValue(BoxColors[i]);
		logpalette->palPalEntry[i].peGreen = GetGValue(BoxColors[i]);
		logpalette->palPalEntry[i].peBlue = GetBValue(BoxColors[i]);
	}
	m_palette.CreatePalette(logpalette);
	free(logpalette);

	black.CreateSolidBrush(RGB(0x00, 0x00, 0x00));
	white.CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));

	m_infotipwnd.SetAutoShow(1);
	m_infotipwnd.SetAutoPos(1);
	m_infotipwnd.SetShowDelay(theApp.m_settings.infotip_delay);
	m_infotipwnd.EnableWindow(0);

	m_nametipwnd.SetAutoShow(1);
	m_nametipwnd.SetShowDelay(theApp.m_settings.nametip_delay);
	m_nametipwnd.SetVPadding(0);
	m_nametipwnd.SetHPadding(1);
	m_nametipwnd.EnableWindow(0);

	UpdateMetrics();

	return 0;
}

// UpdateMetrics
//   Recreates the fonts and recomputes the layout sizes for the current DPI.

void CFolderView::UpdateMetrics(void)
{
	m_dpi = Dpi_ForWindow(m_hWnd);

	if (minifont.m_hObject != NULL) minifont.DeleteObject();
	minifont.Attach(Dpi_CreateUIFont(m_dpi, 7));

	CClientDC dc(this);
	CFont *oldfont = dc.SelectObject(&minifont);
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	dc.SelectObject(oldfont);
	m_line = tm.tmHeight;
	m_border = Px(3);

	// The tip windows own (and delete) the fonts they're given.
	::SendMessage(m_nametipwnd.m_hWnd, WM_SETFONT, (WPARAM)Dpi_CreateUIFont(m_dpi, 7), 0);
	::SendMessage(m_infotipwnd.m_hWnd, WM_SETFONT, (WPARAM)Dpi_CreateUIFont(m_dpi, 9), 0);
}

void CFolderView::OnDestroy()
{
	m_palette.DeleteObject();
	black.DeleteObject();
	white.DeleteObject();
	minifont.DeleteObject();

	m_infotipwnd.DestroyWindow();
	m_nametipwnd.DestroyWindow();

	CFreeView::OnDestroy();
}

CDisplayFolder *CFolderView::GetDisplayFolderFromPoint(const CPoint &point)
{
	CDisplayFolder *cur = displayfolders;

	while (cur != NULL) {
		if (point.x > cur->x && point.y > cur->y
			&& point.x < cur->x + cur->w && point.y < cur->y + cur->h) {
			if (cur->flags & 1) {
				if (point.x < cur->x+m_border || point.y < cur->y+m_border+m_line
					|| point.x > cur->x+cur->w-m_border || point.y > cur->y+cur->h-m_border)
					break;
			}
			else break;
		}
		cur = cur->next;
	}

	if (cur != NULL && cur->name == NULL)
		cur = NULL;
	if (cur != NULL && cur->name[0] == L'<')
		cur = NULL;

	return cur;
}

CDisplayFolder *CFolderView::GetContainerDisplayFolderFromPoint(const CPoint &point)
{
	CDisplayFolder *cur = displayfolders, *best = NULL;
	CRect minrect(-32768,-32768,32767,32767);

	while (cur != NULL) {
		if (cur->flags & 1
			&& point.x > cur->x && point.y > cur->y
			&& point.x < cur->x + cur->w && point.y < cur->y + cur->h
			&& cur->x >= minrect.left && cur->y >= minrect.top
			&& cur->x+cur->w <= minrect.right && cur->y+cur->h <= minrect.bottom) {
			// Found a tighter folder, so use it.
			minrect.left = cur->x;
			minrect.top = cur->y;
			minrect.right = cur->x+cur->w;
			minrect.bottom = cur->y+cur->h;
			best = cur;
		}
		cur = cur->next;
	}

	return best;
}

void CFolderView::HighlightPathAtPoint(const CPoint &point)
{
	CDisplayFolder *cur = displayfolders;

	CDC *pDC = GetDC();
	CFont *oldfont = pDC->SelectObject(&minifont);
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextAlign(TA_LEFT | TA_TOP | TA_NOUPDATECP);
	while (cur != NULL) {
		if (cur->name != NULL && cur->name[0] != L'<') {
			if (point.x > cur->x && point.y > cur->y
				&& point.x < cur->x + cur->w && point.y < cur->y + cur->h) {
				if (!(cur->flags & 4)) {
					cur->flags |= 4;
					MinimalDrawDisplayFolder(pDC, cur, selected == cur);
				}
			}
			else {
				if (cur->flags & 4) {
					cur->flags &= ~4;
					MinimalDrawDisplayFolder(pDC, cur, selected == cur);
				}
			}
		}
		cur = cur->next;
	}
	pDC->SelectObject(oldfont);
	ReleaseDC(pDC);
}

void CFolderView::OnLButtonDown(UINT flags, CPoint point)
{
	SelectFolder(GetDisplayFolderFromPoint(point));
	lastcur = NULL;
	OnMouseMove(flags, point);
}

void CFolderView::OnLButtonDblClk(UINT flags, CPoint point)
{
	CDisplayFolder *cur = GetDisplayFolderFromPoint(point);
	SelectFolder(cur);

	if (cur != NULL) {
		if (cur->flags & 1) ZoomIn(cur);
		else theApp.OnFileRun();
	}
	lastcur = NULL;
	OnMouseMove(flags, point);
}

void CFolderView::OnMouseMove(UINT nFlags, CPoint point)
{
	CDisplayFolder *cur = GetDisplayFolderFromPoint(point);

	if (theApp.m_settings.rollover_box) HighlightPathAtPoint(point);
	else HighlightPathAtPoint(CPoint(-1,-1));

	if (theApp.m_settings.show_info_tips) {
		if (cur == NULL)
			m_infotipwnd.EnableWindow(0);
		else if (lastcur != cur)
			SetupInfoTip(cur);
	}
	else m_infotipwnd.EnableWindow(0);

	if (theApp.m_settings.show_name_tips) {
		if (cur == NULL)
			m_nametipwnd.EnableWindow(0);
		else if (lastcur != cur)
			SetupNameTip(cur);
	}
	else m_nametipwnd.EnableWindow(0);

	lastcur = cur;
}

static void PrintFileSize(CString &string, ui64 size)
{
	wchar_t sizebuf[64];
	wchar_t *dest = sizebuf+63;
	int ctr = 0;
	sizebuf[63] = L'\0';
	if (size == 0) *--dest = L'0';
	while (size != 0) {
		if (++ctr == 4) *--dest = CurLang->digitcomma, ctr = 1;
		*--dest = (wchar_t)(size % 10) + L'0';
		size /= 10;
	}
	string += dest;
	string += L" ";
	string += CurLang->bytes;
}

static CString FormatCount(ui64 value)
{
	CString result;
	result.Format(L"%I64u", value);
	for (int pos = result.GetLength() - 3; pos > 0; pos -= 3)
		result.Insert(pos, CurLang->digitcomma);
	return result;
}

static void PrintDate(CString &string, ui64 date)
{
	CString tempstring;
	SYSTEMTIME mtime;
	FILETIME local;
	FileTimeToLocalFileTime((FILETIME *)&date, &local);
	FileTimeToSystemTime(&local, &mtime);
	if (mtime.wMonth < 1 || mtime.wMonth > 12) mtime.wMonth = 1;
	tempstring.Format(L"%02d %s %04d   %d:%02d:%02d",
		mtime.wDay, CurLang->monthnames[mtime.wMonth-1], mtime.wYear,
		mtime.wHour, mtime.wMinute, mtime.wSecond);
	string += tempstring;
}

CString CFolderView::GetFullPath(CDisplayFolder *cur)
{
	CString path;
	CFolderTree *ft = (CFolderTree *)GetDocument();
	if (cur == NULL || ft == NULL) return path;
	path = ft->m_path;
	BuildTitleReverse(cur->source, path);
	path += cur->source->names[cur->index];
	return path;
}

void CFolderView::SetupInfoTip(CDisplayFolder *cur)
{
	if (cur == NULL) {
		m_infotipwnd.EnableWindow(0);
		return;
	}

	CString path;
	CString string, tempstring;

	// Get the full pathname to the file.
	CFolderTree *ft = (CFolderTree *)GetDocument();
	path += ft->m_path;
	BuildTitleReverse(cur->source, tempstring);
	path += tempstring;
	CString fullpath = path + cur->source->names[cur->index];

	// Load in the file's attributes.
	WIN32_FILE_ATTRIBUTE_DATA info;
	if (!GetFileAttributesEx(MakeLongPath(fullpath), GetFileExInfoStandard, &info))
		info.dwFileAttributes = 0;

	if (theApp.m_settings.infotip_flags & TIP_PATH)
		string += path;
	if (theApp.m_settings.infotip_flags & TIP_NAME)
		string += cur->source->names[cur->index];
	if (theApp.m_settings.infotip_flags & (TIP_NAME|TIP_PATH))
		string += L'\n';
	if (theApp.m_settings.infotip_flags & TIP_SIZE)
		PrintFileSize(string, cur->source->actualsizes[cur->index]);
	if (theApp.m_settings.infotip_flags & TIP_ATTRIB) {
		if ((theApp.m_settings.infotip_flags & TIP_SIZE)
			&& info.dwFileAttributes != 0) string += L"  /  ";
		if (info.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)			string += L" ", string += CurLang->attribnames[0];
		if (info.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED)		string += L" ", string += CurLang->attribnames[1];
		if (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)		string += L" ", string += CurLang->attribnames[2];
		if (info.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)		string += L" ", string += CurLang->attribnames[3];
		if (info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)			string += L" ", string += CurLang->attribnames[4];
		if (info.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE)			string += L" ", string += CurLang->attribnames[5];
		if (info.dwFileAttributes & FILE_ATTRIBUTE_READONLY)		string += L" ", string += CurLang->attribnames[6];
		if (info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)	string += L" ", string += CurLang->attribnames[7];
		if (info.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE)		string += L" ", string += CurLang->attribnames[8];
		if (info.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)			string += L" ", string += CurLang->attribnames[9];
		if (info.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY)		string += L" ", string += CurLang->attribnames[10];
	}
	if (theApp.m_settings.infotip_flags & (TIP_ATTRIB|TIP_SIZE))
		string += L'\n';
	if (theApp.m_settings.infotip_flags & TIP_DATE) {
		PrintDate(string, cur->source->times[cur->index]);
		string += L'\n';
	}
	if (theApp.m_settings.infotip_flags & TIP_ICON) {
		SHFILEINFO fileinfo;
		memset(&fileinfo, 0, sizeof(SHFILEINFO));
		SHGetFileInfo(fullpath, 0, &fileinfo, sizeof(SHFILEINFO), SHGFI_ICON);
		HICON oldicon = (HICON)m_infotipwnd.SendMessage(WM_GETICON, 0, 0);
		m_infotipwnd.SetIcon(fileinfo.hIcon, 1);
		m_infotipwnd.SetIconPos(TW_LEFT);
		if (oldicon != NULL && oldicon != fileinfo.hIcon) DestroyIcon(oldicon);
	}

	m_infotipwnd.SetBgColor(Theme_Color(COLOR_INFOBK));
	m_infotipwnd.SetTextColor(Theme_Color(COLOR_INFOTEXT));
	m_infotipwnd.SetBorderColor(Theme_IsDark() ? RGB(0x60, 0x60, 0x60) : RGB(0, 0, 0));
	m_infotipwnd.SetWindowText(string);
	m_infotipwnd.AutoSize();
	m_infotipwnd.EnableWindow(1);
	m_infotipwnd.RedrawWindow();
}

void CFolderView::SetupNameTip(CDisplayFolder *cur)
{
	m_nametipwnd.EnableWindow(0);

	if (cur == NULL) return;

	CDC *pDC = GetDC();
	CFont *oldfont = pDC->SelectObject(&minifont);

	int tx, ty, failed = 0;
	CSize size = pDC->GetTextExtent(cur->name, (int)wcslen(cur->name));
	pDC->SelectObject(oldfont);
	ReleaseDC(pDC);

	int x = cur->x, y = cur->y, w = cur->w + 1, h = cur->h + 1;
	if (size.cx > w - 2) tx = x + 2;
	else if (cur->flags & 1) tx = x + 2, failed++;
	else tx = x + (w - size.cx) / 2, failed++;
	if (size.cy > h - 2) ty = y + 1;
	else if (cur->flags & 1) ty = y + 1, failed++;
	else ty = y + (h - size.cy) / 2, failed++;
	if (failed == 2) return;

	if (!(cur->flags & 1) && h >= 3 * m_line + 4 && w >= Px(48))
		ty -= m_line;

	if (selected == cur) {
		m_nametipwnd.SetBgColor(ThemeSelectedColor());
		m_nametipwnd.SetTextColor(ThemeSelectedTextColor());
	}
	else if (cur->flags & 4) {
		m_nametipwnd.SetBgColor(ThemeBoxColor(BoxColors[(cur->depth & 7) + 8]));
		m_nametipwnd.SetTextColor(ThemeBoxTextColor());
	}
	else {
		m_nametipwnd.SetBgColor(ThemeBoxColor(BoxColors[cur->depth & 7]));
		m_nametipwnd.SetTextColor(ThemeBoxTextColor());
	}
	m_nametipwnd.SetWindowText(cur->name);
	m_nametipwnd.AutoSize();
	m_nametipwnd.MoveWindow(this, tx-2, ty-1);
	m_nametipwnd.PushOnScreen();

	m_nametipwnd.EnableWindow(1);
	m_nametipwnd.RedrawWindow();
}

static void AddMenuEntry(HMENU menu, UINT cmd = 0, const wchar_t *string = NULL,
	BOOL checked = 0, BOOL disabled = 0, BOOL bold = 0)
{
	MENUITEMINFO mii;
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_CHECKMARKS|MIIM_ID|MIIM_STATE|MIIM_TYPE;
	mii.fType = ((string != NULL) ? MFT_STRING : MFT_SEPARATOR);
	mii.fState = 0;
	if (bold) mii.fState |= MFS_DEFAULT;
	if (checked) mii.fState |= MFS_CHECKED;
	if (disabled) mii.fState |= MFS_GRAYED|MFS_DISABLED;
	mii.wID = cmd;
	mii.hSubMenu = NULL;
	mii.hbmpChecked = NULL;
	mii.hbmpUnchecked = NULL;
	mii.dwTypeData = (wchar_t *)string;
	if (string != NULL) mii.cch = (UINT)wcslen(string);
	else mii.cch = 0;

	::InsertMenuItem(menu, GetMenuItemCount(menu), TRUE, &mii);
}

void CFolderView::OnRButtonUp(UINT flags, CPoint point)
{
	CDisplayFolder *cur = GetDisplayFolderFromPoint(point);
	SelectFolder(cur);
	ClientToScreen(&point);

	bool showinfo = m_infotipwnd.IsWindowEnabled() != 0;
	bool showname = m_nametipwnd.IsWindowEnabled() != 0;
	m_infotipwnd.EnableWindow(0);
	m_nametipwnd.EnableWindow(0);

	CSpaceMonger *app = (CSpaceMonger *)AfxGetApp();
	HMENU menu = ::CreatePopupMenu();
	::AddMenuEntry(menu, ID_VIEW_ZOOM_IN, CurLang->zoomin, 0, cur == NULL, (cur != NULL) && (cur->flags & 1) != 0);
	::AddMenuEntry(menu, ID_VIEW_ZOOM_OUT, CurLang->zoomout, 0, zoomlevel == 0, 0);
	::AddMenuEntry(menu, ID_VIEW_ZOOM_FULL, CurLang->zoomfull, 0, zoomlevel == 0, 0);
	::AddMenuEntry(menu);
	::AddMenuEntry(menu, ID_FILE_RUN, CurLang->run, 0, cur == NULL, (cur != NULL) && (cur->flags & 1) == 0);
	::AddMenuEntry(menu, ID_FILE_EXPLORE, CurLang->showinexplorer, 0, cur == NULL, 0);
	::AddMenuEntry(menu, ID_FILE_DELETE, CurLang->del, 0,
		app->m_settings.disable_delete || cur == NULL, 0);
	::AddMenuEntry(menu);
	::AddMenuEntry(menu, ID_FILE_OPEN, CurLang->opendrive, 0, 0, 0);
	::AddMenuEntry(menu, ID_FILE_OPEN_FOLDER, CurLang->openfolder, 0, 0, 0);
	::AddMenuEntry(menu, ID_FILE_REFRESH, CurLang->rescandrive, 0, rootfolder == NULL, 0);
	::AddMenuEntry(menu, ID_VIEW_FREE, CurLang->showfreespace, showfreespace, rootfolder == NULL, 0);
	::AddMenuEntry(menu);
	::AddMenuEntry(menu, ID_FILE_PROPERTIES, CurLang->properties, 0, cur == NULL, 0);
	::TrackPopupMenuEx(menu, TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON,
		point.x, point.y, theApp.m_mainframe->m_hWnd, NULL);
	::DestroyMenu(menu);

	if (showname) m_nametipwnd.EnableWindow(1);
	if (showinfo) m_infotipwnd.EnableWindow(1);
}

void CFolderView::SelectFolder(CDisplayFolder *cur)
{
	if (cur == selected) return;

	CDC *dc = GetDC();

	if (selected != NULL) {
		DrawDisplayFolder(dc, selected, 0);
		selected = NULL;
	}

	selected = cur;
	if (selected != NULL)
		DrawDisplayFolder(dc, selected, 1);

	ReleaseDC(dc);
	UpdateTitleBar();
}

static CString GetSizeString(ui64 size, ui64 totalspace, BOOL percent)
{
	CString result;

	if (totalspace == 0) totalspace = (ui64)(-1);

	if (percent) {
		size = (ui64)(((double)size * 1000.0) / (double)totalspace);
		result.Format(CurLang->percent_format, (ui32)size / 10, (ui32)size % 10);
	}
	else {
		static const ui64 KB = 1024, MB = KB * 1024, GB = MB * 1024, TB = GB * 1024;
		ui64 unit;
		const wchar_t *displaytype;

		if (size < KB) unit = 1, displaytype = CurLang->bytes;
		else if (size < MB) unit = KB, displaytype = CurLang->kb;
		else if (size < GB) unit = MB, displaytype = CurLang->mb;
		else if (size < TB) unit = GB, displaytype = CurLang->gb;
		else unit = TB, displaytype = CurLang->tb;

		ui32 displayfull = (ui32)(size / unit);
		ui32 displayfractional = (ui32)(10 * (size % unit) / unit);
		result.Format(CurLang->size_format, displayfull, displayfractional, displaytype);
	}

	return result;
}

// Percentages are of the whole disk, or of the folder when scanning a folder.
ui64 CFolderView::GetPercentBase(void)
{
	CFolderTree *ft = (CFolderTree *)GetDocument();
	if (ft == NULL) return 0;
	if (ft->IsVolumeRoot() || ft->GetRoot() == NULL) return ft->totalspace;
	return ft->GetRoot()->SizeTotal();
}

void CFolderView::UpdateTitleBar(void)
{
	CMainFrame *mainfrm = (CMainFrame *)GetTopLevelFrame();
	CFolderTree *ft = (CFolderTree *)GetDocument();
	CString title = L"";
	ui64 size;

	if (mainfrm == NULL) return;
	mainfrm->m_toolbar.UpdateButtonsForView(this);

	if (ft == NULL) {
		mainfrm->SetWindowText(L"SpaceMonger");
		return;
	}

	if (selected != NULL && selected->name != NULL) {
		title += ft->m_path;
		BuildTitleReverse(selected->source, title);
		title += selected->source->names[selected->index];
		size = selected->source->sizes[selected->index];
		title += L"  -  " + GetSizeString(size, GetPercentBase(), 1)
			+ L"  -  " + GetSizeString(size, GetPercentBase(), 0) + L"  -  ";
	}
	else if (rootfolder != NULL) {
		title += ft->m_path;
		BuildTitleReverse(rootfolder, title);
		if (rootfolder->parent == NULL && ft->IsVolumeRoot()) size = ft->totalspace; // Kludge
		else size = rootfolder->SizeTotal();
		title += L"  -  " + GetSizeString(size, ft->totalspace, 0)
			+ L" " + CurLang->total + L"  -  " + GetSizeString(ft->freespace, ft->freespace, 0)
			+ L" " + CurLang->free + L"  -  ";
	}

	title += L"SpaceMonger";

	mainfrm->SetWindowText(title);
}

void CFolderView::BuildTitleReverse(CFolder *folder, CString &string)
{
	if (folder->parent != NULL)
		BuildTitleReverse(folder->parent, string);
	if (folder->parent == NULL) return;

	string += folder->parent->names[folder->parentindex];
	string += L"\\";
}

BOOL CFolderView::OnEraseBkgnd(CDC *pDC)
{
	return 1;		// OnPaint draws everything
}

void CFolderView::OnPaint()
{
	// Draw into an off-screen bitmap first, so the view doesn't flicker.
	CPaintDC dc(this);
	CRect rect;
	GetClientRect(&rect);
	if (rect.IsRectEmpty()) return;

	CDC memdc;
	memdc.CreateCompatibleDC(&dc);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	CBitmap *oldbitmap = memdc.SelectObject(&bitmap);

	OnDraw(&memdc);

	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &memdc, 0, 0, SRCCOPY);
	memdc.SelectObject(oldbitmap);
}

void CFolderView::Redraw(void)
{
	Invalidate(FALSE);
	UpdateWindow();
}

void CFolderView::OnDraw(CDC *pDC)
{
	CFont *oldfont = pDC->SelectObject(&minifont);
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextAlign(TA_LEFT | TA_TOP | TA_NOUPDATECP);

	CDisplayFolder *cur = displayfolders;

	FillBox(pDC->m_hDC, ColorFlat, 0, 0, m_width, m_height);
	DrawBox(pDC->m_hDC, ColorDark, 0, 0, m_width, m_height);

	while (cur != NULL) {
		MinimalDrawDisplayFolder(pDC, cur, selected == cur);
		cur = cur->next;
	}
	pDC->SelectObject(oldfont);
}

void CFolderView::DrawDisplayFolder(CDC *pDC, CDisplayFolder *cur, BOOL sel)
{
	CFont *oldfont = pDC->SelectObject(&minifont);
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextAlign(TA_LEFT | TA_TOP | TA_NOUPDATECP);

	MinimalDrawDisplayFolder(pDC, cur, sel);
	pDC->SelectObject(oldfont);
}

// Fills a rectangular ring "thickness" pixels wide.
static void FillFrame(HDC dc, HBRUSH brush, int x, int y, int w, int h, int thickness)
{
	if (thickness * 2 >= w || thickness * 2 >= h) {
		FillBox(dc, brush, x, y, w, h);
		return;
	}
	FillBox(dc, brush, x, y, w, thickness);
	FillBox(dc, brush, x, y + h - thickness, w, thickness);
	FillBox(dc, brush, x, y + thickness, thickness, h - thickness * 2);
	FillBox(dc, brush, x + w - thickness, y + thickness, thickness, h - thickness * 2);
}

void CFolderView::MinimalDrawDisplayFolder(CDC *pDC, CDisplayFolder *cur, BOOL sel)
{
	int x, y, w, h;
	int B = m_border;

	x = cur->x, y = cur->y, w = cur->w + 1, h = cur->h + 1;

	if (cur->depth != -1)
		DrawBox(pDC->m_hDC, black, x, y, w, h);

	if (w > 2 && h > 2) {
		COLORREF color, bright, dark;
		CBrush brush, bbrush, dbrush;
		int colortype = (cur->flags & 1) ? theApp.m_settings.folder_color
			: theApp.m_settings.file_color;

		if (sel && !(cur->flags & 2))
			color = bright = dark = ThemeSelectedColor();
		else if (cur->depth != -1) {
			if (colortype == 0) {
				color = BoxColors[cur->depth & 7];
				bright = BoxColors[(cur->depth & 7) + 8];
				dark = BoxColors[(cur->depth & 7) + 16];
			}
			else if (colortype == 1) {
				color = Theme_Color(COLOR_3DFACE);
				bright = Theme_Color(COLOR_3DHILIGHT);
				dark = Theme_Color(COLOR_3DSHADOW);
			}
			else {
				color = FixedColors[colortype-2];
				bright = FixedColors[colortype-2 + 10];
				dark = FixedColors[colortype-2 + 20];
			}
			if (theApp.m_settings.rollover_box) {
				if (cur->flags & 4) dark = color, color = bright, bright = RGB(255,255,255);
				else bright = color, color = dark;
			}
			if (colortype != 1) {
				color = ThemeBoxColor(color);
				bright = ThemeBoxColor(bright);
				dark = ThemeBoxColor(dark);
			}
		}
		else color = bright = dark = Theme_Color(COLOR_3DFACE);

		brush.CreateSolidBrush(color);
		bbrush.CreateSolidBrush(bright);
		dbrush.CreateSolidBrush(dark);

		DrawDualBox(pDC->m_hDC, bbrush, dbrush, x + 1, y + 1, w - 2, h - 2);
		if (cur->flags & 1) {
			// Folders: a border, plus a header band for the name.  The
			// inside is left alone, since the folder's contents go there.
			FillFrame(pDC->m_hDC, brush, x + 2, y + 2, w - 4, h - 4, B - 2);
			FillBox(pDC->m_hDC, brush, x + B, y + B, w - B * 2, m_line);
		}
		else FillBox(pDC->m_hDC, brush, x + 2, y + 2, w - 4, h - 4);
	}

	if (cur->name != NULL) {
		CRgn rgn;
		rgn.CreateRectRgn(x, y, x + w, y + h);
		pDC->SelectClipRgn(&rgn);

		int len = (int)wcslen(cur->name);
		CSize size;
		int tx, ty;

		size = pDC->GetTextExtent(cur->name, len);
		if (size.cx > w - 2 || (cur->flags & 1)) tx = x + 2;
		else tx = x + (w - size.cx) / 2;
		if (size.cy > h - 2 || (cur->flags & 1)) ty = y + B - 1;
		else ty = y + (h - size.cy) / 2;

		pDC->SetTextColor((sel && !(cur->flags & 2)) ? ThemeSelectedTextColor() : ThemeBoxTextColor());
		if (cur->flags & 2) pDC->SetTextColor(Theme_Color(COLOR_WINDOWTEXT));

		if (cur->flags & 2) {
			// There's only one free-space block, so we can afford to
			// be a little less efficient with it.
			CFolderTree *ft = (CFolderTree *)GetDocument();
			CString string;
			ui64 ts = ft->totalspace;
			if (ts <= 1) ts = 1;
			si32 freepercent = (si32)(ft->freespace * (ui64)1000 / ts);
			int top = y + h / 2 - m_line * 2;
			string.Format(CurLang->freespace_format, freepercent / 10, freepercent % 10);
			size = pDC->GetTextExtent(string);
			if (size.cx > w-2) tx = x + 2;
			else tx = x + (w - size.cx) / 2;
			pDC->TextOut(tx, top, string);
			pDC->TextOut(tx, top + m_line, GetSizeString(ft->freespace, ft->totalspace, 0)
				+ L" " + CurLang->free);
			pDC->TextOut(tx, top + m_line * 2, CString(CurLang->files_total) + L"  "
				+ FormatCount(ft->numfiles));
			pDC->TextOut(tx, top + m_line * 3, CString(CurLang->folders_total) + L"  "
				+ FormatCount(ft->numfolders));
		}
		else {
			if (!(cur->flags & 1) && h >= 3 * m_line + 4 && w >= Px(48)) {
				// Enough room (probably) for the date and file size
				CString string;
				CSize size;
				string.Empty();
				PrintFileSize(string, cur->source->actualsizes[cur->index]);
				size = pDC->GetTextExtent(string);
				if (size.cx > w-2)
					pDC->TextOut(x+2, ty+1, string);
				else pDC->TextOut(x+(w-size.cx)/2, ty+1, string);
				string.Empty();
				PrintDate(string, cur->source->times[cur->index]);
				size = pDC->GetTextExtent(string);
				if (size.cx > w-2)
					pDC->TextOut(x+2, ty+1+m_line, string);
				else pDC->TextOut(x+(w-size.cx)/2, ty+1+m_line, string);
				ty -= m_line;
			}
			pDC->TextOut(tx, ty, cur->name, len);
		}
		pDC->SelectClipRgn(NULL);
	}
}

void CFolderView::SetDocument(CFreeDoc *doc)
{
	CFreeView::SetDocument(doc);

	if (doc != NULL)
		rootfolder = ((CFolderTree *)doc)->GetRoot();
	else rootfolder = NULL;
	zoomlevel = 0;

	UpdateTitleBar();
	OnSize(0, m_width, m_height);
	Redraw();
}

void CFolderView::OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized)
{
	if (nState == WA_ACTIVE || nState == WA_CLICKACTIVE)
		SetPalette();
}

void CFolderView::SetPalette(void)
{
}

void CFolderView::OnUpdate(CFreeDoc *doc)
{
	UpdateTitleBar();

	OnSize(0, m_width, m_height);
	Redraw();

	m_infotipwnd.SetShowDelay(theApp.m_settings.infotip_delay);
	m_nametipwnd.SetShowDelay(theApp.m_settings.nametip_delay);
}

void CFolderView::OnSize(UINT nType, int cx, int cy)
{
	CRect rect;
	GetClientRect(&rect);
	m_width = cx = rect.right - rect.left;
	m_height = cy = rect.bottom - rect.top;

	ClearDisplayFolders();
	lastcur = NULL;

	if (rootfolder == NULL) {
		if (GetDocument() == NULL) return;
		rootfolder = ((CFolderTree *)GetDocument())->GetRoot();
		zoomlevel = 0;
		if (rootfolder == NULL) return;
	}

	BuildFolderLayout(0, 0, cx - 1, cy - 1, rootfolder, zoomlevel);
	if (nType != 0) Invalidate(FALSE);
}

CDisplayFolder *CFolderView::AddDisplayFolder(CFolder *source, ui32 index,
	si32 depth, si16 x, si16 y, si16 w, si16 h, ui32 flags)
{
	CDisplayFolder *newfolder = new CDisplayFolder;
	wchar_t c;

	if (source != NULL && index != (ui32)-1) {
		newfolder->name = source->names[index];
		c = newfolder->name[0];
	}
	else newfolder->name = NULL, c = 0;

	if (c == L'<')
		newfolder->depth = -1, flags |= 2;
	else newfolder->depth = depth;

	newfolder->flags = flags;
	newfolder->source = source;
	newfolder->index = index;
	newfolder->x = x, newfolder->y = y;
	newfolder->w = w, newfolder->h = h;
	newfolder->next = NULL;

	if (displayend == NULL)
		displayfolders = displayend = newfolder;
	else
		displayend = (displayend->next = newfolder);

	return(newfolder);
}

void CFolderView::ClearDisplayFolders(void)
{
	CDisplayFolder *cur = displayfolders, *temp;
	while (cur != NULL) {
		temp = cur->next;
		delete cur;
		cur = temp;
	}
	displayfolders = NULL;
	displayend = NULL;
	selected = NULL;
}

void CFolderView::BuildFolderLayout(int x, int y, int w, int h, CFolder *folder, int depth)
{
	unsigned int i;

	if (folder == NULL) return;

	int *indices = new int[folder->cur];
	for (i = 0; i < folder->cur; i++) indices[i] = i;

	static const int minsizes[][2] = {
		{ 96, 64, },
		{ 64, 48, },
		{ 48, 32, },
		{ 32, 24, },
		{ 24, 16, },
		{ 16, 12, },
		{ 8, 6, },
	};
	hmin = Px(minsizes[theApp.m_settings.density + 3][0]);
	vmin = Px(minsizes[theApp.m_settings.density + 3][1]);

	SizeFolders(x, y, w, h, folder, indices, (int)folder->cur, depth);

	delete[] indices;
}

void CFolderView::SizeFolders(int x, int y, int w, int h, CFolder *folder, int *index, int numindices, int depth)
{
	if (folder == NULL) return;

	int *list1 = new int[numindices];
	int *list2 = new int[numindices];
	int numlist1, numlist2, largest;
	si64 list1sum, list2sum, bignum;
	int x1, y1, w1, h1;
	int x2, y2, w2, h2;
	int split;
	int B = m_border, T = m_line;

	// Split the lists evenly.  We assume the sizes are sorted
	// in descending order.  Overall, this is a greedy algorithm,
	// so it should produce fairly good results.

	numlist1 = numlist2 = 0;
	list1sum = list2sum = 0;

	for (largest = 0; largest < numindices; largest++) {
		bignum = (si64)folder->sizes[index[largest]];
		if (folder->names[index[largest]][0] == L'<' && !showfreespace)
			bignum = 0;
		if (bignum != 0) {
			if (list1sum <= list2sum) {
				list1[numlist1++] = index[largest];
				list1sum += bignum;
			}
			else {
				list2[numlist2++] = index[largest];
				list2sum += bignum;
			}
		}
	}

	// Don't bother if the files have no space
	if (list1sum + list2sum <= 0) {
		delete[] list1;
		delete[] list2;
		return;
	}

	// Okay, we're now as even as we can safely get.  Now we know how to
	// split up the space.
	int wbias, hbias;
	if (theApp.m_settings.bias > 0)
		wbias = theApp.m_settings.bias + 8, hbias = 8;
	else if (theApp.m_settings.bias < 0)
		hbias = -theApp.m_settings.bias + 8, wbias = 8;
	else wbias = 8, hbias = 8;
	if (((w * wbias) / 8) > ((h * hbias) / 8)) {
		split = (int)(((double)w * (double)list1sum) / (double)(list1sum + list2sum));
		x1 = x, y1 = y, w1 = split, h1 = h;
		x2 = x + split, y2 = y, w2 = w - split, h2 = h;
	}
	else {
		split = (int)(((double)h * (double)list1sum) / (double)(list1sum + list2sum));
		x1 = x, y1 = y, w1 = w, h1 = split;
		x2 = x, y2 = y + split, w2 = w, h2 = h - split;
	}
	// Now if a given rectangle has more than one file and is
	// large enough to be subdivided again, subdivide again
	if (numlist1 > 1 && w1 > hmin && h1 > vmin)
		SizeFolders(x1, y1, w1, h1, folder, list1, numlist1, depth);
	else if (numlist1 > 0) {
		if (w1 > hmin && h1 > vmin) {
			AddDisplayFolder(folder, list1[0],
				depth, x1, y1, w1, h1,
				(folder->children[list1[0]] != NULL));
			if (folder->children[list1[0]] != NULL) {
				BuildFolderLayout(x1 + B, y1 + B + T, w1 - B * 2, h1 - B * 2 - T,
					folder->children[list1[0]], depth+1);
			}
		}
		else AddDisplayFolder(folder, (ui32)-1, depth, x1, y1, w1, h1, 0);
	}
	if (numlist2 > 1 && w2 > hmin && h2 > vmin)
		SizeFolders(x2, y2, w2, h2, folder, list2, numlist2, depth);
	else if (numlist2 > 0) {
		if (w2 > hmin && h2 > vmin) {
			AddDisplayFolder(folder, list2[0],
				depth, x2, y2, w2, h2,
				(folder->children[list2[0]] != NULL));
			if (folder->children[list2[0]] != NULL) {
				BuildFolderLayout(x2 + B, y2 + B + T, w2 - B * 2, h2 - B * 2 - T,
					folder->children[list2[0]], depth+1);
			}
		}
		else AddDisplayFolder(folder, (ui32)-1, depth, x2, y2, w2, h2, 0);
	}

	delete[] list1;
	delete[] list2;
}

void CFolderView::ZoomIn(CDisplayFolder *folder)
{
	CFolderTree *doc = (CFolderTree *)GetDocument();
	CFolder *oldroot = rootfolder;

	if (folder == NULL) return;
	if (folder->source->children[folder->index] != NULL)
		rootfolder = folder->source->children[folder->index];

	if (rootfolder == oldroot) return;

	CRect start(folder->x, folder->y,
		folder->x+folder->w, folder->y+folder->h);
	CRect end;
	GetClientRect(&end);
	end.OffsetRect(-end.left, -end.top);
	if (theApp.m_settings.animated_zoom) AnimateBox(start, end);

	CFolder *parent = rootfolder->parent;
	zoomlevel = 0;
	while (parent != NULL) zoomlevel++, parent = parent->parent;
	OnUpdate(doc);
	UpdateTitleBar();
}

void CFolderView::ZoomOut(void)
{
	CFolderTree *doc = (CFolderTree *)GetDocument();
	CFolder *oldroot = rootfolder;

	if (rootfolder != NULL && rootfolder->parent != NULL)
		rootfolder = rootfolder->parent;

	if (rootfolder == oldroot) return;

	CRect start;
	GetClientRect(&start);
	start.OffsetRect(-start.left, -start.top);
	CRect end;
	end.left = end.right = start.right / 2;
	end.top = end.bottom = start.bottom / 2;
	if (theApp.m_settings.animated_zoom) AnimateBox(start, end);

	zoomlevel--;
	OnUpdate(doc);
	UpdateTitleBar();
}

void CFolderView::ZoomFull(void)
{
	CFolderTree *doc = (CFolderTree *)GetDocument();
	CFolder *oldroot = rootfolder;

	if (doc != NULL)
		rootfolder = ((CFolderTree *)doc)->GetRoot();
	else rootfolder = NULL;

	if (rootfolder == oldroot) return;

	CRect start;
	GetClientRect(&start);
	start.OffsetRect(-start.left, -start.top);
	CRect end;
	end.left = end.right = start.right / 2;
	end.top = end.bottom = start.bottom / 2;
	if (theApp.m_settings.animated_zoom) AnimateBox(start, end);

	zoomlevel = 0;
	OnUpdate(doc);
	UpdateTitleBar();
}

void CFolderView::ShowFreeSpace(BOOL shown)
{
	if (showfreespace == shown) return;
	showfreespace = shown;
	OnUpdate(GetDocument());
}

static void ComputeNewRect(CRect &result, const CRect &start, const CRect &end, ui32 max, ui32 step)
{
	if (max <= 0) max = 1;
	result.left = ((start.left * (max - step)) + (end.left * step)) / max;
	result.top = ((start.top * (max - step)) + (end.top * step)) / max;
	result.right = ((start.right * (max - step)) + (end.right * step)) / max;
	result.bottom = ((start.bottom * (max - step)) + (end.bottom * step)) / max;
}

static void MinimalRect(CDC *dc, const CRect &rect)
{
	dc->MoveTo(rect.left, rect.top);
	dc->LineTo(rect.right, rect.top);
	dc->LineTo(rect.right, rect.bottom);
	dc->LineTo(rect.left, rect.bottom);
	dc->LineTo(rect.left, rect.top);
}

void CFolderView::AnimateBox(const CRect &start, const CRect &end)
{
	// Total time of animation: 400 milliseconds + render time, or
	// about a half a second.

	ui32 step;
	CRect rect = start;
	CDC *dc = GetDC();
	CPen pen(PS_SOLID, Px(1), RGB(0, 0, 0));
	CPen *oldpen = dc->SelectObject(&pen);
	dc->SetROP2(R2_NOT);
	for (step = 0; step < 8; step++) {
		ComputeNewRect(rect, start, end, 8, step);
		MinimalRect(dc, rect);
		GdiFlush();
		Sleep(25);
	}
	for (step = 0; step < 8; step++) {
		ComputeNewRect(rect, start, end, 8, step);
		MinimalRect(dc, rect);
		GdiFlush();
		Sleep(25);
	}
	dc->SetROP2(R2_COPYPEN);
	dc->SelectObject(oldpen);
	ReleaseDC(dc);
}

void CFolderView::OnIgnoreUpdate(CCmdUI *ui)
{
	GeneralIgnoreUpdate(ui);
}
