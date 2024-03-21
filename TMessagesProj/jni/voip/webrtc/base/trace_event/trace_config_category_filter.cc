// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/trace_event/trace_config_category_filter.h"

#include "base/memory/ptr_util.h"
#include "base/strings/pattern.h"
#include "base/strings/string_split.h"
#include "base/strings/string_tokenizer.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/trace_event/trace_event.h"

namespace base {
namespace trace_event {

namespace {
const char kIncludedCategoriesParam[] = "included_categories";
const char kExcludedCategoriesParam[] = "excluded_categories";
}

TraceConfigCategoryFilter::TraceConfigCategoryFilter() = default;

TraceConfigCategoryFilter::TraceConfigCategoryFilter(
    const TraceConfigCategoryFilter& other) = default;

TraceConfigCategoryFilter::~TraceConfigCategoryFilter() = default;

TraceConfigCategoryFilter& TraceConfigCategoryFilter::operator=(
    const TraceConfigCategoryFilter& rhs) = default;

void TraceConfigCategoryFilter::InitializeFromString(
    const StringPiece& category_filter_string) {
  std::vector<StringPiece> split = SplitStringPiece(
      category_filter_string, ",", TRIM_WHITESPACE, SPLIT_WANT_ALL);
  for (const StringPiece& category : split) {

    if (category.empty())
      continue;
    if (category.front() == '-') {


      excluded_categories_.push_back(category.substr(1).as_string());
    } else if (category.starts_with(TRACE_DISABLED_BY_DEFAULT(""))) {
      disabled_categories_.push_back(category.as_string());
    } else {
      included_categories_.push_back(category.as_string());
    }
  }
}

void TraceConfigCategoryFilter::InitializeFromConfigDict(const Value& dict) {
  const Value* included_category_list =
      dict.FindListKey(kIncludedCategoriesParam);
  if (included_category_list)
    SetCategoriesFromIncludedList(*included_category_list);
  const Value* excluded_category_list =
      dict.FindListKey(kExcludedCategoriesParam);
  if (excluded_category_list)
    SetCategoriesFromExcludedList(*excluded_category_list);
}

bool TraceConfigCategoryFilter::IsCategoryGroupEnabled(
    const StringPiece& category_group_name) const {
  bool had_enabled_by_default = false;
  DCHECK(!category_group_name.empty());
  CStringTokenizer category_group_tokens(category_group_name.begin(),
                                         category_group_name.end(), ",");
  while (category_group_tokens.GetNext()) {
    StringPiece category_group_token = category_group_tokens.token_piece();

    DCHECK(IsCategoryNameAllowed(category_group_token))
        << "Disallowed category string";
    if (IsCategoryEnabled(category_group_token))
      return true;

    if (!MatchPattern(category_group_token, TRACE_DISABLED_BY_DEFAULT("*")))
      had_enabled_by_default = true;
  }


  category_group_tokens.Reset();
  bool category_group_disabled = false;
  while (category_group_tokens.GetNext()) {
    StringPiece category_group_token = category_group_tokens.token_piece();
    for (const std::string& category : excluded_categories_) {
      if (MatchPattern(category_group_token, category)) {




        category_group_disabled = true;
        break;
      }




      if (!MatchPattern(category_group_token, TRACE_DISABLED_BY_DEFAULT("*")))
        category_group_disabled = false;
    }



    if (!category_group_disabled)
      break;
  }



  return !category_group_disabled && had_enabled_by_default &&
         included_categories_.empty();
}

bool TraceConfigCategoryFilter::IsCategoryEnabled(
    const StringPiece& category_name) const {


  for (const std::string& category : disabled_categories_) {
    if (MatchPattern(category_name, category))
      return true;
  }

  if (MatchPattern(category_name, TRACE_DISABLED_BY_DEFAULT("*")))
    return false;

  for (const std::string& category : included_categories_) {
    if (MatchPattern(category_name, category))
      return true;
  }

  return false;
}

void TraceConfigCategoryFilter::Merge(const TraceConfigCategoryFilter& config) {



  if (!included_categories_.empty() && !config.included_categories_.empty()) {
    included_categories_.insert(included_categories_.end(),
                                config.included_categories_.begin(),
                                config.included_categories_.end());
  } else {
    included_categories_.clear();
  }

  disabled_categories_.insert(disabled_categories_.end(),
                              config.disabled_categories_.begin(),
                              config.disabled_categories_.end());
  excluded_categories_.insert(excluded_categories_.end(),
                              config.excluded_categories_.begin(),
                              config.excluded_categories_.end());
}

void TraceConfigCategoryFilter::Clear() {
  included_categories_.clear();
  disabled_categories_.clear();
  excluded_categories_.clear();
}

void TraceConfigCategoryFilter::ToDict(Value* dict) const {
  StringList categories(included_categories_);
  categories.insert(categories.end(), disabled_categories_.begin(),
                    disabled_categories_.end());
  AddCategoriesToDict(categories, kIncludedCategoriesParam, dict);
  AddCategoriesToDict(excluded_categories_, kExcludedCategoriesParam, dict);
}

std::string TraceConfigCategoryFilter::ToFilterString() const {
  std::string filter_string;
  WriteCategoryFilterString(included_categories_, &filter_string, true);
  WriteCategoryFilterString(disabled_categories_, &filter_string, true);
  WriteCategoryFilterString(excluded_categories_, &filter_string, false);
  return filter_string;
}

void TraceConfigCategoryFilter::SetCategoriesFromIncludedList(
    const Value& included_list) {
  included_categories_.clear();
  for (const Value& item : included_list.GetList()) {
    if (!item.is_string())
      continue;
    const std::string& category = item.GetString();
    if (category.compare(0, strlen(TRACE_DISABLED_BY_DEFAULT("")),
                         TRACE_DISABLED_BY_DEFAULT("")) == 0) {
      disabled_categories_.push_back(category);
    } else {
      included_categories_.push_back(category);
    }
  }
}

void TraceConfigCategoryFilter::SetCategoriesFromExcludedList(
    const Value& excluded_list) {
  excluded_categories_.clear();
  for (const Value& item : excluded_list.GetList()) {
    if (item.is_string())
      excluded_categories_.push_back(item.GetString());
  }
}

void TraceConfigCategoryFilter::AddCategoriesToDict(
    const StringList& categories,
    const char* param,
    Value* dict) const {
  if (categories.empty())
    return;

  std::vector<base::Value> list;
  for (const std::string& category : categories)
    list.emplace_back(category);
  dict->SetKey(param, base::Value(std::move(list)));
}

void TraceConfigCategoryFilter::WriteCategoryFilterString(
    const StringList& values,
    std::string* out,
    bool included) const {
  bool prepend_comma = !out->empty();
  int token_cnt = 0;
  for (const std::string& category : values) {
    if (token_cnt > 0 || prepend_comma)
      StringAppendF(out, ",");
    StringAppendF(out, "%s%s", (included ? "" : "-"), category.c_str());
    ++token_cnt;
  }
}

bool TraceConfigCategoryFilter::IsCategoryNameAllowed(StringPiece str) {
  return !str.empty() && str.front() != ' ' && str.back() != ' ';
}

}  // namespace trace_event
}  // namespace base
