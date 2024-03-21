/* Written by Lenka Fibikova <fibikova@exp-math.uni-essen.de>
 * and Bodo Moeller for the OpenSSL project. */
/* ====================================================================
 * Copyright (c) 1998-2000 The OpenSSL Project.  All rights reserved.
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
 * Hudson (tjh@cryptsoft.com). */

#include <openssl/bn.h>

#include <openssl/err.h>

#include "internal.h"


BIGNUM *BN_mod_sqrt(BIGNUM *in, const BIGNUM *a, const BIGNUM *p, BN_CTX *ctx) {




  BIGNUM *ret = in;
  int err = 1;
  int r;
  BIGNUM *A, *b, *q, *t, *x, *y;
  int e, i, j;

  if (!BN_is_odd(p) || BN_abs_is_word(p, 1)) {
    if (BN_abs_is_word(p, 2)) {
      if (ret == NULL) {
        ret = BN_new();
      }
      if (ret == NULL) {
        goto end;
      }
      if (!BN_set_word(ret, BN_is_bit_set(a, 0))) {
        if (ret != in) {
          BN_free(ret);
        }
        return NULL;
      }
      return ret;
    }

    OPENSSL_PUT_ERROR(BN, BN_R_P_IS_NOT_PRIME);
    return (NULL);
  }

  if (BN_is_zero(a) || BN_is_one(a)) {
    if (ret == NULL) {
      ret = BN_new();
    }
    if (ret == NULL) {
      goto end;
    }
    if (!BN_set_word(ret, BN_is_one(a))) {
      if (ret != in) {
        BN_free(ret);
      }
      return NULL;
    }
    return ret;
  }

  BN_CTX_start(ctx);
  A = BN_CTX_get(ctx);
  b = BN_CTX_get(ctx);
  q = BN_CTX_get(ctx);
  t = BN_CTX_get(ctx);
  x = BN_CTX_get(ctx);
  y = BN_CTX_get(ctx);
  if (y == NULL) {
    goto end;
  }

  if (ret == NULL) {
    ret = BN_new();
  }
  if (ret == NULL) {
    goto end;
  }

  if (!BN_nnmod(A, a, p, ctx)) {
    goto end;
  }

  e = 1;
  while (!BN_is_bit_set(p, e)) {
    e++;
  }


  if (e == 1) {






    if (!BN_rshift(q, p, 2)) {
      goto end;
    }
    q->neg = 0;
    if (!BN_add_word(q, 1) ||
        !BN_mod_exp_mont(ret, A, q, p, ctx, NULL)) {
      goto end;
    }
    err = 0;
    goto vrfy;
  }

  if (e == 2) {


























    if (!bn_mod_lshift1_consttime(t, A, p, ctx)) {
      goto end;
    }

    if (!BN_rshift(q, p, 3)) {
      goto end;
    }
    q->neg = 0;
    if (!BN_mod_exp_mont(b, t, q, p, ctx, NULL)) {
      goto end;
    }

    if (!BN_mod_sqr(y, b, p, ctx)) {
      goto end;
    }

    if (!BN_mod_mul(t, t, y, p, ctx) ||
        !BN_sub_word(t, 1)) {
      goto end;
    }

    if (!BN_mod_mul(x, A, b, p, ctx) ||
        !BN_mod_mul(x, x, t, p, ctx)) {
      goto end;
    }

    if (!BN_copy(ret, x)) {
      goto end;
    }
    err = 0;
    goto vrfy;
  }


  if (!BN_copy(q, p)) {
    goto end;  // use 'q' as temp
  }
  q->neg = 0;
  i = 2;
  do {


    if (i < 22) {
      if (!BN_set_word(y, i)) {
        goto end;
      }
    } else {
      if (!BN_pseudo_rand(y, BN_num_bits(p), 0, 0)) {
        goto end;
      }
      if (BN_ucmp(y, p) >= 0) {
        if (!(p->neg ? BN_add : BN_sub)(y, y, p)) {
          goto end;
        }
      }

      if (BN_is_zero(y)) {
        if (!BN_set_word(y, i)) {
          goto end;
        }
      }
    }

    r = bn_jacobi(y, q, ctx);  // here 'q' is |p|
    if (r < -1) {
      goto end;
    }
    if (r == 0) {

      OPENSSL_PUT_ERROR(BN, BN_R_P_IS_NOT_PRIME);
      goto end;
    }
  } while (r == 1 && ++i < 82);

  if (r != -1) {




    OPENSSL_PUT_ERROR(BN, BN_R_TOO_MANY_ITERATIONS);
    goto end;
  }

  if (!BN_rshift(q, q, e)) {
    goto end;
  }


  if (!BN_mod_exp_mont(y, y, q, p, ctx, NULL)) {
    goto end;
  }
  if (BN_is_one(y)) {
    OPENSSL_PUT_ERROR(BN, BN_R_P_IS_NOT_PRIME);
    goto end;
  }

















  if (!BN_rshift1(t, q)) {
    goto end;
  }

  if (BN_is_zero(t))  // special case: p = 2^e + 1
  {
    if (!BN_nnmod(t, A, p, ctx)) {
      goto end;
    }
    if (BN_is_zero(t)) {

      BN_zero(ret);
      err = 0;
      goto end;
    } else if (!BN_one(x)) {
      goto end;
    }
  } else {
    if (!BN_mod_exp_mont(x, A, t, p, ctx, NULL)) {
      goto end;
    }
    if (BN_is_zero(x)) {

      BN_zero(ret);
      err = 0;
      goto end;
    }
  }

  if (!BN_mod_sqr(b, x, p, ctx) ||
      !BN_mod_mul(b, b, A, p, ctx)) {
    goto end;
  }

  if (!BN_mod_mul(x, x, A, p, ctx)) {
    goto end;
  }

  while (1) {








    if (BN_is_one(b)) {
      if (!BN_copy(ret, x)) {
        goto end;
      }
      err = 0;
      goto vrfy;
    }

    i = 1;
    if (!BN_mod_sqr(t, b, p, ctx)) {
      goto end;
    }
    while (!BN_is_one(t)) {
      i++;
      if (i == e) {
        OPENSSL_PUT_ERROR(BN, BN_R_NOT_A_SQUARE);
        goto end;
      }
      if (!BN_mod_mul(t, t, t, p, ctx)) {
        goto end;
      }
    }

    if (!BN_copy(t, y)) {
      goto end;
    }
    for (j = e - i - 1; j > 0; j--) {
      if (!BN_mod_sqr(t, t, p, ctx)) {
        goto end;
      }
    }
    if (!BN_mod_mul(y, t, t, p, ctx) ||
        !BN_mod_mul(x, x, t, p, ctx) ||
        !BN_mod_mul(b, b, y, p, ctx)) {
      goto end;
    }
    e = i;
  }

vrfy:
  if (!err) {



    if (!BN_mod_sqr(x, ret, p, ctx)) {
      err = 1;
    }

    if (!err && 0 != BN_cmp(x, A)) {
      OPENSSL_PUT_ERROR(BN, BN_R_NOT_A_SQUARE);
      err = 1;
    }
  }

end:
  if (err) {
    if (ret != in) {
      BN_clear_free(ret);
    }
    ret = NULL;
  }
  BN_CTX_end(ctx);
  return ret;
}

int BN_sqrt(BIGNUM *out_sqrt, const BIGNUM *in, BN_CTX *ctx) {
  BIGNUM *estimate, *tmp, *delta, *last_delta, *tmp2;
  int ok = 0, last_delta_valid = 0;

  if (in->neg) {
    OPENSSL_PUT_ERROR(BN, BN_R_NEGATIVE_NUMBER);
    return 0;
  }
  if (BN_is_zero(in)) {
    BN_zero(out_sqrt);
    return 1;
  }

  BN_CTX_start(ctx);
  if (out_sqrt == in) {
    estimate = BN_CTX_get(ctx);
  } else {
    estimate = out_sqrt;
  }
  tmp = BN_CTX_get(ctx);
  last_delta = BN_CTX_get(ctx);
  delta = BN_CTX_get(ctx);
  if (estimate == NULL || tmp == NULL || last_delta == NULL || delta == NULL) {
    OPENSSL_PUT_ERROR(BN, ERR_R_MALLOC_FAILURE);
    goto err;
  }

  if (!BN_lshift(estimate, BN_value_one(), BN_num_bits(in)/2)) {
    goto err;
  }


  for (;;) {

    if (!BN_div(tmp, NULL, in, estimate, ctx) ||
        !BN_add(tmp, tmp, estimate) ||
        !BN_rshift1(estimate, tmp) ||

        !BN_sqr(tmp, estimate, ctx) ||

        !BN_sub(delta, in, tmp)) {
      OPENSSL_PUT_ERROR(BN, ERR_R_BN_LIB);
      goto err;
    }

    delta->neg = 0;



    if (last_delta_valid && BN_cmp(delta, last_delta) >= 0) {
      break;
    }

    last_delta_valid = 1;

    tmp2 = last_delta;
    last_delta = delta;
    delta = tmp2;
  }

  if (BN_cmp(tmp, in) != 0) {
    OPENSSL_PUT_ERROR(BN, BN_R_NOT_A_SQUARE);
    goto err;
  }

  ok = 1;

err:
  if (ok && out_sqrt == in && !BN_copy(out_sqrt, estimate)) {
    ok = 0;
  }
  BN_CTX_end(ctx);
  return ok;
}
