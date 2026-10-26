
#include "stdafx.h"
#include "FileTypes.h"

#include <string>
#include <unordered_map>

static const struct { int type; const wchar_t *extensions; } TypeTable[] = {
	{ FT_VIDEO,		L"mp4 mkv avi mov wmv webm m4v flv mpg mpeg m2ts mts ts vob 3gp ogv" },
	{ FT_PICTURE,	L"jpg jpeg png gif bmp tif tiff heic heif webp raw cr2 cr3 nef arw dng orf rw2 psd svg ico avif jxl" },
	{ FT_MUSIC,		L"mp3 wav flac aac m4a ogg opus wma aiff aif mid midi alac ape" },
	{ FT_DOCUMENT,	L"pdf doc docx xls xlsx xlsm ppt pptx txt rtf odt ods odp csv md epub one pages key numbers tex log json xml html htm" },
	{ FT_ARCHIVE,	L"zip rar 7z tar gz tgz bz2 xz zst cab iso img vhd vhdx vdi vmdk qcow2 wim esd dmg" },
	{ FT_PROGRAM,	L"exe dll msi msix appx appxbundle msixbundle ocx so jar apk lib pdb pak xpak ff vpk bsa ba2 uasset ucas utoc forge rpf bnk wem big arc" },
	{ FT_SYSTEM,	L"sys etl evtx regtrans-ms blf cat mum manifest" },
};

static const std::unordered_map<std::wstring, int> &ExtensionMap(void)
{
	static std::unordered_map<std::wstring, int> map = []() {
		std::unordered_map<std::wstring, int> m;
		for (const auto &entry : TypeTable) {
			const wchar_t *p = entry.extensions;
			while (*p != L'\0') {
				const wchar_t *end = p;
				while (*end != L'\0' && *end != L' ') end++;
				m[std::wstring(p, end - p)] = entry.type;
				p = (*end == L' ') ? end + 1 : end;
			}
		}
		return m;
	}();
	return map;
}

int GetFileType(const wchar_t *name)
{
	if (name == NULL) return FT_OTHER;

	// Windows' own big files.
	if (!_wcsicmp(name, L"pagefile.sys") || !_wcsicmp(name, L"hiberfil.sys")
		|| !_wcsicmp(name, L"swapfile.sys"))
		return FT_SYSTEM;

	const wchar_t *dot = wcsrchr(name, L'.');
	if (dot == NULL || dot[1] == L'\0') return FT_OTHER;

	wchar_t ext[16];
	size_t i;
	for (i = 0; i < 15 && dot[i + 1] != L'\0'; i++)
		ext[i] = towlower(dot[i + 1]);
	if (dot[i + 1] != L'\0') return FT_OTHER;	// Too long to be one we know
	ext[i] = L'\0';

	const auto &map = ExtensionMap();
	auto found = map.find(ext);
	return (found != map.end()) ? found->second : FT_OTHER;
}

COLORREF GetFileTypeColor(int type, int shade)
{
	static const COLORREF colors[FT_COUNT] = {
		RGB(0xFF, 0x80, 0x80),		// Videos: red
		RGB(0x80, 0xE0, 0x80),		// Pictures: green
		RGB(0xD0, 0x98, 0xFF),		// Music: purple
		RGB(0x80, 0xB8, 0xFF),		// Documents: blue
		RGB(0xFF, 0xC0, 0x60),		// Archives: orange
		RGB(0xFF, 0xF0, 0x60),		// Programs and games: yellow
		RGB(0x98, 0x98, 0x98),		// System: gray
		RGB(0xD8, 0xD8, 0xD8),		// Other: light gray
	};
	if (type < 0 || type >= FT_COUNT) type = FT_OTHER;
	COLORREF c = colors[type];
	int r = GetRValue(c), g = GetGValue(c), b = GetBValue(c);
	if (shade == 1) r = (r + 255) / 2, g = (g + 255) / 2, b = (b + 255) / 2;
	else if (shade == 2) r = r * 3 / 4, g = g * 3 / 4, b = b * 3 / 4;
	return RGB(r, g, b);
}
