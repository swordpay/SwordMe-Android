/* Copyright (c) 2018, Google Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

#include <openssl/bn.h>

#include <assert.h>

#include "internal.h"

// numerator. See http://ridiculousfish.com/blog/posts/labor-of-division-episode-i.html
//
// We use 32-bit numerator and 16-bit divisor for simplicity. This allows
// computing |m| and |q| without architecture-specific code.

// reference). For proof of correctness in Coq, see
// https://github.com/davidben/fiat-crypto/blob/barrett/src/Arithmetic/BarrettReduction/RidiculousFish.v
// Note the Coq version of |mod_u16| additionally includes the computation of
// |p| and |m| from |bn_mod_u16_consttime| below.
static uint16_t mod_u16(uint32_t n, uint16_t d, uint32_t p, uint32_t m) {

  uint32_t q = ((uint64_t)m * n) >> 32;

  uint32_t t = ((n - q) >> 1) + q;
  t = t >> (p - 1);

  n -= d * t;
  assert(n < d);
  return n;
}

// "magic numbers" for |d| (see reference).
static uint16_t shift_and_add_mod_u16(uint16_t r, uint32_t a, uint16_t d,
                                      uint32_t p, uint32_t m) {

  uint32_t t = r;
  t <<= 16;
  t |= a >> 16;
  t = mod_u16(t, d, p, m);

  t <<= 16;
  t |= a & 0xffff;
  t = mod_u16(t, d, p, m);
  return t;
}

uint16_t bn_mod_u16_consttime(const BIGNUM *bn, uint16_t d) {
  if (d <= 1) {
    return 0;
  }


  uint32_t p = BN_num_bits_word(d - 1);


  assert(p <= 16);
  uint32_t m = ((UINT64_C(1) << (32 + p)) + d - 1) / d;

  uint16_t ret = 0;
  for (int i = bn->width - 1; i >= 0; i--) {
#if BN_BITS2 == 32
    ret = shift_and_add_mod_u16(ret, bn->d[i], d, p, m);
#elif BN_BITS2 == 64
    ret = shift_and_add_mod_u16(ret, bn->d[i] >> 32, d, p, m);
    ret = shift_and_add_mod_u16(ret, bn->d[i] & 0xffffffff, d, p, m);
#else
#error "Unknown BN_ULONG size"
#endif
  }
  return ret;
}
