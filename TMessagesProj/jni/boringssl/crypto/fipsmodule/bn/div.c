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

#include <openssl/bn.h>

#include <assert.h>
#include <limits.h>

#include <openssl/err.h>

#include "internal.h"


#if !defined(BN_CAN_DIVIDE_ULLONG) && !defined(BN_CAN_USE_INLINE_ASM)
// bn_div_words divides a double-width |h|,|l| by |d| and returns the result,
// which must fit in a |BN_ULONG|.
static BN_ULONG bn_div_words(BN_ULONG h, BN_ULONG l, BN_ULONG d) {
  BN_ULONG dh, dl, q, ret = 0, th, tl, t;
  int i, count = 2;

  if (d == 0) {
    return BN_MASK2;
  }

  i = BN_num_bits_word(d);
  assert((i == BN_BITS2) || (h <= (BN_ULONG)1 << i));

  i = BN_BITS2 - i;
  if (h >= d) {
    h -= d;
  }

  if (i) {
    d <<= i;
    h = (h << i) | (l >> (BN_BITS2 - i));
    l <<= i;
  }
  dh = (d & BN_MASK2h) >> BN_BITS4;
  dl = (d & BN_MASK2l);
  for (;;) {
    if ((h >> BN_BITS4) == dh) {
      q = BN_MASK2l;
    } else {
      q = h / dh;
    }

    th = q * dh;
    tl = dl * q;
    for (;;) {
      t = h - th;
      if ((t & BN_MASK2h) ||
          ((tl) <= ((t << BN_BITS4) | ((l & BN_MASK2h) >> BN_BITS4)))) {
        break;
      }
      q--;
      th -= dh;
      tl -= dl;
    }
    t = (tl >> BN_BITS4);
    tl = (tl << BN_BITS4) & BN_MASK2h;
    th += t;

    if (l < tl) {
      th++;
    }
    l -= tl;
    if (h < th) {
      h += d;
      q--;
    }
    h -= th;

    if (--count == 0) {
      break;
    }

    ret = q << BN_BITS4;
    h = (h << BN_BITS4) | (l >> BN_BITS4);
    l = (l & BN_MASK2l) << BN_BITS4;
  }

  ret |= q;
  return ret;
}
#endif  // !defined(BN_CAN_DIVIDE_ULLONG) && !defined(BN_CAN_USE_INLINE_ASM)

static inline void bn_div_rem_words(BN_ULONG *quotient_out, BN_ULONG *rem_out,
                                    BN_ULONG n0, BN_ULONG n1, BN_ULONG d0) {
















#if defined(BN_CAN_USE_INLINE_ASM) && defined(OPENSSL_X86)
  __asm__ volatile("divl %4"
                   : "=a"(*quotient_out), "=d"(*rem_out)
                   : "a"(n1), "d"(n0), "rm"(d0)
                   : "cc");
#elif defined(BN_CAN_USE_INLINE_ASM) && defined(OPENSSL_X86_64)
  __asm__ volatile("divq %4"
                   : "=a"(*quotient_out), "=d"(*rem_out)
                   : "a"(n1), "d"(n0), "rm"(d0)
                   : "cc");
#else
#if defined(BN_CAN_DIVIDE_ULLONG)
  BN_ULLONG n = (((BN_ULLONG)n0) << BN_BITS2) | n1;
  *quotient_out = (BN_ULONG)(n / d0);
#else
  *quotient_out = bn_div_words(n0, n1, d0);
#endif
  *rem_out = n1 - (*quotient_out * d0);
#endif
}

// and sets up |rem| such that "quotient * divisor + rem = numerator" holds.
//
// Thus:
//
//     quotient->neg == numerator->neg ^ divisor->neg
//        (unless the result is zero)
//     rem->neg == numerator->neg
//        (unless the remainder is zero)
//
// If |quotient| or |rem| is NULL, the respective value is not returned.
//
// This was specifically designed to contain fewer branches that may leak
// sensitive information; see "New Branch Prediction Vulnerabilities in OpenSSL
// and Necessary Software Countermeasures" by Onur Acıçmez, Shay Gueron, and
// Jean-Pierre Seifert.
int BN_div(BIGNUM *quotient, BIGNUM *rem, const BIGNUM *numerator,
           const BIGNUM *divisor, BN_CTX *ctx) {
  int norm_shift, loop;
  BIGNUM wnum;
  BN_ULONG *resp, *wnump;
  BN_ULONG d0, d1;
  int num_n, div_n;





  int numerator_width = bn_minimal_width(numerator);
  int divisor_width = bn_minimal_width(divisor);
  if ((numerator_width > 0 && numerator->d[numerator_width - 1] == 0) ||
      (divisor_width > 0 && divisor->d[divisor_width - 1] == 0)) {
    OPENSSL_PUT_ERROR(BN, BN_R_NOT_INITIALIZED);
    return 0;
  }

  if (BN_is_zero(divisor)) {
    OPENSSL_PUT_ERROR(BN, BN_R_DIV_BY_ZERO);
    return 0;
  }

  BN_CTX_start(ctx);
  BIGNUM *tmp = BN_CTX_get(ctx);
  BIGNUM *snum = BN_CTX_get(ctx);
  BIGNUM *sdiv = BN_CTX_get(ctx);
  BIGNUM *res = NULL;
  if (quotient == NULL) {
    res = BN_CTX_get(ctx);
  } else {
    res = quotient;
  }
  if (sdiv == NULL || res == NULL) {
    goto err;
  }

  norm_shift = BN_BITS2 - (BN_num_bits(divisor) % BN_BITS2);
  if (!BN_lshift(sdiv, divisor, norm_shift)) {
    goto err;
  }
  bn_set_minimal_width(sdiv);
  sdiv->neg = 0;
  norm_shift += BN_BITS2;
  if (!BN_lshift(snum, numerator, norm_shift)) {
    goto err;
  }
  bn_set_minimal_width(snum);
  snum->neg = 0;



  if (snum->width <= sdiv->width + 1) {
    if (!bn_wexpand(snum, sdiv->width + 2)) {
      goto err;
    }
    for (int i = snum->width; i < sdiv->width + 2; i++) {
      snum->d[i] = 0;
    }
    snum->width = sdiv->width + 2;
  } else {
    if (!bn_wexpand(snum, snum->width + 1)) {
      goto err;
    }
    snum->d[snum->width] = 0;
    snum->width++;
  }

  div_n = sdiv->width;
  num_n = snum->width;
  loop = num_n - div_n;



  wnum.neg = 0;
  wnum.d = &(snum->d[loop]);
  wnum.width = div_n;

  wnum.dmax = snum->dmax - loop;  // so we don't step out of bounds


  d0 = sdiv->d[div_n - 1];
  d1 = (div_n == 1) ? 0 : sdiv->d[div_n - 2];

  wnump = &(snum->d[num_n - 1]);

  res->neg = (numerator->neg ^ divisor->neg);
  if (!bn_wexpand(res, loop + 1)) {
    goto err;
  }
  res->width = loop - 1;
  resp = &(res->d[loop - 1]);

  if (!bn_wexpand(tmp, div_n + 1)) {
    goto err;
  }


  if (res->width == 0) {
    res->neg = 0;
  } else {
    resp--;
  }

  for (int i = 0; i < loop - 1; i++, wnump--, resp--) {
    BN_ULONG q, l0;


    BN_ULONG n0, n1, rm = 0;

    n0 = wnump[0];
    n1 = wnump[-1];
    if (n0 == d0) {
      q = BN_MASK2;
    } else {

      bn_div_rem_words(&q, &rm, n0, n1, d0);

#ifdef BN_ULLONG
      BN_ULLONG t2 = (BN_ULLONG)d1 * q;
      for (;;) {
        if (t2 <= ((((BN_ULLONG)rm) << BN_BITS2) | wnump[-2])) {
          break;
        }
        q--;
        rm += d0;
        if (rm < d0) {
          break;  // don't let rm overflow
        }
        t2 -= d1;
      }
#else  // !BN_ULLONG
      BN_ULONG t2l, t2h;
      BN_UMULT_LOHI(t2l, t2h, d1, q);
      for (;;) {
        if (t2h < rm ||
            (t2h == rm && t2l <= wnump[-2])) {
          break;
        }
        q--;
        rm += d0;
        if (rm < d0) {
          break;  // don't let rm overflow
        }
        if (t2l < d1) {
          t2h--;
        }
        t2l -= d1;
      }
#endif  // !BN_ULLONG
    }

    l0 = bn_mul_words(tmp->d, sdiv->d, div_n, q);
    tmp->d[div_n] = l0;
    wnum.d--;


    if (bn_sub_words(wnum.d, wnum.d, tmp->d, div_n + 1)) {




      q--;
      if (bn_add_words(wnum.d, wnum.d, sdiv->d, div_n)) {



        (*wnump)++;
      }
    }

    *resp = q;
  }

  bn_set_minimal_width(snum);

  if (rem != NULL) {


    int neg = numerator->neg;
    if (!BN_rshift(rem, snum, norm_shift)) {
      goto err;
    }
    if (!BN_is_zero(rem)) {
      rem->neg = neg;
    }
  }

  bn_set_minimal_width(res);
  BN_CTX_end(ctx);
  return 1;

err:
  BN_CTX_end(ctx);
  return 0;
}

int BN_nnmod(BIGNUM *r, const BIGNUM *m, const BIGNUM *d, BN_CTX *ctx) {
  if (!(BN_mod(r, m, d, ctx))) {
    return 0;
  }
  if (!r->neg) {
    return 1;
  }

  return (d->neg ? BN_sub : BN_add)(r, r, d);
}

BN_ULONG bn_reduce_once(BN_ULONG *r, const BN_ULONG *a, BN_ULONG carry,
                        const BN_ULONG *m, size_t num) {
  assert(r != a);


  carry -= bn_sub_words(r, a, m, num);









  assert(carry == 0 || carry == (BN_ULONG)-1);
  bn_select_words(r, carry, a /* r < 0 */, r /* r >= 0 */, num);
  return carry;
}

BN_ULONG bn_reduce_once_in_place(BN_ULONG *r, BN_ULONG carry, const BN_ULONG *m,
                                 BN_ULONG *tmp, size_t num) {

  carry -= bn_sub_words(tmp, r, m, num);
  assert(carry == 0 || carry == (BN_ULONG)-1);
  bn_select_words(r, carry, r /* tmp < 0 */, tmp /* tmp >= 0 */, num);
  return carry;
}

void bn_mod_sub_words(BN_ULONG *r, const BN_ULONG *a, const BN_ULONG *b,
                      const BN_ULONG *m, BN_ULONG *tmp, size_t num) {

  BN_ULONG borrow = bn_sub_words(r, a, b, num);

  bn_add_words(tmp, r, m, num);
  bn_select_words(r, 0 - borrow, tmp /* r < 0 */, r /* r >= 0 */, num);
}

void bn_mod_add_words(BN_ULONG *r, const BN_ULONG *a, const BN_ULONG *b,
                      const BN_ULONG *m, BN_ULONG *tmp, size_t num) {
  BN_ULONG carry = bn_add_words(r, a, b, num);
  bn_reduce_once_in_place(r, carry, m, tmp, num);
}

int bn_div_consttime(BIGNUM *quotient, BIGNUM *remainder,
                     const BIGNUM *numerator, const BIGNUM *divisor,
                     BN_CTX *ctx) {
  if (BN_is_negative(numerator) || BN_is_negative(divisor)) {
    OPENSSL_PUT_ERROR(BN, BN_R_NEGATIVE_NUMBER);
    return 0;
  }
  if (BN_is_zero(divisor)) {
    OPENSSL_PUT_ERROR(BN, BN_R_DIV_BY_ZERO);
    return 0;
  }




  int ret = 0;
  BN_CTX_start(ctx);
  BIGNUM *q = quotient, *r = remainder;
  if (quotient == NULL || quotient == numerator || quotient == divisor) {
    q = BN_CTX_get(ctx);
  }
  if (remainder == NULL || remainder == numerator || remainder == divisor) {
    r = BN_CTX_get(ctx);
  }
  BIGNUM *tmp = BN_CTX_get(ctx);
  if (q == NULL || r == NULL || tmp == NULL ||
      !bn_wexpand(q, numerator->width) ||
      !bn_wexpand(r, divisor->width) ||
      !bn_wexpand(tmp, divisor->width)) {
    goto err;
  }

  OPENSSL_memset(q->d, 0, numerator->width * sizeof(BN_ULONG));
  q->width = numerator->width;
  q->neg = 0;

  OPENSSL_memset(r->d, 0, divisor->width * sizeof(BN_ULONG));
  r->width = divisor->width;
  r->neg = 0;


  for (int i = numerator->width - 1; i >= 0; i--) {
    for (int bit = BN_BITS2 - 1; bit >= 0; bit--) {



      BN_ULONG carry = bn_add_words(r->d, r->d, r->d, divisor->width);
      r->d[0] |= (numerator->d[i] >> bit) & 1;




      BN_ULONG subtracted = bn_reduce_once_in_place(r->d, carry, divisor->d,
                                                    tmp->d, divisor->width);

      q->d[i] |= (~subtracted & 1) << bit;
    }
  }

  if ((quotient != NULL && !BN_copy(quotient, q)) ||
      (remainder != NULL && !BN_copy(remainder, r))) {
    goto err;
  }

  ret = 1;

err:
  BN_CTX_end(ctx);
  return ret;
}

static BIGNUM *bn_scratch_space_from_ctx(size_t width, BN_CTX *ctx) {
  BIGNUM *ret = BN_CTX_get(ctx);
  if (ret == NULL ||
      !bn_wexpand(ret, width)) {
    return NULL;
  }
  ret->neg = 0;
  ret->width = width;
  return ret;
}

// error. This is so it may be used with low-level "words" functions. If
// necessary, it allocates a new |BIGNUM| with a lifetime of the current scope
// in |ctx|, so the caller does not need to explicitly free it. |bn| must fit in
// |width| words.
static const BIGNUM *bn_resized_from_ctx(const BIGNUM *bn, size_t width,
                                         BN_CTX *ctx) {
  if ((size_t)bn->width >= width) {

    assert(bn_fits_in_words(bn, width));
    return bn;
  }
  BIGNUM *ret = bn_scratch_space_from_ctx(width, ctx);
  if (ret == NULL ||
      !BN_copy(ret, bn) ||
      !bn_resize_words(ret, width)) {
    return NULL;
  }
  return ret;
}

int BN_mod_add(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, const BIGNUM *m,
               BN_CTX *ctx) {
  if (!BN_add(r, a, b)) {
    return 0;
  }
  return BN_nnmod(r, r, m, ctx);
}

int BN_mod_add_quick(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
                     const BIGNUM *m) {
  BN_CTX *ctx = BN_CTX_new();
  int ok = ctx != NULL &&
           bn_mod_add_consttime(r, a, b, m, ctx);
  BN_CTX_free(ctx);
  return ok;
}

int bn_mod_add_consttime(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
                         const BIGNUM *m, BN_CTX *ctx) {
  BN_CTX_start(ctx);
  a = bn_resized_from_ctx(a, m->width, ctx);
  b = bn_resized_from_ctx(b, m->width, ctx);
  BIGNUM *tmp = bn_scratch_space_from_ctx(m->width, ctx);
  int ok = a != NULL && b != NULL && tmp != NULL &&
           bn_wexpand(r, m->width);
  if (ok) {
    bn_mod_add_words(r->d, a->d, b->d, m->d, tmp->d, m->width);
    r->width = m->width;
    r->neg = 0;
  }
  BN_CTX_end(ctx);
  return ok;
}

int BN_mod_sub(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, const BIGNUM *m,
               BN_CTX *ctx) {
  if (!BN_sub(r, a, b)) {
    return 0;
  }
  return BN_nnmod(r, r, m, ctx);
}

int bn_mod_sub_consttime(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
                         const BIGNUM *m, BN_CTX *ctx) {
  BN_CTX_start(ctx);
  a = bn_resized_from_ctx(a, m->width, ctx);
  b = bn_resized_from_ctx(b, m->width, ctx);
  BIGNUM *tmp = bn_scratch_space_from_ctx(m->width, ctx);
  int ok = a != NULL && b != NULL && tmp != NULL &&
           bn_wexpand(r, m->width);
  if (ok) {
    bn_mod_sub_words(r->d, a->d, b->d, m->d, tmp->d, m->width);
    r->width = m->width;
    r->neg = 0;
  }
  BN_CTX_end(ctx);
  return ok;
}

int BN_mod_sub_quick(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
                     const BIGNUM *m) {
  BN_CTX *ctx = BN_CTX_new();
  int ok = ctx != NULL &&
           bn_mod_sub_consttime(r, a, b, m, ctx);
  BN_CTX_free(ctx);
  return ok;
}

int BN_mod_mul(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, const BIGNUM *m,
               BN_CTX *ctx) {
  BIGNUM *t;
  int ret = 0;

  BN_CTX_start(ctx);
  t = BN_CTX_get(ctx);
  if (t == NULL) {
    goto err;
  }

  if (a == b) {
    if (!BN_sqr(t, a, ctx)) {
      goto err;
    }
  } else {
    if (!BN_mul(t, a, b, ctx)) {
      goto err;
    }
  }

  if (!BN_nnmod(r, t, m, ctx)) {
    goto err;
  }

  ret = 1;

err:
  BN_CTX_end(ctx);
  return ret;
}

int BN_mod_sqr(BIGNUM *r, const BIGNUM *a, const BIGNUM *m, BN_CTX *ctx) {
  if (!BN_sqr(r, a, ctx)) {
    return 0;
  }

  return BN_mod(r, r, m, ctx);
}

int BN_mod_lshift(BIGNUM *r, const BIGNUM *a, int n, const BIGNUM *m,
                  BN_CTX *ctx) {
  BIGNUM *abs_m = NULL;
  int ret;

  if (!BN_nnmod(r, a, m, ctx)) {
    return 0;
  }

  if (m->neg) {
    abs_m = BN_dup(m);
    if (abs_m == NULL) {
      return 0;
    }
    abs_m->neg = 0;
  }

  ret = bn_mod_lshift_consttime(r, r, n, (abs_m ? abs_m : m), ctx);

  BN_free(abs_m);
  return ret;
}

int bn_mod_lshift_consttime(BIGNUM *r, const BIGNUM *a, int n, const BIGNUM *m,
                            BN_CTX *ctx) {
  if (!BN_copy(r, a)) {
    return 0;
  }
  for (int i = 0; i < n; i++) {
    if (!bn_mod_lshift1_consttime(r, r, m, ctx)) {
      return 0;
    }
  }
  return 1;
}

int BN_mod_lshift_quick(BIGNUM *r, const BIGNUM *a, int n, const BIGNUM *m) {
  BN_CTX *ctx = BN_CTX_new();
  int ok = ctx != NULL &&
           bn_mod_lshift_consttime(r, a, n, m, ctx);
  BN_CTX_free(ctx);
  return ok;
}

int BN_mod_lshift1(BIGNUM *r, const BIGNUM *a, const BIGNUM *m, BN_CTX *ctx) {
  if (!BN_lshift1(r, a)) {
    return 0;
  }

  return BN_nnmod(r, r, m, ctx);
}

int bn_mod_lshift1_consttime(BIGNUM *r, const BIGNUM *a, const BIGNUM *m,
                             BN_CTX *ctx) {
  return bn_mod_add_consttime(r, a, a, m, ctx);
}

int BN_mod_lshift1_quick(BIGNUM *r, const BIGNUM *a, const BIGNUM *m) {
  BN_CTX *ctx = BN_CTX_new();
  int ok = ctx != NULL &&
           bn_mod_lshift1_consttime(r, a, m, ctx);
  BN_CTX_free(ctx);
  return ok;
}

BN_ULONG BN_div_word(BIGNUM *a, BN_ULONG w) {
  BN_ULONG ret = 0;
  int i, j;

  if (!w) {

    return (BN_ULONG) - 1;
  }

  if (a->width == 0) {
    return 0;
  }

  j = BN_BITS2 - BN_num_bits_word(w);
  w <<= j;
  if (!BN_lshift(a, a, j)) {
    return (BN_ULONG) - 1;
  }

  for (i = a->width - 1; i >= 0; i--) {
    BN_ULONG l = a->d[i];
    BN_ULONG d;
    BN_ULONG unused_rem;
    bn_div_rem_words(&d, &unused_rem, ret, l, w);
    ret = l - (d * w);
    a->d[i] = d;
  }

  bn_set_minimal_width(a);
  ret >>= j;
  return ret;
}

BN_ULONG BN_mod_word(const BIGNUM *a, BN_ULONG w) {
#ifndef BN_CAN_DIVIDE_ULLONG
  BN_ULONG ret = 0;
#else
  BN_ULLONG ret = 0;
#endif
  int i;

  if (w == 0) {
    return (BN_ULONG) -1;
  }

#ifndef BN_CAN_DIVIDE_ULLONG


  if (w > ((BN_ULONG)1 << BN_BITS4)) {
    BIGNUM *tmp = BN_dup(a);
    if (tmp == NULL) {
      return (BN_ULONG)-1;
    }
    ret = BN_div_word(tmp, w);
    BN_free(tmp);
    return ret;
  }
#endif

  for (i = a->width - 1; i >= 0; i--) {
#ifndef BN_CAN_DIVIDE_ULLONG
    ret = ((ret << BN_BITS4) | ((a->d[i] >> BN_BITS4) & BN_MASK2l)) % w;
    ret = ((ret << BN_BITS4) | (a->d[i] & BN_MASK2l)) % w;
#else
    ret = (BN_ULLONG)(((ret << (BN_ULLONG)BN_BITS2) | a->d[i]) % (BN_ULLONG)w);
#endif
  }
  return (BN_ULONG)ret;
}

int BN_mod_pow2(BIGNUM *r, const BIGNUM *a, size_t e) {
  if (e == 0 || a->width == 0) {
    BN_zero(r);
    return 1;
  }

  size_t num_words = 1 + ((e - 1) / BN_BITS2);

  if ((size_t) a->width < num_words) {
    return BN_copy(r, a) != NULL;
  }


  if (!bn_wexpand(r, num_words)) {
    return 0;
  }

  OPENSSL_memcpy(r->d, a->d, num_words * sizeof(BN_ULONG));

  size_t top_word_exponent = e % (sizeof(BN_ULONG) * 8);
  if (top_word_exponent != 0) {
    r->d[num_words - 1] &= (((BN_ULONG) 1) << top_word_exponent) - 1;
  }

  r->neg = a->neg;
  r->width = (int) num_words;
  bn_set_minimal_width(r);
  return 1;
}

int BN_nnmod_pow2(BIGNUM *r, const BIGNUM *a, size_t e) {
  if (!BN_mod_pow2(r, a, e)) {
    return 0;
  }

  if (BN_is_zero(r) || !r->neg) {
    return 1;
  }

  size_t num_words = 1 + (e - 1) / BN_BITS2;

  if (!bn_wexpand(r, num_words)) {
    return 0;
  }

  OPENSSL_memset(&r->d[r->width], 0, (num_words - r->width) * BN_BYTES);

  r->neg = 0;
  r->width = (int) num_words;



  for (int i = 0; i < r->width; i++) {
    r->d[i] = ~r->d[i];
  }

  size_t top_word_exponent = e % BN_BITS2;
  if (top_word_exponent != 0) {
    r->d[r->width - 1] &= (((BN_ULONG) 1) << top_word_exponent) - 1;
  }

  bn_set_minimal_width(r);

  return BN_add(r, r, BN_value_one());
}
