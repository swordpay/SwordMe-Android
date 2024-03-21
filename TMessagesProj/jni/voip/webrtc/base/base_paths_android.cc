// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Android in base/path_service.cc.

#include <limits.h>
#include <unistd.h>

#include "base/android/jni_android.h"
#include "base/android/path_utils.h"
#include "base/base_paths.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/process/process_metrics.h"

namespace base {

bool PathProviderAndroid(int key, FilePath* result) {
  switch (key) {
    case base::FILE_EXE: {
      FilePath bin_dir;
      if (!ReadSymbolicLink(FilePath(kProcSelfExe), &bin_dir)) {
        NOTREACHED() << "Unable to resolve " << kProcSelfExe << ".";
        return false;
      }
      *result = bin_dir;
      return true;
    }
    case base::FILE_MODULE:

      NOTIMPLEMENTED();
      return false;
    case base::DIR_MODULE:
      return base::android::GetNativeLibraryDirectory(result);
    case base::DIR_SOURCE_ROOT:


      NOTIMPLEMENTED();
      return false;
    case base::DIR_USER_DESKTOP:

      NOTIMPLEMENTED();
      return false;
    case base::DIR_CACHE:
      return base::android::GetCacheDirectory(result);
    case base::DIR_ASSETS:



      return false;
    case base::DIR_ANDROID_APP_DATA:
      return base::android::GetDataDirectory(result);
    case base::DIR_ANDROID_EXTERNAL_STORAGE:
      return base::android::GetExternalStorageDirectory(result);
    default:



      return false;
  }
}

}  // namespace base
