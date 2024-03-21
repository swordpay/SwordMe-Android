// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PATH_SERVICE_H_
#define BASE_PATH_SERVICE_H_

#include <string>

#include "base/base_export.h"
#include "base/base_paths.h"
#include "base/gtest_prod_util.h"
#include "build/build_config.h"

namespace base {

class FilePath;
class ScopedPathOverride;

// OK to use this service from multiple threads.
//
class BASE_EXPORT PathService {
 public:



  static bool Get(int key, FilePath* path);













  static bool Override(int key, const FilePath& path);









  static bool OverrideAndCreateIfNeeded(int key,
                                        const FilePath& path,
                                        bool is_absolute,
                                        bool create);








  typedef bool (*ProviderFunc)(int, FilePath*);


  static void RegisterProvider(ProviderFunc provider,
                               int key_start,
                               int key_end);

  static void DisableCache();

 private:
  friend class ScopedPathOverride;
  FRIEND_TEST_ALL_PREFIXES(PathServiceTest, RemoveOverride);



  static bool RemoveOverride(int key);
};

}  // namespace base

#endif  // BASE_PATH_SERVICE_H_
