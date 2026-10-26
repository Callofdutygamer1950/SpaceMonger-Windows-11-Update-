
#ifndef FOLDERTREE_H
#define FOLDERTREE_H

class CFolderTree;
class CFolderDialog;
struct CFolder;
struct CScanState;

// Flags in CFolder::attribs
enum {
	ENTRY_CLOUD_ONLY = 1,	// Stored only in the cloud; uses no space on this disk
};

// Names of the two special blocks in the root folder of a whole-drive scan.
#define FREE_SPACE_NAME		L"<"
#define HIDDEN_SPACE_NAME	L"<?"
inline bool IsSpecialName(const wchar_t *name) { return name != NULL && name[0] == L'<'; }

struct CFolder {
public:
	CFolder();
	~CFolder();

	void AddFile(CFolderTree *tree, const wchar_t *name, ui32 namelen, ui64 size, ui64 actual_size,
		ui64 time, ui64 fileid = 0, ui8 attrib = 0);
	void AddFolder(CFolderTree *tree, const wchar_t *name, ui32 namelen, CFolder *folder, ui64 time);
	void Finalize(void);
	void DelFile(unsigned int index);
	void RenameFile(unsigned int index, const CString &name);
	unsigned int FindFile(const CString &name);
	inline ui64 SizeFiles() { return(size_self); }
	inline ui64 SizeSub() { return(size_children); }
	inline ui64 SizeTotal() { return(size_self + size_children); }

	void LoadFolder(CScanState *state, const CString &path);
	void CollectFileIds(std::vector<ui64> &ids);
	void RemoveDuplicateLinks(const std::vector<ui64> &dups, std::unordered_set<ui64> &seen);
	void ComputeSizes(void);

private:
	void MoreEntries(void);

public:
	CFolder *parent;
	unsigned int parentindex;

	wchar_t **names;
	CFolder **children;
	ui64 *sizes;			// Space used on disk
	ui64 *actualsizes;		// Logical file size
	ui64 *times;
	ui64 *fileids;			// NTFS file ID (for counting hard links once), or 0
	ui8 *attribs;			// ENTRY_* flags
	ui64 size_self, size_children;

	unsigned int cur, max;
};

// Everything the background scanning threads share.
struct CScanState {
	CScanState();

	ui64 clustersize;
	BOOL skipcloudonly;		// Leave out files that are only stored in the cloud
	BOOL hardlinks;			// Volume supports hard links, so count each file once

	std::atomic<ui64> numfiles, numfolders, filespace;
	std::atomic<ui64> cloudspace;	// Part of filespace that's cloud-only
	std::atomic<bool> cancelled, done;

	std::mutex pathlock;	// Protects curpath
	CString curpath;
	std::atomic<ui64> lastpathtick;

	void SetPath(const CString &path);
	CString GetPath(void);
};

class CFolderTree : public CFreeDoc {
public:
	CFolderTree();
	virtual ~CFolderTree();

	BOOL LoadTree(const CString &path, BOOL includespace, CWnd *modalwin);
	void GetSpace(const CString &path);
	CFolder *GetRoot(void);

	CFolder *SetCur(const CString &path);
	CFolder *GetCur(void);
	CFolder *Down(unsigned int index);
	CFolder *Up(void);

	BOOL IsVolumeRoot(void) const;

	// After a rescan, the old tree stays alive until the view has let go of
	// it (the view keeps pointers into it), then this frees it.
	void FreeOldTree(void);
	CString GetFullPath(CFolder *folder, unsigned int index) const;
	BOOL FindPath(const CString &path, CFolder *&folder, unsigned int &index) const;

protected:
	CFolder *root, *cur;
	CFolder *oldroot;		// The previous scan, until FreeOldTree()

public:
	CString m_path;
	ui64 freespace, usedspace, totalspace, clustersize;
	ui64 numfiles, numfolders;
	ui64 filespace;
	ui64 hiddenspace;		// Used space the scan couldn't account for
	BOOL hardlinks;
};

class CFolderDialog : public CDialog {
public:
	CFolderDialog();
	DECLARE_DYNCREATE(CFolderDialog)
	virtual ~CFolderDialog();

	virtual void Reset(BOOL wholedrive);
	virtual void UpdateDisplay(CFolderTree *tree, CScanState *state);
	virtual void ForcedUpdate(CFolderTree *tree, CScanState *state);

	virtual void OnCancel(void);

protected:
	void DrawAnimation(CDC *dc);

	//{{AFX_MSG(CFolderDialog)
	afx_msg HBRUSH OnCtlColor(CDC *pDC, CWnd *pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC *pDC);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	CScanState *state;

protected:
	BOOL wholedrive;
	ULONGLONG last_tick;
	ui32 cur_frame;
};

// Prefixes a path with \\?\ so that Windows accepts paths longer than 260
// characters, whether or not long paths are enabled system-wide.
CString MakeLongPath(const CString &path);

#endif
