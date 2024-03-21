// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MEMORY_DISCARDABLE_MEMORY_H_
#define BASE_MEMORY_DISCARDABLE_MEMORY_H_

#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "build/build_config.h"

namespace base {

namespace trace_event {
class MemoryAllocatorDump;
class ProcessMemoryDump;
}  // namespace trace_event

// blowing out memory, both on mobile devices where there is no swap, and
// desktop devices where unused free memory should be used to help the user
// experience. This is preferable to releasing memory in response to an OOM
// signal because it is simpler and provides system-wide management of
// purgable memory, though it has less flexibility as to which objects get
// discarded.
//
// Discardable memory has two states: locked and unlocked. While the memory is
// locked, it will not be discarded. Unlocking the memory allows the
// discardable memory system and the OS to reclaim it if needed. Locks do not
// nest.
//
// Notes:
//   - The paging behavior of memory while it is locked is not specified. While
//     mobile platforms will not swap it out, it may qualify for swapping
//     on desktop platforms. It is not expected that this will matter, as the
//     preferred pattern of usage for DiscardableMemory is to lock down the
//     memory, use it as quickly as possible, and then unlock it.
//   - Because of memory alignment, the amount of memory allocated can be
//     larger than the requested memory size. It is not very efficient for
//     small allocations.
//   - A discardable memory instance is not thread safe. It is the
//     responsibility of users of discardable memory to ensure there are no
//     races.
//
class BASE_EXPORT DiscardableMemory {
 public:
  DiscardableMemory();
  virtual ~DiscardableMemory();



  virtual bool Lock() WARN_UNUSED_RESULT = 0;


  virtual void Unlock() = 0;


  virtual void* data() const = 0;


  virtual void DiscardForTesting() = 0;

  template<typename T> T* data_as() const {
    return reinterpret_cast<T*>(data());
  }




  virtual trace_event::MemoryAllocatorDump* CreateMemoryAllocatorDump(
      const char* name,
      trace_event::ProcessMemoryDump* pmd) const = 0;
};

enum class DiscardableMemoryBacking { kSharedMemory, kMadvFree };
BASE_EXPORT DiscardableMemoryBacking GetDiscardableMemoryBacking();

}  // namespace base

#endif  // BASE_MEMORY_DISCARDABLE_MEMORY_H_
