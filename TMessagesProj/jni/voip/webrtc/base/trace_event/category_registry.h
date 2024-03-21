// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_CATEGORY_REGISTRY_H_
#define BASE_TRACE_EVENT_CATEGORY_REGISTRY_H_

#include <stddef.h>
#include <stdint.h>

#include "base/atomicops.h"
#include "base/base_export.h"
#include "base/logging.h"
#include "base/stl_util.h"
#include "base/trace_event/builtin_categories.h"
#include "base/trace_event/common/trace_event_common.h"
#include "base/trace_event/trace_category.h"
#include "build/build_config.h"

namespace base {
namespace trace_event {

class TraceCategoryTest;
class TraceLog;

// All the methods in this class can be concurrently called on multiple threads,
// unless otherwise noted (e.g., GetOrCreateCategoryLocked).
// The reason why this is a fully static class with global state is to allow to
// statically define known categories as global linker-initialized structs,
// without requiring static initializers.
class BASE_EXPORT CategoryRegistry {
 public:

  class Range {
   public:
    Range(TraceCategory* begin, TraceCategory* end) : begin_(begin), end_(end) {
      DCHECK_LE(begin, end);
    }
    TraceCategory* begin() const { return begin_; }
    TraceCategory* end() const { return end_; }

   private:
    TraceCategory* const begin_;
    TraceCategory* const end_;
  };

  static TraceCategory* const kCategoryExhausted;
  static TraceCategory* const kCategoryMetadata;
  static TraceCategory* const kCategoryAlreadyShutdown;






  static const TraceCategory* GetCategoryByStatePtr(
      const uint8_t* category_state);




  static TraceCategory* GetCategoryByName(const char* category_name);



  static constexpr TraceCategory* GetBuiltinCategoryByName(
      const char* category_group) {
#if defined(OS_WIN) && defined(COMPONENT_BUILD)


    return nullptr;
#else
    for (size_t i = 0; i < BuiltinCategories::Size(); ++i) {
      if (StrEqConstexpr(category_group, BuiltinCategories::At(i)))
        return &categories_[i];
    }
    return nullptr;
#endif
  }


  static bool IsMetaCategory(const TraceCategory* category);

 private:
  friend class TraceCategoryTest;
  friend class TraceLog;
  using CategoryInitializerFn = void (*)(TraceCategory*);

  static constexpr size_t kMaxCategories = 300;

  static_assert(BuiltinCategories::Size() <= kMaxCategories,
                "kMaxCategories must be greater than kNumBuiltinCategories");

  static void Initialize();

  static void ResetForTesting();





  static bool GetOrCreateCategoryLocked(const char* category_name,
                                        CategoryInitializerFn,
                                        TraceCategory**);


  static Range GetAllCategories();

  static bool IsValidCategoryPtr(const TraceCategory* category);

  static TraceCategory categories_[kMaxCategories];

  static base::subtle::AtomicWord category_index_;
};

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_CATEGORY_REGISTRY_H_
