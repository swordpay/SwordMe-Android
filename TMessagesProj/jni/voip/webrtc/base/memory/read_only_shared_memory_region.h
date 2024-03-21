// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MEMORY_READ_ONLY_SHARED_MEMORY_REGION_H_
#define BASE_MEMORY_READ_ONLY_SHARED_MEMORY_REGION_H_

#include <utility>

#include "base/macros.h"
#include "base/memory/platform_shared_memory_region.h"
#include "base/memory/shared_memory_mapping.h"

namespace base {

struct MappedReadOnlyRegion;

// owns the platform handle it wraps. Mappings created by this region are
// read-only. These mappings remain valid even after the region handle is moved
// or destroyed.
class BASE_EXPORT ReadOnlySharedMemoryRegion {
 public:
  using MappingType = ReadOnlySharedMemoryMapping;










  static MappedReadOnlyRegion Create(size_t size);
  using CreateFunction = decltype(Create);






  static ReadOnlySharedMemoryRegion Deserialize(
      subtle::PlatformSharedMemoryRegion handle);




  static subtle::PlatformSharedMemoryRegion TakeHandleForSerialization(
      ReadOnlySharedMemoryRegion region);

  ReadOnlySharedMemoryRegion();

  ReadOnlySharedMemoryRegion(ReadOnlySharedMemoryRegion&&);
  ReadOnlySharedMemoryRegion& operator=(ReadOnlySharedMemoryRegion&&);


  ~ReadOnlySharedMemoryRegion();




  ReadOnlySharedMemoryRegion Duplicate() const;





  ReadOnlySharedMemoryMapping Map() const;




  ReadOnlySharedMemoryMapping MapAt(off_t offset, size_t size) const;

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

  explicit ReadOnlySharedMemoryRegion(
      subtle::PlatformSharedMemoryRegion handle);

  static void set_create_hook(CreateFunction* hook) { create_hook_ = hook; }

  static CreateFunction* create_hook_;

  subtle::PlatformSharedMemoryRegion handle_;

  DISALLOW_COPY_AND_ASSIGN(ReadOnlySharedMemoryRegion);
};

struct MappedReadOnlyRegion {
  ReadOnlySharedMemoryRegion region;
  WritableSharedMemoryMapping mapping;



  bool IsValid() const {
    DCHECK_EQ(region.IsValid(), mapping.IsValid());
    return region.IsValid() && mapping.IsValid();
  }
};

}  // namespace base

#endif  // BASE_MEMORY_READ_ONLY_SHARED_MEMORY_REGION_H_
