// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_TRACE_CONFIG_CATEGORY_FILTER_H_
#define BASE_TRACE_EVENT_TRACE_CONFIG_CATEGORY_FILTER_H_

#include <string>
#include <vector>

#include "base/base_export.h"
#include "base/strings/string_piece.h"
#include "base/values.h"

namespace base {
namespace trace_event {

class BASE_EXPORT TraceConfigCategoryFilter {
 public:
  using StringList = std::vector<std::string>;

  TraceConfigCategoryFilter();
  TraceConfigCategoryFilter(const TraceConfigCategoryFilter& other);
  ~TraceConfigCategoryFilter();

  TraceConfigCategoryFilter& operator=(const TraceConfigCategoryFilter& rhs);


  void InitializeFromString(const StringPiece& category_filter_string);

  void InitializeFromConfigDict(const Value& dict);

  void Merge(const TraceConfigCategoryFilter& config);
  void Clear();



  bool IsCategoryGroupEnabled(const StringPiece& category_group_name) const;




  bool IsCategoryEnabled(const StringPiece& category_name) const;

  void ToDict(Value* dict) const;

  std::string ToFilterString() const;

  static bool IsCategoryNameAllowed(StringPiece str);

  const StringList& included_categories() const { return included_categories_; }
  const StringList& excluded_categories() const { return excluded_categories_; }

 private:
  void SetCategoriesFromIncludedList(const Value& included_list);
  void SetCategoriesFromExcludedList(const Value& excluded_list);

  void AddCategoriesToDict(const StringList& categories,
                           const char* param,
                           Value* dict) const;

  void WriteCategoryFilterString(const StringList& values,
                                 std::string* out,
                                 bool included) const;

  StringList included_categories_;
  StringList disabled_categories_;
  StringList excluded_categories_;
};

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_TRACE_CONFIG_CATEGORY_FILTER_H_
