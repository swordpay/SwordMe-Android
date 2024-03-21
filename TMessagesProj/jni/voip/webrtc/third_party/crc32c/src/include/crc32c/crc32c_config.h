// Copyright 2017 The CRC32C Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file. See the AUTHORS file for names of contributors.

#ifndef CRC32C_CRC32C_CONFIG_H_
#define CRC32C_CRC32C_CONFIG_H_

//#cmakedefine01 BYTE_ORDER_BIG_ENDIAN

#define HAVE_BUILTIN_PREFETCH 1


#if HAVE_SSE42 && (defined(_M_X64) || defined(__x86_64__))
#define HAVE_MM_PREFETCH 1
#endif

// intrinsics.
#if defined(__i386) || defined(__x86_64) || defined(_M_IX86)
//#define HAVE_SSE42 1
#endif

// the vmull_p64 intrinsics.
#if defined(__aarch64__)
//#define HAVE_ARM64_CRC32C 1
#endif

// <sys/auxv.h> header. Should be true on Linux and Android API level 20+.
#define HAVE_STRONG_GETAUXVAL 1

// Should be true for any compiler that supports __attribute__((weak)).
#define HAVE_WEAK_GETAUXVAL

//#cmakedefine01 CRC32C_TESTS_BUILT_WITH_GLOG

#endif  // CRC32C_CRC32C_CONFIG_H_
