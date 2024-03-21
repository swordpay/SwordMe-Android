/*
 * Copyright 2014-2016 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright (c) 2014, Intel Corporation. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 *
 * Originally written by Shay Gueron (1, 2), and Vlad Krasnov (1)
 * (1) Intel Corporation, Israel Development Center, Haifa, Israel
 * (2) University of Haifa, Israel
 *
 * Reference:
 * S.Gueron and V.Krasnov, "Fast Prime Field Elliptic Curve Cryptography with
 *                          256 Bit Primes"
 */

#ifndef OPENSSL_HEADER_EC_P256_X86_64_H
#define OPENSSL_HEADER_EC_P256_X86_64_H

#include <openssl/base.h>

#include <openssl/bn.h>

#include "../bn/internal.h"

#if defined(__cplusplus)
extern "C" {
#endif


#if !defined(OPENSSL_NO_ASM) && defined(OPENSSL_X86_64) && \
    !defined(OPENSSL_SMALL)

//
// An element mod P in P-256 is represented as a little-endian array of
// |P256_LIMBS| |BN_ULONG|s, spanning the full range of values.
//
// The following functions take fully-reduced inputs mod P and give
// fully-reduced outputs. They may be used in-place.

#define P256_LIMBS (256 / BN_BITS2)

void ecp_nistz256_neg(BN_ULONG res[P256_LIMBS], const BN_ULONG a[P256_LIMBS]);

void ecp_nistz256_mul_mont(BN_ULONG res[P256_LIMBS],
                           const BN_ULONG a[P256_LIMBS],
                           const BN_ULONG b[P256_LIMBS]);

void ecp_nistz256_sqr_mont(BN_ULONG res[P256_LIMBS],
                           const BN_ULONG a[P256_LIMBS]);

// by multiplying with 1.
static inline void ecp_nistz256_from_mont(BN_ULONG res[P256_LIMBS],
                                          const BN_ULONG in[P256_LIMBS]) {
  static const BN_ULONG ONE[P256_LIMBS] = { 1 };
  ecp_nistz256_mul_mont(res, in, ONE);
}

// by multiplying with RR = 2^512 mod P precomputed for NIST P256 curve.
static inline void ecp_nistz256_to_mont(BN_ULONG res[P256_LIMBS],
                                        const BN_ULONG in[P256_LIMBS]) {
  static const BN_ULONG RR[P256_LIMBS] = {
      TOBN(0x00000000, 0x00000003), TOBN(0xfffffffb, 0xffffffff),
      TOBN(0xffffffff, 0xfffffffe), TOBN(0x00000004, 0xfffffffd)};
  ecp_nistz256_mul_mont(res, in, RR);
}

//
// The following functions compute modulo N, where N is the order of P-256. They
// take fully-reduced inputs and give fully-reduced outputs.

// are in Montgomery form. That is, |res| is |a| * |b| * 2^-256 mod N.
void ecp_nistz256_ord_mul_mont(BN_ULONG res[P256_LIMBS],
                               const BN_ULONG a[P256_LIMBS],
                               const BN_ULONG b[P256_LIMBS]);

// outputs are in Montgomery form. That is, |res| is
// (|a| * 2^-256)^(2*|rep|) * 2^256 mod N.
void ecp_nistz256_ord_sqr_mont(BN_ULONG res[P256_LIMBS],
                               const BN_ULONG a[P256_LIMBS], BN_ULONG rep);

// Assumption: 0 < a < p < 2^(256) and p is odd.
int beeu_mod_inverse_vartime(BN_ULONG out[P256_LIMBS],
                             const BN_ULONG a[P256_LIMBS],
                             const BN_ULONG p[P256_LIMBS]);

//
// The following functions may be used in-place. All coordinates are in the
// Montgomery domain.

typedef struct {
  BN_ULONG X[P256_LIMBS];
  BN_ULONG Y[P256_LIMBS];
  BN_ULONG Z[P256_LIMBS];
} P256_POINT;

// is encoded as (0, 0).
typedef struct {
  BN_ULONG X[P256_LIMBS];
  BN_ULONG Y[P256_LIMBS];
} P256_POINT_AFFINE;

// and all zeros (the point at infinity) if |index| is 0. This is done in
// constant time.
void ecp_nistz256_select_w5(P256_POINT *val, const P256_POINT in_t[16],
                            int index);

// and all zeros (the point at infinity) if |index| is 0. This is done in
// constant time.
void ecp_nistz256_select_w7(P256_POINT_AFFINE *val,
                            const P256_POINT_AFFINE in_t[64], int index);

void ecp_nistz256_point_double(P256_POINT *r, const P256_POINT *a);

void ecp_nistz256_point_add(P256_POINT *r, const P256_POINT *a,
                            const P256_POINT *b);

// |r|. |a| and |b| must not represent the same point unless they are both
// infinity.
void ecp_nistz256_point_add_affine(P256_POINT *r, const P256_POINT *a,
                                   const P256_POINT_AFFINE *b);

#endif /* !defined(OPENSSL_NO_ASM) && defined(OPENSSL_X86_64) && \
           !defined(OPENSSL_SMALL) */


#if defined(__cplusplus)
}  // extern C++
#endif

#endif  // OPENSSL_HEADER_EC_P256_X86_64_H
