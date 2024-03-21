// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ANDROID_PATH_UTILS_H_
#define BASE_ANDROID_PATH_UTILS_H_

#include <jni.h>
#include <vector>

#include "base/base_export.h"

namespace base {

class FilePath;

namespace android {

// application. The result is placed in the FilePath pointed to by 'result'.
// This method is dedicated for base_paths_android.c, Using
// PathService::Get(base::DIR_ANDROID_APP_DATA, ...) gets the data dir.
BASE_EXPORT bool GetDataDirectory(FilePath* result);

// the FilePath pointed to by 'result'. This method is dedicated for
// base_paths_android.c, Using PathService::Get(base::DIR_CACHE, ...) gets the
// cache dir.
BASE_EXPORT bool GetCacheDirectory(FilePath* result);

// in the FilePath pointed to by 'result'.
BASE_EXPORT bool GetThumbnailCacheDirectory(FilePath* result);

// in the FilePath pointed to by 'result'.
BASE_EXPORT bool GetDownloadsDirectory(FilePath* result);

// directory, and a private directory on external SD card.
BASE_EXPORT std::vector<FilePath> GetAllPrivateDownloadsDirectories();

// ApplicationInfo.nativeLibraryDir on the Java side. The result is placed in
// the FilePath pointed to by 'result'.
BASE_EXPORT bool GetNativeLibraryDirectory(FilePath* result);

// is placed in the FilePath pointed to by 'result'.
BASE_EXPORT bool GetExternalStorageDirectory(FilePath* result);

}  // namespace android
}  // namespace base

#endif  // BASE_ANDROID_PATH_UTILS_H_
