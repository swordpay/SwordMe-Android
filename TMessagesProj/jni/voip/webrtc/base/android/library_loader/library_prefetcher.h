// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ANDROID_LIBRARY_LOADER_LIBRARY_PREFETCHER_H_
#define BASE_ANDROID_LIBRARY_LOADER_LIBRARY_PREFETCHER_H_

#include <jni.h>

#include <stdint.h>
#include <string>

#include "base/android/library_loader/anchor_functions_buildflags.h"
#include "base/base_export.h"
#include "base/gtest_prod_util.h"
#include "base/macros.h"

#if BUILDFLAG(SUPPORTS_CODE_ORDERING)

namespace base {
namespace android {

// a forked process for the following reasons:
// - Isolating the main process from mistakes in getting the address range, only
//   crashing the forked process in case of mistake.
// - Not inflating the memory used by the main process uselessly, which could
//   increase its likelihood to be killed.
// The forked process has background priority and, since it is not declared to
// the Android runtime, can be killed at any time, which is not an issue here.
class BASE_EXPORT NativeLibraryPrefetcher {
 public:




  static void ForkAndPrefetchNativeLibrary(bool ordered_only);


  static int PercentageOfResidentNativeLibraryCode();


  static void PeriodicallyCollectResidency();


  static void MadviseForOrderfile();


  static void MadviseForResidencyCollection();

 private:


  static int PercentageOfResidentCode(size_t start, size_t end);

  FRIEND_TEST_ALL_PREFIXES(NativeLibraryPrefetcherTest,
                           TestPercentageOfResidentCode);

  DISALLOW_IMPLICIT_CONSTRUCTORS(NativeLibraryPrefetcher);
};

}  // namespace android
}  // namespace base

#endif  // BUILDFLAG(SUPPORTS_CODE_ORDERING)

#endif  // BASE_ANDROID_LIBRARY_LOADER_LIBRARY_PREFETCHER_H_
