// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/strings/utf_offset_string_conversions.h"

#include <stdint.h>

#include <algorithm>
#include <memory>

#include "base/logging.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversion_utils.h"

namespace base {

OffsetAdjuster::Adjustment::Adjustment(size_t original_offset,
                                       size_t original_length,
                                       size_t output_length)
    : original_offset(original_offset),
      original_length(original_length),
      output_length(output_length) {
}

void OffsetAdjuster::AdjustOffsets(const Adjustments& adjustments,
                                   std::vector<size_t>* offsets_for_adjustment,
                                   size_t limit) {
  DCHECK(offsets_for_adjustment);
  for (auto& i : *offsets_for_adjustment)
    AdjustOffset(adjustments, &i, limit);
}

void OffsetAdjuster::AdjustOffset(const Adjustments& adjustments,
                                  size_t* offset,
                                  size_t limit) {
  DCHECK(offset);
  if (*offset == string16::npos)
    return;
  int adjustment = 0;
  for (const auto& i : adjustments) {
    if (*offset <= i.original_offset)
      break;
    if (*offset < (i.original_offset + i.original_length)) {
      *offset = string16::npos;
      return;
    }
    adjustment += static_cast<int>(i.original_length - i.output_length);
  }
  *offset -= adjustment;

  if (*offset > limit)
    *offset = string16::npos;
}

void OffsetAdjuster::UnadjustOffsets(
    const Adjustments& adjustments,
    std::vector<size_t>* offsets_for_unadjustment) {
  if (!offsets_for_unadjustment || adjustments.empty())
    return;
  for (auto& i : *offsets_for_unadjustment)
    UnadjustOffset(adjustments, &i);
}

void OffsetAdjuster::UnadjustOffset(const Adjustments& adjustments,
                                    size_t* offset) {
  if (*offset == string16::npos)
    return;
  int adjustment = 0;
  for (const auto& i : adjustments) {
    if (*offset + adjustment <= i.original_offset)
      break;
    adjustment += static_cast<int>(i.original_length - i.output_length);
    if ((*offset + adjustment) < (i.original_offset + i.original_length)) {
      *offset = string16::npos;
      return;
    }
  }
  *offset += adjustment;
}

void OffsetAdjuster::MergeSequentialAdjustments(
    const Adjustments& first_adjustments,
    Adjustments* adjustments_on_adjusted_string) {
  auto adjusted_iter = adjustments_on_adjusted_string->begin();
  auto first_iter = first_adjustments.begin();








  size_t shift = 0;
  size_t currently_collapsing = 0;






  Adjustments adjustments_builder;
  while (adjusted_iter != adjustments_on_adjusted_string->end()) {
    if ((first_iter == first_adjustments.end()) ||
        ((adjusted_iter->original_offset + shift +
          adjusted_iter->original_length) <= first_iter->original_offset)) {





      adjusted_iter->original_offset += shift;
      shift += currently_collapsing;
      currently_collapsing = 0;
      adjustments_builder.push_back(*adjusted_iter);
      ++adjusted_iter;
    } else if ((adjusted_iter->original_offset + shift) >
               first_iter->original_offset) {








      DCHECK_LE(first_iter->original_offset + first_iter->output_length,
                adjusted_iter->original_offset + shift);

      shift += first_iter->original_length - first_iter->output_length;
      adjustments_builder.push_back(*first_iter);
      ++first_iter;
    } else {













      const int collapse = static_cast<int>(first_iter->original_length) -
          static_cast<int>(first_iter->output_length);


      DCHECK_GT(collapse, 0);
      adjusted_iter->original_length += collapse;
      currently_collapsing += collapse;
      ++first_iter;
    }
  }
  DCHECK_EQ(0u, currently_collapsing);
  if (first_iter != first_adjustments.end()) {



    DCHECK(adjusted_iter == adjustments_on_adjusted_string->end());
    adjustments_builder.insert(adjustments_builder.end(), first_iter,
                               first_adjustments.end());
  }
  *adjustments_on_adjusted_string = std::move(adjustments_builder);
}

// Unicode character type as a STL string. The given input buffer and size
// determine the source, and the given output STL string will be replaced by
// the result.  If non-NULL, |adjustments| is set to reflect the all the
// alterations to the string that are not one-character-to-one-character.
// It will always be sorted by increasing offset.
template<typename SrcChar, typename DestStdString>
bool ConvertUnicode(const SrcChar* src,
                    size_t src_len,
                    DestStdString* output,
                    OffsetAdjuster::Adjustments* adjustments) {
  if (adjustments)
    adjustments->clear();

  bool success = true;
  int32_t src_len32 = static_cast<int32_t>(src_len);
  for (int32_t i = 0; i < src_len32; i++) {
    uint32_t code_point;
    size_t original_i = i;
    size_t chars_written = 0;
    if (ReadUnicodeCharacter(src, src_len32, &i, &code_point)) {
      chars_written = WriteUnicodeCharacter(code_point, output);
    } else {
      chars_written = WriteUnicodeCharacter(0xFFFD, output);
      success = false;
    }






    if (adjustments && ((i - original_i + 1) != chars_written)) {
      adjustments->push_back(OffsetAdjuster::Adjustment(
          original_i, i - original_i + 1, chars_written));
    }
  }
  return success;
}

bool UTF8ToUTF16WithAdjustments(
    const char* src,
    size_t src_len,
    string16* output,
    base::OffsetAdjuster::Adjustments* adjustments) {
  PrepareForUTF16Or32Output(src, src_len, output);
  return ConvertUnicode(src, src_len, output, adjustments);
}

string16 UTF8ToUTF16WithAdjustments(
    const base::StringPiece& utf8,
    base::OffsetAdjuster::Adjustments* adjustments) {
  string16 result;
  UTF8ToUTF16WithAdjustments(utf8.data(), utf8.length(), &result, adjustments);
  return result;
}

string16 UTF8ToUTF16AndAdjustOffsets(
    const base::StringPiece& utf8,
    std::vector<size_t>* offsets_for_adjustment) {
  for (size_t& offset : *offsets_for_adjustment) {
    if (offset > utf8.length())
      offset = string16::npos;
  }
  OffsetAdjuster::Adjustments adjustments;
  string16 result = UTF8ToUTF16WithAdjustments(utf8, &adjustments);
  OffsetAdjuster::AdjustOffsets(adjustments, offsets_for_adjustment);
  return result;
}

std::string UTF16ToUTF8AndAdjustOffsets(
    const base::StringPiece16& utf16,
    std::vector<size_t>* offsets_for_adjustment) {
  for (size_t& offset : *offsets_for_adjustment) {
    if (offset > utf16.length())
      offset = string16::npos;
  }
  std::string result;
  PrepareForUTF8Output(utf16.data(), utf16.length(), &result);
  OffsetAdjuster::Adjustments adjustments;
  ConvertUnicode(utf16.data(), utf16.length(), &result, &adjustments);
  OffsetAdjuster::AdjustOffsets(adjustments, offsets_for_adjustment);
  return result;
}

}  // namespace base
