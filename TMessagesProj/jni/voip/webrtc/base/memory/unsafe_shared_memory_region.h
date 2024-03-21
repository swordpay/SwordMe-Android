// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MEMORY_UNSAFE_SHARED_MEMORY_REGION_H_
#define BASE_MEMORY_UNSAFE_SHARED_MEMORY_REGION_H_

#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/memory/platform_shared_memory_region.h"
#include "base/memory/shared_memory_mapping.h"

namespace base {

// owns the platform handle it wraps. Mappings created by this region are
// writable. These mappings remain valid even after the region handle is moved
// or destroyed.
//
// NOTE: UnsafeSharedMemoryRegion cannot be converted to a read-only region. Use
// with caution as the region will be writable to any process with a handle to
// the region.
//
// Use this if and only if the following is true:
// - You do not need to share the region as read-only, and,
// - You need to have several instances of the region simultaneously, possibly
//   in different processes, that can produce writable mappings.

class BASE_EXPORT UnsafeSharedMemoryRegion {
 public:
  using MappingType = WritableSharedMemoryMapping;


  static UnsafeSharedMemoryRegion Create(size_t size);
  using CreateFunction = decltype(Create);






  static UnsafeSharedMemoryRegion Deserialize(
      subtle::PlatformSharedMemoryRegion handle);




  static subtle::PlatformSharedMemoryRegion TakeHandleForSerialization(
      UnsafeSharedMemoryRegion region);

  UnsafeSharedMemoryRegion();

  UnsafeSharedMemoryRegion(UnsafeSharedMemoryRegion&&);
  UnsafeSharedMemoryRegion& operator=(UnsafeSharedMemoryRegion&&);


  ~UnsafeSharedMemoryRegion();




  UnsafeSharedMemoryRegion Duplicate() const;





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


  subtle::PlatformSharedMemoryRegion::PlatformHandle GetPlatformHandle() const {
    DCHECK(IsValid());
    return handle_.GetPlatformHandle();
  }

 private:
  friend class SharedMemoryHooks;

  explicit UnsafeSharedMemoryRegion(subtle::PlatformSharedMemoryRegion handle);

  static void set_create_hook(CreateFunction* hook) { create_hook_ = hook; }

  static CreateFunction* create_hook_;

  subtle::PlatformSharedMemoryRegion handle_;

  DISALLOW_COPY_AND_ASSIGN(UnsafeSharedMemoryRegion);
};

}  // namespace base

#endif  // BASE_MEMORY_UNSAFE_SHARED_MEMORY_REGION_H_
