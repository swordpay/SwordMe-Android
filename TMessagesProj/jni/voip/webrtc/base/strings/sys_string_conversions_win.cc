// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/strings/sys_string_conversions.h"

#include <windows.h>
#include <stdint.h>

#include "base/strings/string_piece.h"

namespace base {

std::string SysWideToUTF8(const std::wstring& wide) {
  return SysWideToMultiByte(wide, CP_UTF8);
}

std::wstring SysUTF8ToWide(StringPiece utf8) {
  return SysMultiByteToWide(utf8, CP_UTF8);
}

std::string SysWideToNativeMB(const std::wstring& wide) {
  return SysWideToMultiByte(wide, CP_ACP);
}

std::wstring SysNativeMBToWide(StringPiece native_mb) {
  return SysMultiByteToWide(native_mb, CP_ACP);
}

std::wstring SysMultiByteToWide(StringPiece mb, uint32_t code_page) {
  if (mb.empty())
    return std::wstring();

  int mb_length = static_cast<int>(mb.length());

  int charcount = MultiByteToWideChar(code_page, 0,
                                      mb.data(), mb_length, NULL, 0);
  if (charcount == 0)
    return std::wstring();

  std::wstring wide;
  wide.resize(charcount);
  MultiByteToWideChar(code_page, 0, mb.data(), mb_length, &wide[0], charcount);

  return wide;
}

std::string SysWideToMultiByte(const std::wstring& wide, uint32_t code_page) {
  int wide_length = static_cast<int>(wide.length());
  if (wide_length == 0)
    return std::string();

  int charcount = WideCharToMultiByte(code_page, 0, wide.data(), wide_length,
                                      NULL, 0, NULL, NULL);
  if (charcount == 0)
    return std::string();

  std::string mb;
  mb.resize(charcount);
  WideCharToMultiByte(code_page, 0, wide.data(), wide_length,
                      &mb[0], charcount, NULL, NULL);

  return mb;
}

}  // namespace base
