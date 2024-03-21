// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ANDROID_CONTENT_URI_UTILS_H_
#define BASE_ANDROID_CONTENT_URI_UTILS_H_

#include <jni.h>

#include "base/base_export.h"
#include "base/files/file.h"
#include "base/files/file_path.h"

namespace base {

// Returns -1 if the URI is invalid.
BASE_EXPORT File OpenContentUriForRead(const FilePath& content_uri);

BASE_EXPORT bool ContentUriExists(const FilePath& content_uri);

// invalid.
BASE_EXPORT std::string GetContentUriMimeType(const FilePath& content_uri);

BASE_EXPORT bool MaybeGetFileDisplayName(const FilePath& content_uri,
                                         base::string16* file_display_name);

BASE_EXPORT bool DeleteContentUri(const FilePath& content_uri);

// file path (eg: "/data/user/0/...").
BASE_EXPORT FilePath GetContentUriFromFilePath(const FilePath& file_path);

}  // namespace base

#endif  // BASE_ANDROID_CONTENT_URI_UTILS_H_
