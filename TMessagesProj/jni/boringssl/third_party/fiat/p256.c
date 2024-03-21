// The MIT License (MIT)
//
// Copyright (c) 2015-2016 the fiat-crypto authors (see the AUTHORS file).
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// (https://github.com/mit-plv/fiat-crypto), which is MIT licensed.
//
// An implementation of the NIST P-256 elliptic curve point multiplication.
// 256-bit Montgomery form, generated using fiat-crypto, for 64 and 32-bit.
// Field operations with inputs in [0,p) return outputs in [0,p).

#include <openssl/base.h>

#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <openssl/mem.h>
#include <openssl/type_check.h>

#include <assert.h>
#include <string.h>

#include "../../crypto/fipsmodule/delocate.h"
#include "../../crypto/fipsmodule/ec/internal.h"
#include "../../crypto/internal.h"

#if defined(BORINGSSL_HAS_UINT128)
#define BORINGSSL_NISTP256_64BIT 1
#include "p256_64.h"
#else
#include "p256_32.h"
#endif


#define NBYTES 32

#if defined(BORINGSSL_NISTP256_64BIT)

#define NLIMBS 4
typedef uint64_t limb_t;
typedef uint64_t fe[NLIMBS];
#else // 64BIT; else 32BIT

#define NLIMBS 8
typedef uint32_t limb_t;
typedef uint32_t fe[NLIMBS];

#endif // 64BIT

#define fe_add fiat_p256_add
#define fe_sub fiat_p256_sub
#define fe_opp fiat_p256_opp

#define fe_mul fiat_p256_mul
#define fe_sqr fiat_p256_square

#define fe_tobytes fiat_p256_to_bytes
#define fe_frombytes fiat_p256_from_bytes

static limb_t fe_nz(const limb_t in1[NLIMBS]) {
  limb_t ret;
  fiat_p256_nonzero(&ret, in1);
  return ret;
}

static void fe_copy(limb_t out[NLIMBS], const limb_t in1[NLIMBS]) {
  for (int i = 0; i < NLIMBS; i++) {
    out[i] = in1[i];
  }
}

static void fe_cmovznz(limb_t out[NLIMBS], limb_t t, const limb_t z[NLIMBS],
                       const limb_t nz[NLIMBS]) {
  fiat_p256_selectznz(out, !!t, z, nz);
}

static void fe_from_montgomery(fe x) {
  fiat_p256_from_montgomery(x, x);
}

static void fe_from_generic(fe out, const EC_FELEM *in) {
  fe_frombytes(out, in->bytes);
}

static void fe_to_generic(EC_FELEM *out, const fe in) {


  OPENSSL_STATIC_ASSERT(
      256 / 8 == sizeof(BN_ULONG) * ((256 + BN_BITS2 - 1) / BN_BITS2),
      "fe_tobytes leaves bytes uninitialized");
  fe_tobytes(out->bytes, in);
}

//
// Based on Fermat's Little Theorem:
//   a^p = a (mod p)
//   a^{p-1} = 1 (mod p)
//   a^{p-2} = a^{-1} (mod p)
static void fe_inv(fe out, const fe in) {
  fe ftmp, ftmp2;

  fe e2, e4, e8, e16, e32, e64;

  fe_sqr(ftmp, in);  // 2^1
  fe_mul(ftmp, in, ftmp);  // 2^2 - 2^0
  fe_copy(e2, ftmp);
  fe_sqr(ftmp, ftmp);  // 2^3 - 2^1
  fe_sqr(ftmp, ftmp);  // 2^4 - 2^2
  fe_mul(ftmp, ftmp, e2);  // 2^4 - 2^0
  fe_copy(e4, ftmp);
  fe_sqr(ftmp, ftmp);  // 2^5 - 2^1
  fe_sqr(ftmp, ftmp);  // 2^6 - 2^2
  fe_sqr(ftmp, ftmp);  // 2^7 - 2^3
  fe_sqr(ftmp, ftmp);  // 2^8 - 2^4
  fe_mul(ftmp, ftmp, e4);  // 2^8 - 2^0
  fe_copy(e8, ftmp);
  for (size_t i = 0; i < 8; i++) {
    fe_sqr(ftmp, ftmp);
  }  // 2^16 - 2^8
  fe_mul(ftmp, ftmp, e8);  // 2^16 - 2^0
  fe_copy(e16, ftmp);
  for (size_t i = 0; i < 16; i++) {
    fe_sqr(ftmp, ftmp);
  }  // 2^32 - 2^16
  fe_mul(ftmp, ftmp, e16);  // 2^32 - 2^0
  fe_copy(e32, ftmp);
  for (size_t i = 0; i < 32; i++) {
    fe_sqr(ftmp, ftmp);
  }  // 2^64 - 2^32
  fe_copy(e64, ftmp);
  fe_mul(ftmp, ftmp, in);  // 2^64 - 2^32 + 2^0
  for (size_t i = 0; i < 192; i++) {
    fe_sqr(ftmp, ftmp);
  }  // 2^256 - 2^224 + 2^192

  fe_mul(ftmp2, e64, e32);  // 2^64 - 2^0
  for (size_t i = 0; i < 16; i++) {
    fe_sqr(ftmp2, ftmp2);
  }  // 2^80 - 2^16
  fe_mul(ftmp2, ftmp2, e16);  // 2^80 - 2^0
  for (size_t i = 0; i < 8; i++) {
    fe_sqr(ftmp2, ftmp2);
  }  // 2^88 - 2^8
  fe_mul(ftmp2, ftmp2, e8);  // 2^88 - 2^0
  for (size_t i = 0; i < 4; i++) {
    fe_sqr(ftmp2, ftmp2);
  }  // 2^92 - 2^4
  fe_mul(ftmp2, ftmp2, e4);  // 2^92 - 2^0
  fe_sqr(ftmp2, ftmp2);  // 2^93 - 2^1
  fe_sqr(ftmp2, ftmp2);  // 2^94 - 2^2
  fe_mul(ftmp2, ftmp2, e2);  // 2^94 - 2^0
  fe_sqr(ftmp2, ftmp2);  // 2^95 - 2^1
  fe_sqr(ftmp2, ftmp2);  // 2^96 - 2^2
  fe_mul(ftmp2, ftmp2, in);  // 2^96 - 3

  fe_mul(out, ftmp2, ftmp);  // 2^256 - 2^224 + 2^192 + 2^96 - 3
}

// ----------------
//
// Building on top of the field operations we have the operations on the
// elliptic curve group itself. Points on the curve are represented in Jacobian
// coordinates.
//
// Both operations were transcribed to Coq and proven to correspond to naive
// implementations using Affine coordinates, for all suitable fields.  In the
// Coq proofs, issues of constant-time execution and memory layout (aliasing)
// conventions were not considered. Specification of affine coordinates:
// <https://github.com/mit-plv/fiat-crypto/blob/79f8b5f39ed609339f0233098dee1a3c4e6b3080/src/Spec/WeierstrassCurve.v#L28>
// As a sanity check, a proof that these points form a commutative group:
// <https://github.com/mit-plv/fiat-crypto/blob/79f8b5f39ed609339f0233098dee1a3c4e6b3080/src/Curves/Weierstrass/AffineProofs.v#L33>

//
// The method is taken from:
//   http://hyperelliptic.org/EFD/g1p/auto-shortw-jacobian-3.html#doubling-dbl-2001-b
//
// Coq transcription and correctness proof:
// <https://github.com/mit-plv/fiat-crypto/blob/79f8b5f39ed609339f0233098dee1a3c4e6b3080/src/Curves/Weierstrass/Jacobian.v#L93>
// <https://github.com/mit-plv/fiat-crypto/blob/79f8b5f39ed609339f0233098dee1a3c4e6b3080/src/Curves/Weierstrass/Jacobian.v#L201>
//
// Outputs can equal corresponding inputs, i.e., x_out == x_in is allowed.
// while x_out == y_in is not (maybe this works, but it's not tested).
static void point_double(fe x_out, fe y_out, fe z_out,
                         const fe x_in, const fe y_in, const fe z_in) {
  fe delta, gamma, beta, ftmp, ftmp2, tmptmp, alpha, fourbeta;

  fe_sqr(delta, z_in);

  fe_sqr(gamma, y_in);

  fe_mul(beta, x_in, gamma);

  fe_sub(ftmp, x_in, delta);
  fe_add(ftmp2, x_in, delta);

  fe_add(tmptmp, ftmp2, ftmp2);
  fe_add(ftmp2, ftmp2, tmptmp);
  fe_mul(alpha, ftmp, ftmp2);

  fe_sqr(x_out, alpha);
  fe_add(fourbeta, beta, beta);
  fe_add(fourbeta, fourbeta, fourbeta);
  fe_add(tmptmp, fourbeta, fourbeta);
  fe_sub(x_out, x_out, tmptmp);

  fe_add(delta, gamma, delta);
  fe_add(ftmp, y_in, z_in);
  fe_sqr(z_out, ftmp);
  fe_sub(z_out, z_out, delta);

  fe_sub(y_out, fourbeta, x_out);
  fe_add(gamma, gamma, gamma);
  fe_sqr(gamma, gamma);
  fe_mul(y_out, alpha, y_out);
  fe_add(gamma, gamma, gamma);
  fe_sub(y_out, y_out, gamma);
}

//
// The method is taken from:
//   http://hyperelliptic.org/EFD/g1p/auto-shortw-jacobian-3.html#addition-add-2007-bl,
// adapted for mixed addition (z2 = 1, or z2 = 0 for the point at infinity).
//
// Coq transcription and correctness proof:
// <https://github.com/mit-plv/fiat-crypto/blob/79f8b5f39ed609339f0233098dee1a3c4e6b3080/src/Curves/Weierstrass/Jacobian.v#L135>
// <https://github.com/mit-plv/fiat-crypto/blob/79f8b5f39ed609339f0233098dee1a3c4e6b3080/src/Curves/Weierstrass/Jacobian.v#L205>
//
// This function includes a branch for checking whether the two input points
// are equal, (while not equal to the point at infinity). This case never
// happens during single point multiplication, so there is no timing leak for
// ECDH or ECDSA signing.
static void point_add(fe x3, fe y3, fe z3, const fe x1,
                      const fe y1, const fe z1, const int mixed,
                      const fe x2, const fe y2, const fe z2) {
  fe x_out, y_out, z_out;
  limb_t z1nz = fe_nz(z1);
  limb_t z2nz = fe_nz(z2);

  fe z1z1; fe_sqr(z1z1, z1);

  fe u1, s1, two_z1z2;
  if (!mixed) {

    fe z2z2; fe_sqr(z2z2, z2);

    fe_mul(u1, x1, z2z2);

    fe_add(two_z1z2, z1, z2);
    fe_sqr(two_z1z2, two_z1z2);
    fe_sub(two_z1z2, two_z1z2, z1z1);
    fe_sub(two_z1z2, two_z1z2, z2z2);

    fe_mul(s1, z2, z2z2);
    fe_mul(s1, s1, y1);
  } else {


    fe_copy(u1, x1);

    fe_add(two_z1z2, z1, z1);

    fe_copy(s1, y1);
  }

  fe u2; fe_mul(u2, x2, z1z1);

  fe h; fe_sub(h, u2, u1);

  limb_t xneq = fe_nz(h);

  fe_mul(z_out, h, two_z1z2);

  fe z1z1z1; fe_mul(z1z1z1, z1, z1z1);

  fe s2; fe_mul(s2, y2, z1z1z1);

  fe r;
  fe_sub(r, s2, s1);
  fe_add(r, r, r);

  limb_t yneq = fe_nz(r);

  limb_t is_nontrivial_double = constant_time_is_zero_w(xneq | yneq) &
                                ~constant_time_is_zero_w(z1nz) &
                                ~constant_time_is_zero_w(z2nz);
  if (is_nontrivial_double) {
    point_double(x3, y3, z3, x1, y1, z1);
    return;
  }

  fe i;
  fe_add(i, h, h);
  fe_sqr(i, i);

  fe j; fe_mul(j, h, i);

  fe v; fe_mul(v, u1, i);

  fe_sqr(x_out, r);
  fe_sub(x_out, x_out, j);
  fe_sub(x_out, x_out, v);
  fe_sub(x_out, x_out, v);

  fe_sub(y_out, v, x_out);
  fe_mul(y_out, y_out, r);
  fe s1j;
  fe_mul(s1j, s1, j);
  fe_sub(y_out, y_out, s1j);
  fe_sub(y_out, y_out, s1j);

  fe_cmovznz(x_out, z1nz, x2, x_out);
  fe_cmovznz(x3, z2nz, x1, x_out);
  fe_cmovznz(y_out, z1nz, y2, y_out);
  fe_cmovznz(y3, z2nz, y1, y_out);
  fe_cmovznz(z_out, z1nz, z2, z_out);
  fe_cmovznz(z3, z2nz, z1, z_out);
}

// --------------------------
//
// Two different sorts of precomputed tables are used in the following code.
// Each contain various points on the curve, where each point is three field
// elements (x, y, z).
//
// For the base point table, z is usually 1 (0 for the point at infinity).
// This table has 2 * 16 elements, starting with the following:
// index | bits    | point
// ------+---------+------------------------------
//     0 | 0 0 0 0 | 0G
//     1 | 0 0 0 1 | 1G
//     2 | 0 0 1 0 | 2^64G
//     3 | 0 0 1 1 | (2^64 + 1)G
//     4 | 0 1 0 0 | 2^128G
//     5 | 0 1 0 1 | (2^128 + 1)G
//     6 | 0 1 1 0 | (2^128 + 2^64)G
//     7 | 0 1 1 1 | (2^128 + 2^64 + 1)G
//     8 | 1 0 0 0 | 2^192G
//     9 | 1 0 0 1 | (2^192 + 1)G
//    10 | 1 0 1 0 | (2^192 + 2^64)G
//    11 | 1 0 1 1 | (2^192 + 2^64 + 1)G
//    12 | 1 1 0 0 | (2^192 + 2^128)G
//    13 | 1 1 0 1 | (2^192 + 2^128 + 1)G
//    14 | 1 1 1 0 | (2^192 + 2^128 + 2^64)G
//    15 | 1 1 1 1 | (2^192 + 2^128 + 2^64 + 1)G
// followed by a copy of this with each element multiplied by 2^32.
//
// The reason for this is so that we can clock bits into four different
// locations when doing simple scalar multiplies against the base point,
// and then another four locations using the second 16 elements.
//
// Tables for other points have table[i] = iG for i in 0 .. 16.

#if defined(BORINGSSL_NISTP256_64BIT)
static const fe g_pre_comp[2][16][3] = {
    {{{0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0}},
     {{0x79e730d418a9143c, 0x75ba95fc5fedb601, 0x79fb732b77622510,
       0x18905f76a53755c6},
      {0xddf25357ce95560a, 0x8b4ab8e4ba19e45c, 0xd2e88688dd21f325,
       0x8571ff1825885d85},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0x4f922fc516a0d2bb, 0xd5cc16c1a623499, 0x9241cf3a57c62c8b,
       0x2f5e6961fd1b667f},
      {0x5c15c70bf5a01797, 0x3d20b44d60956192, 0x4911b37071fdb52,
       0xf648f9168d6f0f7b},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0x9e566847e137bbbc, 0xe434469e8a6a0bec, 0xb1c4276179d73463,
       0x5abe0285133d0015},
      {0x92aa837cc04c7dab, 0x573d9f4c43260c07, 0xc93156278e6cc37,
       0x94bb725b6b6f7383},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0x62a8c244bfe20925, 0x91c19ac38fdce867, 0x5a96a5d5dd387063,
       0x61d587d421d324f6},
      {0xe87673a2a37173ea, 0x2384800853778b65, 0x10f8441e05bab43e,
       0xfa11fe124621efbe},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0x1c891f2b2cb19ffd, 0x1ba8d5bb1923c23, 0xb6d03d678ac5ca8e,
       0x586eb04c1f13bedc},
      {0xc35c6e527e8ed09, 0x1e81a33c1819ede2, 0x278fd6c056c652fa,
       0x19d5ac0870864f11},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0x62577734d2b533d5, 0x673b8af6a1bdddc0, 0x577e7c9aa79ec293,
       0xbb6de651c3b266b1},
      {0xe7e9303ab65259b3, 0xd6a0afd3d03a7480, 0xc5ac83d19b3cfc27,
       0x60b4619a5d18b99b},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0xbd6a38e11ae5aa1c, 0xb8b7652b49e73658, 0xb130014ee5f87ed,
       0x9d0f27b2aeebffcd},
      {0xca9246317a730a55, 0x9c955b2fddbbc83a, 0x7c1dfe0ac019a71,
       0x244a566d356ec48d},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0x56f8410ef4f8b16a, 0x97241afec47b266a, 0xa406b8e6d9c87c1,
       0x803f3e02cd42ab1b},
      {0x7f0309a804dbec69, 0xa83b85f73bbad05f, 0xc6097273ad8e197f,
       0xc097440e5067adc1},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0x846a56f2c379ab34, 0xa8ee068b841df8d1, 0x20314459176c68ef,
       0xf1af32d5915f1f30},
      {0x99c375315d75bd50, 0x837cffbaf72f67bc, 0x613a41848d7723f,
       0x23d0f130e2d41c8b},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0xed93e225d5be5a2b, 0x6fe799835934f3c6, 0x4314092622626ffc,
       0x50bbb4d97990216a},
      {0x378191c6e57ec63e, 0x65422c40181dcdb2, 0x41a8099b0236e0f6,
       0x2b10011801fe49c3},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0xfc68b5c59b391593, 0xc385f5a2598270fc, 0x7144f3aad19adcbb,
       0xdd55899983fbae0c},
      {0x93b88b8e74b82ff4, 0xd2e03c4071e734c9, 0x9a7a9eaf43c0322a,
       0xe6e4c551149d6041},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0x5fe14bfe80ec21fe, 0xf6ce116ac255be82, 0x98bc5a072f4a5d67,
       0xfad27148db7e63af},
      {0x90c0b6ac29ab05b3, 0x37a9a83c4e251ae6, 0xa7dc875c2aade7d,
       0x77387de39f0e1a84},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0x1e9ecc49a56c0dd7, 0xa5cffcd846086c74, 0x8f7a1408f505aece,
       0xb37b85c0bef0c47e},
      {0x3596b6e4cc0e6a8f, 0xfd6d4bbf6b388f23, 0xaba453fac39cef4e,
       0x9c135ac8f9f628d5},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0xa1c729495c8f8be, 0x2961c4803bf362bf, 0x9e418403df63d4ac,
       0xc109f9cb91ece900},
      {0xc2d095d058945705, 0xb9083d96ddeb85c0, 0x84692b8d7a40449b,
       0x9bc3344f2eee1ee1},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0xd5ae35642913074, 0x55491b2748a542b1, 0x469ca665b310732a,
       0x29591d525f1a4cc1},
      {0xe76f5b6bb84f983f, 0xbe7eef419f5f84e1, 0x1200d49680baa189,
       0x6376551f18ef332c},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}}},
    {{{0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0}, {0x0, 0x0, 0x0, 0x0}},
     {{0x202886024147519a, 0xd0981eac26b372f0, 0xa9d4a7caa785ebc8,
       0xd953c50ddbdf58e9},
      {0x9d6361ccfd590f8f, 0x72e9626b44e6c917, 0x7fd9611022eb64cf,
       0x863ebb7e9eb288f3},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0x4fe7ee31b0e63d34, 0xf4600572a9e54fab, 0xc0493334d5e7b5a4,
       0x8589fb9206d54831},
      {0xaa70f5cc6583553a, 0x879094ae25649e5, 0xcc90450710044652,
       0xebb0696d02541c4f},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0xabbaa0c03b89da99, 0xa6f2d79eb8284022, 0x27847862b81c05e8,
       0x337a4b5905e54d63},
      {0x3c67500d21f7794a, 0x207005b77d6d7f61, 0xa5a378104cfd6e8,
       0xd65e0d5f4c2fbd6},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0xd433e50f6d3549cf, 0x6f33696ffacd665e, 0x695bfdacce11fcb4,
       0x810ee252af7c9860},
      {0x65450fe17159bb2c, 0xf7dfbebe758b357b, 0x2b057e74d69fea72,
       0xd485717a92731745},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0xce1f69bbe83f7669, 0x9f8ae8272877d6b, 0x9548ae543244278d,
       0x207755dee3c2c19c},
      {0x87bd61d96fef1945, 0x18813cefb12d28c3, 0x9fbcd1d672df64aa,
       0x48dc5ee57154b00d},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0xef0f469ef49a3154, 0x3e85a5956e2b2e9a, 0x45aaec1eaa924a9c,
       0xaa12dfc8a09e4719},
      {0x26f272274df69f1d, 0xe0e4c82ca2ff5e73, 0xb9d8ce73b7a9dd44,
       0x6c036e73e48ca901},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0xe1e421e1a47153f0, 0xb86c3b79920418c9, 0x93bdce87705d7672,
       0xf25ae793cab79a77},
      {0x1f3194a36d869d0c, 0x9d55c8824986c264, 0x49fb5ea3096e945e,
       0x39b8e65313db0a3e},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0xe3417bc035d0b34a, 0x440b386b8327c0a7, 0x8fb7262dac0362d1,
       0x2c41114ce0cdf943},
      {0x2ba5cef1ad95a0b1, 0xc09b37a867d54362, 0x26d6cdd201e486c9,
       0x20477abf42ff9297},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0xf121b41bc0a67d2, 0x62d4760a444d248a, 0xe044f1d659b4737,
       0x8fde365250bb4a8},
      {0xaceec3da848bf287, 0xc2a62182d3369d6e, 0x3582dfdc92449482,
       0x2f7e2fd2565d6cd7},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0xa0122b5178a876b, 0x51ff96ff085104b4, 0x50b31ab14f29f76,
       0x84abb28b5f87d4e6},
      {0xd5ed439f8270790a, 0x2d6cb59d85e3f46b, 0x75f55c1b6c1e2212,
       0xe5436f6717655640},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0xc2965ecc9aeb596d, 0x1ea03e7023c92b4, 0x4704b4b62e013961,
       0xca8fd3f905ea367},
      {0x92523a42551b2b61, 0x1eb7a89c390fcd06, 0xe7f1d2be0392a63e,
       0x96dca2644ddb0c33},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0x231c210e15339848, 0xe87a28e870778c8d, 0x9d1de6616956e170,
       0x4ac3c9382bb09c0b},
      {0x19be05516998987d, 0x8b2376c4ae09f4d6, 0x1de0b7651a3f933d,
       0x380d94c7e39705f4},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0x3685954b8c31c31d, 0x68533d005bf21a0c, 0xbd7626e75c79ec9,
       0xca17754742c69d54},
      {0xcc6edafff6d2dbb2, 0xfd0d8cbd174a9d18, 0x875e8793aa4578e8,
       0xa976a7139cab2ce6},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0xce37ab11b43ea1db, 0xa7ff1a95259d292, 0x851b02218f84f186,
       0xa7222beadefaad13},
      {0xa2ac78ec2b0a9144, 0x5a024051f2fa59c5, 0x91d1eca56147ce38,
       0xbe94d523bc2ac690},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}},
     {{0x2d8daefd79ec1a0f, 0x3bbcd6fdceb39c97, 0xf5575ffc58f61a95,
       0xdbd986c4adf7b420},
      {0x81aa881415f39eb7, 0x6ee2fcf5b98d976c, 0x5465475dcf2f717d,
       0x8e24d3c46860bbd0},
      {0x1, 0xffffffff00000000, 0xffffffffffffffff, 0xfffffffe}}}};
#else
static const fe g_pre_comp[2][16][3] = {
    {{{0x0,0x0, 0x0,0x0, 0x0,0x0, 0x0,0x0},
      {0x0,0x0, 0x0,0x0, 0x0,0x0, 0x0,0x0},
      {0x0,0x0, 0x0,0x0, 0x0,0x0, 0x0,0x0}},
     {{0x18a9143c,0x79e730d4, 0x5fedb601,0x75ba95fc, 0x77622510,0x79fb732b,
       0xa53755c6,0x18905f76},
      {0xce95560a,0xddf25357, 0xba19e45c,0x8b4ab8e4, 0xdd21f325,0xd2e88688,
       0x25885d85,0x8571ff18},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0x16a0d2bb,0x4f922fc5, 0x1a623499,0xd5cc16c, 0x57c62c8b,0x9241cf3a,
       0xfd1b667f,0x2f5e6961},
      {0xf5a01797,0x5c15c70b, 0x60956192,0x3d20b44d, 0x71fdb52,0x4911b37,
       0x8d6f0f7b,0xf648f916},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0xe137bbbc,0x9e566847, 0x8a6a0bec,0xe434469e, 0x79d73463,0xb1c42761,
       0x133d0015,0x5abe0285},
      {0xc04c7dab,0x92aa837c, 0x43260c07,0x573d9f4c, 0x78e6cc37,0xc931562,
       0x6b6f7383,0x94bb725b},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0xbfe20925,0x62a8c244, 0x8fdce867,0x91c19ac3, 0xdd387063,0x5a96a5d5,
       0x21d324f6,0x61d587d4},
      {0xa37173ea,0xe87673a2, 0x53778b65,0x23848008, 0x5bab43e,0x10f8441e,
       0x4621efbe,0xfa11fe12},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0x2cb19ffd,0x1c891f2b, 0xb1923c23,0x1ba8d5b, 0x8ac5ca8e,0xb6d03d67,
       0x1f13bedc,0x586eb04c},
      {0x27e8ed09,0xc35c6e5, 0x1819ede2,0x1e81a33c, 0x56c652fa,0x278fd6c0,
       0x70864f11,0x19d5ac08},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0xd2b533d5,0x62577734, 0xa1bdddc0,0x673b8af6, 0xa79ec293,0x577e7c9a,
       0xc3b266b1,0xbb6de651},
      {0xb65259b3,0xe7e9303a, 0xd03a7480,0xd6a0afd3, 0x9b3cfc27,0xc5ac83d1,
       0x5d18b99b,0x60b4619a},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0x1ae5aa1c,0xbd6a38e1, 0x49e73658,0xb8b7652b, 0xee5f87ed,0xb130014,
       0xaeebffcd,0x9d0f27b2},
      {0x7a730a55,0xca924631, 0xddbbc83a,0x9c955b2f, 0xac019a71,0x7c1dfe0,
       0x356ec48d,0x244a566d},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0xf4f8b16a,0x56f8410e, 0xc47b266a,0x97241afe, 0x6d9c87c1,0xa406b8e,
       0xcd42ab1b,0x803f3e02},
      {0x4dbec69,0x7f0309a8, 0x3bbad05f,0xa83b85f7, 0xad8e197f,0xc6097273,
       0x5067adc1,0xc097440e},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0xc379ab34,0x846a56f2, 0x841df8d1,0xa8ee068b, 0x176c68ef,0x20314459,
       0x915f1f30,0xf1af32d5},
      {0x5d75bd50,0x99c37531, 0xf72f67bc,0x837cffba, 0x48d7723f,0x613a418,
       0xe2d41c8b,0x23d0f130},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0xd5be5a2b,0xed93e225, 0x5934f3c6,0x6fe79983, 0x22626ffc,0x43140926,
       0x7990216a,0x50bbb4d9},
      {0xe57ec63e,0x378191c6, 0x181dcdb2,0x65422c40, 0x236e0f6,0x41a8099b,
       0x1fe49c3,0x2b100118},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0x9b391593,0xfc68b5c5, 0x598270fc,0xc385f5a2, 0xd19adcbb,0x7144f3aa,
       0x83fbae0c,0xdd558999},
      {0x74b82ff4,0x93b88b8e, 0x71e734c9,0xd2e03c40, 0x43c0322a,0x9a7a9eaf,
       0x149d6041,0xe6e4c551},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0x80ec21fe,0x5fe14bfe, 0xc255be82,0xf6ce116a, 0x2f4a5d67,0x98bc5a07,
       0xdb7e63af,0xfad27148},
      {0x29ab05b3,0x90c0b6ac, 0x4e251ae6,0x37a9a83c, 0xc2aade7d,0xa7dc875,
       0x9f0e1a84,0x77387de3},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0xa56c0dd7,0x1e9ecc49, 0x46086c74,0xa5cffcd8, 0xf505aece,0x8f7a1408,
       0xbef0c47e,0xb37b85c0},
      {0xcc0e6a8f,0x3596b6e4, 0x6b388f23,0xfd6d4bbf, 0xc39cef4e,0xaba453fa,
       0xf9f628d5,0x9c135ac8},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0x95c8f8be,0xa1c7294, 0x3bf362bf,0x2961c480, 0xdf63d4ac,0x9e418403,
       0x91ece900,0xc109f9cb},
      {0x58945705,0xc2d095d0, 0xddeb85c0,0xb9083d96, 0x7a40449b,0x84692b8d,
       0x2eee1ee1,0x9bc3344f},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0x42913074,0xd5ae356, 0x48a542b1,0x55491b27, 0xb310732a,0x469ca665,
       0x5f1a4cc1,0x29591d52},
      {0xb84f983f,0xe76f5b6b, 0x9f5f84e1,0xbe7eef41, 0x80baa189,0x1200d496,
       0x18ef332c,0x6376551f},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}}},
    {{{0x0,0x0, 0x0,0x0, 0x0,0x0, 0x0,0x0},
      {0x0,0x0, 0x0,0x0, 0x0,0x0, 0x0,0x0},
      {0x0,0x0, 0x0,0x0, 0x0,0x0, 0x0,0x0}},
     {{0x4147519a,0x20288602, 0x26b372f0,0xd0981eac, 0xa785ebc8,0xa9d4a7ca,
       0xdbdf58e9,0xd953c50d},
      {0xfd590f8f,0x9d6361cc, 0x44e6c917,0x72e9626b, 0x22eb64cf,0x7fd96110,
       0x9eb288f3,0x863ebb7e},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0xb0e63d34,0x4fe7ee31, 0xa9e54fab,0xf4600572, 0xd5e7b5a4,0xc0493334,
       0x6d54831,0x8589fb92},
      {0x6583553a,0xaa70f5cc, 0xe25649e5,0x879094a, 0x10044652,0xcc904507,
       0x2541c4f,0xebb0696d},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0x3b89da99,0xabbaa0c0, 0xb8284022,0xa6f2d79e, 0xb81c05e8,0x27847862,
       0x5e54d63,0x337a4b59},
      {0x21f7794a,0x3c67500d, 0x7d6d7f61,0x207005b7, 0x4cfd6e8,0xa5a3781,
       0xf4c2fbd6,0xd65e0d5},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0x6d3549cf,0xd433e50f, 0xfacd665e,0x6f33696f, 0xce11fcb4,0x695bfdac,
       0xaf7c9860,0x810ee252},
      {0x7159bb2c,0x65450fe1, 0x758b357b,0xf7dfbebe, 0xd69fea72,0x2b057e74,
       0x92731745,0xd485717a},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0xe83f7669,0xce1f69bb, 0x72877d6b,0x9f8ae82, 0x3244278d,0x9548ae54,
       0xe3c2c19c,0x207755de},
      {0x6fef1945,0x87bd61d9, 0xb12d28c3,0x18813cef, 0x72df64aa,0x9fbcd1d6,
       0x7154b00d,0x48dc5ee5},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0xf49a3154,0xef0f469e, 0x6e2b2e9a,0x3e85a595, 0xaa924a9c,0x45aaec1e,
       0xa09e4719,0xaa12dfc8},
      {0x4df69f1d,0x26f27227, 0xa2ff5e73,0xe0e4c82c, 0xb7a9dd44,0xb9d8ce73,
       0xe48ca901,0x6c036e73},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0xa47153f0,0xe1e421e1, 0x920418c9,0xb86c3b79, 0x705d7672,0x93bdce87,
       0xcab79a77,0xf25ae793},
      {0x6d869d0c,0x1f3194a3, 0x4986c264,0x9d55c882, 0x96e945e,0x49fb5ea3,
       0x13db0a3e,0x39b8e653},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0x35d0b34a,0xe3417bc0, 0x8327c0a7,0x440b386b, 0xac0362d1,0x8fb7262d,
       0xe0cdf943,0x2c41114c},
      {0xad95a0b1,0x2ba5cef1, 0x67d54362,0xc09b37a8, 0x1e486c9,0x26d6cdd2,
       0x42ff9297,0x20477abf},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0xbc0a67d2,0xf121b41, 0x444d248a,0x62d4760a, 0x659b4737,0xe044f1d,
       0x250bb4a8,0x8fde365},
      {0x848bf287,0xaceec3da, 0xd3369d6e,0xc2a62182, 0x92449482,0x3582dfdc,
       0x565d6cd7,0x2f7e2fd2},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0x178a876b,0xa0122b5, 0x85104b4,0x51ff96ff, 0x14f29f76,0x50b31ab,
       0x5f87d4e6,0x84abb28b},
      {0x8270790a,0xd5ed439f, 0x85e3f46b,0x2d6cb59d, 0x6c1e2212,0x75f55c1b,
       0x17655640,0xe5436f67},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0x9aeb596d,0xc2965ecc, 0x23c92b4,0x1ea03e7, 0x2e013961,0x4704b4b6,
       0x905ea367,0xca8fd3f},
      {0x551b2b61,0x92523a42, 0x390fcd06,0x1eb7a89c, 0x392a63e,0xe7f1d2be,
       0x4ddb0c33,0x96dca264},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0x15339848,0x231c210e, 0x70778c8d,0xe87a28e8, 0x6956e170,0x9d1de661,
       0x2bb09c0b,0x4ac3c938},
      {0x6998987d,0x19be0551, 0xae09f4d6,0x8b2376c4, 0x1a3f933d,0x1de0b765,
       0xe39705f4,0x380d94c7},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0x8c31c31d,0x3685954b, 0x5bf21a0c,0x68533d00, 0x75c79ec9,0xbd7626e,
       0x42c69d54,0xca177547},
      {0xf6d2dbb2,0xcc6edaff, 0x174a9d18,0xfd0d8cbd, 0xaa4578e8,0x875e8793,
       0x9cab2ce6,0xa976a713},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0xb43ea1db,0xce37ab11, 0x5259d292,0xa7ff1a9, 0x8f84f186,0x851b0221,
       0xdefaad13,0xa7222bea},
      {0x2b0a9144,0xa2ac78ec, 0xf2fa59c5,0x5a024051, 0x6147ce38,0x91d1eca5,
       0xbc2ac690,0xbe94d523},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}},
     {{0x79ec1a0f,0x2d8daefd, 0xceb39c97,0x3bbcd6fd, 0x58f61a95,0xf5575ffc,
       0xadf7b420,0xdbd986c4},
      {0x15f39eb7,0x81aa8814, 0xb98d976c,0x6ee2fcf5, 0xcf2f717d,0x5465475d,
       0x6860bbd0,0x8e24d3c4},
      {0x1,0x0, 0x0,0xffffffff, 0xffffffff,0xffffffff, 0xfffffffe,0x0}}}};
#endif

// copies it to out.
static void select_point(const limb_t idx, size_t size,
                         const fe pre_comp[/*size*/][3],
                         fe out[3]) {
  OPENSSL_memset(out, 0, sizeof(fe) * 3);
  for (size_t i = 0; i < size; i++) {
    limb_t mismatch = i ^ idx;
    fe_cmovznz(out[0], mismatch, pre_comp[i][0], out[0]);
    fe_cmovznz(out[1], mismatch, pre_comp[i][1], out[1]);
    fe_cmovznz(out[2], mismatch, pre_comp[i][2], out[2]);
  }
}

static char get_bit(const uint8_t *in, int i) {
  if (i < 0 || i >= 256) {
    return 0;
  }
  return (in[i >> 3] >> (i & 7)) & 1;
}


// (X/Z^2, Y/Z^3).
static int ec_GFp_nistp256_point_get_affine_coordinates(
    const EC_GROUP *group, const EC_RAW_POINT *point, EC_FELEM *x_out,
    EC_FELEM *y_out) {
  if (ec_GFp_simple_is_at_infinity(group, point)) {
    OPENSSL_PUT_ERROR(EC, EC_R_POINT_AT_INFINITY);
    return 0;
  }

  fe z1, z2;
  fe_from_generic(z1, &point->Z);
  fe_inv(z2, z1);
  fe_sqr(z1, z2);



  fe_from_montgomery(z1);

  if (x_out != NULL) {
    fe x;
    fe_from_generic(x, &point->X);
    fe_mul(x, x, z1);
    fe_to_generic(x_out, x);
  }

  if (y_out != NULL) {
    fe y;
    fe_from_generic(y, &point->Y);
    fe_mul(z1, z1, z2);
    fe_mul(y, y, z1);
    fe_to_generic(y_out, y);
  }

  return 1;
}

static void ec_GFp_nistp256_add(const EC_GROUP *group, EC_RAW_POINT *r,
                                const EC_RAW_POINT *a, const EC_RAW_POINT *b) {
  fe x1, y1, z1, x2, y2, z2;
  fe_from_generic(x1, &a->X);
  fe_from_generic(y1, &a->Y);
  fe_from_generic(z1, &a->Z);
  fe_from_generic(x2, &b->X);
  fe_from_generic(y2, &b->Y);
  fe_from_generic(z2, &b->Z);
  point_add(x1, y1, z1, x1, y1, z1, 0 /* both Jacobian */, x2, y2, z2);
  fe_to_generic(&r->X, x1);
  fe_to_generic(&r->Y, y1);
  fe_to_generic(&r->Z, z1);
}

static void ec_GFp_nistp256_dbl(const EC_GROUP *group, EC_RAW_POINT *r,
                                const EC_RAW_POINT *a) {
  fe x, y, z;
  fe_from_generic(x, &a->X);
  fe_from_generic(y, &a->Y);
  fe_from_generic(z, &a->Z);
  point_double(x, y, z, x, y, z);
  fe_to_generic(&r->X, x);
  fe_to_generic(&r->Y, y);
  fe_to_generic(&r->Z, z);
}

static void ec_GFp_nistp256_point_mul(const EC_GROUP *group, EC_RAW_POINT *r,
                                      const EC_RAW_POINT *p,
                                      const EC_SCALAR *scalar) {
  fe p_pre_comp[17][3];
  OPENSSL_memset(&p_pre_comp, 0, sizeof(p_pre_comp));

  fe_from_generic(p_pre_comp[1][0], &p->X);
  fe_from_generic(p_pre_comp[1][1], &p->Y);
  fe_from_generic(p_pre_comp[1][2], &p->Z);
  for (size_t j = 2; j <= 16; ++j) {
    if (j & 1) {
      point_add(p_pre_comp[j][0], p_pre_comp[j][1], p_pre_comp[j][2],
                p_pre_comp[1][0], p_pre_comp[1][1], p_pre_comp[1][2], 0,
                p_pre_comp[j - 1][0], p_pre_comp[j - 1][1],
                p_pre_comp[j - 1][2]);
    } else {
      point_double(p_pre_comp[j][0], p_pre_comp[j][1], p_pre_comp[j][2],
                   p_pre_comp[j / 2][0], p_pre_comp[j / 2][1],
                   p_pre_comp[j / 2][2]);
    }
  }

  fe nq[3] = {{0}, {0}, {0}}, ftmp, tmp[3];

  int skip = 1;  // Save two point operations in the first round.
  for (size_t i = 255; i < 256; i--) {

    if (!skip) {
      point_double(nq[0], nq[1], nq[2], nq[0], nq[1], nq[2]);
    }

    if (i % 5 == 0) {
      uint64_t bits = get_bit(scalar->bytes, i + 4) << 5;
      bits |= get_bit(scalar->bytes, i + 3) << 4;
      bits |= get_bit(scalar->bytes, i + 2) << 3;
      bits |= get_bit(scalar->bytes, i + 1) << 2;
      bits |= get_bit(scalar->bytes, i) << 1;
      bits |= get_bit(scalar->bytes, i - 1);
      uint8_t sign, digit;
      ec_GFp_nistp_recode_scalar_bits(&sign, &digit, bits);

      select_point(digit, 17, (const fe(*)[3])p_pre_comp, tmp);
      fe_opp(ftmp, tmp[1]);  // (X, -Y, Z) is the negative point.
      fe_cmovznz(tmp[1], sign, tmp[1], ftmp);

      if (!skip) {
        point_add(nq[0], nq[1], nq[2], nq[0], nq[1], nq[2], 0 /* mixed */,
                  tmp[0], tmp[1], tmp[2]);
      } else {
        fe_copy(nq[0], tmp[0]);
        fe_copy(nq[1], tmp[1]);
        fe_copy(nq[2], tmp[2]);
        skip = 0;
      }
    }
  }

  fe_to_generic(&r->X, nq[0]);
  fe_to_generic(&r->Y, nq[1]);
  fe_to_generic(&r->Z, nq[2]);
}

static void ec_GFp_nistp256_point_mul_base(const EC_GROUP *group,
                                           EC_RAW_POINT *r,
                                           const EC_SCALAR *scalar) {

  fe nq[3] = {{0}, {0}, {0}}, tmp[3];

  int skip = 1;  // Save two point operations in the first round.
  for (size_t i = 31; i < 32; i--) {
    if (!skip) {
      point_double(nq[0], nq[1], nq[2], nq[0], nq[1], nq[2]);
    }

    uint64_t bits = get_bit(scalar->bytes, i + 224) << 3;
    bits |= get_bit(scalar->bytes, i + 160) << 2;
    bits |= get_bit(scalar->bytes, i + 96) << 1;
    bits |= get_bit(scalar->bytes, i + 32);

    select_point(bits, 16, g_pre_comp[1], tmp);

    if (!skip) {
      point_add(nq[0], nq[1], nq[2], nq[0], nq[1], nq[2], 1 /* mixed */, tmp[0],
                tmp[1], tmp[2]);
    } else {
      fe_copy(nq[0], tmp[0]);
      fe_copy(nq[1], tmp[1]);
      fe_copy(nq[2], tmp[2]);
      skip = 0;
    }

    bits = get_bit(scalar->bytes, i + 192) << 3;
    bits |= get_bit(scalar->bytes, i + 128) << 2;
    bits |= get_bit(scalar->bytes, i + 64) << 1;
    bits |= get_bit(scalar->bytes, i);

    select_point(bits, 16, g_pre_comp[0], tmp);
    point_add(nq[0], nq[1], nq[2], nq[0], nq[1], nq[2], 1 /* mixed */, tmp[0],
              tmp[1], tmp[2]);
  }

  fe_to_generic(&r->X, nq[0]);
  fe_to_generic(&r->Y, nq[1]);
  fe_to_generic(&r->Z, nq[2]);
}

static void ec_GFp_nistp256_point_mul_public(const EC_GROUP *group,
                                             EC_RAW_POINT *r,
                                             const EC_SCALAR *g_scalar,
                                             const EC_RAW_POINT *p,
                                             const EC_SCALAR *p_scalar) {
#define P256_WSIZE_PUBLIC 4

  fe p_pre_comp[1 << (P256_WSIZE_PUBLIC-1)][3];
  fe_from_generic(p_pre_comp[0][0], &p->X);
  fe_from_generic(p_pre_comp[0][1], &p->Y);
  fe_from_generic(p_pre_comp[0][2], &p->Z);
  fe p2[3];
  point_double(p2[0], p2[1], p2[2], p_pre_comp[0][0], p_pre_comp[0][1],
               p_pre_comp[0][2]);
  for (size_t i = 1; i < OPENSSL_ARRAY_SIZE(p_pre_comp); i++) {
    point_add(p_pre_comp[i][0], p_pre_comp[i][1], p_pre_comp[i][2],
              p_pre_comp[i - 1][0], p_pre_comp[i - 1][1], p_pre_comp[i - 1][2],
              0 /* not mixed */, p2[0], p2[1], p2[2]);
  }

  int8_t p_wNAF[257];
  ec_compute_wNAF(group, p_wNAF, p_scalar, 256, P256_WSIZE_PUBLIC);

  int skip = 1;  // Save some point operations.
  fe ret[3] = {{0},{0},{0}};
  for (int i = 256; i >= 0; i--) {
    if (!skip) {
      point_double(ret[0], ret[1], ret[2], ret[0], ret[1], ret[2]);
    }


    if (i <= 31) {

      uint64_t bits = get_bit(g_scalar->bytes, i + 224) << 3;
      bits |= get_bit(g_scalar->bytes, i + 160) << 2;
      bits |= get_bit(g_scalar->bytes, i + 96) << 1;
      bits |= get_bit(g_scalar->bytes, i + 32);
      point_add(ret[0], ret[1], ret[2], ret[0], ret[1], ret[2], 1 /* mixed */,
                g_pre_comp[1][bits][0], g_pre_comp[1][bits][1],
                g_pre_comp[1][bits][2]);
      skip = 0;

      bits = get_bit(g_scalar->bytes, i + 192) << 3;
      bits |= get_bit(g_scalar->bytes, i + 128) << 2;
      bits |= get_bit(g_scalar->bytes, i + 64) << 1;
      bits |= get_bit(g_scalar->bytes, i);
      point_add(ret[0], ret[1], ret[2], ret[0], ret[1], ret[2], 1 /* mixed */,
                g_pre_comp[0][bits][0], g_pre_comp[0][bits][1],
                g_pre_comp[0][bits][2]);
    }

    int digit = p_wNAF[i];
    if (digit != 0) {
      assert(digit & 1);
      int idx = digit < 0 ? (-digit) >> 1 : digit >> 1;
      fe *y = &p_pre_comp[idx][1], tmp;
      if (digit < 0) {
        fe_opp(tmp, p_pre_comp[idx][1]);
        y = &tmp;
      }
      if (!skip) {
        point_add(ret[0], ret[1], ret[2], ret[0], ret[1], ret[2],
                  0 /* not mixed */, p_pre_comp[idx][0], *y, p_pre_comp[idx][2]);
      } else {
        fe_copy(ret[0], p_pre_comp[idx][0]);
        fe_copy(ret[1], *y);
        fe_copy(ret[2], p_pre_comp[idx][2]);
        skip = 0;
      }
    }
  }

  fe_to_generic(&r->X, ret[0]);
  fe_to_generic(&r->Y, ret[1]);
  fe_to_generic(&r->Z, ret[2]);
}

static int ec_GFp_nistp256_cmp_x_coordinate(const EC_GROUP *group,
                                            const EC_RAW_POINT *p,
                                            const EC_SCALAR *r) {
  if (ec_GFp_simple_is_at_infinity(group, p)) {
    return 0;
  }



  fe Z2_mont;
  fe_from_generic(Z2_mont, &p->Z);
  fe_mul(Z2_mont, Z2_mont, Z2_mont);

  fe r_Z2;
  fe_frombytes(r_Z2, r->bytes);  // r < order < p, so this is valid.
  fe_mul(r_Z2, r_Z2, Z2_mont);

  fe X;
  fe_from_generic(X, &p->X);
  fe_from_montgomery(X);

  if (OPENSSL_memcmp(&r_Z2, &X, sizeof(r_Z2)) == 0) {
    return 1;
  }




  assert(group->field.width == group->order.width);
  if (bn_less_than_words(r->words, group->field_minus_order.words,
                         group->field.width)) {

    EC_FELEM tmp;
    bn_add_words(tmp.words, r->words, group->order.d, group->order.width);
    fe_from_generic(r_Z2, &tmp);
    fe_mul(r_Z2, r_Z2, Z2_mont);
    if (OPENSSL_memcmp(&r_Z2, &X, sizeof(r_Z2)) == 0) {
      return 1;
    }
  }

  return 0;
}

DEFINE_METHOD_FUNCTION(EC_METHOD, EC_GFp_nistp256_method) {
  out->group_init = ec_GFp_mont_group_init;
  out->group_finish = ec_GFp_mont_group_finish;
  out->group_set_curve = ec_GFp_mont_group_set_curve;
  out->point_get_affine_coordinates =
    ec_GFp_nistp256_point_get_affine_coordinates;
  out->add = ec_GFp_nistp256_add;
  out->dbl = ec_GFp_nistp256_dbl;
  out->mul = ec_GFp_nistp256_point_mul;
  out->mul_base = ec_GFp_nistp256_point_mul_base;
  out->mul_public = ec_GFp_nistp256_point_mul_public;
  out->felem_mul = ec_GFp_mont_felem_mul;
  out->felem_sqr = ec_GFp_mont_felem_sqr;
  out->bignum_to_felem = ec_GFp_mont_bignum_to_felem;
  out->felem_to_bignum = ec_GFp_mont_felem_to_bignum;
  out->scalar_inv_montgomery = ec_simple_scalar_inv_montgomery;
  out->scalar_inv_montgomery_vartime = ec_GFp_simple_mont_inv_mod_ord_vartime;
  out->cmp_x_coordinate = ec_GFp_nistp256_cmp_x_coordinate;
}

#undef BORINGSSL_NISTP256_64BIT
