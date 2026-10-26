
#ifndef LANG_H
#define LANG_H

// Text added after 1.5's first release lives here, so that the big
// positional Lang tables don't keep growing.
struct LangExtra {
	const wchar_t *largesttitle;	// "Largest Files" window title
	const wchar_t *largestintro;	// %s = folder being viewed
	const wchar_t *exceptionsintro;
	const wchar_t *colname, *colsize, *colfolder, *colmodified;
	const wchar_t *gotofile, *addexception, *removeexception, *showexceptions, *close;
	const wchar_t *nofiles;
	const wchar_t *hiddenformat;	// "<...: %d.%d%%>"
	const wchar_t *hiddennote;
	const wchar_t *hiddenadmin;
	const wchar_t *runasadmin;		// Context menu
	const wchar_t *exportcsv;		// Context menu
	const wchar_t *exporttitle;		// Save dialog title
	const wchar_t *exportmany;		// %s = row count
	const wchar_t *exportfailed;
	const wchar_t *exportdone;		// %s = row count, %s = file
	const wchar_t *searchcue;
	const wchar_t *searchmatches;	// %s = count, %s = size
	const wchar_t *searchmatch;		// Same, for exactly one match
	const wchar_t *searchnone;
	const wchar_t **typenames;		// File-type categories, see FileTypes.h
	const wchar_t *openfailed;		// %s = path
};

struct Lang {
	int toolbar_bitmap_id;
	const wchar_t *about_spacemonger;
	const wchar_t *freeware;
	const wchar_t *warranty;
	const wchar_t *email;
	const wchar_t *ok, *cancel;
	const wchar_t *bytes, *kb, *mb, *gb;
	const wchar_t *freespace_format;
	const wchar_t *zoomin, *zoomout, *zoomfull, *run, *del;
	const wchar_t *opendrive, *rescandrive, *showfreespace;
	const wchar_t *percent_format, *size_format;
	const wchar_t *total, *free, *files_total, *folders_total, *files_found, *folders_found;
	wchar_t digitpt, digitcomma;
	const wchar_t **densitynames;
	const wchar_t **colornames;
	const wchar_t **monthnames;
	const wchar_t **attribnames;
	const wchar_t *deleting;
	const wchar_t *selectdrive;
	const wchar_t *scanning;
	const wchar_t *settings;
	const wchar_t *layout, *density, *bias, *horz, *equal, *vert;
	const wchar_t *displaycolors, *files, *folders;
	const wchar_t *tooltips, *shownametips, *showinfotips, *delay, *msec;
	const wchar_t *fullpath, *filename, *smallicon, *icon, *datetime, *filesize, *attrib;
	const wchar_t *miscoptions;
	const wchar_t *autorescan, *disabledelete, *animatedzoom, *savepos;
	const wchar_t *showrolloverbox;
	const wchar_t *properties;

	// Added in 1.5 (Windows 11 update)
	const wchar_t **toolbarnames;	// Open, Reload, Zoom Full, Zoom In, Zoom Out,
									// Free Space, Run, Delete, Setup, About, Largest
	const wchar_t *tb;
	const wchar_t *showinexplorer;
	const wchar_t *openfolder;		// Context menu: open a folder to scan
	const wchar_t *browsefolder;	// Drive dialog button
	const wchar_t *choosefolder;	// Folder picker title
	const wchar_t *recycleconfirm;	// printf format, %s = path
	const wchar_t *recyclefailed;
	const wchar_t *scanoptions;
	const wchar_t *skipcloudonly;
	const wchar_t *theme;
	const wchar_t **themenames;		// System, Light, Dark
	const wchar_t *scanningfolder;
	const LangExtra *ex;
};

struct LangEntry {
	const wchar_t *abbrev;
	const wchar_t *name;
	Lang *text;
};

extern Lang *CurLang;		// Current language

extern Lang LangUs;			// English (US)
extern Lang LangUk;			// English (UK/Aust.)
extern Lang LangFr;			// Français (Nationale)

extern LangEntry Langs[];	// List of languages

#endif
