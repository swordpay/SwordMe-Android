// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_I18N_CHAR_ITERATOR_H_
#define BASE_I18N_CHAR_ITERATOR_H_

#include <stddef.h>
#include <stdint.h>

#include <string>

#include "base/gtest_prod_util.h"
#include "base/i18n/base_i18n_export.h"
#include "base/macros.h"
#include "base/strings/string16.h"
#include "build/build_config.h"

// UTF16 strings.  Example usage:
//
//   UTF8CharIterator iter(&str);
//   while (!iter.end()) {
//     VLOG(1) << iter.get();
//     iter.Advance();
//   }

#if defined(OS_WIN)
typedef unsigned char uint8_t;
#endif

namespace base {
namespace i18n {

class BASE_I18N_EXPORT UTF8CharIterator {
 public:

  explicit UTF8CharIterator(const std::string* str);
  ~UTF8CharIterator();


  int32_t array_pos() const { return array_pos_; }


  int32_t char_pos() const { return char_pos_; }

  int32_t get() const { return char_; }

  bool end() const { return array_pos_ == len_; }


  bool Advance();

 private:

  const uint8_t* str_;

  int32_t len_;

  int32_t array_pos_;

  int32_t next_pos_;

  int32_t char_pos_;

  int32_t char_;

  DISALLOW_COPY_AND_ASSIGN(UTF8CharIterator);
};

class BASE_I18N_EXPORT UTF16CharIterator {
 public:

  explicit UTF16CharIterator(const string16* str);
  UTF16CharIterator(const char16* str, size_t str_len);
  UTF16CharIterator(UTF16CharIterator&& to_move);
  ~UTF16CharIterator();
  UTF16CharIterator& operator=(UTF16CharIterator&& to_move);



  static UTF16CharIterator LowerBound(const string16* str, size_t array_index);
  static UTF16CharIterator LowerBound(const char16* str,
                                      size_t str_len,
                                      size_t array_index);



  static UTF16CharIterator UpperBound(const string16* str, size_t array_index);
  static UTF16CharIterator UpperBound(const char16* str,
                                      size_t str_len,
                                      size_t array_index);


  int32_t array_pos() const { return array_pos_; }



  int32_t char_offset() const { return char_offset_; }

  int32_t get() const { return char_; }




  int32_t NextCodePoint() const;



  int32_t PreviousCodePoint() const;

  bool start() const { return array_pos_ == 0; }

  bool end() const { return array_pos_ == len_; }


  bool Advance();


  bool Rewind();

 private:
  UTF16CharIterator(const string16* str, int32_t initial_pos);
  UTF16CharIterator(const char16* str, size_t str_len, int32_t initial_pos);


  void ReadChar();

  const char16* str_;

  int32_t len_;

  int32_t array_pos_;

  int32_t next_pos_;

  int32_t char_offset_;

  int32_t char_;

  DISALLOW_COPY_AND_ASSIGN(UTF16CharIterator);
};

}  // namespace i18n
}  // namespace base

#endif  // BASE_I18N_CHAR_ITERATOR_H_
