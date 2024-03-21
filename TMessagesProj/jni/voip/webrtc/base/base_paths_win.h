// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_BASE_PATHS_WIN_H_
#define BASE_BASE_PATHS_WIN_H_

// These can be used with the PathService to access various special
// directories and files.

namespace base {

enum {
  PATH_WIN_START = 100,

  DIR_WINDOWS,  // Windows directory, usually "c:\windows"
  DIR_SYSTEM,   // Usually c:\windows\system32"





  DIR_PROGRAM_FILES,      // See table above.
  DIR_PROGRAM_FILESX86,   // See table above.
  DIR_PROGRAM_FILES6432,  // See table above.

  DIR_IE_INTERNET_CACHE,       // Temporary Internet Files directory.
  DIR_COMMON_START_MENU,       // Usually "C:\ProgramData\Microsoft\Windows\

  DIR_START_MENU,              // Usually "C:\Users\<user>\AppData\Roaming\

  DIR_APP_DATA,                // Application Data directory under the user

  DIR_LOCAL_APP_DATA,          // "Local Settings\Application Data" directory

  DIR_COMMON_APP_DATA,         // Usually "C:\ProgramData".
  DIR_APP_SHORTCUTS,           // Where tiles on the start screen are stored,


  DIR_COMMON_DESKTOP,          // Directory for the common desktop (visible

  DIR_USER_QUICK_LAUNCH,       // Directory for the quick launch shortcuts.
  DIR_TASKBAR_PINS,            // Directory for the shortcuts pinned to taskbar.
  DIR_IMPLICIT_APP_SHORTCUTS,  // The implicit user pinned shortcut directory.
  DIR_WINDOWS_FONTS,           // Usually C:\Windows\Fonts.

  PATH_WIN_END
};

}  // namespace base

#endif  // BASE_BASE_PATHS_WIN_H_
