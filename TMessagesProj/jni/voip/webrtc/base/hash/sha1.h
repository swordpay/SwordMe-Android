// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_HASH_SHA1_H_
#define BASE_HASH_SHA1_H_

#include <stddef.h>

#include <array>
#include <string>

#include "base/base_export.h"
#include "base/containers/span.h"

namespace base {


enum { kSHA1Length = 20 };  // Length in bytes of a SHA-1 hash.

using SHA1Digest = std::array<uint8_t, kSHA1Length>;

BASE_EXPORT SHA1Digest SHA1HashSpan(span<const uint8_t> data);

// hash.
BASE_EXPORT std::string SHA1HashString(const std::string& str);

// in |hash|. |hash| must be kSHA1Length bytes long.
BASE_EXPORT void SHA1HashBytes(const unsigned char* data,
                               size_t len,
                               unsigned char* hash);

}  // namespace base

#endif  // BASE_HASH_SHA1_H_
