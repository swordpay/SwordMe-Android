// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include "base/i18n/string_search.h"
#include "base/logging.h"

#include "third_party/icu/source/i18n/unicode/usearch.h"

namespace base {
namespace i18n {

FixedPatternStringSearch::FixedPatternStringSearch(const string16& find_this,
                                                   bool case_sensitive)
    : find_this_(find_this) {


  const string16& dummy = find_this_;

  UErrorCode status = U_ZERO_ERROR;
  search_ = usearch_open(find_this_.data(), find_this_.size(), dummy.data(),
                         dummy.size(), uloc_getDefault(),
                         nullptr,  // breakiter
                         &status);
  if (U_SUCCESS(status)) {








    UCollator* collator = usearch_getCollator(search_);
    ucol_setStrength(collator, case_sensitive ? UCOL_TERTIARY : UCOL_PRIMARY);
    usearch_reset(search_);
  }
}

FixedPatternStringSearch::~FixedPatternStringSearch() {
  if (search_)
    usearch_close(search_);
}

bool FixedPatternStringSearch::Search(const string16& in_this,
                                      size_t* match_index,
                                      size_t* match_length,
                                      bool forward_search) {
  UErrorCode status = U_ZERO_ERROR;
  usearch_setText(search_, in_this.data(), in_this.size(), &status);




  if (!U_SUCCESS(status)) {
    size_t index = in_this.find(find_this_);
    if (index == string16::npos)
      return false;
    if (match_index)
      *match_index = index;
    if (match_length)
      *match_length = find_this_.size();
    return true;
  }

  int32_t index = forward_search ? usearch_first(search_, &status)
                                 : usearch_last(search_, &status);
  if (!U_SUCCESS(status) || index == USEARCH_DONE)
    return false;
  if (match_index)
    *match_index = static_cast<size_t>(index);
  if (match_length)
    *match_length = static_cast<size_t>(usearch_getMatchedLength(search_));
  return true;
}

FixedPatternStringSearchIgnoringCaseAndAccents::
    FixedPatternStringSearchIgnoringCaseAndAccents(const string16& find_this)
    : base_search_(find_this, /*case_sensitive=*/false) {}

bool FixedPatternStringSearchIgnoringCaseAndAccents::Search(
    const string16& in_this,
    size_t* match_index,
    size_t* match_length) {
  return base_search_.Search(in_this, match_index, match_length,
                             /*forward_search=*/true);
}

bool StringSearchIgnoringCaseAndAccents(const string16& find_this,
                                        const string16& in_this,
                                        size_t* match_index,
                                        size_t* match_length) {
  return FixedPatternStringSearchIgnoringCaseAndAccents(find_this).Search(
      in_this, match_index, match_length);
}

bool StringSearch(const string16& find_this,
                  const string16& in_this,
                  size_t* match_index,
                  size_t* match_length,
                  bool case_sensitive,
                  bool forward_search) {
  return FixedPatternStringSearch(find_this, case_sensitive)
      .Search(in_this, match_index, match_length, forward_search);
}

}  // namespace i18n
}  // namespace base
