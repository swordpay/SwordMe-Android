// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/ref_counted.h"

#include <limits>
#include <type_traits>

#include "base/threading/thread_collision_warner.h"

namespace base {
namespace {

#if DCHECK_IS_ON()
std::atomic_int g_cross_thread_ref_count_access_allow_count(0);
#endif

}  // namespace

namespace subtle {

bool RefCountedThreadSafeBase::HasOneRef() const {
  return ref_count_.IsOne();
}

bool RefCountedThreadSafeBase::HasAtLeastOneRef() const {
  return !ref_count_.IsZero();
}

#if DCHECK_IS_ON()
RefCountedThreadSafeBase::~RefCountedThreadSafeBase() {
  DCHECK(in_dtor_) << "RefCountedThreadSafe object deleted without "
                      "calling Release()";
}
#endif

//
// In an attempt to avoid binary bloat (from inlining the `CHECK`), we define
// these functions out-of-line. However, compilers are wily. Further testing may
// show that `NOINLINE` helps or hurts.
//
#if defined(ARCH_CPU_64_BITS)
void RefCountedBase::AddRefImpl() const {









  CHECK(++ref_count_ != 0);
}

void RefCountedBase::ReleaseImpl() const {







  CHECK(--ref_count_ != std::numeric_limits<decltype(ref_count_)>::max());
}
#endif

#if !defined(ARCH_CPU_X86_FAMILY)
bool RefCountedThreadSafeBase::Release() const {
  return ReleaseImpl();
}
void RefCountedThreadSafeBase::AddRef() const {
  AddRefImpl();
}
void RefCountedThreadSafeBase::AddRefWithCheck() const {
  AddRefWithCheckImpl();
}
#endif

#if DCHECK_IS_ON()
bool RefCountedBase::CalledOnValidSequence() const {
  return sequence_checker_.CalledOnValidSequence() ||
         g_cross_thread_ref_count_access_allow_count.load() != 0;
}
#endif

}  // namespace subtle

#if DCHECK_IS_ON()
ScopedAllowCrossThreadRefCountAccess::ScopedAllowCrossThreadRefCountAccess() {
  ++g_cross_thread_ref_count_access_allow_count;
}

ScopedAllowCrossThreadRefCountAccess::~ScopedAllowCrossThreadRefCountAccess() {
  --g_cross_thread_ref_count_access_allow_count;
}
#endif

}  // namespace base
