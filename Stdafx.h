// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxmt.h>          // MFC multithreading extensions
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <shellapi.h>		// Shell (Explorer) API
#include <shlobj.h>			// Shell object definitions
#include <shobjidl.h>		// IFileOperation, IFileOpenDialog
#include <shlwapi.h>		// Path helpers
#include <dwmapi.h>			// Dark title bars
#include <uxtheme.h>		// Dark control themes

#include <atomic>
#include <mutex>
#include <vector>
#include <unordered_set>

#include "xapp.h"			// Extended MFC utils
#include "e.h"				// Enhanced portable types
