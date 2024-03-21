// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MEMORY_DISCARDABLE_MEMORY_ALLOCATOR_H_
#define BASE_MEMORY_DISCARDABLE_MEMORY_ALLOCATOR_H_

#include <stddef.h>

#include <memory>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/macros.h"
#include "base/memory/discardable_memory.h"

namespace base {
class DiscardableMemory;

// itself should be created via CreateDiscardableMemoryAllocator, which
// selects an appropriate implementation depending on platform support.
class BASE_EXPORT DiscardableMemoryAllocator {
 public:
  DiscardableMemoryAllocator() = default;
  virtual ~DiscardableMemoryAllocator() = default;

  static DiscardableMemoryAllocator* GetInstance();


  static void SetInstance(DiscardableMemoryAllocator* allocator);





  virtual std::unique_ptr<DiscardableMemory> AllocateLockedDiscardableMemory(
      size_t size) = 0;









  std::unique_ptr<DiscardableMemory>
  AllocateLockedDiscardableMemoryWithRetryOrDie(size_t size,
                                                OnceClosure on_no_memory);


  virtual size_t GetBytesAllocated() const = 0;


  virtual void ReleaseFreeMemory() = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(DiscardableMemoryAllocator);
};

}  // namespace base

#endif  // BASE_MEMORY_DISCARDABLE_MEMORY_ALLOCATOR_H_
