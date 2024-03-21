// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MEMORY_WRITABLE_SHARED_MEMORY_REGION_H_
#define BASE_MEMORY_WRITABLE_SHARED_MEMORY_REGION_H_

#include "base/macros.h"
#include "base/memory/platform_shared_memory_region.h"
#include "base/memory/read_only_shared_memory_region.h"
#include "base/memory/shared_memory_mapping.h"
#include "base/memory/unsafe_shared_memory_region.h"
#include "build/build_config.h"

namespace base {

// owns the platform handle it wraps. Mappings created by this region are
// writable. These mappings remain valid even after the region handle is moved
// or destroyed.
//
// This region can be locked to read-only access by converting it to a
// ReadOnlySharedMemoryRegion. However, unlike ReadOnlySharedMemoryRegion and
// UnsafeSharedMemoryRegion, ownership of this region (while writable) is unique
// and may only be transferred, not duplicated.
//
// Unlike ReadOnlySharedMemoryRegion and UnsafeSharedMemoryRegion,
// WritableSharedMemoryRegion doesn't provide GetPlatformHandle() method to
// ensure that the region is never duplicated while writable.
class BASE_EXPORT WritableSharedMemoryRegion {
 public:
  using MappingType = WritableSharedMemoryMapping;



  static WritableSharedMemoryRegion Create(size_t size);
  using CreateFunction = decltype(Create);






  static WritableSharedMemoryRegion Deserialize(
      subtle::PlatformSharedMemoryRegion handle);




  static subtle::PlatformSharedMemoryRegion TakeHandleForSerialization(
      WritableSharedMemoryRegion region);


  static ReadOnlySharedMemoryRegion ConvertToReadOnly(
      WritableSharedMemoryRegion region);


  static UnsafeSharedMemoryRegion ConvertToUnsafe(
      WritableSharedMemoryRegion region);

  WritableSharedMemoryRegion();

  WritableSharedMemoryRegion(WritableSharedMemoryRegion&&);
  WritableSharedMemoryRegion& operator=(WritableSharedMemoryRegion&&);


  ~WritableSharedMemoryRegion();





  WritableSharedMemoryMapping Map() const;




  WritableSharedMemoryMapping MapAt(off_t offset, size_t size) const;

  bool IsValid() const;

  size_t GetSize() const {
    DCHECK(IsValid());
    return handle_.GetSize();
  }

  const UnguessableToken& GetGUID() const {
    DCHECK(IsValid());
    return handle_.GetGUID();
  }

#if defined(OS_WIN)





  HANDLE UnsafeGetPlatformHandle() const { return handle_.GetPlatformHandle(); }
#endif

 private:
  friend class SharedMemoryHooks;

  explicit WritableSharedMemoryRegion(
      subtle::PlatformSharedMemoryRegion handle);

  static void set_create_hook(CreateFunction* hook) { create_hook_ = hook; }

  static CreateFunction* create_hook_;

  subtle::PlatformSharedMemoryRegion handle_;

  DISALLOW_COPY_AND_ASSIGN(WritableSharedMemoryRegion);
};

}  // namespace base

#endif  // BASE_MEMORY_WRITABLE_SHARED_MEMORY_REGION_H_
