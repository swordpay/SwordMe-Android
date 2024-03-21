// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_STRINGS_UTF_OFFSET_STRING_CONVERSIONS_H_
#define BASE_STRINGS_UTF_OFFSET_STRING_CONVERSIONS_H_

#include <stddef.h>

#include <string>
#include <vector>

#include "base/base_export.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"

namespace base {

// string in response to various adjustments one might do to that string
// (e.g., eliminating a range).  For details on offsets, see the comments by
// the AdjustOffsets() function below.
class BASE_EXPORT OffsetAdjuster {
 public:
  struct BASE_EXPORT Adjustment {
    Adjustment(size_t original_offset,
               size_t original_length,
               size_t output_length);

    size_t original_offset;
    size_t original_length;
    size_t output_length;
  };
  typedef std::vector<Adjustment> Adjustments;











  static void AdjustOffsets(const Adjustments& adjustments,
                            std::vector<size_t>* offsets_for_adjustment,
                            size_t limit = string16::npos);


  static void AdjustOffset(const Adjustments& adjustments,
                           size_t* offset,
                           size_t limit = string16::npos);






  static void UnadjustOffsets(const Adjustments& adjustments,
                              std::vector<size_t>* offsets_for_unadjustment);


  static void UnadjustOffset(const Adjustments& adjustments,
                             size_t* offset);















  static void MergeSequentialAdjustments(
      const Adjustments& first_adjustments,
      Adjustments* adjustments_on_adjusted_string);
};

// |adjustments| parameter that reflects the alterations done to the string.
// It may be NULL.
BASE_EXPORT bool UTF8ToUTF16WithAdjustments(
    const char* src,
    size_t src_len,
    string16* output,
    base::OffsetAdjuster::Adjustments* adjustments);
BASE_EXPORT string16 UTF8ToUTF16WithAdjustments(
    const base::StringPiece& utf8,
    base::OffsetAdjuster::Adjustments* adjustments) WARN_UNUSED_RESULT;
// As above, but instead internally examines the adjustments and applies them
// to |offsets_for_adjustment|.  Input offsets greater than the length of the
// input string will be set to string16::npos.  See comments by AdjustOffsets().
BASE_EXPORT string16 UTF8ToUTF16AndAdjustOffsets(
    const base::StringPiece& utf8,
    std::vector<size_t>* offsets_for_adjustment);
BASE_EXPORT std::string UTF16ToUTF8AndAdjustOffsets(
    const base::StringPiece16& utf16,
    std::vector<size_t>* offsets_for_adjustment);

}  // namespace base

#endif  // BASE_STRINGS_UTF_OFFSET_STRING_CONVERSIONS_H_
