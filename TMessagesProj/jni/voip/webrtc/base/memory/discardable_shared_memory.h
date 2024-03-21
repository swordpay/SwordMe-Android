// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MEMORY_DISCARDABLE_SHARED_MEMORY_H_
#define BASE_MEMORY_DISCARDABLE_SHARED_MEMORY_H_

#include <stddef.h>

#include "base/base_export.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/shared_memory_mapping.h"
#include "base/memory/unsafe_shared_memory_region.h"
#include "base/threading/thread_collision_warner.h"
#include "base/time/time.h"
#include "build/build_config.h"

#if DCHECK_IS_ON()
#include <set>
#endif

// which has the behavior of reliably causing zero-fill-on-demand pages to
// be returned after a call. Here we define
// DISCARDABLE_SHARED_MEMORY_ZERO_FILL_ON_DEMAND_PAGES_AFTER_PURGE on Linux
// and Android to indicate that this type of behavior can be expected on
// those platforms. Note that madvise() will still be used on other POSIX
// platforms but doesn't provide the zero-fill-on-demand pages guarantee.
#if defined(OS_LINUX) || defined(OS_ANDROID)
#define DISCARDABLE_SHARED_MEMORY_ZERO_FILL_ON_DEMAND_PAGES_AFTER_PURGE
#endif

namespace base {

namespace trace_event {
class MemoryAllocatorDump;
class ProcessMemoryDump;
}  // namespace trace_event

//
// This class is not thread-safe. Clients are responsible for synchronizing
// access to an instance of this class.
class BASE_EXPORT DiscardableSharedMemory {
 public:
  enum LockResult { SUCCESS, PURGED, FAILED };

  DiscardableSharedMemory();


  explicit DiscardableSharedMemory(UnsafeSharedMemoryRegion region);

  virtual ~DiscardableSharedMemory();


  bool CreateAndMap(size_t size);


  bool Map(size_t size);




  bool Unmap();

  size_t mapped_size() const { return mapped_size_; }


  UnsafeSharedMemoryRegion DuplicateRegion() const {
    return shared_memory_region_.Duplicate();
  }



  const UnguessableToken& mapped_id() const {
    return shared_memory_mapping_.guid();
  }












  LockResult Lock(size_t offset, size_t length);






  void Unlock(size_t offset, size_t length);


  void* memory() const;



  Time last_known_usage() const { return last_known_usage_; }












  bool Purge(Time current_time);

  bool IsMemoryResident() const;

  bool IsMemoryLocked() const;


  void Close();





  void CreateSharedMemoryOwnershipEdge(
      trace_event::MemoryAllocatorDump* local_segment_dump,
      trace_event::ProcessMemoryDump* pmd,
      bool is_owned) const;

#if defined(OS_ANDROID)


  static bool IsAshmemDeviceSupportedForTesting();
#endif

 private:





  static LockResult LockPages(const UnsafeSharedMemoryRegion& region,
                              size_t offset,
                              size_t length);

  static void UnlockPages(const UnsafeSharedMemoryRegion& region,
                          size_t offset,
                          size_t length);

  virtual Time Now() const;

  UnsafeSharedMemoryRegion shared_memory_region_;
  WritableSharedMemoryMapping shared_memory_mapping_;
  size_t mapped_size_;
  size_t locked_page_count_;
#if DCHECK_IS_ON()
  std::set<size_t> locked_pages_;
#endif


  DFAKE_MUTEX(thread_collision_warner_);
  Time last_known_usage_;

  DISALLOW_COPY_AND_ASSIGN(DiscardableSharedMemory);
};

}  // namespace base

#endif  // BASE_MEMORY_DISCARDABLE_SHARED_MEMORY_H_
