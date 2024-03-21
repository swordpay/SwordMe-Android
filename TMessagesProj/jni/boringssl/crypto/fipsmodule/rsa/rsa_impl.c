/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
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
 * [including the GNU Public Licence.] */

#include <openssl/rsa.h>

#include <assert.h>
#include <limits.h>
#include <string.h>

#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/mem.h>
#include <openssl/thread.h>
#include <openssl/type_check.h>

#include "internal.h"
#include "../bn/internal.h"
#include "../../internal.h"
#include "../delocate.h"


static int check_modulus_and_exponent_sizes(const RSA *rsa) {
  unsigned rsa_bits = BN_num_bits(rsa->n);

  if (rsa_bits > 16 * 1024) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_MODULUS_TOO_LARGE);
    return 0;
  }









  static const unsigned kMaxExponentBits = 33;

  if (BN_num_bits(rsa->e) > kMaxExponentBits) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_BAD_E_VALUE);
    return 0;
  }




  if (rsa_bits <= kMaxExponentBits) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_KEY_SIZE_TOO_SMALL);
    return 0;
  }
  assert(BN_ucmp(rsa->n, rsa->e) > 0);

  return 1;
}

static int ensure_fixed_copy(BIGNUM **out, const BIGNUM *in, int width) {
  if (*out != NULL) {
    return 1;
  }
  BIGNUM *copy = BN_dup(in);
  if (copy == NULL ||
      !bn_resize_words(copy, width)) {
    BN_free(copy);
    return 0;
  }
  *out = copy;
  CONSTTIME_SECRET(copy->d, sizeof(BN_ULONG) * width);

  return 1;
}

// After this function has returned, |rsa| may not be changed. This is needed
// because |RSA| is a public struct and, additionally, OpenSSL 1.1.0 opaquified
// it wrong (see https://github.com/openssl/openssl/issues/5158).
static int freeze_private_key(RSA *rsa, BN_CTX *ctx) {
  CRYPTO_MUTEX_lock_read(&rsa->lock);
  int frozen = rsa->private_key_frozen;
  CRYPTO_MUTEX_unlock_read(&rsa->lock);
  if (frozen) {
    return 1;
  }

  int ret = 0;
  CRYPTO_MUTEX_lock_write(&rsa->lock);
  if (rsa->private_key_frozen) {
    ret = 1;
    goto err;
  }






  if (rsa->mont_n == NULL) {
    rsa->mont_n = BN_MONT_CTX_new_for_modulus(rsa->n, ctx);
    if (rsa->mont_n == NULL) {
      goto err;
    }
  }
  const BIGNUM *n_fixed = &rsa->mont_n->N;




  if (rsa->d != NULL &&
      !ensure_fixed_copy(&rsa->d_fixed, rsa->d, n_fixed->width)) {
    goto err;
  }

  if (rsa->p != NULL && rsa->q != NULL) {





    if (rsa->mont_p == NULL) {
      rsa->mont_p = BN_MONT_CTX_new_consttime(rsa->p, ctx);
      if (rsa->mont_p == NULL) {
        goto err;
      }
    }
    const BIGNUM *p_fixed = &rsa->mont_p->N;

    if (rsa->mont_q == NULL) {
      rsa->mont_q = BN_MONT_CTX_new_consttime(rsa->q, ctx);
      if (rsa->mont_q == NULL) {
        goto err;
      }
    }
    const BIGNUM *q_fixed = &rsa->mont_q->N;

    if (rsa->dmp1 != NULL && rsa->dmq1 != NULL) {

      if (rsa->iqmp == NULL) {
        BIGNUM *iqmp = BN_new();
        if (iqmp == NULL ||
            !bn_mod_inverse_secret_prime(iqmp, rsa->q, rsa->p, ctx,
                                         rsa->mont_p)) {
          BN_free(iqmp);
          goto err;
        }
        rsa->iqmp = iqmp;
      }



      if (!ensure_fixed_copy(&rsa->dmp1_fixed, rsa->dmp1, p_fixed->width) ||
          !ensure_fixed_copy(&rsa->dmq1_fixed, rsa->dmq1, q_fixed->width)) {
        goto err;
      }


      if (rsa->inv_small_mod_large_mont == NULL) {
        BIGNUM *inv_small_mod_large_mont = BN_new();
        int ok;
        if (BN_cmp(rsa->p, rsa->q) < 0) {
          ok = inv_small_mod_large_mont != NULL &&
               bn_mod_inverse_secret_prime(inv_small_mod_large_mont, rsa->p,
                                           rsa->q, ctx, rsa->mont_q) &&
               BN_to_montgomery(inv_small_mod_large_mont,
                                inv_small_mod_large_mont, rsa->mont_q, ctx);
        } else {
          ok = inv_small_mod_large_mont != NULL &&
               BN_to_montgomery(inv_small_mod_large_mont, rsa->iqmp,
                                rsa->mont_p, ctx);
        }
        if (!ok) {
          BN_free(inv_small_mod_large_mont);
          goto err;
        }
        rsa->inv_small_mod_large_mont = inv_small_mod_large_mont;
        CONSTTIME_SECRET(
            rsa->inv_small_mod_large_mont->d,
            sizeof(BN_ULONG) * rsa->inv_small_mod_large_mont->width);
      }
    }
  }

  rsa->private_key_frozen = 1;
  ret = 1;

err:
  CRYPTO_MUTEX_unlock_write(&rsa->lock);
  return ret;
}

size_t rsa_default_size(const RSA *rsa) {
  return BN_num_bytes(rsa->n);
}

int RSA_encrypt(RSA *rsa, size_t *out_len, uint8_t *out, size_t max_out,
                const uint8_t *in, size_t in_len, int padding) {
  if (rsa->n == NULL || rsa->e == NULL) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_VALUE_MISSING);
    return 0;
  }

  const unsigned rsa_size = RSA_size(rsa);
  BIGNUM *f, *result;
  uint8_t *buf = NULL;
  BN_CTX *ctx = NULL;
  int i, ret = 0;

  if (max_out < rsa_size) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_OUTPUT_BUFFER_TOO_SMALL);
    return 0;
  }

  if (!check_modulus_and_exponent_sizes(rsa)) {
    return 0;
  }

  ctx = BN_CTX_new();
  if (ctx == NULL) {
    goto err;
  }

  BN_CTX_start(ctx);
  f = BN_CTX_get(ctx);
  result = BN_CTX_get(ctx);
  buf = OPENSSL_malloc(rsa_size);
  if (!f || !result || !buf) {
    OPENSSL_PUT_ERROR(RSA, ERR_R_MALLOC_FAILURE);
    goto err;
  }

  switch (padding) {
    case RSA_PKCS1_PADDING:
      i = RSA_padding_add_PKCS1_type_2(buf, rsa_size, in, in_len);
      break;
    case RSA_PKCS1_OAEP_PADDING:

      i = RSA_padding_add_PKCS1_OAEP_mgf1(buf, rsa_size, in, in_len,
                                          NULL, 0, NULL, NULL);
      break;
    case RSA_NO_PADDING:
      i = RSA_padding_add_none(buf, rsa_size, in, in_len);
      break;
    default:
      OPENSSL_PUT_ERROR(RSA, RSA_R_UNKNOWN_PADDING_TYPE);
      goto err;
  }

  if (i <= 0) {
    goto err;
  }

  if (BN_bin2bn(buf, rsa_size, f) == NULL) {
    goto err;
  }

  if (BN_ucmp(f, rsa->n) >= 0) {

    OPENSSL_PUT_ERROR(RSA, RSA_R_DATA_TOO_LARGE_FOR_MODULUS);
    goto err;
  }

  if (!BN_MONT_CTX_set_locked(&rsa->mont_n, &rsa->lock, rsa->n, ctx) ||
      !BN_mod_exp_mont(result, f, rsa->e, &rsa->mont_n->N, ctx, rsa->mont_n)) {
    goto err;
  }


  if (!BN_bn2bin_padded(out, rsa_size, result)) {
    OPENSSL_PUT_ERROR(RSA, ERR_R_INTERNAL_ERROR);
    goto err;
  }

  *out_len = rsa_size;
  ret = 1;

err:
  if (ctx != NULL) {
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
  }
  OPENSSL_free(buf);

  return ret;
}

// RSA*. Then this limit is exceeded, BN_BLINDING objects will be created and
// destroyed as needed.
#define MAX_BLINDINGS_PER_RSA 1024

// allocating one of the cached BN_BLINDING objects in |rsa->blindings|. If
// none are free, the cache will be extended by a extra element and the new
// BN_BLINDING is returned.
//
// On success, the index of the assigned BN_BLINDING is written to
// |*index_used| and must be passed to |rsa_blinding_release| when finished.
static BN_BLINDING *rsa_blinding_get(RSA *rsa, unsigned *index_used,
                                     BN_CTX *ctx) {
  assert(ctx != NULL);
  assert(rsa->mont_n != NULL);

  BN_BLINDING *ret = NULL;
  BN_BLINDING **new_blindings;
  uint8_t *new_blindings_inuse;
  char overflow = 0;

  CRYPTO_MUTEX_lock_write(&rsa->lock);

  unsigned i;
  for (i = 0; i < rsa->num_blindings; i++) {
    if (rsa->blindings_inuse[i] == 0) {
      rsa->blindings_inuse[i] = 1;
      ret = rsa->blindings[i];
      *index_used = i;
      break;
    }
  }

  if (ret != NULL) {
    CRYPTO_MUTEX_unlock_write(&rsa->lock);
    return ret;
  }

  overflow = rsa->num_blindings >= MAX_BLINDINGS_PER_RSA;



  CRYPTO_MUTEX_unlock_write(&rsa->lock);
  ret = BN_BLINDING_new();
  if (ret == NULL) {
    return NULL;
  }

  if (overflow) {


    *index_used = MAX_BLINDINGS_PER_RSA;
    return ret;
  }

  CRYPTO_MUTEX_lock_write(&rsa->lock);

  new_blindings =
      OPENSSL_malloc(sizeof(BN_BLINDING *) * (rsa->num_blindings + 1));
  if (new_blindings == NULL) {
    goto err1;
  }
  OPENSSL_memcpy(new_blindings, rsa->blindings,
         sizeof(BN_BLINDING *) * rsa->num_blindings);
  new_blindings[rsa->num_blindings] = ret;

  new_blindings_inuse = OPENSSL_malloc(rsa->num_blindings + 1);
  if (new_blindings_inuse == NULL) {
    goto err2;
  }
  OPENSSL_memcpy(new_blindings_inuse, rsa->blindings_inuse, rsa->num_blindings);
  new_blindings_inuse[rsa->num_blindings] = 1;
  *index_used = rsa->num_blindings;

  OPENSSL_free(rsa->blindings);
  rsa->blindings = new_blindings;
  OPENSSL_free(rsa->blindings_inuse);
  rsa->blindings_inuse = new_blindings_inuse;
  rsa->num_blindings++;

  CRYPTO_MUTEX_unlock_write(&rsa->lock);
  return ret;

err2:
  OPENSSL_free(new_blindings);

err1:
  CRYPTO_MUTEX_unlock_write(&rsa->lock);
  BN_BLINDING_free(ret);
  return NULL;
}

// for other threads to use.
static void rsa_blinding_release(RSA *rsa, BN_BLINDING *blinding,
                                 unsigned blinding_index) {
  if (blinding_index == MAX_BLINDINGS_PER_RSA) {

    BN_BLINDING_free(blinding);
    return;
  }

  CRYPTO_MUTEX_lock_write(&rsa->lock);
  rsa->blindings_inuse[blinding_index] = 0;
  CRYPTO_MUTEX_unlock_write(&rsa->lock);
}

int rsa_default_sign_raw(RSA *rsa, size_t *out_len, uint8_t *out,
                         size_t max_out, const uint8_t *in, size_t in_len,
                         int padding) {
  const unsigned rsa_size = RSA_size(rsa);
  uint8_t *buf = NULL;
  int i, ret = 0;

  if (max_out < rsa_size) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_OUTPUT_BUFFER_TOO_SMALL);
    return 0;
  }

  buf = OPENSSL_malloc(rsa_size);
  if (buf == NULL) {
    OPENSSL_PUT_ERROR(RSA, ERR_R_MALLOC_FAILURE);
    goto err;
  }

  switch (padding) {
    case RSA_PKCS1_PADDING:
      i = RSA_padding_add_PKCS1_type_1(buf, rsa_size, in, in_len);
      break;
    case RSA_NO_PADDING:
      i = RSA_padding_add_none(buf, rsa_size, in, in_len);
      break;
    default:
      OPENSSL_PUT_ERROR(RSA, RSA_R_UNKNOWN_PADDING_TYPE);
      goto err;
  }

  if (i <= 0) {
    goto err;
  }

  if (!RSA_private_transform(rsa, out, buf, rsa_size)) {
    goto err;
  }

  CONSTTIME_DECLASSIFY(out, rsa_size);
  *out_len = rsa_size;
  ret = 1;

err:
  OPENSSL_free(buf);

  return ret;
}

int rsa_default_decrypt(RSA *rsa, size_t *out_len, uint8_t *out, size_t max_out,
                        const uint8_t *in, size_t in_len, int padding) {
  const unsigned rsa_size = RSA_size(rsa);
  uint8_t *buf = NULL;
  int ret = 0;

  if (max_out < rsa_size) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_OUTPUT_BUFFER_TOO_SMALL);
    return 0;
  }

  if (padding == RSA_NO_PADDING) {
    buf = out;
  } else {

    buf = OPENSSL_malloc(rsa_size);
    if (buf == NULL) {
      OPENSSL_PUT_ERROR(RSA, ERR_R_MALLOC_FAILURE);
      goto err;
    }
  }

  if (in_len != rsa_size) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_DATA_LEN_NOT_EQUAL_TO_MOD_LEN);
    goto err;
  }

  if (!RSA_private_transform(rsa, buf, in, rsa_size)) {
    goto err;
  }

  switch (padding) {
    case RSA_PKCS1_PADDING:
      ret =
          RSA_padding_check_PKCS1_type_2(out, out_len, rsa_size, buf, rsa_size);
      break;
    case RSA_PKCS1_OAEP_PADDING:

      ret = RSA_padding_check_PKCS1_OAEP_mgf1(out, out_len, rsa_size, buf,
                                              rsa_size, NULL, 0, NULL, NULL);
      break;
    case RSA_NO_PADDING:
      *out_len = rsa_size;
      ret = 1;
      break;
    default:
      OPENSSL_PUT_ERROR(RSA, RSA_R_UNKNOWN_PADDING_TYPE);
      goto err;
  }

  CONSTTIME_DECLASSIFY(&ret, sizeof(ret));
  if (!ret) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_PADDING_CHECK_FAILED);
  } else {
    CONSTTIME_DECLASSIFY(out, *out_len);
  }

err:
  if (padding != RSA_NO_PADDING) {
    OPENSSL_free(buf);
  }

  return ret;
}

static int mod_exp(BIGNUM *r0, const BIGNUM *I, RSA *rsa, BN_CTX *ctx);

int RSA_verify_raw(RSA *rsa, size_t *out_len, uint8_t *out, size_t max_out,
                   const uint8_t *in, size_t in_len, int padding) {
  if (rsa->n == NULL || rsa->e == NULL) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_VALUE_MISSING);
    return 0;
  }

  const unsigned rsa_size = RSA_size(rsa);
  BIGNUM *f, *result;

  if (max_out < rsa_size) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_OUTPUT_BUFFER_TOO_SMALL);
    return 0;
  }

  if (in_len != rsa_size) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_DATA_LEN_NOT_EQUAL_TO_MOD_LEN);
    return 0;
  }

  if (!check_modulus_and_exponent_sizes(rsa)) {
    return 0;
  }

  BN_CTX *ctx = BN_CTX_new();
  if (ctx == NULL) {
    return 0;
  }

  int ret = 0;
  uint8_t *buf = NULL;

  BN_CTX_start(ctx);
  f = BN_CTX_get(ctx);
  result = BN_CTX_get(ctx);
  if (f == NULL || result == NULL) {
    OPENSSL_PUT_ERROR(RSA, ERR_R_MALLOC_FAILURE);
    goto err;
  }

  if (padding == RSA_NO_PADDING) {
    buf = out;
  } else {

    buf = OPENSSL_malloc(rsa_size);
    if (buf == NULL) {
      OPENSSL_PUT_ERROR(RSA, ERR_R_MALLOC_FAILURE);
      goto err;
    }
  }

  if (BN_bin2bn(in, in_len, f) == NULL) {
    goto err;
  }

  if (BN_ucmp(f, rsa->n) >= 0) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_DATA_TOO_LARGE_FOR_MODULUS);
    goto err;
  }

  if (!BN_MONT_CTX_set_locked(&rsa->mont_n, &rsa->lock, rsa->n, ctx) ||
      !BN_mod_exp_mont(result, f, rsa->e, &rsa->mont_n->N, ctx, rsa->mont_n)) {
    goto err;
  }

  if (!BN_bn2bin_padded(buf, rsa_size, result)) {
    OPENSSL_PUT_ERROR(RSA, ERR_R_INTERNAL_ERROR);
    goto err;
  }

  switch (padding) {
    case RSA_PKCS1_PADDING:
      ret =
          RSA_padding_check_PKCS1_type_1(out, out_len, rsa_size, buf, rsa_size);
      break;
    case RSA_NO_PADDING:
      ret = 1;
      *out_len = rsa_size;
      break;
    default:
      OPENSSL_PUT_ERROR(RSA, RSA_R_UNKNOWN_PADDING_TYPE);
      goto err;
  }

  if (!ret) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_PADDING_CHECK_FAILED);
    goto err;
  }

err:
  BN_CTX_end(ctx);
  BN_CTX_free(ctx);
  if (buf != out) {
    OPENSSL_free(buf);
  }
  return ret;
}

int rsa_default_private_transform(RSA *rsa, uint8_t *out, const uint8_t *in,
                                  size_t len) {
  if (rsa->n == NULL || rsa->d == NULL) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_VALUE_MISSING);
    return 0;
  }

  BIGNUM *f, *result;
  BN_CTX *ctx = NULL;
  unsigned blinding_index = 0;
  BN_BLINDING *blinding = NULL;
  int ret = 0;

  ctx = BN_CTX_new();
  if (ctx == NULL) {
    goto err;
  }
  BN_CTX_start(ctx);
  f = BN_CTX_get(ctx);
  result = BN_CTX_get(ctx);

  if (f == NULL || result == NULL) {
    OPENSSL_PUT_ERROR(RSA, ERR_R_MALLOC_FAILURE);
    goto err;
  }

  if (BN_bin2bn(in, len, f) == NULL) {
    goto err;
  }

  if (BN_ucmp(f, rsa->n) >= 0) {

    OPENSSL_PUT_ERROR(RSA, RSA_R_DATA_TOO_LARGE_FOR_MODULUS);
    goto err;
  }

  if (!freeze_private_key(rsa, ctx)) {
    OPENSSL_PUT_ERROR(RSA, ERR_R_INTERNAL_ERROR);
    goto err;
  }

  const int do_blinding = (rsa->flags & RSA_FLAG_NO_BLINDING) == 0;

  if (rsa->e == NULL && do_blinding) {




    OPENSSL_PUT_ERROR(RSA, RSA_R_NO_PUBLIC_EXPONENT);
    goto err;
  }

  if (do_blinding) {
    blinding = rsa_blinding_get(rsa, &blinding_index, ctx);
    if (blinding == NULL) {
      OPENSSL_PUT_ERROR(RSA, ERR_R_INTERNAL_ERROR);
      goto err;
    }
    if (!BN_BLINDING_convert(f, blinding, rsa->e, rsa->mont_n, ctx)) {
      goto err;
    }
  }

  if (rsa->p != NULL && rsa->q != NULL && rsa->e != NULL && rsa->dmp1 != NULL &&
      rsa->dmq1 != NULL && rsa->iqmp != NULL &&




      bn_less_than_montgomery_R(rsa->q, rsa->mont_p) &&
      bn_less_than_montgomery_R(rsa->p, rsa->mont_q)) {
    if (!mod_exp(result, f, rsa, ctx)) {
      goto err;
    }
  } else if (!BN_mod_exp_mont_consttime(result, f, rsa->d_fixed, rsa->n, ctx,
                                        rsa->mont_n)) {
    goto err;
  }









  if (rsa->e != NULL) {
    BIGNUM *vrfy = BN_CTX_get(ctx);
    if (vrfy == NULL ||
        !BN_mod_exp_mont(vrfy, result, rsa->e, rsa->n, ctx, rsa->mont_n) ||
        !BN_equal_consttime(vrfy, f)) {
      OPENSSL_PUT_ERROR(RSA, ERR_R_INTERNAL_ERROR);
      goto err;
    }

  }

  if (do_blinding &&
      !BN_BLINDING_invert(result, blinding, rsa->mont_n, ctx)) {
    goto err;
  }





  assert(result->width == rsa->mont_n->N.width);
  if (!BN_bn2bin_padded(out, len, result)) {
    OPENSSL_PUT_ERROR(RSA, ERR_R_INTERNAL_ERROR);
    goto err;
  }

  ret = 1;

err:
  if (ctx != NULL) {
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
  }
  if (blinding != NULL) {
    rsa_blinding_release(rsa, blinding, blinding_index);
  }

  return ret;
}

// modulo |p| times |q|. It returns one on success and zero on error.
static int mod_montgomery(BIGNUM *r, const BIGNUM *I, const BIGNUM *p,
                          const BN_MONT_CTX *mont_p, const BIGNUM *q,
                          BN_CTX *ctx) {



  if (!bn_less_than_montgomery_R(q, mont_p)) {
    OPENSSL_PUT_ERROR(RSA, ERR_R_INTERNAL_ERROR);
    return 0;
  }

  if (// Reduce mod p with Montgomery reduction. This computes I * R^-1 mod p.
      !BN_from_montgomery(r, I, mont_p, ctx) ||


      !BN_to_montgomery(r, r, mont_p, ctx)) {
    return 0;
  }







  return 1;
}

static int mod_exp(BIGNUM *r0, const BIGNUM *I, RSA *rsa, BN_CTX *ctx) {
  assert(ctx != NULL);

  assert(rsa->n != NULL);
  assert(rsa->e != NULL);
  assert(rsa->d != NULL);
  assert(rsa->p != NULL);
  assert(rsa->q != NULL);
  assert(rsa->dmp1 != NULL);
  assert(rsa->dmq1 != NULL);
  assert(rsa->iqmp != NULL);

  BIGNUM *r1, *m1;
  int ret = 0;

  BN_CTX_start(ctx);
  r1 = BN_CTX_get(ctx);
  m1 = BN_CTX_get(ctx);
  if (r1 == NULL ||
      m1 == NULL) {
    goto err;
  }

  if (!freeze_private_key(rsa, ctx)) {
    goto err;
  }


  const BIGNUM *dmp1 = rsa->dmp1_fixed, *dmq1 = rsa->dmq1_fixed;
  const BN_MONT_CTX *mont_p = rsa->mont_p, *mont_q = rsa->mont_q;
  if (BN_cmp(rsa->p, rsa->q) < 0) {
    mont_p = rsa->mont_q;
    mont_q = rsa->mont_p;
    dmp1 = rsa->dmq1_fixed;
    dmq1 = rsa->dmp1_fixed;
  }



  const BIGNUM *n = &rsa->mont_n->N;
  const BIGNUM *p = &mont_p->N;
  const BIGNUM *q = &mont_q->N;


  assert(BN_ucmp(I, n) < 0);

  if (// |m1| is the result modulo |q|.
      !mod_montgomery(r1, I, q, mont_q, p, ctx) ||
      !BN_mod_exp_mont_consttime(m1, r1, dmq1, q, ctx, mont_q) ||

      !mod_montgomery(r1, I, p, mont_p, q, ctx) ||
      !BN_mod_exp_mont_consttime(r0, r1, dmp1, p, ctx, mont_p) ||


      !bn_mod_sub_consttime(r0, r0, m1, p, ctx) ||



      !BN_mod_mul_montgomery(r0, r0, rsa->inv_small_mod_large_mont, mont_p,
                             ctx) ||





      !bn_mul_consttime(r0, r0, q, ctx) ||
      !bn_uadd_consttime(r0, r0, m1) ||


      !bn_resize_words(r0, n->width)) {
    goto err;
  }

  ret = 1;

err:
  BN_CTX_end(ctx);
  return ret;
}

static int ensure_bignum(BIGNUM **out) {
  if (*out == NULL) {
    *out = BN_new();
  }
  return *out != NULL;
}

// chosen to give enough precision for 3072-bit RSA, the largest key size FIPS
// specifies. Key sizes beyond this will round up.
//
// To verify this number, check that n² < 2³⁰⁷¹ < (n+1)², where n is value
// represented here. Note the components are listed in little-endian order. Here
// is some sample Python code to check:
//
//   >>> TOBN = lambda a, b: a << 32 | b
//   >>> l = [ <paste the contents of kSqrtTwo> ]
//   >>> n = sum(a * 2**(64*i) for i, a in enumerate(l))
//   >>> n**2 < 2**3071 < (n+1)**2
//   True
const BN_ULONG kBoringSSLRSASqrtTwo[] = {
    TOBN(0xdea06241, 0xf7aa81c2), TOBN(0xf6a1be3f, 0xca221307),
    TOBN(0x332a5e9f, 0x7bda1ebf), TOBN(0x0104dc01, 0xfe32352f),
    TOBN(0xb8cf341b, 0x6f8236c7), TOBN(0x4264dabc, 0xd528b651),
    TOBN(0xf4d3a02c, 0xebc93e0c), TOBN(0x81394ab6, 0xd8fd0efd),
    TOBN(0xeaa4a089, 0x9040ca4a), TOBN(0xf52f120f, 0x836e582e),
    TOBN(0xcb2a6343, 0x31f3c84d), TOBN(0xc6d5a8a3, 0x8bb7e9dc),
    TOBN(0x460abc72, 0x2f7c4e33), TOBN(0xcab1bc91, 0x1688458a),
    TOBN(0x53059c60, 0x11bc337b), TOBN(0xd2202e87, 0x42af1f4e),
    TOBN(0x78048736, 0x3dfa2768), TOBN(0x0f74a85e, 0x439c7b4a),
    TOBN(0xa8b1fe6f, 0xdc83db39), TOBN(0x4afc8304, 0x3ab8a2c3),
    TOBN(0xed17ac85, 0x83339915), TOBN(0x1d6f60ba, 0x893ba84c),
    TOBN(0x597d89b3, 0x754abe9f), TOBN(0xb504f333, 0xf9de6484),
};
const size_t kBoringSSLRSASqrtTwoLen = OPENSSL_ARRAY_SIZE(kBoringSSLRSASqrtTwo);

// relatively prime to |e|. If |p| is non-NULL, |out| will also not be close to
// |p|. |sqrt2| must be ⌊2^(bits-1)×√2⌋ (or a slightly overestimate for large
// sizes), and |pow2_bits_100| must be 2^(bits-100).
//
// This function fails with probability around 2^-21.
static int generate_prime(BIGNUM *out, int bits, const BIGNUM *e,
                          const BIGNUM *p, const BIGNUM *sqrt2,
                          const BIGNUM *pow2_bits_100, BN_CTX *ctx,
                          BN_GENCB *cb) {
  if (bits < 128 || (bits % BN_BITS2) != 0) {
    OPENSSL_PUT_ERROR(RSA, ERR_R_INTERNAL_ERROR);
    return 0;
  }
  assert(BN_is_pow2(pow2_bits_100));
  assert(BN_is_bit_set(pow2_bits_100, bits - 100));





























  if (bits >= INT_MAX/32) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_MODULUS_TOO_LARGE);
    return 0;
  }
  int limit = BN_is_word(e, 3) ? bits * 8 : bits * 5;

  int ret = 0, tries = 0, rand_tries = 0;
  BN_CTX_start(ctx);
  BIGNUM *tmp = BN_CTX_get(ctx);
  if (tmp == NULL) {
    goto err;
  }

  for (;;) {



    if (!BN_rand(out, bits, BN_RAND_TOP_ONE, BN_RAND_BOTTOM_ODD) ||
        !BN_GENCB_call(cb, BN_GENCB_GENERATED, rand_tries++)) {
      goto err;
    }

    if (p != NULL) {

      if (!bn_abs_sub_consttime(tmp, out, p, ctx)) {
        goto err;
      }
      if (BN_cmp(tmp, pow2_bits_100) <= 0) {
        continue;
      }
    }







    if (BN_cmp(out, sqrt2) <= 0) {
      continue;
    }


    if (!bn_odd_number_is_obviously_composite(out)) {

      int relatively_prime;
      if (!BN_sub(tmp, out, BN_value_one()) ||
          !bn_is_relatively_prime(&relatively_prime, tmp, e, ctx)) {
        goto err;
      }
      if (relatively_prime) {

        int is_probable_prime;
        if (!BN_primality_test(&is_probable_prime, out, BN_prime_checks, ctx, 0,
                               cb)) {
          goto err;
        }
        if (is_probable_prime) {
          ret = 1;
          goto err;
        }
      }
    }


    tries++;
    if (tries >= limit) {
      OPENSSL_PUT_ERROR(RSA, RSA_R_TOO_MANY_ITERATIONS);
      goto err;
    }
    if (!BN_GENCB_call(cb, 2, tries)) {
      goto err;
    }
  }

err:
  BN_CTX_end(ctx);
  return ret;
}

// FIPS 186-4 appendix B.3. |RSA_generate_key_fips| performs additional checks
// for FIPS-compliant key generation.
//
// This function returns one on success and zero on failure. It has a failure
// probability of about 2^-20.
static int rsa_generate_key_impl(RSA *rsa, int bits, const BIGNUM *e_value,
                                 BN_GENCB *cb) {





  bits &= ~127;

  if (bits < 256) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_KEY_SIZE_TOO_SMALL);
    return 0;
  }






  if (BN_num_bits(e_value) > 32) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_BAD_E_VALUE);
    return 0;
  }

  int ret = 0;
  int prime_bits = bits / 2;
  BN_CTX *ctx = BN_CTX_new();
  if (ctx == NULL) {
    goto bn_err;
  }
  BN_CTX_start(ctx);
  BIGNUM *totient = BN_CTX_get(ctx);
  BIGNUM *pm1 = BN_CTX_get(ctx);
  BIGNUM *qm1 = BN_CTX_get(ctx);
  BIGNUM *sqrt2 = BN_CTX_get(ctx);
  BIGNUM *pow2_prime_bits_100 = BN_CTX_get(ctx);
  BIGNUM *pow2_prime_bits = BN_CTX_get(ctx);
  if (totient == NULL || pm1 == NULL || qm1 == NULL || sqrt2 == NULL ||
      pow2_prime_bits_100 == NULL || pow2_prime_bits == NULL ||
      !BN_set_bit(pow2_prime_bits_100, prime_bits - 100) ||
      !BN_set_bit(pow2_prime_bits, prime_bits)) {
    goto bn_err;
  }

  if (!ensure_bignum(&rsa->n) ||
      !ensure_bignum(&rsa->d) ||
      !ensure_bignum(&rsa->e) ||
      !ensure_bignum(&rsa->p) ||
      !ensure_bignum(&rsa->q) ||
      !ensure_bignum(&rsa->dmp1) ||
      !ensure_bignum(&rsa->dmq1)) {
    goto bn_err;
  }

  if (!BN_copy(rsa->e, e_value)) {
    goto bn_err;
  }

  if (!bn_set_words(sqrt2, kBoringSSLRSASqrtTwo, kBoringSSLRSASqrtTwoLen)) {
    goto bn_err;
  }
  int sqrt2_bits = kBoringSSLRSASqrtTwoLen * BN_BITS2;
  assert(sqrt2_bits == (int)BN_num_bits(sqrt2));
  if (sqrt2_bits > prime_bits) {


    if (!BN_rshift(sqrt2, sqrt2, sqrt2_bits - prime_bits)) {
      goto bn_err;
    }
  } else if (prime_bits > sqrt2_bits) {


    if (!BN_add_word(sqrt2, 1) ||
        !BN_lshift(sqrt2, sqrt2, prime_bits - sqrt2_bits)) {
      goto bn_err;
    }
  }
  assert(prime_bits == (int)BN_num_bits(sqrt2));

  do {





    if (!generate_prime(rsa->p, prime_bits, rsa->e, NULL, sqrt2,
                        pow2_prime_bits_100, ctx, cb) ||
        !BN_GENCB_call(cb, 3, 0) ||
        !generate_prime(rsa->q, prime_bits, rsa->e, rsa->p, sqrt2,
                        pow2_prime_bits_100, ctx, cb) ||
        !BN_GENCB_call(cb, 3, 1)) {
      goto bn_err;
    }

    if (BN_cmp(rsa->p, rsa->q) < 0) {
      BIGNUM *tmp = rsa->p;
      rsa->p = rsa->q;
      rsa->q = tmp;
    }







    int no_inverse;
    if (!bn_usub_consttime(pm1, rsa->p, BN_value_one()) ||
        !bn_usub_consttime(qm1, rsa->q, BN_value_one()) ||
        !bn_lcm_consttime(totient, pm1, qm1, ctx) ||
        !bn_mod_inverse_consttime(rsa->d, &no_inverse, rsa->e, totient, ctx)) {
      goto bn_err;
    }


  } while (BN_cmp(rsa->d, pow2_prime_bits) <= 0);

  if (// Calculate n.
      !bn_mul_consttime(rsa->n, rsa->p, rsa->q, ctx) ||

      !bn_div_consttime(NULL, rsa->dmp1, rsa->d, pm1, ctx) ||

      !bn_div_consttime(NULL, rsa->dmq1, rsa->d, qm1, ctx)) {
    goto bn_err;
  }
  bn_set_minimal_width(rsa->n);


  if (BN_num_bits(rsa->n) != (unsigned)bits) {
    OPENSSL_PUT_ERROR(RSA, ERR_R_INTERNAL_ERROR);
    goto err;
  }


  if (!freeze_private_key(rsa, ctx)) {
    goto bn_err;
  }



  if (!RSA_check_key(rsa)) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_INTERNAL_ERROR);
    goto err;
  }

  ret = 1;

bn_err:
  if (!ret) {
    OPENSSL_PUT_ERROR(RSA, ERR_LIB_BN);
  }
err:
  if (ctx != NULL) {
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);
  }
  return ret;
}

static void replace_bignum(BIGNUM **out, BIGNUM **in) {
  BN_free(*out);
  *out = *in;
  *in = NULL;
}

static void replace_bn_mont_ctx(BN_MONT_CTX **out, BN_MONT_CTX **in) {
  BN_MONT_CTX_free(*out);
  *out = *in;
  *in = NULL;
}

int RSA_generate_key_ex(RSA *rsa, int bits, const BIGNUM *e_value,
                        BN_GENCB *cb) {




  for (int i = 0; i < 4; i++) {
    ERR_clear_error();

    RSA *tmp = RSA_new();
    if (tmp == NULL) {
      return 0;
    }
    if (rsa_generate_key_impl(tmp, bits, e_value, cb)) {
      replace_bignum(&rsa->n, &tmp->n);
      replace_bignum(&rsa->e, &tmp->e);
      replace_bignum(&rsa->d, &tmp->d);
      replace_bignum(&rsa->p, &tmp->p);
      replace_bignum(&rsa->q, &tmp->q);
      replace_bignum(&rsa->dmp1, &tmp->dmp1);
      replace_bignum(&rsa->dmq1, &tmp->dmq1);
      replace_bignum(&rsa->iqmp, &tmp->iqmp);
      replace_bn_mont_ctx(&rsa->mont_n, &tmp->mont_n);
      replace_bn_mont_ctx(&rsa->mont_p, &tmp->mont_p);
      replace_bn_mont_ctx(&rsa->mont_q, &tmp->mont_q);
      replace_bignum(&rsa->d_fixed, &tmp->d_fixed);
      replace_bignum(&rsa->dmp1_fixed, &tmp->dmp1_fixed);
      replace_bignum(&rsa->dmq1_fixed, &tmp->dmq1_fixed);
      replace_bignum(&rsa->inv_small_mod_large_mont,
                     &tmp->inv_small_mod_large_mont);
      rsa->private_key_frozen = tmp->private_key_frozen;
      RSA_free(tmp);
      return 1;
    }
    uint32_t err = ERR_peek_error();
    RSA_free(tmp);
    tmp = NULL;


    if (ERR_GET_LIB(err) != ERR_LIB_RSA ||
        ERR_GET_REASON(err) != RSA_R_TOO_MANY_ITERATIONS) {
      return 0;
    }
  }

  return 0;
}

int RSA_generate_key_fips(RSA *rsa, int bits, BN_GENCB *cb) {


  if (bits != 2048 && bits != 3072) {
    OPENSSL_PUT_ERROR(RSA, RSA_R_BAD_RSA_PARAMETERS);
    return 0;
  }

  BIGNUM *e = BN_new();
  int ret = e != NULL &&
            BN_set_word(e, RSA_F4) &&
            RSA_generate_key_ex(rsa, bits, e, cb) &&
            RSA_check_fips(rsa);
  BN_free(e);
  return ret;
}

DEFINE_METHOD_FUNCTION(RSA_METHOD, RSA_default_method) {



  OPENSSL_memset(out, 0, sizeof(RSA_METHOD));
  out->common.is_static = 1;
}
