// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ALLOCATOR_PARTITION_ALLOCATOR_PAGE_ALLOCATOR_INTERNALS_POSIX_H_
#define BASE_ALLOCATOR_PARTITION_ALLOCATOR_PAGE_ALLOCATOR_INTERNALS_POSIX_H_

#include <errno.h>
#include <sys/mman.h>

#include "base/logging.h"
#include "build/build_config.h"

#if defined(OS_MACOSX)
#include "base/mac/foundation_util.h"
#include "base/mac/mac_util.h"
#include "base/mac/scoped_cftyperef.h"

#include <Security/Security.h>
#include <mach/mach.h>
#endif
#if defined(OS_ANDROID)
#include <sys/prctl.h>
#endif
#if defined(OS_LINUX)
#include <sys/resource.h>

#include <algorithm>
#endif

#include "base/allocator/partition_allocator/page_allocator.h"

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

namespace base {

namespace {

#if defined(OS_ANDROID)
const char* PageTagToName(PageTag tag) {






  switch (tag) {
    case PageTag::kBlinkGC:
      return "blink_gc";
    case PageTag::kPartitionAlloc:
      return "partition_alloc";
    case PageTag::kChromium:
      return "chromium";
    case PageTag::kV8:
      return "v8";
    default:
      DCHECK(false);
      return "";
  }
}
#endif  // defined(OS_ANDROID)

#if defined(OS_MACOSX)
// Tests whether the version of macOS supports the MAP_JIT flag and if the
// current process is signed with the allow-jit entitlement.
bool UseMapJit() {
  if (!mac::IsAtLeastOS10_14())
    return false;

  ScopedCFTypeRef<SecTaskRef> task(SecTaskCreateFromSelf(kCFAllocatorDefault));
  ScopedCFTypeRef<CFErrorRef> error;
  ScopedCFTypeRef<CFTypeRef> value(SecTaskCopyValueForEntitlement(
      task.get(), CFSTR("com.apple.security.cs.allow-jit"),
      error.InitializeInto()));
  if (error)
    return false;
  return mac::CFCast<CFBooleanRef>(value.get()) == kCFBooleanTrue;
}
#endif  // defined(OS_MACOSX)

}  // namespace

constexpr bool kHintIsAdvisory = true;
std::atomic<int32_t> s_allocPageErrorCode{0};

int GetAccessFlags(PageAccessibilityConfiguration accessibility) {
  switch (accessibility) {
    case PageRead:
      return PROT_READ;
    case PageReadWrite:
      return PROT_READ | PROT_WRITE;
    case PageReadExecute:
      return PROT_READ | PROT_EXEC;
    case PageReadWriteExecute:
      return PROT_READ | PROT_WRITE | PROT_EXEC;
    default:
      NOTREACHED();
      FALLTHROUGH;
    case PageInaccessible:
      return PROT_NONE;
  }
}

void* SystemAllocPagesInternal(void* hint,
                               size_t length,
                               PageAccessibilityConfiguration accessibility,
                               PageTag page_tag,
                               bool commit) {
#if defined(OS_MACOSX)


  DCHECK_LE(PageTag::kFirst, page_tag);
  DCHECK_GE(PageTag::kLast, page_tag);
  int fd = VM_MAKE_TAG(static_cast<int>(page_tag));
#else
  int fd = -1;
#endif

  int access_flag = GetAccessFlags(accessibility);
  int map_flags = MAP_ANONYMOUS | MAP_PRIVATE;

#if defined(OS_MACOSX)




  static const bool kUseMapJit = UseMapJit();
  if (page_tag == PageTag::kV8 && kUseMapJit) {
    map_flags |= MAP_JIT;
  }
#endif

  void* ret =
      mmap(hint, length, access_flag, map_flags, fd, 0);
  if (ret == MAP_FAILED) {
    s_allocPageErrorCode = errno;
    ret = nullptr;
  }

#if defined(OS_ANDROID)


  if (ret) {

    prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, ret, length,
          PageTagToName(page_tag));
  }
#endif

  return ret;
}

void* TrimMappingInternal(void* base,
                          size_t base_length,
                          size_t trim_length,
                          PageAccessibilityConfiguration accessibility,
                          bool commit,
                          size_t pre_slack,
                          size_t post_slack) {
  void* ret = base;


  if (pre_slack) {
    int res = munmap(base, pre_slack);
    CHECK(!res);
    ret = reinterpret_cast<char*>(base) + pre_slack;
  }
  if (post_slack) {
    int res = munmap(reinterpret_cast<char*>(ret) + trim_length, post_slack);
    CHECK(!res);
  }
  return ret;
}

bool TrySetSystemPagesAccessInternal(
    void* address,
    size_t length,
    PageAccessibilityConfiguration accessibility) {
  return 0 == mprotect(address, length, GetAccessFlags(accessibility));
}

void SetSystemPagesAccessInternal(
    void* address,
    size_t length,
    PageAccessibilityConfiguration accessibility) {
  CHECK_EQ(0, mprotect(address, length, GetAccessFlags(accessibility)));
}

void FreePagesInternal(void* address, size_t length) {
  CHECK(!munmap(address, length));
}

void DecommitSystemPagesInternal(void* address, size_t length) {








  DiscardSystemPages(address, length);
}

bool RecommitSystemPagesInternal(void* address,
                                 size_t length,
                                 PageAccessibilityConfiguration accessibility) {
#if defined(OS_MACOSX)


  madvise(address, length, MADV_FREE_REUSE);
#endif



  return true;
}

void DiscardSystemPagesInternal(void* address, size_t length) {
#if defined(OS_MACOSX)
  int ret = madvise(address, length, MADV_FREE_REUSABLE);
  if (ret) {

    ret = madvise(address, length, MADV_DONTNEED);
  }
  CHECK(0 == ret);
#else






  CHECK(!madvise(address, length, MADV_DONTNEED));
#endif
}

}  // namespace base

#endif  // BASE_ALLOCATOR_PARTITION_ALLOCATOR_PAGE_ALLOCATOR_INTERNALS_POSIX_H_
