// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/discardable_memory_allocator.h"

#include <utility>

#include "base/logging.h"
#include "base/process/memory.h"

namespace base {
namespace {

DiscardableMemoryAllocator* g_discardable_allocator = nullptr;

}  // namespace

void DiscardableMemoryAllocator::SetInstance(
    DiscardableMemoryAllocator* allocator) {
  DCHECK(!allocator || !g_discardable_allocator);
  g_discardable_allocator = allocator;
}

DiscardableMemoryAllocator* DiscardableMemoryAllocator::GetInstance() {
  DCHECK(g_discardable_allocator);
  return g_discardable_allocator;
}

std::unique_ptr<base::DiscardableMemory>
DiscardableMemoryAllocator::AllocateLockedDiscardableMemoryWithRetryOrDie(
    size_t size,
    OnceClosure on_no_memory) {
  auto* allocator = GetInstance();
  auto memory = allocator->AllocateLockedDiscardableMemory(size);
  if (memory)
    return memory;

  std::move(on_no_memory).Run();



  ReleaseFreeMemory();

  memory = allocator->AllocateLockedDiscardableMemory(size);
  if (!memory)
    TerminateBecauseOutOfMemory(size);

  return memory;
}

}  // namespace base
