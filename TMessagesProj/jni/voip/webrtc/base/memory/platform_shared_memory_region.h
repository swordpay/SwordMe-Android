// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MEMORY_PLATFORM_SHARED_MEMORY_REGION_H_
#define BASE_MEMORY_PLATFORM_SHARED_MEMORY_REGION_H_

#include <utility>

#include "base/compiler_specific.h"
#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/unguessable_token.h"
#include "build/build_config.h"

#if defined(OS_MACOSX) && !defined(OS_IOS)
#include <mach/mach.h>
#include "base/mac/scoped_mach_port.h"
#elif defined(OS_FUCHSIA)
#include <lib/zx/vmo.h>
#elif defined(OS_WIN)
#include "base/win/scoped_handle.h"
#include "base/win/windows_types.h"
#elif defined(OS_POSIX)
#include <sys/types.h>
#include "base/file_descriptor_posix.h"
#include "base/files/scoped_file.h"
#endif

#if defined(OS_LINUX)
namespace content {
class SandboxIPCHandler;
}
#endif

namespace base {
namespace subtle {

#if defined(OS_POSIX) && (!defined(OS_MACOSX) || defined(OS_IOS)) && \
    !defined(OS_ANDROID)
// Helper structs to keep two descriptors on POSIX. It's needed to support
// ConvertToReadOnly().
struct BASE_EXPORT FDPair {


  int fd;


  int readonly_fd;
};

struct BASE_EXPORT ScopedFDPair {
  ScopedFDPair();
  ScopedFDPair(ScopedFD in_fd, ScopedFD in_readonly_fd);
  ScopedFDPair(ScopedFDPair&&);
  ScopedFDPair& operator=(ScopedFDPair&&);
  ~ScopedFDPair();

  FDPair get() const;

  ScopedFD fd;
  ScopedFD readonly_fd;
};
#endif

//
// This class does the following:
//
// - Wraps and owns a shared memory region platform handle.
// - Provides a way to allocate a new region of platform shared memory of given
//   size.
// - Provides a way to create mapping of the region in the current process'
//   address space, under special access-control constraints (see Mode).
// - Provides methods to help transferring the handle across process boundaries.
// - Holds a 128-bit unique identifier used to uniquely identify the same
//   kernel region resource across processes (used for memory tracking).
// - Has a method to retrieve the region's size in bytes.
//
// IMPORTANT NOTE: Users should never use this directly, but
// ReadOnlySharedMemoryRegion, WritableSharedMemoryRegion or
// UnsafeSharedMemoryRegion since this is an implementation class.
class BASE_EXPORT PlatformSharedMemoryRegion {
 public:












  enum class Mode {
    kReadOnly,  // ReadOnlySharedMemoryRegion
    kWritable,  // WritableSharedMemoryRegion
    kUnsafe,    // UnsafeSharedMemoryRegion
    kMaxValue = kUnsafe
  };



  enum class CreateError {
    SUCCESS = 0,
    SIZE_ZERO = 1,
    SIZE_TOO_LARGE = 2,
    INITIALIZE_ACL_FAILURE = 3,
    INITIALIZE_SECURITY_DESC_FAILURE = 4,
    SET_SECURITY_DESC_FAILURE = 5,
    CREATE_FILE_MAPPING_FAILURE = 6,
    REDUCE_PERMISSIONS_FAILURE = 7,
    ALREADY_EXISTS = 8,
    ALLOCATE_FILE_REGION_FAILURE = 9,
    FSTAT_FAILURE = 10,
    INODES_MISMATCH = 11,
    GET_SHMEM_TEMP_DIR_FAILURE = 12,
    kMaxValue = GET_SHMEM_TEMP_DIR_FAILURE
  };

#if defined(OS_LINUX)

  struct ExecutableRegion {
   private:










    static ScopedFD CreateFD(size_t size);

    friend class content::SandboxIPCHandler;
  };
#endif

#if defined(OS_MACOSX) && !defined(OS_IOS)
  using PlatformHandle = mach_port_t;
  using ScopedPlatformHandle = mac::ScopedMachSendRight;
#elif defined(OS_FUCHSIA)
  using PlatformHandle = zx::unowned_vmo;
  using ScopedPlatformHandle = zx::vmo;
#elif defined(OS_WIN)
  using PlatformHandle = HANDLE;
  using ScopedPlatformHandle = win::ScopedHandle;
#elif defined(OS_ANDROID)
  using PlatformHandle = int;
  using ScopedPlatformHandle = ScopedFD;
#else
  using PlatformHandle = FDPair;
  using ScopedPlatformHandle = ScopedFDPair;
#endif


  enum { kMapMinimumAlignment = 32 };



  static PlatformSharedMemoryRegion CreateWritable(size_t size);
  static PlatformSharedMemoryRegion CreateUnsafe(size_t size);






  static PlatformSharedMemoryRegion Take(ScopedPlatformHandle handle,
                                         Mode mode,
                                         size_t size,
                                         const UnguessableToken& guid);
#if defined(OS_POSIX) && !defined(OS_ANDROID) && \
    !(defined(OS_MACOSX) && !defined(OS_IOS))


  static PlatformSharedMemoryRegion Take(ScopedFD handle,
                                         Mode mode,
                                         size_t size,
                                         const UnguessableToken& guid);
#endif


  PlatformSharedMemoryRegion();

  PlatformSharedMemoryRegion(PlatformSharedMemoryRegion&&);
  PlatformSharedMemoryRegion& operator=(PlatformSharedMemoryRegion&&);


  ~PlatformSharedMemoryRegion();




  ScopedPlatformHandle PassPlatformHandle() WARN_UNUSED_RESULT;


  PlatformHandle GetPlatformHandle() const;

  bool IsValid() const;





  PlatformSharedMemoryRegion Duplicate() const;




  bool ConvertToReadOnly();
#if defined(OS_MACOSX) && !defined(OS_IOS)




  bool ConvertToReadOnly(void* mapped_addr);
#endif  // defined(OS_MACOSX) && !defined(OS_IOS)




  bool ConvertToUnsafe();








  bool MapAt(off_t offset,
             size_t size,
             void** memory,
             size_t* mapped_size) const;

  const UnguessableToken& GetGUID() const { return guid_; }

  size_t GetSize() const { return size_; }

  Mode GetMode() const { return mode_; }

 private:
  FRIEND_TEST_ALL_PREFIXES(PlatformSharedMemoryRegionTest,
                           CreateReadOnlyRegionDeathTest);
  FRIEND_TEST_ALL_PREFIXES(PlatformSharedMemoryRegionTest,
                           CheckPlatformHandlePermissionsCorrespondToMode);
  static PlatformSharedMemoryRegion Create(Mode mode,
                                           size_t size
#if defined(OS_LINUX)
                                           ,
                                           bool executable = false
#endif
  );

  static bool CheckPlatformHandlePermissionsCorrespondToMode(
      PlatformHandle handle,
      Mode mode,
      size_t size);

  PlatformSharedMemoryRegion(ScopedPlatformHandle handle,
                             Mode mode,
                             size_t size,
                             const UnguessableToken& guid);

  bool MapAtInternal(off_t offset,
                     size_t size,
                     void** memory,
                     size_t* mapped_size) const;

  ScopedPlatformHandle handle_;
  Mode mode_ = Mode::kReadOnly;
  size_t size_ = 0;
  UnguessableToken guid_;

  DISALLOW_COPY_AND_ASSIGN(PlatformSharedMemoryRegion);
};

}  // namespace subtle
}  // namespace base

#endif  // BASE_MEMORY_PLATFORM_SHARED_MEMORY_REGION_H_
