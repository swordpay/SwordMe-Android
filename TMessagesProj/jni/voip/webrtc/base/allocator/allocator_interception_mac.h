// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ALLOCATOR_ALLOCATOR_INTERCEPTION_MAC_H_
#define BASE_ALLOCATOR_ALLOCATOR_INTERCEPTION_MAC_H_

#include <stddef.h>

#include "base/base_export.h"
#include "third_party/apple_apsl/malloc.h"

namespace base {
namespace allocator {

struct MallocZoneFunctions;

void StoreFunctionsForDefaultZone();

void StoreFunctionsForAllZones();

// |functions|.
void ReplaceFunctionsForStoredZones(const MallocZoneFunctions* functions);

extern bool g_replaced_default_zone;

bool UncheckedMallocMac(size_t size, void** result);
bool UncheckedCallocMac(size_t num_items, size_t size, void** result);

// Foundation and Objective-C allocations.
// Has no effect on the default malloc zone if the allocator shim already
// performs that interception.
BASE_EXPORT void InterceptAllocationsMac();

// Also calls ClearAllMallocZonesForTesting.
BASE_EXPORT void UninterceptMallocZonesForTesting();

// minute.
BASE_EXPORT void PeriodicallyShimNewMallocZones();

BASE_EXPORT void ShimNewMallocZones();
BASE_EXPORT void ReplaceZoneFunctions(ChromeMallocZone* zone,
                                      const MallocZoneFunctions* functions);

}  // namespace allocator
}  // namespace base

#endif  // BASE_ALLOCATOR_ALLOCATOR_INTERCEPTION_MAC_H_
