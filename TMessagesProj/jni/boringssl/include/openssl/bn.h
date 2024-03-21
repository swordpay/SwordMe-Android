/* Copyright (C) 1995-1997 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */
/* ====================================================================
 * Copyright (c) 1998-2006 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */
/* ====================================================================
 * Copyright 2002 Sun Microsystems, Inc. ALL RIGHTS RESERVED.
 *
 * Portions of the attached software ("Contribution") are developed by
 * SUN MICROSYSTEMS, INC., and are contributed to the OpenSSL project.
 *
 * The Contribution is licensed pursuant to the Eric Young open source
 * license provided above.
 *
 * The binary polynomial arithmetic software is originally written by
 * Sheueling Chang Shantz and Douglas Stebila of Sun Microsystems
 * Laboratories. */

#ifndef OPENSSL_HEADER_BN_H
#define OPENSSL_HEADER_BN_H

#include <openssl/base.h>
#include <openssl/thread.h>

#include <inttypes.h>  // for PRIu64 and friends
#include <stdio.h>  // for FILE*

#if defined(__cplusplus)
extern "C" {
#endif

// although the largest integer supported by the compiler might be 64 bits, BN
// will allow you to work with numbers until you run out of memory.

//
// Note: on some platforms, inttypes.h does not define print format macros in
// C++ unless |__STDC_FORMAT_MACROS| defined. This is due to text in C99 which
// was never adopted in any C++ standard and explicitly overruled in C++11. As
// this is a public header, bn.h does not define |__STDC_FORMAT_MACROS| itself.
// Projects which use |BN_*_FMT*| with outdated C headers may need to define it
// externally.
#if defined(OPENSSL_64_BIT)
#define BN_ULONG uint64_t
#define BN_BITS2 64
#define BN_DEC_FMT1 "%" PRIu64
#define BN_DEC_FMT2 "%019" PRIu64
#define BN_HEX_FMT1 "%" PRIx64
#define BN_HEX_FMT2 "%016" PRIx64
#elif defined(OPENSSL_32_BIT)
#define BN_ULONG uint32_t
#define BN_BITS2 32
#define BN_DEC_FMT1 "%" PRIu32
#define BN_DEC_FMT2 "%09" PRIu32
#define BN_HEX_FMT1 "%" PRIx32
#define BN_HEX_FMT2 "%08" PRIx32
#else
#error "Must define either OPENSSL_32_BIT or OPENSSL_64_BIT"
#endif


OPENSSL_EXPORT BIGNUM *BN_new(void);

OPENSSL_EXPORT void BN_init(BIGNUM *bn);

// allocated on the heap, frees |bn| also.
OPENSSL_EXPORT void BN_free(BIGNUM *bn);

// originally allocated on the heap, frees |bn| also.
OPENSSL_EXPORT void BN_clear_free(BIGNUM *bn);

// allocated BIGNUM on success or NULL otherwise.
OPENSSL_EXPORT BIGNUM *BN_dup(const BIGNUM *src);

// failure.
OPENSSL_EXPORT BIGNUM *BN_copy(BIGNUM *dest, const BIGNUM *src);

OPENSSL_EXPORT void BN_clear(BIGNUM *bn);

OPENSSL_EXPORT const BIGNUM *BN_value_one(void);


// absolute value of |bn|.
OPENSSL_EXPORT unsigned BN_num_bits(const BIGNUM *bn);

// absolute value of |bn|.
OPENSSL_EXPORT unsigned BN_num_bytes(const BIGNUM *bn);

OPENSSL_EXPORT void BN_zero(BIGNUM *bn);

// failure.
OPENSSL_EXPORT int BN_one(BIGNUM *bn);

// allocation failure.
OPENSSL_EXPORT int BN_set_word(BIGNUM *bn, BN_ULONG value);

// allocation failure.
OPENSSL_EXPORT int BN_set_u64(BIGNUM *bn, uint64_t value);

OPENSSL_EXPORT void BN_set_negative(BIGNUM *bn, int sign);

OPENSSL_EXPORT int BN_is_negative(const BIGNUM *bn);


// a big-endian number, and returns |ret|. If |ret| is NULL then a fresh
// |BIGNUM| is allocated and returned. It returns NULL on allocation
// failure.
OPENSSL_EXPORT BIGNUM *BN_bin2bn(const uint8_t *in, size_t len, BIGNUM *ret);

// integer, which must have |BN_num_bytes| of space available. It returns the
// number of bytes written. Note this function leaks the magnitude of |in|. If
// |in| is secret, use |BN_bn2bin_padded| instead.
OPENSSL_EXPORT size_t BN_bn2bin(const BIGNUM *in, uint8_t *out);

// a little-endian number, and returns |ret|. If |ret| is NULL then a fresh
// |BIGNUM| is allocated and returned. It returns NULL on allocation
// failure.
OPENSSL_EXPORT BIGNUM *BN_le2bn(const uint8_t *in, size_t len, BIGNUM *ret);

// little-endian integer, which must have |len| of space available, padding
// out the remainder of out with zeros. If |len| is smaller than |BN_num_bytes|,
// the function fails and returns 0. Otherwise, it returns 1.
OPENSSL_EXPORT int BN_bn2le_padded(uint8_t *out, size_t len, const BIGNUM *in);

// big-endian integer. The integer is padded with leading zeros up to size
// |len|. If |len| is smaller than |BN_num_bytes|, the function fails and
// returns 0. Otherwise, it returns 1.
OPENSSL_EXPORT int BN_bn2bin_padded(uint8_t *out, size_t len, const BIGNUM *in);

OPENSSL_EXPORT int BN_bn2cbb_padded(CBB *out, size_t len, const BIGNUM *in);

// representation of |bn|. If |bn| is negative, the first char in the resulting
// string will be '-'. Returns NULL on allocation failure.
OPENSSL_EXPORT char *BN_bn2hex(const BIGNUM *bn);

// a '-' to indicate a negative number and may contain trailing, non-hex data.
// If |outp| is not NULL, it constructs a BIGNUM equal to the hex number and
// stores it in |*outp|. If |*outp| is NULL then it allocates a new BIGNUM and
// updates |*outp|. It returns the number of bytes of |in| processed or zero on
// error.
OPENSSL_EXPORT int BN_hex2bn(BIGNUM **outp, const char *in);

// decimal representation of |bn|. If |bn| is negative, the first char in the
// resulting string will be '-'. Returns NULL on allocation failure.
OPENSSL_EXPORT char *BN_bn2dec(const BIGNUM *a);

// proceeded by a '-' to indicate a negative number and may contain trailing,
// non-decimal data. If |outp| is not NULL, it constructs a BIGNUM equal to the
// decimal number and stores it in |*outp|. If |*outp| is NULL then it
// allocates a new BIGNUM and updates |*outp|. It returns the number of bytes
// of |in| processed or zero on error.
OPENSSL_EXPORT int BN_dec2bn(BIGNUM **outp, const char *in);

// begins with "0X" or "0x" (indicating hex) or not (indicating decimal). A
// leading '-' is still permitted and comes before the optional 0X/0x. It
// returns one on success or zero on error.
OPENSSL_EXPORT int BN_asc2bn(BIGNUM **outp, const char *in);

// and zero on error.
OPENSSL_EXPORT int BN_print(BIO *bio, const BIGNUM *a);

OPENSSL_EXPORT int BN_print_fp(FILE *fp, const BIGNUM *a);

// too large to be represented as a single word, the maximum possible value
// will be returned.
OPENSSL_EXPORT BN_ULONG BN_get_word(const BIGNUM *bn);

// returns one. If |bn| is too large to be represented as a |uint64_t|, it
// returns zero.
OPENSSL_EXPORT int BN_get_u64(const BIGNUM *bn, uint64_t *out);


// the result to |ret|. It returns one on success and zero on failure.
OPENSSL_EXPORT int BN_parse_asn1_unsigned(CBS *cbs, BIGNUM *ret);

// result to |cbb|. It returns one on success and zero on failure.
OPENSSL_EXPORT int BN_marshal_asn1(CBB *cbb, const BIGNUM *bn);

//
// Certain BIGNUM operations need to use many temporary variables and
// allocating and freeing them can be quite slow. Thus such operations typically
// take a |BN_CTX| parameter, which contains a pool of |BIGNUMs|. The |ctx|
// argument to a public function may be NULL, in which case a local |BN_CTX|
// will be created just for the lifetime of that call.
//
// A function must call |BN_CTX_start| first. Then, |BN_CTX_get| may be called
// repeatedly to obtain temporary |BIGNUM|s. All |BN_CTX_get| calls must be made
// before calling any other functions that use the |ctx| as an argument.
//
// Finally, |BN_CTX_end| must be called before returning from the function.
// When |BN_CTX_end| is called, the |BIGNUM| pointers obtained from
// |BN_CTX_get| become invalid.

OPENSSL_EXPORT BN_CTX *BN_CTX_new(void);

// itself.
OPENSSL_EXPORT void BN_CTX_free(BN_CTX *ctx);

// calls to |BN_CTX_get|.
OPENSSL_EXPORT void BN_CTX_start(BN_CTX *ctx);

// |BN_CTX_get| has returned NULL, all future calls will also return NULL until
// |BN_CTX_end| is called.
OPENSSL_EXPORT BIGNUM *BN_CTX_get(BN_CTX *ctx);

// matching |BN_CTX_start| call.
OPENSSL_EXPORT void BN_CTX_end(BN_CTX *ctx);


// or |b|. It returns one on success and zero on allocation failure.
OPENSSL_EXPORT int BN_add(BIGNUM *r, const BIGNUM *a, const BIGNUM *b);

// be the same pointer as either |a| or |b|. It returns one on success and zero
// on allocation failure.
OPENSSL_EXPORT int BN_uadd(BIGNUM *r, const BIGNUM *a, const BIGNUM *b);

OPENSSL_EXPORT int BN_add_word(BIGNUM *a, BN_ULONG w);

// or |b|. It returns one on success and zero on allocation failure.
OPENSSL_EXPORT int BN_sub(BIGNUM *r, const BIGNUM *a, const BIGNUM *b);

// |b| < |a| and |r| may be the same pointer as either |a| or |b|. It returns
// one on success and zero on allocation failure.
OPENSSL_EXPORT int BN_usub(BIGNUM *r, const BIGNUM *a, const BIGNUM *b);

// allocation failure.
OPENSSL_EXPORT int BN_sub_word(BIGNUM *a, BN_ULONG w);

// |b|. Returns one on success and zero otherwise.
OPENSSL_EXPORT int BN_mul(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
                          BN_CTX *ctx);

// allocation failure.
OPENSSL_EXPORT int BN_mul_word(BIGNUM *bn, BN_ULONG w);

// |a|. Returns one on success and zero otherwise. This is more efficient than
// BN_mul(r, a, a, ctx).
OPENSSL_EXPORT int BN_sqr(BIGNUM *r, const BIGNUM *a, BN_CTX *ctx);

// and the remainder in |rem|. Either of |quotient| or |rem| may be NULL, in
// which case the respective value is not returned. The result is rounded
// towards zero; thus if |numerator| is negative, the remainder will be zero or
// negative. It returns one on success or zero on error.
OPENSSL_EXPORT int BN_div(BIGNUM *quotient, BIGNUM *rem,
                          const BIGNUM *numerator, const BIGNUM *divisor,
                          BN_CTX *ctx);

// remainder or (BN_ULONG)-1 on error.
OPENSSL_EXPORT BN_ULONG BN_div_word(BIGNUM *numerator, BN_ULONG divisor);

// square root of |in|, using |ctx|. It returns one on success or zero on
// error. Negative numbers and non-square numbers will result in an error with
// appropriate errors on the error queue.
OPENSSL_EXPORT int BN_sqrt(BIGNUM *out_sqrt, const BIGNUM *in, BN_CTX *ctx);


// less than, equal to or greater than |b|, respectively.
OPENSSL_EXPORT int BN_cmp(const BIGNUM *a, const BIGNUM *b);

// |BN_ULONG| instead of a |BIGNUM|.
OPENSSL_EXPORT int BN_cmp_word(const BIGNUM *a, BN_ULONG b);

// absolute value of |a| is less than, equal to or greater than the absolute
// value of |b|, respectively.
OPENSSL_EXPORT int BN_ucmp(const BIGNUM *a, const BIGNUM *b);

// It takes an amount of time dependent on the sizes of |a| and |b|, but
// independent of the contents (including the signs) of |a| and |b|.
OPENSSL_EXPORT int BN_equal_consttime(const BIGNUM *a, const BIGNUM *b);

// otherwise.
OPENSSL_EXPORT int BN_abs_is_word(const BIGNUM *bn, BN_ULONG w);

OPENSSL_EXPORT int BN_is_zero(const BIGNUM *bn);

OPENSSL_EXPORT int BN_is_one(const BIGNUM *bn);

OPENSSL_EXPORT int BN_is_word(const BIGNUM *bn, BN_ULONG w);

OPENSSL_EXPORT int BN_is_odd(const BIGNUM *bn);

OPENSSL_EXPORT int BN_is_pow2(const BIGNUM *a);


// same |BIGNUM|. It returns one on success and zero on allocation failure.
OPENSSL_EXPORT int BN_lshift(BIGNUM *r, const BIGNUM *a, int n);

// pointer. It returns one on success and zero on allocation failure.
OPENSSL_EXPORT int BN_lshift1(BIGNUM *r, const BIGNUM *a);

// pointer. It returns one on success and zero on allocation failure.
OPENSSL_EXPORT int BN_rshift(BIGNUM *r, const BIGNUM *a, int n);

// pointer. It returns one on success and zero on allocation failure.
OPENSSL_EXPORT int BN_rshift1(BIGNUM *r, const BIGNUM *a);

// is 2 then setting bit zero will make it 3. It returns one on success or zero
// on allocation failure.
OPENSSL_EXPORT int BN_set_bit(BIGNUM *a, int n);

// |a| is 3, clearing bit zero will make it two. It returns one on success or
// zero on allocation failure.
OPENSSL_EXPORT int BN_clear_bit(BIGNUM *a, int n);

// and is set. Otherwise, it returns zero.
OPENSSL_EXPORT int BN_is_bit_set(const BIGNUM *a, int n);

// on success or zero if |n| is negative.
//
// This differs from OpenSSL which additionally returns zero if |a|'s word
// length is less than or equal to |n|, rounded down to a number of words. Note
// word size is platform-dependent, so this behavior is also difficult to rely
// on in OpenSSL and not very useful.
OPENSSL_EXPORT int BN_mask_bits(BIGNUM *a, int n);

// the number of factors of two which divide it. It returns zero if |bn| is
// zero.
OPENSSL_EXPORT int BN_count_low_zero_bits(const BIGNUM *bn);


OPENSSL_EXPORT BN_ULONG BN_mod_word(const BIGNUM *a, BN_ULONG w);

// 0 on error.
OPENSSL_EXPORT int BN_mod_pow2(BIGNUM *r, const BIGNUM *a, size_t e);

// It returns 1 on success and 0 on error.
OPENSSL_EXPORT int BN_nnmod_pow2(BIGNUM *r, const BIGNUM *a, size_t e);

#define BN_mod(rem, numerator, divisor, ctx) \
  BN_div(NULL, (rem), (numerator), (divisor), (ctx))

// |rem| < |divisor| is always true. It returns one on success and zero on
// error.
OPENSSL_EXPORT int BN_nnmod(BIGNUM *rem, const BIGNUM *numerator,
                            const BIGNUM *divisor, BN_CTX *ctx);

// on error.
OPENSSL_EXPORT int BN_mod_add(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
                              const BIGNUM *m, BN_CTX *ctx);

// non-negative and less than |m|.
OPENSSL_EXPORT int BN_mod_add_quick(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
                                    const BIGNUM *m);

// on error.
OPENSSL_EXPORT int BN_mod_sub(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
                              const BIGNUM *m, BN_CTX *ctx);

// non-negative and less than |m|.
OPENSSL_EXPORT int BN_mod_sub_quick(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
                                    const BIGNUM *m);

// on error.
OPENSSL_EXPORT int BN_mod_mul(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
                              const BIGNUM *m, BN_CTX *ctx);

// on error.
OPENSSL_EXPORT int BN_mod_sqr(BIGNUM *r, const BIGNUM *a, const BIGNUM *m,
                              BN_CTX *ctx);

// same pointer. It returns one on success and zero on error.
OPENSSL_EXPORT int BN_mod_lshift(BIGNUM *r, const BIGNUM *a, int n,
                                 const BIGNUM *m, BN_CTX *ctx);

// non-negative and less than |m|.
OPENSSL_EXPORT int BN_mod_lshift_quick(BIGNUM *r, const BIGNUM *a, int n,
                                       const BIGNUM *m);

// same pointer. It returns one on success and zero on error.
OPENSSL_EXPORT int BN_mod_lshift1(BIGNUM *r, const BIGNUM *a, const BIGNUM *m,
                                  BN_CTX *ctx);

// non-negative and less than |m|.
OPENSSL_EXPORT int BN_mod_lshift1_quick(BIGNUM *r, const BIGNUM *a,
                                        const BIGNUM *m);

// r^2 == a (mod p). |p| must be a prime. It returns NULL on error or if |a| is
// not a square mod |p|. In the latter case, it will add |BN_R_NOT_A_SQUARE| to
// the error queue.
OPENSSL_EXPORT BIGNUM *BN_mod_sqrt(BIGNUM *in, const BIGNUM *a, const BIGNUM *p,
                                   BN_CTX *ctx);


#define BN_RAND_TOP_ANY    (-1)
#define BN_RAND_TOP_ONE     0
#define BN_RAND_TOP_TWO     1

#define BN_RAND_BOTTOM_ANY  0
#define BN_RAND_BOTTOM_ODD  1

// success and zero otherwise.
//
// |top| must be one of the |BN_RAND_TOP_*| values. If |BN_RAND_TOP_ONE|, the
// most-significant bit, if any, will be set. If |BN_RAND_TOP_TWO|, the two
// most significant bits, if any, will be set. If |BN_RAND_TOP_ANY|, no extra
// action will be taken and |BN_num_bits(rnd)| may not equal |bits| if the most
// significant bits randomly ended up as zeros.
//
// |bottom| must be one of the |BN_RAND_BOTTOM_*| values. If
// |BN_RAND_BOTTOM_ODD|, the least-significant bit, if any, will be set. If
// |BN_RAND_BOTTOM_ANY|, no extra action will be taken.
OPENSSL_EXPORT int BN_rand(BIGNUM *rnd, int bits, int top, int bottom);

OPENSSL_EXPORT int BN_pseudo_rand(BIGNUM *rnd, int bits, int top, int bottom);

// to zero and |max_exclusive| set to |range|.
OPENSSL_EXPORT int BN_rand_range(BIGNUM *rnd, const BIGNUM *range);

// [min_inclusive..max_exclusive). It returns one on success and zero
// otherwise.
OPENSSL_EXPORT int BN_rand_range_ex(BIGNUM *r, BN_ULONG min_inclusive,
                                    const BIGNUM *max_exclusive);

OPENSSL_EXPORT int BN_pseudo_rand_range(BIGNUM *rnd, const BIGNUM *range);

#define BN_GENCB_GENERATED 0
#define BN_GENCB_PRIME_TEST 1

// generation functions that can take a very long time to complete. Use
// |BN_GENCB_set| to initialise a |BN_GENCB| structure.
//
// The callback receives the address of that |BN_GENCB| structure as its last
// argument and the user is free to put an arbitrary pointer in |arg|. The other
// arguments are set as follows:
//   event=BN_GENCB_GENERATED, n=i:   after generating the i'th possible prime
//                                    number.
//   event=BN_GENCB_PRIME_TEST, n=-1: when finished trial division primality
//                                    checks.
//   event=BN_GENCB_PRIME_TEST, n=i:  when the i'th primality test has finished.
//
// The callback can return zero to abort the generation progress or one to
// allow it to continue.
//
// When other code needs to call a BN generation function it will often take a
// BN_GENCB argument and may call the function with other argument values.
struct bn_gencb_st {
  void *arg;        // callback-specific data
  int (*callback)(int event, int n, struct bn_gencb_st *);
};

// |arg|.
OPENSSL_EXPORT void BN_GENCB_set(BN_GENCB *callback,
                                 int (*f)(int event, int n, BN_GENCB *),
                                 void *arg);

// the callback, or 1 if |callback| is NULL.
OPENSSL_EXPORT int BN_GENCB_call(BN_GENCB *callback, int event, int n);

// is non-zero then the prime will be such that (ret-1)/2 is also a prime.
// (This is needed for Diffie-Hellman groups to ensure that the only subgroups
// are of size 2 and (p-1)/2.).
//
// If |add| is not NULL, the prime will fulfill the condition |ret| % |add| ==
// |rem| in order to suit a given generator. (If |rem| is NULL then |ret| %
// |add| == 1.)
//
// If |cb| is not NULL, it will be called during processing to give an
// indication of progress. See the comments for |BN_GENCB|. It returns one on
// success and zero otherwise.
OPENSSL_EXPORT int BN_generate_prime_ex(BIGNUM *ret, int bits, int safe,
                                        const BIGNUM *add, const BIGNUM *rem,
                                        BN_GENCB *cb);

// the primality testing functions in order to automatically select a number of
// Miller-Rabin checks that gives a false positive rate of ~2^{-80}.
#define BN_prime_checks 0

enum bn_primality_result_t {
  bn_probably_prime,
  bn_composite,
  bn_non_prime_power_composite,
};

// number using the Enhanced Miller-Rabin Test (FIPS 186-4 C.3.2) with
// |iterations| iterations and returns the result in |out_result|. Enhanced
// Miller-Rabin tests primality for odd integers greater than 3, returning
// |bn_probably_prime| if the number is probably prime,
// |bn_non_prime_power_composite| if the number is a composite that is not the
// power of a single prime, and |bn_composite| otherwise. It returns one on
// success and zero on failure. If |cb| is not NULL, then it is called during
// each iteration of the primality test.
//
// If |iterations| is |BN_prime_checks|, then a value that results in a false
// positive rate lower than the number-field sieve security level of |w| is
// used, provided |w| was generated randomly. |BN_prime_checks| is not suitable
// for inputs potentially crafted by an adversary.
OPENSSL_EXPORT int BN_enhanced_miller_rabin_primality_test(
    enum bn_primality_result_t *out_result, const BIGNUM *w, int iterations,
    BN_CTX *ctx, BN_GENCB *cb);

// probably a prime number by the Miller-Rabin test or zero if it's certainly
// not.
//
// If |do_trial_division| is non-zero then |candidate| will be tested against a
// list of small primes before Miller-Rabin tests. The probability of this
// function returning a false positive is 2^{2*checks}. If |checks| is
// |BN_prime_checks| then a value that results in a false positive rate lower
// than the number-field sieve security level of |candidate| is used, provided
// |candidate| was generated randomly. |BN_prime_checks| is not suitable for
// inputs potentially crafted by an adversary.
//
// If |cb| is not NULL then it is called during the checking process. See the
// comment above |BN_GENCB|.
//
// The function returns one on success and zero on error.
OPENSSL_EXPORT int BN_primality_test(int *is_probably_prime,
                                     const BIGNUM *candidate, int checks,
                                     BN_CTX *ctx, int do_trial_division,
                                     BN_GENCB *cb);

// number by the Miller-Rabin test, zero if it's certainly not and -1 on error.
//
// If |do_trial_division| is non-zero then |candidate| will be tested against a
// list of small primes before Miller-Rabin tests. The probability of this
// function returning one when |candidate| is composite is 2^{2*checks}. If
// |checks| is |BN_prime_checks| then a value that results in a false positive
// rate lower than the number-field sieve security level of |candidate| is used,
// provided |candidate| was generated randomly. |BN_prime_checks| is not
// suitable for inputs potentially crafted by an adversary.
//
// If |cb| is not NULL then it is called during the checking process. See the
// comment above |BN_GENCB|.
//
// WARNING: deprecated. Use |BN_primality_test|.
OPENSSL_EXPORT int BN_is_prime_fasttest_ex(const BIGNUM *candidate, int checks,
                                           BN_CTX *ctx, int do_trial_division,
                                           BN_GENCB *cb);

// |do_trial_division| set to zero.
//
// WARNING: deprecated: Use |BN_primality_test|.
OPENSSL_EXPORT int BN_is_prime_ex(const BIGNUM *candidate, int checks,
                                  BN_CTX *ctx, BN_GENCB *cb);


// otherwise.
OPENSSL_EXPORT int BN_gcd(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
                          BN_CTX *ctx);

// fresh BIGNUM is allocated. It returns the result or NULL on error.
//
// If |n| is even then the operation is performed using an algorithm that avoids
// some branches but which isn't constant-time. This function shouldn't be used
// for secret values; use |BN_mod_inverse_blinded| instead. Or, if |n| is
// guaranteed to be prime, use
// |BN_mod_exp_mont_consttime(out, a, m_minus_2, m, ctx, m_mont)|, taking
// advantage of Fermat's Little Theorem.
OPENSSL_EXPORT BIGNUM *BN_mod_inverse(BIGNUM *out, const BIGNUM *a,
                                      const BIGNUM *n, BN_CTX *ctx);

// Montgomery modulus for |mont|. |a| must be non-negative and must be less
// than |n|. |n| must be greater than 1. |a| is blinded (masked by a random
// value) to protect it against side-channel attacks. On failure, if the failure
// was caused by |a| having no inverse mod |n| then |*out_no_inverse| will be
// set to one; otherwise it will be set to zero.
//
// Note this function may incorrectly report |a| has no inverse if the random
// blinding value has no inverse. It should only be used when |n| has few
// non-invertible elements, such as an RSA modulus.
int BN_mod_inverse_blinded(BIGNUM *out, int *out_no_inverse, const BIGNUM *a,
                           const BN_MONT_CTX *mont, BN_CTX *ctx);

// non-negative and must be less than |n|. |n| must be odd. This function
// shouldn't be used for secret values; use |BN_mod_inverse_blinded| instead.
// Or, if |n| is guaranteed to be prime, use
// |BN_mod_exp_mont_consttime(out, a, m_minus_2, m, ctx, m_mont)|, taking
// advantage of Fermat's Little Theorem. It returns one on success or zero on
// failure. On failure, if the failure was caused by |a| having no inverse mod
// |n| then |*out_no_inverse| will be set to one; otherwise it will be set to
// zero.
int BN_mod_inverse_odd(BIGNUM *out, int *out_no_inverse, const BIGNUM *a,
                       const BIGNUM *n, BN_CTX *ctx);


// Montgomery domain.

// |mod| or NULL on error. Note this function assumes |mod| is public.
OPENSSL_EXPORT BN_MONT_CTX *BN_MONT_CTX_new_for_modulus(const BIGNUM *mod,
                                                        BN_CTX *ctx);

// treats |mod| as secret.
OPENSSL_EXPORT BN_MONT_CTX *BN_MONT_CTX_new_consttime(const BIGNUM *mod,
                                                      BN_CTX *ctx);

OPENSSL_EXPORT void BN_MONT_CTX_free(BN_MONT_CTX *mont);

// NULL on error.
OPENSSL_EXPORT BN_MONT_CTX *BN_MONT_CTX_copy(BN_MONT_CTX *to,
                                             const BN_MONT_CTX *from);

// so, it creates a new |BN_MONT_CTX| and sets the modulus for it to |mod|. It
// then stores it as |*pmont|. It returns one on success and zero on error. Note
// this function assumes |mod| is public.
//
// If |*pmont| is already non-NULL then it does nothing and returns one.
int BN_MONT_CTX_set_locked(BN_MONT_CTX **pmont, CRYPTO_MUTEX *lock,
                           const BIGNUM *mod, BN_CTX *bn_ctx);

// assumed to be in the range [0, n), where |n| is the Montgomery modulus. It
// returns one on success or zero on error.
OPENSSL_EXPORT int BN_to_montgomery(BIGNUM *ret, const BIGNUM *a,
                                    const BN_MONT_CTX *mont, BN_CTX *ctx);

// of the Montgomery domain. |a| is assumed to be in the range [0, n), where |n|
// is the Montgomery modulus. It returns one on success or zero on error.
OPENSSL_EXPORT int BN_from_montgomery(BIGNUM *ret, const BIGNUM *a,
                                      const BN_MONT_CTX *mont, BN_CTX *ctx);

// Both |a| and |b| must already be in the Montgomery domain (by
// |BN_to_montgomery|). In particular, |a| and |b| are assumed to be in the
// range [0, n), where |n| is the Montgomery modulus. It returns one on success
// or zero on error.
OPENSSL_EXPORT int BN_mod_mul_montgomery(BIGNUM *r, const BIGNUM *a,
                                         const BIGNUM *b,
                                         const BN_MONT_CTX *mont, BN_CTX *ctx);


// algorithm that leaks side-channel information. It returns one on success or
// zero otherwise.
OPENSSL_EXPORT int BN_exp(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
                          BN_CTX *ctx);

// algorithm for the values provided. It returns one on success or zero
// otherwise. The |BN_mod_exp_mont_consttime| variant must be used if the
// exponent is secret.
OPENSSL_EXPORT int BN_mod_exp(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
                              const BIGNUM *m, BN_CTX *ctx);

// requires 0 <= |a| < |m|.
OPENSSL_EXPORT int BN_mod_exp_mont(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
                                   const BIGNUM *m, BN_CTX *ctx,
                                   const BN_MONT_CTX *mont);

// |m| as secret and requires 0 <= |a| < |m|.
OPENSSL_EXPORT int BN_mod_exp_mont_consttime(BIGNUM *rr, const BIGNUM *a,
                                             const BIGNUM *p, const BIGNUM *m,
                                             BN_CTX *ctx,
                                             const BN_MONT_CTX *mont);


// of the number's length in bytes represented as a 4-byte big-endian number,
// and the number itself in big-endian format, where the most significant bit
// signals a negative number. (The representation of numbers with the MSB set is
// prefixed with null byte). |out| must have sufficient space available; to
// find the needed amount of space, call the function with |out| set to NULL.
OPENSSL_EXPORT size_t BN_bn2mpi(const BIGNUM *in, uint8_t *out);

// bytes at |in| are expected to be in the format emitted by |BN_bn2mpi|.
//
// If |out| is NULL then a fresh |BIGNUM| is allocated and returned, otherwise
// |out| is reused and returned. On error, NULL is returned and the error queue
// is updated.
OPENSSL_EXPORT BIGNUM *BN_mpi2bn(const uint8_t *in, size_t len, BIGNUM *out);

// given as a |BN_ULONG| instead of a |BIGNUM *|. It returns one on success
// or zero otherwise.
OPENSSL_EXPORT int BN_mod_exp_mont_word(BIGNUM *r, BN_ULONG a, const BIGNUM *p,
                                        const BIGNUM *m, BN_CTX *ctx,
                                        const BN_MONT_CTX *mont);

// or zero otherwise.
OPENSSL_EXPORT int BN_mod_exp2_mont(BIGNUM *r, const BIGNUM *a1,
                                    const BIGNUM *p1, const BIGNUM *a2,
                                    const BIGNUM *p2, const BIGNUM *m,
                                    BN_CTX *ctx, const BN_MONT_CTX *mont);

// Use |BN_MONT_CTX_new_for_modulus| instead.
OPENSSL_EXPORT BN_MONT_CTX *BN_MONT_CTX_new(void);

// returns one on success and zero on error. Use |BN_MONT_CTX_new_for_modulus|
// instead.
OPENSSL_EXPORT int BN_MONT_CTX_set(BN_MONT_CTX *mont, const BIGNUM *mod,
                                   BN_CTX *ctx);

// and -1 on error.
//
// Use |BN_bn2bin_padded| instead. It is |size_t|-clean.
OPENSSL_EXPORT int BN_bn2binpad(const BIGNUM *in, uint8_t *out, int len);


struct bignum_st {


  BN_ULONG *d;














  int width;

  int dmax;

  int neg;

  int flags;
};

struct bn_mont_ctx_st {


  BIGNUM RR;


  BIGNUM N;
  BN_ULONG n0[2];  // least significant words of (R*Ri-1)/N
};

OPENSSL_EXPORT unsigned BN_num_bits_word(BN_ULONG l);

#define BN_FLG_MALLOCED 0x01
#define BN_FLG_STATIC_DATA 0x02
// |BN_FLG_CONSTTIME| has been removed and intentionally omitted so code relying
// on it will not compile. Consumers outside BoringSSL should use the
// higher-level cryptographic algorithms exposed by other modules. Consumers
// within the library should call the appropriate timing-sensitive algorithm
// directly.


#if defined(__cplusplus)
}  // extern C

#if !defined(BORINGSSL_NO_CXX)
extern "C++" {

BSSL_NAMESPACE_BEGIN

BORINGSSL_MAKE_DELETER(BIGNUM, BN_free)
BORINGSSL_MAKE_DELETER(BN_CTX, BN_CTX_free)
BORINGSSL_MAKE_DELETER(BN_MONT_CTX, BN_MONT_CTX_free)

class BN_CTXScope {
 public:
  BN_CTXScope(BN_CTX *ctx) : ctx_(ctx) { BN_CTX_start(ctx_); }
  ~BN_CTXScope() { BN_CTX_end(ctx_); }

 private:
  BN_CTX *ctx_;

  BN_CTXScope(BN_CTXScope &) = delete;
  BN_CTXScope &operator=(BN_CTXScope &) = delete;
};

BSSL_NAMESPACE_END

}  // extern C++
#endif

#endif

#define BN_R_ARG2_LT_ARG3 100
#define BN_R_BAD_RECIPROCAL 101
#define BN_R_BIGNUM_TOO_LONG 102
#define BN_R_BITS_TOO_SMALL 103
#define BN_R_CALLED_WITH_EVEN_MODULUS 104
#define BN_R_DIV_BY_ZERO 105
#define BN_R_EXPAND_ON_STATIC_BIGNUM_DATA 106
#define BN_R_INPUT_NOT_REDUCED 107
#define BN_R_INVALID_RANGE 108
#define BN_R_NEGATIVE_NUMBER 109
#define BN_R_NOT_A_SQUARE 110
#define BN_R_NOT_INITIALIZED 111
#define BN_R_NO_INVERSE 112
#define BN_R_PRIVATE_KEY_TOO_LARGE 113
#define BN_R_P_IS_NOT_PRIME 114
#define BN_R_TOO_MANY_ITERATIONS 115
#define BN_R_TOO_MANY_TEMPORARY_VARIABLES 116
#define BN_R_BAD_ENCODING 117
#define BN_R_ENCODE_ERROR 118
#define BN_R_INVALID_INPUT 119

#endif  // OPENSSL_HEADER_BN_H
