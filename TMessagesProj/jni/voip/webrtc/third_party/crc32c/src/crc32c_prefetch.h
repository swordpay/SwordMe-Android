// Copyright 2017 The CRC32C Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef CRC32C_CRC32C_PREFETCH_H_
#define CRC32C_CRC32C_PREFETCH_H_

#include <cstddef>
#include <cstdint>

#include "crc32c/crc32c_config.h"

#if HAVE_MM_PREFETCH

#if defined(_MSC_VER)
#include <intrin.h>
#else  // !defined(_MSC_VER)
#include <xmmintrin.h>
#endif  // defined(_MSC_VER)

#endif  // HAVE_MM_PREFETCH

namespace crc32c {

inline void RequestPrefetch(const uint8_t* address) {
#if HAVE_BUILTIN_PREFETCH


  __builtin_prefetch(reinterpret_cast<const char*>(address), 0 /* Read only. */,
                     0 /* No temporal locality. */);
#elif HAVE_MM_PREFETCH


  _mm_prefetch(reinterpret_cast<const char*>(address), _MM_HINT_NTA);
#else

  (void)address;
#endif  // HAVE_BUILTIN_PREFETCH
}

}  // namespace crc32c

#endif  // CRC32C_CRC32C_ROUND_UP_H_
