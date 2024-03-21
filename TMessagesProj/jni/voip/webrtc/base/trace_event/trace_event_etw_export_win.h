// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_TRACE_EVENT_ETW_EXPORT_WIN_H_
#define BASE_TRACE_EVENT_TRACE_EVENT_ETW_EXPORT_WIN_H_

#include <stdint.h>

#include <map>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "base/trace_event/trace_event_impl.h"

namespace base {

template <typename Type>
struct StaticMemorySingletonTraits;

namespace trace_event {

class BASE_EXPORT TraceEventETWExport {
 public:
  ~TraceEventETWExport();


  static TraceEventETWExport* GetInstance();




  static TraceEventETWExport* GetInstanceIfExists();


  static void EnableETWExport();


  static void AddEvent(char phase,
                       const unsigned char* category_group_enabled,
                       const char* name,
                       unsigned long long id,
                       const TraceArguments* args);

  static void AddCompleteEndEvent(const char* name);

  static bool IsCategoryGroupEnabled(StringPiece category_group_name);


  static void OnETWEnableUpdate();

 private:

  friend struct StaticMemorySingletonTraits<TraceEventETWExport>;

  TraceEventETWExport();


  bool UpdateEnabledCategories();

  bool IsCategoryEnabled(StringPiece category_name) const;

  static bool is_registration_complete_;

  std::map<StringPiece, bool> categories_status_;

  uint64_t etw_match_any_keyword_;

  DISALLOW_COPY_AND_ASSIGN(TraceEventETWExport);
};

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_TRACE_EVENT_ETW_EXPORT_WIN_H_
