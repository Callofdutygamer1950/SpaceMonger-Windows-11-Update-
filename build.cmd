@echo off
rem Builds SpaceMonger (64-bit) with the Visual Studio 2026 C++ compiler and MFC.
rem Usage: build [Release|Debug]
rem
rem This calls the compiler directly, so it only needs the "MSVC" and "C++ MFC"
rem components.  To work in the Visual Studio IDE instead, open SpaceMonger.sln
rem (that needs the "Desktop development with C++" workload).
setlocal enabledelayedexpansion
set CONFIG=%~1
if "%CONFIG%"=="" set CONFIG=Release
cd /d "%~dp0"

rem Find the newest MSVC that has MFC installed, and the newest Windows SDK.
for /f "usebackq delims=" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -prerelease -products * -property installationPath`) do set VSDIR=%%i
if not defined VSDIR goto :notools
for /d %%v in ("%VSDIR%\VC\Tools\MSVC\*") do if exist "%%v\atlmfc\lib\x64\uafxcw.lib" set MSVC=%%v
if not defined MSVC goto :notools
set SDKROOT=%ProgramFiles(x86)%\Windows Kits\10
for /d %%v in ("%SDKROOT%\Lib\10.*") do if exist "%%v\um\x64\kernel32.lib" set SDKVER=%%~nxv
if not defined SDKVER goto :notools

set INCLUDE=%MSVC%\include;%MSVC%\atlmfc\include;%SDKROOT%\Include\%SDKVER%\ucrt;%SDKROOT%\Include\%SDKVER%\um;%SDKROOT%\Include\%SDKVER%\shared;%SDKROOT%\Include\%SDKVER%\winrt
set LIB=%MSVC%\lib\x64;%MSVC%\atlmfc\lib\x64;%SDKROOT%\Lib\%SDKVER%\ucrt\x64;%SDKROOT%\Lib\%SDKVER%\um\x64
set PATH=%MSVC%\bin\Hostx64\x64;%SDKROOT%\bin\%SDKVER%\x64;%PATH%

set OBJ=obj\x64\%CONFIG%
set BIN=bin\x64\%CONFIG%
if not exist %OBJ% mkdir %OBJ%
if not exist %BIN% mkdir %BIN%

set DEFS=/DUNICODE /D_UNICODE /DWINVER=0x0A00 /D_WIN32_WINNT=0x0A00 /D_CRT_SECURE_NO_WARNINGS
set CFLAGS=/nologo /c /EHsc /W3 /std:c++17 /utf-8 /permissive- /Zc:wchar_t /Zi /Fd%OBJ%\vc.pdb /Fo%OBJ%\ %DEFS%
if /i "%CONFIG%"=="Debug" (
  set CFLAGS=!CFLAGS! /Od /MTd /D_DEBUG /RTC1
) else (
  set CFLAGS=!CFLAGS! /O2 /MT /DNDEBUG /GL
)
set PCH=/YuStdAfx.h /Fp%OBJ%\SpaceMonger.pch

echo Compiling...
cl %CFLAGS% /YcStdAfx.h /Fp%OBJ%\SpaceMonger.pch StdAfx.cpp || exit /b 1
cl %CFLAGS% %PCH% AboutDialog.cpp DriveDialog.cpp FolderTree.cpp FolderView.cpp Freedoc.cpp Freeview.cpp lang_en.cpp lang_fr.cpp Mainfrm.cpp SetupDlg.cpp SpaceMonger.cpp Theme.cpp TipWnd.cpp FileTypes.cpp LargestDialog.cpp || exit /b 1
rc /nologo %DEFS% /fo %OBJ%\SpaceMonger.res SpaceMonger.rc || exit /b 1

echo Linking...
set LFLAGS=/nologo /SUBSYSTEM:WINDOWS /ENTRY:wWinMainCRTStartup /DEBUG /MANIFEST:EMBED /MANIFESTINPUT:SpaceMonger.manifest /MAP:%OBJ%\SpaceMonger.map
if /i not "%CONFIG%"=="Debug" set LFLAGS=%LFLAGS% /LTCG /OPT:REF /OPT:ICF
link %LFLAGS% /OUT:%BIN%\SpaceMonger.exe %OBJ%\*.obj %OBJ%\SpaceMonger.res shlwapi.lib dwmapi.lib uxtheme.lib comctl32.lib msimg32.lib || exit /b 1

rem The MIT License asks that its notice go along with every copy.
copy /y LICENSE.txt %BIN%\LICENSE.txt >nul

echo Built %BIN%\SpaceMonger.exe
exit /b 0

:notools
echo Could not find Visual Studio's C++ compiler with MFC, or the Windows SDK.
echo Install Visual Studio 2026 with "MSVC" and "C++ MFC for x64/x86 (Latest MSVC)".
exit /b 1
