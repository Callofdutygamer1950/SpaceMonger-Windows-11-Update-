
#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////////
//  Light/dark theme support.
//
//  Windows has no official dark mode for classic Win32/MFC controls, so we
//  do what most Win32 apps do: dark title bars via DWM, the "DarkMode_*"
//  control themes that Explorer uses, WM_CTLCOLOR* for dialog backgrounds,
//  and uxtheme's SetPreferredAppMode() so that popup menus turn dark too.
//  Everything is looked up at runtime and fails gracefully if missing.

static BOOL g_dark = 0;
static HBRUSH g_brushes[COLOR_MENUBAR + 1];

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

enum PreferredAppMode { AppModeDefault, AppModeAllowDark, AppModeForceDark, AppModeForceLight };
typedef PreferredAppMode (WINAPI *SetPreferredAppModeFunc)(PreferredAppMode mode);
typedef void (WINAPI *FlushMenuThemesFunc)(void);
typedef BOOL (WINAPI *AllowDarkModeForWindowFunc)(HWND hwnd, BOOL allow);

static SetPreferredAppModeFunc pSetPreferredAppMode = NULL;
static FlushMenuThemesFunc pFlushMenuThemes = NULL;
static AllowDarkModeForWindowFunc pAllowDarkModeForWindow = NULL;

static BOOL SystemUsesDarkApps(void)
{
	DWORD value = 1, size = sizeof(value);
	if (RegGetValue(HKEY_CURRENT_USER,
			L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
			L"AppsUseLightTheme", RRF_RT_REG_DWORD, NULL, &value, &size) != ERROR_SUCCESS)
		return 0;
	return value == 0;
}

void Theme_Init(void)
{
	HMODULE uxtheme = LoadLibraryEx(L"uxtheme.dll", NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
	if (uxtheme != NULL) {
		pSetPreferredAppMode = (SetPreferredAppModeFunc)GetProcAddress(uxtheme, MAKEINTRESOURCEA(135));
		pFlushMenuThemes = (FlushMenuThemesFunc)GetProcAddress(uxtheme, MAKEINTRESOURCEA(136));
		pAllowDarkModeForWindow = (AllowDarkModeForWindowFunc)GetProcAddress(uxtheme, MAKEINTRESOURCEA(133));
	}
}

void Theme_Update(int setting)
{
	if (setting == THEME_DARK) g_dark = 1;
	else if (setting == THEME_LIGHT) g_dark = 0;
	else g_dark = SystemUsesDarkApps();

	for (int i = 0; i <= COLOR_MENUBAR; i++) {
		if (g_brushes[i] != NULL) DeleteObject(g_brushes[i]);
		g_brushes[i] = NULL;
	}

	if (pSetPreferredAppMode != NULL)
		pSetPreferredAppMode(g_dark ? AppModeForceDark : AppModeForceLight);
	if (pFlushMenuThemes != NULL)
		pFlushMenuThemes();
}

BOOL Theme_IsDark(void)
{
	return g_dark;
}

COLORREF Theme_Color(int syscolor)
{
	if (!g_dark) return GetSysColor(syscolor);

	switch (syscolor) {
	case COLOR_3DFACE:			return RGB(43, 43, 43);
	case COLOR_3DHIGHLIGHT:		return RGB(72, 72, 72);
	case COLOR_3DLIGHT:			return RGB(60, 60, 60);
	case COLOR_3DSHADOW:		return RGB(24, 24, 24);
	case COLOR_3DDKSHADOW:		return RGB(12, 12, 12);
	case COLOR_WINDOW:			return RGB(32, 32, 32);
	case COLOR_APPWORKSPACE:	return RGB(32, 32, 32);
	case COLOR_WINDOWTEXT:
	case COLOR_BTNTEXT:
	case COLOR_CAPTIONTEXT:
	case COLOR_MENUTEXT:		return RGB(240, 240, 240);
	case COLOR_INFOBK:			return RGB(48, 48, 48);
	case COLOR_INFOTEXT:		return RGB(240, 240, 240);
	case COLOR_GRAYTEXT:		return RGB(130, 130, 130);
	default:					return GetSysColor(syscolor);
	}
}

HBRUSH Theme_Brush(int syscolor)
{
	if (syscolor < 0 || syscolor > COLOR_MENUBAR)
		return GetSysColorBrush(syscolor);
	if (!g_dark) return GetSysColorBrush(syscolor);
	if (g_brushes[syscolor] == NULL)
		g_brushes[syscolor] = CreateSolidBrush(Theme_Color(syscolor));
	return g_brushes[syscolor];
}

// A list's column headers report to the list, not the dialog, so the list
// is subclassed, and in dark mode it draws the header itself.
static LRESULT CALLBACK ListViewSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam,
	UINT_PTR, DWORD_PTR)
{
	if (msg == WM_NOTIFY && g_dark) {
		NMHDR *nm = (NMHDR *)lParam;
		HWND header = ListView_GetHeader(hwnd);
		if (nm->code == NM_CUSTOMDRAW && nm->hwndFrom == header) {
			NMCUSTOMDRAW *cd = (NMCUSTOMDRAW *)lParam;
			COLORREF back = RGB(0x2B, 0x2B, 0x2B), line = RGB(0x45, 0x45, 0x45);
			if (cd->dwDrawStage == CDDS_PREPAINT)
				return CDRF_NOTIFYITEMDRAW|CDRF_NOTIFYPOSTPAINT;
			if (cd->dwDrawStage == CDDS_ITEMPREPAINT) {
				RECT r = cd->rc;
				SetBkColor(cd->hdc, back);
				ExtTextOut(cd->hdc, 0, 0, ETO_OPAQUE, &r, NULL, 0, NULL);
				RECT sep = { r.right - 1, r.top + 4, r.right, r.bottom - 4 };
				SetBkColor(cd->hdc, line);
				ExtTextOut(cd->hdc, 0, 0, ETO_OPAQUE, &sep, NULL, 0, NULL);

				wchar_t text[128] = L"";
				HDITEM item;
				item.mask = HDI_TEXT|HDI_FORMAT;
				item.pszText = text;
				item.cchTextMax = 128;
				Header_GetItem(header, (int)cd->dwItemSpec, &item);
				int pad = MulDiv(6, Dpi_ForWindow(hwnd), 96);
				r.left += pad, r.right -= pad;
				SetBkMode(cd->hdc, TRANSPARENT);
				SetTextColor(cd->hdc, Theme_Color(COLOR_WINDOWTEXT));
				HGDIOBJ oldfont = SelectObject(cd->hdc, (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0));
				UINT align = (item.fmt & HDF_RIGHT) ? DT_RIGHT : (item.fmt & HDF_CENTER) ? DT_CENTER : DT_LEFT;
				DrawText(cd->hdc, text, -1, &r, align|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS|DT_NOPREFIX);
				SelectObject(cd->hdc, oldfont);
				return CDRF_SKIPDEFAULT;
			}
			if (cd->dwDrawStage == CDDS_POSTPAINT) {
				// The empty part of the header, past the last column.
				RECT client, last = { 0 };
				GetClientRect(header, &client);
				int count = Header_GetItemCount(header);
				if (count > 0) Header_GetItemRect(header, Header_OrderToIndex(header, count - 1), &last);
				client.left = last.right;
				SetBkColor(cd->hdc, back);
				ExtTextOut(cd->hdc, 0, 0, ETO_OPAQUE, &client, NULL, 0, NULL);
				return CDRF_DODEFAULT;
			}
		}
	}
	return DefSubclassProc(hwnd, msg, wParam, lParam);
}

static BOOL CALLBACK ThemeChildProc(HWND hwnd, LPARAM)
{
	wchar_t cls[64];
	if (!GetClassName(hwnd, cls, 64)) return TRUE;

	const wchar_t *theme = NULL;		// NULL = restore the default theme
	BOOL classic = 0;					// Turn off visual styles entirely

	if (!_wcsicmp(cls, L"Button")) {
		LONG type = GetWindowLong(hwnd, GWL_STYLE) & BS_TYPEMASK;
		if (type == BS_PUSHBUTTON || type == BS_DEFPUSHBUTTON) {
			if (g_dark) theme = L"DarkMode_Explorer";
		}
		// Themed check boxes and group boxes always draw black text, so in
		// dark mode they're drawn unthemed, which honors WM_CTLCOLORSTATIC.
		else if (g_dark) classic = 1;
	}
	else if (!_wcsicmp(cls, L"ComboBox") || !_wcsicmp(cls, L"Edit")) {
		if (g_dark) theme = L"DarkMode_CFD";
	}
	else if (!_wcsicmp(cls, WC_LISTVIEW)) {
		if (g_dark) theme = L"DarkMode_Explorer";
		ListView_SetBkColor(hwnd, Theme_Color(COLOR_WINDOW));
		ListView_SetTextBkColor(hwnd, Theme_Color(COLOR_WINDOW));
		ListView_SetTextColor(hwnd, Theme_Color(COLOR_WINDOWTEXT));
		HWND header = ListView_GetHeader(hwnd);
		if (header != NULL) {
			if (pAllowDarkModeForWindow != NULL) {
				pAllowDarkModeForWindow(hwnd, g_dark);
				pAllowDarkModeForWindow(header, g_dark);
			}
			SetWindowTheme(header, g_dark ? L"DarkMode_ItemsView" : NULL, NULL);
			SendMessage(header, WM_THEMECHANGED, 0, 0);
			SetWindowSubclass(hwnd, ListViewSubclassProc, 1, 0);
		}
	}
	else if (!_wcsicmp(cls, L"ScrollBar")) {
		if (g_dark) theme = L"DarkMode_Explorer";
	}

	if (classic) SetWindowTheme(hwnd, L"", L"");
	else SetWindowTheme(hwnd, theme, NULL);
	return TRUE;
}

void Theme_ApplyToWindow(HWND hwnd)
{
	if (!IsWindow(hwnd)) return;

	BOOL dark = g_dark;
	DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));

	EnumChildWindows(hwnd, ThemeChildProc, 0);
	RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE|RDW_ERASE|RDW_FRAME|RDW_ALLCHILDREN);
}

HBRUSH Theme_OnCtlColor(HDC dc, HWND ctl, UINT ctltype)
{
	if (!g_dark) return NULL;

	if (ctltype == CTLCOLOR_EDIT || ctltype == CTLCOLOR_LISTBOX) {
		SetTextColor(dc, Theme_Color(COLOR_WINDOWTEXT));
		SetBkColor(dc, Theme_Color(COLOR_3DLIGHT));
		return Theme_Brush(COLOR_3DLIGHT);
	}

	SetTextColor(dc, IsWindowEnabled(ctl) ? Theme_Color(COLOR_WINDOWTEXT)
		: Theme_Color(COLOR_GRAYTEXT));
	SetBkColor(dc, Theme_Color(COLOR_WINDOW));
	return Theme_Brush(COLOR_WINDOW);
}

//////////////////////////////////////////////////////////////////////////////
//  DPI helpers

int Dpi_ForWindow(HWND hwnd)
{
	UINT dpi = (hwnd != NULL) ? GetDpiForWindow(hwnd) : 0;
	if (dpi == 0) dpi = GetDpiForSystem();
	return (int)dpi;
}

HFONT Dpi_CreateUIFont(int dpi, int points, int weight)
{
	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(ncm);
	LOGFONT lf;
	if (SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0, dpi))
		lf = ncm.lfMessageFont;
	else {
		memset(&lf, 0, sizeof(lf));
		wcscpy_s(lf.lfFaceName, L"Segoe UI");
	}
	lf.lfHeight = -MulDiv(points, dpi, 72);
	lf.lfWidth = 0;
	lf.lfWeight = weight;
	lf.lfQuality = CLEARTYPE_QUALITY;
	return CreateFontIndirect(&lf);
}
