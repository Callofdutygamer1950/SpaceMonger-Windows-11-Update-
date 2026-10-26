
#ifndef FILETYPES_H
#define FILETYPES_H

// File-type categories, used by the "By File Type" color scheme, the
// legend, and the spreadsheet export.  The order matches
// CurLang->ex->typenames.
enum {
	FT_VIDEO = 0,
	FT_PICTURE,
	FT_MUSIC,
	FT_DOCUMENT,
	FT_ARCHIVE,
	FT_PROGRAM,
	FT_SYSTEM,
	FT_OTHER,
	FT_COUNT
};

// The color scheme index (after the 12 original ones) for "By File Type".
#define FILE_COLOR_BY_TYPE 12

int GetFileType(const wchar_t *name);

// shade: 0 = normal, 1 = bright (highlight), 2 = dark (shadow)
COLORREF GetFileTypeColor(int type, int shade);

#endif
