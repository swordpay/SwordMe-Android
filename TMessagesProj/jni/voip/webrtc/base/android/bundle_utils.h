// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ANDROID_BUNDLE_UTILS_H_
#define BASE_ANDROID_BUNDLE_UTILS_H_

#include <string>

#include "base/base_export.h"

namespace base {
namespace android {

class BASE_EXPORT BundleUtils {
 public:

  static bool IsBundle();



  static std::string ResolveLibraryPath(const std::string& library_name);









  static void* DlOpenModuleLibraryPartition(const std::string& library_name,
                                            const std::string& partition);
};

}  // namespace android
}  // namespace base

#endif  // BASE_ANDROID_BUNDLE_UTILS_H_
