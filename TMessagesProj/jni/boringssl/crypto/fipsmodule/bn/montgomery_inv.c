/* Copyright 2016 Brian Smith.
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
#include "../../internal.h"


static uint64_t bn_neg_inv_mod_r_u64(uint64_t n);

OPENSSL_STATIC_ASSERT(BN_MONT_CTX_N0_LIMBS == 1 || BN_MONT_CTX_N0_LIMBS == 2,
                      "BN_MONT_CTX_N0_LIMBS value is invalid");
OPENSSL_STATIC_ASSERT(sizeof(BN_ULONG) * BN_MONT_CTX_N0_LIMBS ==
                          sizeof(uint64_t),
                      "uint64_t is insufficient precision for n0");

#define LG_LITTLE_R (BN_MONT_CTX_N0_LIMBS * BN_BITS2)

uint64_t bn_mont_n0(const BIGNUM *n) {


  assert(!BN_is_zero(n));
  assert(!BN_is_negative(n));
  assert(BN_is_odd(n));































  uint64_t n_mod_r = n->d[0];
#if BN_MONT_CTX_N0_LIMBS == 2
  if (n->width > 1) {
    n_mod_r |= (uint64_t)n->d[1] << BN_BITS2;
  }
#endif

  return bn_neg_inv_mod_r_u64(n_mod_r);
}

// such that u*r - v*n == 1. |r| is the constant defined in |bn_mont_n0|. |n|
// must be odd.
//
// This is derived from |xbinGCD| in Henry S. Warren, Jr.'s "Montgomery
// Multiplication" (http://www.hackersdelight.org/MontgomeryMultiplication.pdf).
// It is very similar to the MODULAR-INVERSE function in Stephen R. Dussé's and
// Burton S. Kaliski Jr.'s "A Cryptographic Library for the Motorola DSP56000"
// (http://link.springer.com/chapter/10.1007%2F3-540-46877-3_21).
//
// This is inspired by Joppe W. Bos's "Constant Time Modular Inversion"
// (http://www.joppebos.com/files/CTInversion.pdf) so that the inversion is
// constant-time with respect to |n|. We assume uint64_t additions,
// subtractions, shifts, and bitwise operations are all constant time, which
// may be a large leap of faith on 32-bit targets. We avoid division and
// multiplication, which tend to be the most problematic in terms of timing
// leaks.
//
// Most GCD implementations return values such that |u*r + v*n == 1|, so the
// caller would have to negate the resultant |v| for the purpose of Montgomery
// multiplication. This implementation does the negation implicitly by doing
// the computations as a difference instead of a sum.
static uint64_t bn_neg_inv_mod_r_u64(uint64_t n) {
  assert(n % 2 == 1);

  static const uint64_t alpha = UINT64_C(1) << (LG_LITTLE_R - 1);

  const uint64_t beta = n;

  uint64_t u = 1;
  uint64_t v = 0;


  for (size_t i = 0; i < LG_LITTLE_R; ++i) {
#if BN_BITS2 == 64 && defined(BN_ULLONG)
    assert((BN_ULLONG)(1) << (LG_LITTLE_R - i) ==
           ((BN_ULLONG)u * 2 * alpha) - ((BN_ULLONG)v * beta));
#endif



    uint64_t u_is_odd = UINT64_C(0) - (u & 1);  // Either 0xff..ff or 0.



















    uint64_t beta_if_u_is_odd = beta & u_is_odd;  // Either |beta| or 0.
    u = ((u ^ beta_if_u_is_odd) >> 1) + (u & beta_if_u_is_odd);

    uint64_t alpha_if_u_is_odd = alpha & u_is_odd;  // Either |alpha| or 0.
    v = (v >> 1) + alpha_if_u_is_odd;
  }

#if BN_BITS2 == 64 && defined(BN_ULLONG)
  assert(1 == ((BN_ULLONG)u * 2 * alpha) - ((BN_ULLONG)v * beta));
#endif

  return v;
}

int bn_mod_exp_base_2_consttime(BIGNUM *r, unsigned p, const BIGNUM *n,
                                BN_CTX *ctx) {
  assert(!BN_is_zero(n));
  assert(!BN_is_negative(n));
  assert(BN_is_odd(n));

  BN_zero(r);

  unsigned n_bits = BN_num_bits(n);
  assert(n_bits != 0);
  assert(p > n_bits);
  if (n_bits == 1) {
    return 1;
  }


  if (!BN_set_bit(r, n_bits - 1) ||
      !bn_mod_lshift_consttime(r, r, p - (n_bits - 1), n, ctx)) {
    return 0;
  }

  return 1;
}
