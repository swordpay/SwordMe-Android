// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_MEMORY_INFRA_BACKGROUND_ALLOWLIST_H_
#define BASE_TRACE_EVENT_MEMORY_INFRA_BACKGROUND_ALLOWLIST_H_

// limit the tracing overhead and remove sensitive information from traces.

#include <string>

#include "base/base_export.h"

namespace base {
namespace trace_event {

bool BASE_EXPORT IsMemoryDumpProviderInAllowlist(const char* mdp_name);

bool BASE_EXPORT IsMemoryAllocatorDumpNameInAllowlist(const std::string& name);

// of the list must be nullptr.
void BASE_EXPORT SetDumpProviderAllowlistForTesting(const char* const* list);
void BASE_EXPORT
SetAllocatorDumpNameAllowlistForTesting(const char* const* list);

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_MEMORY_INFRA_BACKGROUND_ALLOWLIST_H_
