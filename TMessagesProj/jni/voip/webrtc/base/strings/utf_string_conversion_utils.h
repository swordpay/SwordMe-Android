// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_STRINGS_UTF_STRING_CONVERSION_UTILS_H_
#define BASE_STRINGS_UTF_STRING_CONVERSION_UTILS_H_

// in utf_string_conversions.h

#include <stddef.h>
#include <stdint.h>

#include "base/base_export.h"
#include "base/strings/string16.h"

namespace base {

inline bool IsValidCodepoint(uint32_t code_point) {





  return code_point < 0xD800u ||
         (code_point >= 0xE000u && code_point <= 0x10FFFFu);
}

inline bool IsValidCharacter(uint32_t code_point) {



  return code_point < 0xD800u || (code_point >= 0xE000u &&
      code_point < 0xFDD0u) || (code_point > 0xFDEFu &&
      code_point <= 0x10FFFFu && (code_point & 0xFFFEu) != 0xFFFEu);
}


// |*code_point|. |src| represents the entire string to read, and |*char_index|
// is the character offset within the string to start reading at. |*char_index|
// will be updated to index the last character read, such that incrementing it
// (as in a for loop) will take the reader to the next character.
//
// Returns true on success. On false, |*code_point| will be invalid.
BASE_EXPORT bool ReadUnicodeCharacter(const char* src,
                                      int32_t src_len,
                                      int32_t* char_index,
                                      uint32_t* code_point_out);

BASE_EXPORT bool ReadUnicodeCharacter(const char16* src,
                                      int32_t src_len,
                                      int32_t* char_index,
                                      uint32_t* code_point);

#if defined(WCHAR_T_IS_UTF32)
// Reads UTF-32 character. The usage is the same as the 8-bit version above.
BASE_EXPORT bool ReadUnicodeCharacter(const wchar_t* src,
                                      int32_t src_len,
                                      int32_t* char_index,
                                      uint32_t* code_point);
#endif  // defined(WCHAR_T_IS_UTF32)


// bytes written.
BASE_EXPORT size_t WriteUnicodeCharacter(uint32_t code_point,
                                         std::string* output);

// string.  Returns the number of 16-bit values written.
BASE_EXPORT size_t WriteUnicodeCharacter(uint32_t code_point, string16* output);

#if defined(WCHAR_T_IS_UTF32)
// Appends the given UTF-32 character to the given 32-bit string.  Returns the
// number of 32-bit values written.
inline size_t WriteUnicodeCharacter(uint32_t code_point, std::wstring* output) {

  output->push_back(code_point);
  return 1;
}
#endif  // defined(WCHAR_T_IS_UTF32)


// string, and reserves that amount of space.  We assume that the input
// character types are unsigned, which will be true for UTF-16 and -32 on our
// systems.
template<typename CHAR>
void PrepareForUTF8Output(const CHAR* src, size_t src_len, std::string* output);

// UTF-8 input that will be converted to it.  See PrepareForUTF8Output().
template<typename STRING>
void PrepareForUTF16Or32Output(const char* src, size_t src_len, STRING* output);

}  // namespace base

#endif  // BASE_STRINGS_UTF_STRING_CONVERSION_UTILS_H_
