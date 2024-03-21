// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_VLOG_H_
#define BASE_VLOG_H_

#include <string>
#include <vector>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/strings/string_piece.h"

namespace logging {

class BASE_EXPORT VlogInfo {
 public:
  static const int kDefaultVlogLevel;


















  VlogInfo(const std::string& v_switch,
           const std::string& vmodule_switch,
           int* min_log_level);
  ~VlogInfo();


  int GetVlogLevel(const base::StringPiece& file) const;

 private:
  void SetMaxVlogLevel(int level);
  int GetMaxVlogLevel() const;


  struct VmodulePattern;
  std::vector<VmodulePattern> vmodule_levels_;
  int* min_log_level_;

  DISALLOW_COPY_AND_ASSIGN(VlogInfo);
};

// vlog pattern string can contain wildcards like * and ?.  ? matches
// exactly one character while * matches 0 or more characters.  Also,
// as a special case, a / or \ character matches either / or \.
//
// Examples:
//   "kh?n" matches "khan" but not "khn" or "khaan"
//   "kh*n" matches "khn", "khan", or even "khaaaaan"
//   "/foo\bar" matches "/foo/bar", "\foo\bar", or "/foo\bar"
//     (disregarding C escaping rules)
BASE_EXPORT bool MatchVlogPattern(const base::StringPiece& string,
                                  const base::StringPiece& vlog_pattern);

}  // namespace logging

#endif  // BASE_VLOG_H_
