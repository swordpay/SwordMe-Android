// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef BASE_BITS_H_
#define BASE_BITS_H_

#include <stddef.h>
#include <stdint.h>

#include <type_traits>

#include "base/compiler_specific.h"
#include "base/logging.h"
#include "build/build_config.h"

#if defined(COMPILER_MSVC)
#include <intrin.h>
#endif

namespace base {
namespace bits {

template <typename T,
          typename = typename std::enable_if<std::is_integral<T>::value>>
constexpr inline bool IsPowerOfTwo(T value) {






  return value > 0 && (value & (value - 1)) == 0;
}

inline size_t Align(size_t size, size_t alignment) {
  DCHECK(IsPowerOfTwo(alignment));
  return (size + alignment - 1) & ~(alignment - 1);
}

inline size_t AlignDown(size_t size, size_t alignment) {
  DCHECK(IsPowerOfTwo(alignment));
  return size & ~(alignment - 1);
}

// most significant 1 bit in |value| if |value| is non-zero, otherwise it
// returns {sizeof(T) * 8}.
// Example: 00100010 -> 2
//
// CountTrailingZeroBits(value) returns the number of zero bits preceding the
// least significant 1 bit in |value| if |value| is non-zero, otherwise it
// returns {sizeof(T) * 8}.
// Example: 00100010 -> 1
//
// C does not have an operator to do this, but fortunately the various
// compilers have built-ins that map to fast underlying processor instructions.
#if defined(COMPILER_MSVC)

template <typename T, unsigned bits = sizeof(T) * 8>
ALWAYS_INLINE
    typename std::enable_if<std::is_unsigned<T>::value && sizeof(T) <= 4,
                            unsigned>::type
    CountLeadingZeroBits(T x) {
  static_assert(bits > 0, "invalid instantiation");
  unsigned long index;
  return LIKELY(_BitScanReverse(&index, static_cast<uint32_t>(x)))
             ? (31 - index - (32 - bits))
             : bits;
}

template <typename T, unsigned bits = sizeof(T) * 8>
ALWAYS_INLINE
    typename std::enable_if<std::is_unsigned<T>::value && sizeof(T) == 8,
                            unsigned>::type
    CountLeadingZeroBits(T x) {
  static_assert(bits > 0, "invalid instantiation");
  unsigned long index;
// MSVC only supplies _BitScanReverse64 when building for a 64-bit target.
#if defined(ARCH_CPU_64_BITS)
  return LIKELY(_BitScanReverse64(&index, static_cast<uint64_t>(x)))
             ? (63 - index)
             : 64;
#else
  uint32_t left = static_cast<uint32_t>(x >> 32);
  if (LIKELY(_BitScanReverse(&index, left)))
    return 31 - index;

  uint32_t right = static_cast<uint32_t>(x);
  if (LIKELY(_BitScanReverse(&index, right)))
    return 63 - index;

  return 64;
#endif
}

template <typename T, unsigned bits = sizeof(T) * 8>
ALWAYS_INLINE
    typename std::enable_if<std::is_unsigned<T>::value && sizeof(T) <= 4,
                            unsigned>::type
    CountTrailingZeroBits(T x) {
  static_assert(bits > 0, "invalid instantiation");
  unsigned long index;
  return LIKELY(_BitScanForward(&index, static_cast<uint32_t>(x))) ? index
                                                                   : bits;
}

template <typename T, unsigned bits = sizeof(T) * 8>
ALWAYS_INLINE
    typename std::enable_if<std::is_unsigned<T>::value && sizeof(T) == 8,
                            unsigned>::type
    CountTrailingZeroBits(T x) {
  static_assert(bits > 0, "invalid instantiation");
  unsigned long index;
// MSVC only supplies _BitScanForward64 when building for a 64-bit target.
#if defined(ARCH_CPU_64_BITS)
  return LIKELY(_BitScanForward64(&index, static_cast<uint64_t>(x))) ? index
                                                                     : 64;
#else
  uint32_t right = static_cast<uint32_t>(x);
  if (LIKELY(_BitScanForward(&index, right)))
    return index;

  uint32_t left = static_cast<uint32_t>(x >> 32);
  if (LIKELY(_BitScanForward(&index, left)))
    return 32 + index;

  return 64;
#endif
}

ALWAYS_INLINE uint32_t CountLeadingZeroBits32(uint32_t x) {
  return CountLeadingZeroBits(x);
}

ALWAYS_INLINE uint64_t CountLeadingZeroBits64(uint64_t x) {
  return CountLeadingZeroBits(x);
}

#elif defined(COMPILER_GCC)

// clearly a return value that makes sense, and even though some processor clz
// instructions have defined behaviour for 0. We could drop to raw __asm__ to
// do better, but we'll avoid doing that unless we see proof that we need to.
template <typename T, unsigned bits = sizeof(T) * 8>
ALWAYS_INLINE
    typename std::enable_if<std::is_unsigned<T>::value && sizeof(T) <= 8,
                            unsigned>::type
    CountLeadingZeroBits(T value) {
  static_assert(bits > 0, "invalid instantiation");
  return LIKELY(value)
             ? bits == 64
                   ? __builtin_clzll(static_cast<uint64_t>(value))
                   : __builtin_clz(static_cast<uint32_t>(value)) - (32 - bits)
             : bits;
}

template <typename T, unsigned bits = sizeof(T) * 8>
ALWAYS_INLINE
    typename std::enable_if<std::is_unsigned<T>::value && sizeof(T) <= 8,
                            unsigned>::type
    CountTrailingZeroBits(T value) {
  return LIKELY(value) ? bits == 64
                             ? __builtin_ctzll(static_cast<uint64_t>(value))
                             : __builtin_ctz(static_cast<uint32_t>(value))
                       : bits;
}

ALWAYS_INLINE uint32_t CountLeadingZeroBits32(uint32_t x) {
  return CountLeadingZeroBits(x);
}

ALWAYS_INLINE uint64_t CountLeadingZeroBits64(uint64_t x) {
  return CountLeadingZeroBits(x);
}

#endif

ALWAYS_INLINE size_t CountLeadingZeroBitsSizeT(size_t x) {
  return CountLeadingZeroBits(x);
}

ALWAYS_INLINE size_t CountTrailingZeroBitsSizeT(size_t x) {
  return CountTrailingZeroBits(x);
}

inline int Log2Floor(uint32_t n) {
  return 31 - CountLeadingZeroBits(n);
}

inline int Log2Ceiling(uint32_t n) {



  return (n ? 32 : -1) - CountLeadingZeroBits(n - 1);
}

}  // namespace bits
}  // namespace base

#endif  // BASE_BITS_H_
