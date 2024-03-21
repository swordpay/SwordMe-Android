/* Originally written by Bodo Moeller and Nils Larsch for the OpenSSL project.
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

#include <openssl/ec.h>

#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/mem.h>

#include "../bn/internal.h"
#include "../delocate.h"
#include "internal.h"


int ec_GFp_mont_group_init(EC_GROUP *group) {
  int ok;

  ok = ec_GFp_simple_group_init(group);
  group->mont = NULL;
  return ok;
}

void ec_GFp_mont_group_finish(EC_GROUP *group) {
  BN_MONT_CTX_free(group->mont);
  group->mont = NULL;
  ec_GFp_simple_group_finish(group);
}

int ec_GFp_mont_group_set_curve(EC_GROUP *group, const BIGNUM *p,
                                const BIGNUM *a, const BIGNUM *b, BN_CTX *ctx) {
  BN_CTX *new_ctx = NULL;
  int ret = 0;

  BN_MONT_CTX_free(group->mont);
  group->mont = NULL;

  if (ctx == NULL) {
    ctx = new_ctx = BN_CTX_new();
    if (ctx == NULL) {
      return 0;
    }
  }

  group->mont = BN_MONT_CTX_new_for_modulus(p, ctx);
  if (group->mont == NULL) {
    OPENSSL_PUT_ERROR(EC, ERR_R_BN_LIB);
    goto err;
  }

  ret = ec_GFp_simple_group_set_curve(group, p, a, b, ctx);

  if (!ret) {
    BN_MONT_CTX_free(group->mont);
    group->mont = NULL;
  }

err:
  BN_CTX_free(new_ctx);
  return ret;
}

static void ec_GFp_mont_felem_to_montgomery(const EC_GROUP *group,
                                            EC_FELEM *out, const EC_FELEM *in) {
  bn_to_montgomery_small(out->words, in->words, group->field.width,
                         group->mont);
}

static void ec_GFp_mont_felem_from_montgomery(const EC_GROUP *group,
                                              EC_FELEM *out,
                                              const EC_FELEM *in) {
  bn_from_montgomery_small(out->words, in->words, group->field.width,
                           group->mont);
}

static void ec_GFp_mont_felem_inv(const EC_GROUP *group, EC_FELEM *out,
                                  const EC_FELEM *a) {
  bn_mod_inverse_prime_mont_small(out->words, a->words, group->field.width,
                                  group->mont);
}

void ec_GFp_mont_felem_mul(const EC_GROUP *group, EC_FELEM *r,
                           const EC_FELEM *a, const EC_FELEM *b) {
  bn_mod_mul_montgomery_small(r->words, a->words, b->words, group->field.width,
                              group->mont);
}

void ec_GFp_mont_felem_sqr(const EC_GROUP *group, EC_FELEM *r,
                           const EC_FELEM *a) {
  bn_mod_mul_montgomery_small(r->words, a->words, a->words, group->field.width,
                              group->mont);
}

int ec_GFp_mont_bignum_to_felem(const EC_GROUP *group, EC_FELEM *out,
                                const BIGNUM *in) {
  if (group->mont == NULL) {
    OPENSSL_PUT_ERROR(EC, EC_R_NOT_INITIALIZED);
    return 0;
  }

  if (!bn_copy_words(out->words, group->field.width, in)) {
    return 0;
  }
  ec_GFp_mont_felem_to_montgomery(group, out, out);
  return 1;
}

int ec_GFp_mont_felem_to_bignum(const EC_GROUP *group, BIGNUM *out,
                                const EC_FELEM *in) {
  if (group->mont == NULL) {
    OPENSSL_PUT_ERROR(EC, EC_R_NOT_INITIALIZED);
    return 0;
  }

  EC_FELEM tmp;
  ec_GFp_mont_felem_from_montgomery(group, &tmp, in);
  return bn_set_words(out, tmp.words, group->field.width);
}

static int ec_GFp_mont_point_get_affine_coordinates(const EC_GROUP *group,
                                                    const EC_RAW_POINT *point,
                                                    EC_FELEM *x, EC_FELEM *y) {
  if (ec_GFp_simple_is_at_infinity(group, point)) {
    OPENSSL_PUT_ERROR(EC, EC_R_POINT_AT_INFINITY);
    return 0;
  }


  EC_FELEM z1, z2;
  ec_GFp_mont_felem_inv(group, &z2, &point->Z);
  ec_GFp_mont_felem_sqr(group, &z1, &z2);




  ec_GFp_mont_felem_from_montgomery(group, &z1, &z1);

  if (x != NULL) {
    ec_GFp_mont_felem_mul(group, x, &point->X, &z1);
  }

  if (y != NULL) {
    ec_GFp_mont_felem_mul(group, &z1, &z1, &z2);
    ec_GFp_mont_felem_mul(group, y, &point->Y, &z1);
  }

  return 1;
}

void ec_GFp_mont_add(const EC_GROUP *group, EC_RAW_POINT *out,
                     const EC_RAW_POINT *a, const EC_RAW_POINT *b) {
  if (a == b) {
    ec_GFp_mont_dbl(group, out, a);
    return;
  }






  EC_FELEM x_out, y_out, z_out;
  BN_ULONG z1nz = ec_felem_non_zero_mask(group, &a->Z);
  BN_ULONG z2nz = ec_felem_non_zero_mask(group, &b->Z);

  EC_FELEM z1z1;
  ec_GFp_mont_felem_sqr(group, &z1z1, &a->Z);

  EC_FELEM z2z2;
  ec_GFp_mont_felem_sqr(group, &z2z2, &b->Z);

  EC_FELEM u1;
  ec_GFp_mont_felem_mul(group, &u1, &a->X, &z2z2);

  EC_FELEM two_z1z2;
  ec_felem_add(group, &two_z1z2, &a->Z, &b->Z);
  ec_GFp_mont_felem_sqr(group, &two_z1z2, &two_z1z2);
  ec_felem_sub(group, &two_z1z2, &two_z1z2, &z1z1);
  ec_felem_sub(group, &two_z1z2, &two_z1z2, &z2z2);

  EC_FELEM s1;
  ec_GFp_mont_felem_mul(group, &s1, &b->Z, &z2z2);
  ec_GFp_mont_felem_mul(group, &s1, &s1, &a->Y);

  EC_FELEM u2;
  ec_GFp_mont_felem_mul(group, &u2, &b->X, &z1z1);

  EC_FELEM h;
  ec_felem_sub(group, &h, &u2, &u1);

  BN_ULONG xneq = ec_felem_non_zero_mask(group, &h);

  ec_GFp_mont_felem_mul(group, &z_out, &h, &two_z1z2);

  EC_FELEM z1z1z1;
  ec_GFp_mont_felem_mul(group, &z1z1z1, &a->Z, &z1z1);

  EC_FELEM s2;
  ec_GFp_mont_felem_mul(group, &s2, &b->Y, &z1z1z1);

  EC_FELEM r;
  ec_felem_sub(group, &r, &s2, &s1);
  ec_felem_add(group, &r, &r, &r);

  BN_ULONG yneq = ec_felem_non_zero_mask(group, &r);

  BN_ULONG is_nontrivial_double = ~xneq & ~yneq & z1nz & z2nz;
  if (is_nontrivial_double) {
    ec_GFp_mont_dbl(group, out, a);
    return;
  }

  EC_FELEM i;
  ec_felem_add(group, &i, &h, &h);
  ec_GFp_mont_felem_sqr(group, &i, &i);

  EC_FELEM j;
  ec_GFp_mont_felem_mul(group, &j, &h, &i);

  EC_FELEM v;
  ec_GFp_mont_felem_mul(group, &v, &u1, &i);

  ec_GFp_mont_felem_sqr(group, &x_out, &r);
  ec_felem_sub(group, &x_out, &x_out, &j);
  ec_felem_sub(group, &x_out, &x_out, &v);
  ec_felem_sub(group, &x_out, &x_out, &v);

  ec_felem_sub(group, &y_out, &v, &x_out);
  ec_GFp_mont_felem_mul(group, &y_out, &y_out, &r);
  EC_FELEM s1j;
  ec_GFp_mont_felem_mul(group, &s1j, &s1, &j);
  ec_felem_sub(group, &y_out, &y_out, &s1j);
  ec_felem_sub(group, &y_out, &y_out, &s1j);

  ec_felem_select(group, &x_out, z1nz, &x_out, &b->X);
  ec_felem_select(group, &out->X, z2nz, &x_out, &a->X);
  ec_felem_select(group, &y_out, z1nz, &y_out, &b->Y);
  ec_felem_select(group, &out->Y, z2nz, &y_out, &a->Y);
  ec_felem_select(group, &z_out, z1nz, &z_out, &b->Z);
  ec_felem_select(group, &out->Z, z2nz, &z_out, &a->Z);
}

void ec_GFp_mont_dbl(const EC_GROUP *group, EC_RAW_POINT *r,
                     const EC_RAW_POINT *a) {
  if (group->a_is_minus3) {






    EC_FELEM delta, gamma, beta, ftmp, ftmp2, tmptmp, alpha, fourbeta;

    ec_GFp_mont_felem_sqr(group, &delta, &a->Z);

    ec_GFp_mont_felem_sqr(group, &gamma, &a->Y);

    ec_GFp_mont_felem_mul(group, &beta, &a->X, &gamma);

    ec_felem_sub(group, &ftmp, &a->X, &delta);
    ec_felem_add(group, &ftmp2, &a->X, &delta);

    ec_felem_add(group, &tmptmp, &ftmp2, &ftmp2);
    ec_felem_add(group, &ftmp2, &ftmp2, &tmptmp);
    ec_GFp_mont_felem_mul(group, &alpha, &ftmp, &ftmp2);

    ec_GFp_mont_felem_sqr(group, &r->X, &alpha);
    ec_felem_add(group, &fourbeta, &beta, &beta);
    ec_felem_add(group, &fourbeta, &fourbeta, &fourbeta);
    ec_felem_add(group, &tmptmp, &fourbeta, &fourbeta);
    ec_felem_sub(group, &r->X, &r->X, &tmptmp);

    ec_felem_add(group, &delta, &gamma, &delta);
    ec_felem_add(group, &ftmp, &a->Y, &a->Z);
    ec_GFp_mont_felem_sqr(group, &r->Z, &ftmp);
    ec_felem_sub(group, &r->Z, &r->Z, &delta);

    ec_felem_sub(group, &r->Y, &fourbeta, &r->X);
    ec_felem_add(group, &gamma, &gamma, &gamma);
    ec_GFp_mont_felem_sqr(group, &gamma, &gamma);
    ec_GFp_mont_felem_mul(group, &r->Y, &alpha, &r->Y);
    ec_felem_add(group, &gamma, &gamma, &gamma);
    ec_felem_sub(group, &r->Y, &r->Y, &gamma);
  } else {






    EC_FELEM xx, yy, yyyy, zz;
    ec_GFp_mont_felem_sqr(group, &xx, &a->X);
    ec_GFp_mont_felem_sqr(group, &yy, &a->Y);
    ec_GFp_mont_felem_sqr(group, &yyyy, &yy);
    ec_GFp_mont_felem_sqr(group, &zz, &a->Z);

    EC_FELEM s;
    ec_felem_add(group, &s, &a->X, &yy);
    ec_GFp_mont_felem_sqr(group, &s, &s);
    ec_felem_sub(group, &s, &s, &xx);
    ec_felem_sub(group, &s, &s, &yyyy);
    ec_felem_add(group, &s, &s, &s);

    EC_FELEM m;
    ec_GFp_mont_felem_sqr(group, &m, &zz);
    ec_GFp_mont_felem_mul(group, &m, &group->a, &m);
    ec_felem_add(group, &m, &m, &xx);
    ec_felem_add(group, &m, &m, &xx);
    ec_felem_add(group, &m, &m, &xx);

    ec_GFp_mont_felem_sqr(group, &r->X, &m);
    ec_felem_sub(group, &r->X, &r->X, &s);
    ec_felem_sub(group, &r->X, &r->X, &s);

    ec_felem_add(group, &r->Z, &a->Y, &a->Z);
    ec_GFp_mont_felem_sqr(group, &r->Z, &r->Z);
    ec_felem_sub(group, &r->Z, &r->Z, &yy);
    ec_felem_sub(group, &r->Z, &r->Z, &zz);

    ec_felem_add(group, &yyyy, &yyyy, &yyyy);
    ec_felem_add(group, &yyyy, &yyyy, &yyyy);
    ec_felem_add(group, &yyyy, &yyyy, &yyyy);
    ec_felem_sub(group, &r->Y, &s, &r->X);
    ec_GFp_mont_felem_mul(group, &r->Y, &r->Y, &m);
    ec_felem_sub(group, &r->Y, &r->Y, &yyyy);
  }
}

static int ec_GFp_mont_cmp_x_coordinate(const EC_GROUP *group,
                                        const EC_RAW_POINT *p,
                                        const EC_SCALAR *r) {
  if (!group->field_greater_than_order ||
      group->field.width != group->order.width) {


    return ec_GFp_simple_cmp_x_coordinate(group, p, r);
  }

  if (ec_GFp_simple_is_at_infinity(group, p)) {
    return 0;
  }



  EC_FELEM r_Z2, Z2_mont, X;
  ec_GFp_mont_felem_mul(group, &Z2_mont, &p->Z, &p->Z);

  OPENSSL_memcpy(r_Z2.words, r->words, group->field.width * sizeof(BN_ULONG));
  ec_GFp_mont_felem_mul(group, &r_Z2, &r_Z2, &Z2_mont);
  ec_GFp_mont_felem_from_montgomery(group, &X, &p->X);

  if (ec_felem_equal(group, &r_Z2, &X)) {
    return 1;
  }




  if (bn_less_than_words(r->words, group->field_minus_order.words,
                         group->field.width)) {

    bn_add_words(r_Z2.words, r->words, group->order.d, group->field.width);
    ec_GFp_mont_felem_mul(group, &r_Z2, &r_Z2, &Z2_mont);
    if (ec_felem_equal(group, &r_Z2, &X)) {
      return 1;
    }
  }

  return 0;
}

DEFINE_METHOD_FUNCTION(EC_METHOD, EC_GFp_mont_method) {
  out->group_init = ec_GFp_mont_group_init;
  out->group_finish = ec_GFp_mont_group_finish;
  out->group_set_curve = ec_GFp_mont_group_set_curve;
  out->point_get_affine_coordinates = ec_GFp_mont_point_get_affine_coordinates;
  out->add = ec_GFp_mont_add;
  out->dbl = ec_GFp_mont_dbl;
  out->mul = ec_GFp_mont_mul;
  out->mul_base = ec_GFp_mont_mul_base;
  out->mul_public = ec_GFp_mont_mul_public;
  out->felem_mul = ec_GFp_mont_felem_mul;
  out->felem_sqr = ec_GFp_mont_felem_sqr;
  out->bignum_to_felem = ec_GFp_mont_bignum_to_felem;
  out->felem_to_bignum = ec_GFp_mont_felem_to_bignum;
  out->scalar_inv_montgomery = ec_simple_scalar_inv_montgomery;
  out->scalar_inv_montgomery_vartime = ec_GFp_simple_mont_inv_mod_ord_vartime;
  out->cmp_x_coordinate = ec_GFp_mont_cmp_x_coordinate;
}
