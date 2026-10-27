
#include "stdafx.h"
#include "SpaceMonger.h"
#include "lang.h"

LangEntry Langs[] = {
	{ L"us", L"English (US)", &LangUs },
	{ L"uk", L"English (UK/Aust.)", &LangUk },
	{ L"fr", L"Français (Nationale)", &LangFr },
	{ NULL, NULL, NULL },
};

Lang *CurLang = &LangUs;

/////////////////////////////////////////////////////////////////////////////

static const wchar_t *densitynames[] = {
	L"Too Few Files", L"Very Few Files", L"Normal", L"Lots of Files",
	L"Very Many Files", L"Too Many Files",
};

static const wchar_t *uscolornames[] = {
	L"Rainbow", L"Windows Colors", L"White", L"Light Gray", L"Dark Gray",
	L"Red", L"Orange", L"Yellow", L"Green", L"Aqua", L"Blue", L"Violet",
	L"By File Type",
};

static const wchar_t *monthnames[] = {
	L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun",
	L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec",
};

static const wchar_t *attribnames[] = {
	L"Arch", L"Compress", L"Folder", L"Encrypt", L"Hidden", L"Offline",
	L"Read-Only", L"Reparse-Pt", L"Sparse", L"System", L"Temp",
};

static const wchar_t *toolbarnames[] = {
	L"Open", L"Reload", L"Zoom Full", L"Zoom In", L"Zoom Out",
	L"Free Space", L"Run / Open", L"Recycle", L"Setup", L"About", L"Largest",
};

static const wchar_t *themenames[] = {
	L"Use Windows setting", L"Light", L"Dark",
};

static const wchar_t *typenames[] = {
	L"Videos", L"Pictures", L"Music", L"Documents",
	L"Archives and disk images", L"Programs and games", L"System", L"Other",
};

static const LangExtra ExtraEn = {
	L"Largest Files",
	L"The 50 largest files in %s. Online-only cloud files aren't listed, since they don't use space on this disk.",
	L"These files are your exceptions. They're left out of the largest-files list.",
	L"Name", L"Size", L"Folder", L"Modified",
	L"&Go To", L"&Add to Exceptions", L"&Remove from Exceptions", L"Show e&xceptions instead", L"Close",
	L"No files to show.",
	L"<Space the scan couldn't see: %d.%d%%>",
	L"Protected system areas and files SpaceMonger isn't allowed to read",
	L"Rescan as administrator to see more",
	L"Rescan as &Administrator",
	L"E&xport to Spreadsheet...",
	L"Export to Spreadsheet",
	L"This will save %s rows. Excel can only open the first 1,048,576.\n\nSave anyway?",
	L"SpaceMonger couldn't save the file.",
	L"Saved %s rows to:\n%s",
	L"Search (name or .ext)",
	L"%s matches, %s",
	L"%s match, %s",
	L"No matches",
	typenames,
	L"SpaceMonger couldn't open this folder. It may not exist, or you may not have permission to read it.\n\n%s",
};

Lang LangUs = {
	IDR_TOOLBAR_EN,
	L"About SpaceMonger",
	L"THIS PROGRAM IS FREE, OPEN-SOURCE SOFTWARE.",
	L"You may share and change it under the GNU General Public License,"
		L" version 3 (see LICENSE.txt).  Versions you share must stay open"
		L" source under the same license.  This program comes with"
		L" NO WARRANTY of any kind, expressed or implied.",
	L"Have questions?  Found a bug?  Want a feature?\n"
		L"E-mail to <JoeysItSupport@gmail.com>.",
	L"OK", L"Cancel",
	L"bytes", L"KB", L"MB", L"GB",
	L"<Free Space: %d.%d%%>",
	L"Zoom &In", L"Zoom &Out", L"Zoom &Full", L"&Run / Open", L"Move to Recycle &Bin",
	L"&Open Drive...", L"&Rescan Drive", L"&Show Free Space",
	L"%lu.%01lu%%", L"%lu.%01lu %s",
	L"Total", L"Free", L"Files Total:", L"Folders Total:", L"Files Found:", L"Folders Found:",
	L'.', L',',
	densitynames, uscolornames, monthnames, attribnames,
	L"Deleting...",
	L"Select Drive to View",
	L"Scanning Disk...",
	L"SpaceMonger Settings",
	L"File Layout", L"Density:", L"Bias:", L"Horz", L"Equal", L"Vert",
	L"Display Colors", L"Files", L"Folders",
	L"ToolTips", L"Show file-name-tips", L"Show file-info-tips", L"Delay:", L"msec",
	L"Full Path", L"Filename", L"Small Icon", L"Icon", L"Date / Time", L"File Size", L"Attributes",
	L"Miscellaneous Options",
	L"Auto Rescan after Recycling", L"Disable \"Recycle\" Command",
	L"Animated Zoom In / Zoom Out", L"Remember Window Position",
	L"Show Rollover Boxes",
	L"Properties...",
	toolbarnames,
	L"TB",
	L"Show in File &Explorer",
	L"Open &Folder...",
	L"&Folder...",
	L"Choose a Folder to View",
	L"Move this to the Recycle Bin?\n\n%s",
	L"Windows could not move this to the Recycle Bin.",
	L"Scanning",
	L"Skip files that are only stored in the cloud (OneDrive, etc.)",
	L"Theme:",
	themenames,
	L"Scanning Folder...",
	&ExtraEn,
};

static const wchar_t *ukcolornames[] = {
	L"Rainbow", L"Windows Colours", L"White", L"Light Gray", L"Dark Gray",
	L"Red", L"Orange", L"Yellow", L"Green", L"Aqua", L"Blue", L"Violet",
	L"By File Type",
};

Lang LangUk = {
	IDR_TOOLBAR_EN,
	L"About SpaceMonger",
	L"THIS PROGRAM IS FREE, OPEN-SOURCE SOFTWARE.",
	L"You may share and change it under the GNU General Public License,"
		L" version 3 (see LICENSE.txt).  Versions you share must stay open"
		L" source under the same license.  This program comes with"
		L" NO WARRANTY of any kind, expressed or implied.",
	L"Have questions?  Found a bug?  Want a feature?\n"
		L"E-mail to <JoeysItSupport@gmail.com>.",
	L"OK", L"Cancel",
	L"bytes", L"KB", L"MB", L"GB",
	L"<Free Space: %d.%d%%>",
	L"Zoom &In", L"Zoom &Out", L"Zoom &Full", L"&Run / Open", L"Move to Recycle &Bin",
	L"&Open Drive...", L"&Rescan Drive", L"&Show Free Space",
	L"%lu.%01lu%%", L"%lu.%01lu %s",
	L"Total", L"Free", L"Files Total:", L"Folders Total:", L"Files Found:", L"Folders Found:",
	L'.', L',',
	densitynames, ukcolornames, monthnames, attribnames,
	L"Deleting...",
	L"Select Drive to View",
	L"Scanning Disk...",
	L"SpaceMonger Settings",
	L"File Layout", L"Density:", L"Bias:", L"Horz", L"Equal", L"Vert",
	L"Display Colours", L"Files", L"Folders",
	L"ToolTips", L"Show file-name-tips", L"Show file-info-tips", L"Delay:", L"msec",
	L"Full Path", L"Filename", L"Small Icon", L"Icon", L"Date / Time", L"File Size", L"Attributes",
	L"Miscellaneous Options",
	L"Auto Rescan after Recycling", L"Disable \"Recycle\" Command",
	L"Animated Zoom In / Zoom Out", L"Remember Window Position",
	L"Show Rollover Boxes",
	L"Properties...",
	toolbarnames,
	L"TB",
	L"Show in File &Explorer",
	L"Open &Folder...",
	L"&Folder...",
	L"Choose a Folder to View",
	L"Move this to the Recycle Bin?\n\n%s",
	L"Windows could not move this to the Recycle Bin.",
	L"Scanning",
	L"Skip files that are only stored in the cloud (OneDrive, etc.)",
	L"Theme:",
	themenames,
	L"Scanning Folder...",
	&ExtraEn,
};

