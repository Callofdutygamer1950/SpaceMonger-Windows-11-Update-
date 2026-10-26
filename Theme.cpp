
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

static SetPreferredAppModeFunc pSetPreferredAppMode = NULL;
static FlushMenuThemesFunc pFlushMenuThemes = NULL;

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
