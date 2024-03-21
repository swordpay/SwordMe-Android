/* Originally written by Bodo Moeller for the OpenSSL project.
 * ====================================================================
 * Copyright (c) 1998-2005 The OpenSSL Project.  All rights reserved.
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
 * The Contribution is licensed pursuant to the OpenSSL open source
 * license provided above.
 *
 * The elliptic curve binary polynomial software is originally written by
 * Sheueling Chang Shantz and Douglas Stebila of Sun Microsystems
 * Laboratories. */

#ifndef OPENSSL_HEADER_EC_H
#define OPENSSL_HEADER_EC_H

#include <openssl/base.h>

#if defined(__cplusplus)
extern "C" {
#endif


// the encoding of a elliptic curve point (x,y)
typedef enum {



  POINT_CONVERSION_COMPRESSED = 2,


  POINT_CONVERSION_UNCOMPRESSED = 4,





  POINT_CONVERSION_HYBRID = 6,
} point_conversion_form_t;


// curve specified by |nid|, or NULL on unsupported NID or allocation failure.
//
// The supported NIDs are:
//   NID_secp224r1 (P-224),
//   NID_X9_62_prime256v1 (P-256),
//   NID_secp384r1 (P-384),
//   NID_secp521r1 (P-521)
//
// If in doubt, use |NID_X9_62_prime256v1|, or see the curve25519.h header for
// more modern primitives.
OPENSSL_EXPORT EC_GROUP *EC_GROUP_new_by_curve_name(int nid);

OPENSSL_EXPORT void EC_GROUP_free(EC_GROUP *group);

OPENSSL_EXPORT EC_GROUP *EC_GROUP_dup(const EC_GROUP *a);

// otherwise.
OPENSSL_EXPORT int EC_GROUP_cmp(const EC_GROUP *a, const EC_GROUP *b,
                                BN_CTX *ignored);

// in |group| that specifies the generator for the group.
OPENSSL_EXPORT const EC_POINT *EC_GROUP_get0_generator(const EC_GROUP *group);

// |group| that specifies the order of the group.
OPENSSL_EXPORT const BIGNUM *EC_GROUP_get0_order(const EC_GROUP *group);

OPENSSL_EXPORT int EC_GROUP_order_bits(const EC_GROUP *group);

// |ctx|, if it's not NULL. It returns one on success and zero otherwise.
OPENSSL_EXPORT int EC_GROUP_get_cofactor(const EC_GROUP *group,
                                         BIGNUM *cofactor, BN_CTX *ctx);

// |*out_p| to the order of the coordinate field and |*out_a| and |*out_b| to
// the parameters of the curve when expressed as y² = x³ + ax + b. Any of the
// output parameters can be NULL. It returns one on success and zero on
// error.
OPENSSL_EXPORT int EC_GROUP_get_curve_GFp(const EC_GROUP *group, BIGNUM *out_p,
                                          BIGNUM *out_a, BIGNUM *out_b,
                                          BN_CTX *ctx);

OPENSSL_EXPORT int EC_GROUP_get_curve_name(const EC_GROUP *group);

// element of the field underlying |group|.
OPENSSL_EXPORT unsigned EC_GROUP_get_degree(const EC_GROUP *group);

// |nid|, or NULL if |nid| is not a NIST curve. For example, it returns "P-256"
// for |NID_X9_62_prime256v1|.
OPENSSL_EXPORT const char *EC_curve_nid2nist(int nid);

// name |name|, or |NID_undef| if |name| is not a recognized name. For example,
// it returns |NID_X9_62_prime256v1| for "P-256".
OPENSSL_EXPORT int EC_curve_nist2nid(const char *name);


// on error.
OPENSSL_EXPORT EC_POINT *EC_POINT_new(const EC_GROUP *group);

OPENSSL_EXPORT void EC_POINT_free(EC_POINT *point);

// zero otherwise.
OPENSSL_EXPORT int EC_POINT_copy(EC_POINT *dest, const EC_POINT *src);

// |src|, or NULL on error.
OPENSSL_EXPORT EC_POINT *EC_POINT_dup(const EC_POINT *src,
                                      const EC_GROUP *group);

// given group.
OPENSSL_EXPORT int EC_POINT_set_to_infinity(const EC_GROUP *group,
                                            EC_POINT *point);

// zero otherwise.
OPENSSL_EXPORT int EC_POINT_is_at_infinity(const EC_GROUP *group,
                                           const EC_POINT *point);

// and zero otherwise or when an error occurs. This is different from OpenSSL,
// which returns -1 on error. If |ctx| is non-NULL, it may be used.
OPENSSL_EXPORT int EC_POINT_is_on_curve(const EC_GROUP *group,
                                        const EC_POINT *point, BN_CTX *ctx);

// not equal and -1 on error. If |ctx| is not NULL, it may be used.
OPENSSL_EXPORT int EC_POINT_cmp(const EC_GROUP *group, const EC_POINT *a,
                                const EC_POINT *b, BN_CTX *ctx);


// |point| using |ctx|, if it's not NULL. It returns one on success and zero
// otherwise.
//
// Either |x| or |y| may be NULL to skip computing that coordinate. This is
// slightly faster in the common case where only the x-coordinate is needed.
OPENSSL_EXPORT int EC_POINT_get_affine_coordinates_GFp(const EC_GROUP *group,
                                                       const EC_POINT *point,
                                                       BIGNUM *x, BIGNUM *y,
                                                       BN_CTX *ctx);

// (|x|, |y|). The |ctx| argument may be used if not NULL. It returns one
// on success or zero on error. Note that, unlike with OpenSSL, it's
// considered an error if the point is not on the curve.
OPENSSL_EXPORT int EC_POINT_set_affine_coordinates_GFp(const EC_GROUP *group,
                                                       EC_POINT *point,
                                                       const BIGNUM *x,
                                                       const BIGNUM *y,
                                                       BN_CTX *ctx);

// into, at most, |len| bytes at |buf|. It returns the number of bytes written
// or zero on error if |buf| is non-NULL, else the number of bytes needed. The
// |ctx| argument may be used if not NULL.
OPENSSL_EXPORT size_t EC_POINT_point2oct(const EC_GROUP *group,
                                         const EC_POINT *point,
                                         point_conversion_form_t form,
                                         uint8_t *buf, size_t len, BN_CTX *ctx);

// serialised point to |cbb|. It returns one on success and zero on error.
OPENSSL_EXPORT int EC_POINT_point2cbb(CBB *out, const EC_GROUP *group,
                                      const EC_POINT *point,
                                      point_conversion_form_t form,
                                      BN_CTX *ctx);

// serialisation in |buf|. It returns one on success and zero otherwise. The
// |ctx| argument may be used if not NULL.
OPENSSL_EXPORT int EC_POINT_oct2point(const EC_GROUP *group, EC_POINT *point,
                                      const uint8_t *buf, size_t len,
                                      BN_CTX *ctx);

// the given |x| coordinate and the y coordinate specified by |y_bit| (see
// X9.62). It returns one on success and zero otherwise.
OPENSSL_EXPORT int EC_POINT_set_compressed_coordinates_GFp(
    const EC_GROUP *group, EC_POINT *point, const BIGNUM *x, int y_bit,
    BN_CTX *ctx);


// zero otherwise. If |ctx| is not NULL, it may be used.
OPENSSL_EXPORT int EC_POINT_add(const EC_GROUP *group, EC_POINT *r,
                                const EC_POINT *a, const EC_POINT *b,
                                BN_CTX *ctx);

// zero otherwise. If |ctx| is not NULL, it may be used.
OPENSSL_EXPORT int EC_POINT_dbl(const EC_GROUP *group, EC_POINT *r,
                                const EC_POINT *a, BN_CTX *ctx);

// zero otherwise. If |ctx| is not NULL, it may be used.
OPENSSL_EXPORT int EC_POINT_invert(const EC_GROUP *group, EC_POINT *a,
                                   BN_CTX *ctx);

// otherwise. If |ctx| is not NULL, it may be used.
OPENSSL_EXPORT int EC_POINT_mul(const EC_GROUP *group, EC_POINT *r,
                                const BIGNUM *n, const EC_POINT *q,
                                const BIGNUM *m, BN_CTX *ctx);


// on the equation y² = x³ + a·x + b. It returns the new group or NULL on
// error.
//
// This new group has no generator. It is an error to use a generator-less group
// with any functions except for |EC_GROUP_free|, |EC_POINT_new|,
// |EC_POINT_set_affine_coordinates_GFp|, and |EC_GROUP_set_generator|.
//
// |EC_GROUP|s returned by this function will always compare as unequal via
// |EC_GROUP_cmp| (even to themselves). |EC_GROUP_get_curve_name| will always
// return |NID_undef|.
//
// Avoid using arbitrary curves and use |EC_GROUP_new_by_curve_name| instead.
OPENSSL_EXPORT EC_GROUP *EC_GROUP_new_curve_GFp(const BIGNUM *p,
                                                const BIGNUM *a,
                                                const BIGNUM *b, BN_CTX *ctx);

// must have the given order and cofactor. It may only be used with |EC_GROUP|
// objects returned by |EC_GROUP_new_curve_GFp| and may only be used once on
// each group. |generator| must have been created using |group|.
OPENSSL_EXPORT int EC_GROUP_set_generator(EC_GROUP *group,
                                          const EC_POINT *generator,
                                          const BIGNUM *order,
                                          const BIGNUM *cofactor);

// NULL. It returns one on success and zero otherwise. |ctx| is ignored. Use
// |EC_GROUP_get0_order| instead.
OPENSSL_EXPORT int EC_GROUP_get_order(const EC_GROUP *group, BIGNUM *order,
                                      BN_CTX *ctx);

OPENSSL_EXPORT void EC_GROUP_set_asn1_flag(EC_GROUP *group, int flag);

#define OPENSSL_EC_NAMED_CURVE 0
#define OPENSSL_EC_EXPLICIT_CURVE 1

typedef struct ec_method_st EC_METHOD;

OPENSSL_EXPORT const EC_METHOD *EC_GROUP_method_of(const EC_GROUP *group);

OPENSSL_EXPORT int EC_METHOD_get_field_type(const EC_METHOD *meth);

// |POINT_CONVERSION_UNCOMPRESSED| and otherwise does nothing.
OPENSSL_EXPORT void EC_GROUP_set_point_conversion_form(
    EC_GROUP *group, point_conversion_form_t form);

typedef struct {
  int nid;
  const char *comment;
} EC_builtin_curve;

// |out_curves| and returns the total number that it would have written, had
// |max_num_curves| been large enough.
//
// The |EC_builtin_curve| items describe the supported elliptic curves.
OPENSSL_EXPORT size_t EC_get_builtin_curves(EC_builtin_curve *out_curves,
                                            size_t max_num_curves);

OPENSSL_EXPORT void EC_POINT_clear_free(EC_POINT *point);


#if defined(__cplusplus)
}  // extern C
#endif

#include <openssl/ec_key.h>

#if defined(__cplusplus)
extern "C++" {

BSSL_NAMESPACE_BEGIN

BORINGSSL_MAKE_DELETER(EC_POINT, EC_POINT_free)
BORINGSSL_MAKE_DELETER(EC_GROUP, EC_GROUP_free)

BSSL_NAMESPACE_END

}  // extern C++

#endif

#define EC_R_BUFFER_TOO_SMALL 100
#define EC_R_COORDINATES_OUT_OF_RANGE 101
#define EC_R_D2I_ECPKPARAMETERS_FAILURE 102
#define EC_R_EC_GROUP_NEW_BY_NAME_FAILURE 103
#define EC_R_GROUP2PKPARAMETERS_FAILURE 104
#define EC_R_I2D_ECPKPARAMETERS_FAILURE 105
#define EC_R_INCOMPATIBLE_OBJECTS 106
#define EC_R_INVALID_COMPRESSED_POINT 107
#define EC_R_INVALID_COMPRESSION_BIT 108
#define EC_R_INVALID_ENCODING 109
#define EC_R_INVALID_FIELD 110
#define EC_R_INVALID_FORM 111
#define EC_R_INVALID_GROUP_ORDER 112
#define EC_R_INVALID_PRIVATE_KEY 113
#define EC_R_MISSING_PARAMETERS 114
#define EC_R_MISSING_PRIVATE_KEY 115
#define EC_R_NON_NAMED_CURVE 116
#define EC_R_NOT_INITIALIZED 117
#define EC_R_PKPARAMETERS2GROUP_FAILURE 118
#define EC_R_POINT_AT_INFINITY 119
#define EC_R_POINT_IS_NOT_ON_CURVE 120
#define EC_R_SLOT_FULL 121
#define EC_R_UNDEFINED_GENERATOR 122
#define EC_R_UNKNOWN_GROUP 123
#define EC_R_UNKNOWN_ORDER 124
#define EC_R_WRONG_ORDER 125
#define EC_R_BIGNUM_OUT_OF_RANGE 126
#define EC_R_WRONG_CURVE_PARAMETERS 127
#define EC_R_DECODE_ERROR 128
#define EC_R_ENCODE_ERROR 129
#define EC_R_GROUP_MISMATCH 130
#define EC_R_INVALID_COFACTOR 131
#define EC_R_PUBLIC_KEY_VALIDATION_FAILED 132
#define EC_R_INVALID_SCALAR 133

#endif  // OPENSSL_HEADER_EC_H
