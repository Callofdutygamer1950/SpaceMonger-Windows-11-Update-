
SpaceMonger Version 1.5.0 (Windows 11 update)
---------------------------------------------


INTRODUCTION
------------

SpaceMonger is a free tool for keeping track of the disk space on your
computer.  Instead of viewing your disk as unhelpful little icons or
pie charts, you see a single picture that shows you at a glance how
much space every file and folder are using.

This is an updated version of the classic SpaceMonger 1.4 for modern
Windows.  The latest version, with its source code, is available at:

    https://github.com/Callofdutygamer1950/SpaceMonger-Windows-11-Update-


SYSTEM REQUIREMENTS
-------------------

   *  Windows 10 or Windows 11, 64-bit.
   *  Nothing else.  There's no installer, and no extra software (such as
      .NET or Visual C++ runtimes) is needed.


INSTALLATION
------------

1. Go to the Releases page of the GitHub project above and download
   SpaceMonger.exe from the latest release.

2. Put it in any folder you like.  SpaceMonger doesn't need to be
   installed; the .exe is the complete program.

3. Double-click it to run it.

The first time you run it, Windows may show "Windows protected your PC",
because the program isn't digitally signed.  Click "More info", then
"Run anyway".

To uninstall SpaceMonger, delete SpaceMonger.exe.  Its settings are kept
in the Windows Registry under HKEY_CURRENT_USER\Software\65\SpaceMonger;
you can delete that key too if you want to remove every trace.


FIRST-TIME INSTRUCTIONS
-----------------------

SpaceMonger is not a hard program to use.  In fact, you probably won't
need any instructions.  But for those of you who like to read, here are
some first-time instructions:

When you first open SpaceMonger, you will initially be presented with a
blank window.  Click "Open".  Click on a drive, like drive "C", and hit
"OK", or click "Folder..." to look at just one folder.  Wait patiently
while the program collects statistics on your drive.  My, don't you
have a lot of files!

SpaceMonger will show you the contents of your drive as little
rectangles.  The larger the rectangle, the more space something is taking
up.  Rectangles that have other rectangles in them are folders.
Rectangles with no other rectangles in them are files.  Colors are used
to show how deeply the files and folders are nested.

When you look at a whole drive, two special blocks appear:

   *  Free Space shows how much room is left on the drive.  Click
      "Free Space" to hide or show it.
   *  A striped block shows space the scan couldn't see, such as system
      restore points and other protected parts of Windows.  Right-click
      and choose "Rescan as Administrator" to see more of it.

You can get more information on a given folder by double-clicking on its
title, or by clicking it and then clicking "Zoom In".  The window will
change to show a larger, more detailed view of the folder and its files.
You can zoom in as many times as you want.  "Zoom Out" will take you back
out (up) by one folder.  "Zoom Full" will take you all the way back to
show the whole drive.

Click "Largest" to see a list of the 50 biggest files on the drive.
Click one to see it in the picture.  If there's a file you want to keep
and don't want to see in that list again, select it and click
"Add to Exceptions".

Type into the search box at the top right to find files by name, or by
type (for example ".mp4").  Matching files light up, and the bottom of
the window shows how many there are and how much space they use.

If you see a file or folder you want to get rid of, click it to select it
and hit "Recycle".  SpaceMonger asks first, then moves it to the Recycle
Bin.  If you want to see what it is first, click on it and hit
"Run / Open", which will try to run it if it is a program or open it if
it is a file.  Right-click it to show it in File Explorer instead.

DISCLAIMER: If you don't know what something is, DON'T DELETE IT.
   Deleting things at random is a good way to break your programs or
   Windows itself or worse.  'Nuff said.

If you click the "Setup" button, you can change how SpaceMonger looks and
works: colors (including coloring files by type), light or dark mode,
tooltips, and whether to skip files that are only stored in the cloud
(OneDrive, etc.).  You can't damage anything with the Setup dialog box,
so feel free to try various options until you like how SpaceMonger looks
and feels.

In addition, you can right-click on a folder or a file to bring up a
menu of options, including "Export to Spreadsheet", which saves the
current view as a CSV file you can open in Excel.

If you double-click on a folder, SpaceMonger will zoom in on it.  If you
double-click on a file (either a program or a document), SpaceMonger
will attempt to run it or open it, whichever is suitable.


DISTRIBUTION
------------

SpaceMonger is free, open-source software, and it is not crippled or
time-limited in any way.  You may share it and change it under the terms
of the GNU General Public License, version 3 (see LICENSE.txt).  If you
share a copy, or a version you changed, it must stay open source under
the same license, and LICENSE.txt and LICENSE-ORIGINAL-MIT.txt must go
along with it.

To build SpaceMonger yourself, install Visual Studio 2026 with the
"Desktop development with C++" workload and the "C++ MFC" component, then
run build.cmd, or open SpaceMonger.sln in Visual Studio.


WHAT'S NEW
----------

Between 1.4.0 and 1.5.0 (the Windows 11 update)
   * Runs natively on 64-bit Windows 10 and 11.
   * Sharp at any display scaling, and follows Windows' light or dark
     mode (or choose one in Setup).
   * Much faster scanning, using several threads at once.
   * Correct sizes: counts the space files really use on disk, counts
     hard-linked files once, skips junction points and symbolic links,
     and handles sparse and compressed files.
   * Supports file names in any language, and paths longer than 260
     characters.
   * Option to skip files that are only stored in the cloud (OneDrive,
     etc.), since they don't use space on your disk.
   * Shows space the scan couldn't see, with an option to rescan as
     administrator.
   * New "Largest Files" list, with exceptions.
   * Search for files by name or type.
   * Optional "By File Type" colors, with a legend.
   * Export the current view to a spreadsheet (CSV).
   * "Delete" now moves things to the Recycle Bin and asks first.
   * Scan any folder, not just whole drives.
   * Smooth zoom animation, and sizes shown in KB, MB, GB, and TB.

Between 1.3.5 and 1.4.0
   * Internals rewritten to support multiple foreign langauges.
     Currently foreign language support exists for British English,
     American English, and French (possibly with grammar errors).
   * Supports popup info-tips that can display the full filename, the
     icon, the date, size, and attributes of any file or folder.
   * Supports popup name-tips that prevent names from being obscured
     by not having enough space to display them.
   * Open dialog box now uses large icons.
   * Includes rollover boxes that highlight the current folder.
   * Files display their sizes and dates if there is enough room.
   * Option to save the current window position.
   * Fixed zero-size bug that caused divide-by-zero errors with some
     Audio CD disks.
   * Minor changes to color palette to handle rollover boxes.
   * Support for Windows Property dialog box.
   * Numerous other bug fixes.
   * Support for Windows NT junction points.

Between 1.3.4 and 1.3.5
   * Toolbar buttons gray out when you can't use them.
   * Setup option to disable the "Delete" button.
   * Fixed size-computation routine that was oversizing
      files on some systems.
   * Double-clicking on files runs or opens them; double-clicking
      on folders zooms in on them.
   * Scanning now includes a percent bar so you can tell how
      long it's taking.

Between 1.3.0 and 1.3.4
   * Major speed improvements using new memory manager subsystem.
      Runs twice as fast and uses half the memory of 1.3.0.
   * Better display of free space information.
   * Right-click opens a pop-up menu of options.
   * Double-click zooms in on a folder.
   * Few minor bug-fixes.
   * Replaced old "Scanning..." window with a better one.

Between 1.2.0 and 1.3.0
   * Added the Setup button with a whole bunch of new options.
   * Internal bug-fixes to scanning routine.

Between 1.0.2 and 1.2.0
   * Lots of stuff.  1.1.0 was never released.  New buttons,
      new look, better code.  Replaced most of what's under
      the hood.
   * 1.2.0 was released for private use only.

Between 1.0.0 and 1.0.2
   * Miscellaneous bug fixes.
   * Added "Zoom In" and "Zoom Out" buttons.
   * Added "Run or Open" and "Delete" buttons.


COPYRIGHT AND CONTACT INFORMATION
---------------------------------

SpaceMonger is Copyright (C) 1998, 1999, and 2000 by Sean Werkema.
The Windows 11 update is Copyright (C) 2026 Joey's IT Support.
Have questions?  Found a bug?  Want a feature?  Send an e-mail to
JoeysItSupport@gmail.com, or open an issue at the GitHub page above.
