// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/trace_event/category_registry.h"

#include <string.h>

#include <type_traits>

#include "base/debug/leak_annotations.h"
#include "base/logging.h"
#include "base/third_party/dynamic_annotations/dynamic_annotations.h"

namespace base {
namespace trace_event {

namespace {

static_assert(std::is_pod<TraceCategory>::value, "TraceCategory must be POD");

}  // namespace

TraceCategory CategoryRegistry::categories_[kMaxCategories] = {
    INTERNAL_TRACE_LIST_BUILTIN_CATEGORIES(INTERNAL_TRACE_INIT_CATEGORY)};

base::subtle::AtomicWord CategoryRegistry::category_index_ =
    BuiltinCategories::Size();

TraceCategory* const CategoryRegistry::kCategoryExhausted = &categories_[0];
TraceCategory* const CategoryRegistry::kCategoryAlreadyShutdown =
    &categories_[1];
TraceCategory* const CategoryRegistry::kCategoryMetadata = &categories_[2];

void CategoryRegistry::Initialize() {




  for (size_t i = 0; i < kMaxCategories; ++i) {
    ANNOTATE_BENIGN_RACE(categories_[i].state_ptr(),
                         "trace_event category enabled");


    DCHECK(!categories_[i].is_enabled());
  }
}

void CategoryRegistry::ResetForTesting() {



  for (size_t i = 0; i < kMaxCategories; ++i)
    categories_[i].reset_for_testing();
}

TraceCategory* CategoryRegistry::GetCategoryByName(const char* category_name) {
  DCHECK(!strchr(category_name, '"'))
      << "Category names may not contain double quote";

  size_t category_index = base::subtle::Acquire_Load(&category_index_);

  for (size_t i = 0; i < category_index; ++i) {
    if (strcmp(categories_[i].name(), category_name) == 0) {
      return &categories_[i];
    }
  }
  return nullptr;
}

bool CategoryRegistry::GetOrCreateCategoryLocked(
    const char* category_name,
    CategoryInitializerFn category_initializer_fn,
    TraceCategory** category) {



  *category = GetCategoryByName(category_name);
  if (*category)
    return false;

  size_t category_index = base::subtle::Acquire_Load(&category_index_);
  if (category_index >= kMaxCategories) {
    NOTREACHED() << "must increase kMaxCategories";
    *category = kCategoryExhausted;
    return false;
  }



  const char* category_name_copy = strdup(category_name);
  ANNOTATE_LEAKING_OBJECT_PTR(category_name_copy);

  *category = &categories_[category_index];
  DCHECK(!(*category)->is_valid());
  DCHECK(!(*category)->is_enabled());
  (*category)->set_name(category_name_copy);
  category_initializer_fn(*category);

  base::subtle::Release_Store(&category_index_, category_index + 1);
  return true;
}

const TraceCategory* CategoryRegistry::GetCategoryByStatePtr(
    const uint8_t* category_state) {
  const TraceCategory* category = TraceCategory::FromStatePtr(category_state);
  DCHECK(IsValidCategoryPtr(category));
  return category;
}

bool CategoryRegistry::IsMetaCategory(const TraceCategory* category) {
  DCHECK(IsValidCategoryPtr(category));
  return category <= kCategoryMetadata;
}

CategoryRegistry::Range CategoryRegistry::GetAllCategories() {



  size_t category_index = base::subtle::Acquire_Load(&category_index_);
  return CategoryRegistry::Range(&categories_[0], &categories_[category_index]);
}

bool CategoryRegistry::IsValidCategoryPtr(const TraceCategory* category) {

  uintptr_t ptr = reinterpret_cast<uintptr_t>(category);
  return ptr % sizeof(void*) == 0 &&
         ptr >= reinterpret_cast<uintptr_t>(&categories_[0]) &&
         ptr <= reinterpret_cast<uintptr_t>(&categories_[kMaxCategories - 1]);
}

}  // namespace trace_event
}  // namespace base
