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

#define BN_lsw(n) (((n)->width == 0) ? (BN_ULONG) 0 : (n)->d[0])

int bn_jacobi(const BIGNUM *a, const BIGNUM *b, BN_CTX *ctx) {





  static const int tab[8] = {0, 1, 0, -1, 0, -1, 0, 1};

  if (!BN_is_odd(b)) {
    OPENSSL_PUT_ERROR(BN, BN_R_CALLED_WITH_EVEN_MODULUS);
    return -2;
  }

  if (BN_is_negative(b)) {
    OPENSSL_PUT_ERROR(BN, BN_R_NEGATIVE_NUMBER);
    return -2;
  }

  int ret = -2;
  BN_CTX_start(ctx);
  BIGNUM *A = BN_CTX_get(ctx);
  BIGNUM *B = BN_CTX_get(ctx);
  if (B == NULL) {
    goto end;
  }

  if (!BN_copy(A, a) ||
      !BN_copy(B, b)) {
    goto end;
  }




  ret = 1;

  while (1) {


    if (BN_is_zero(A)) {
      ret = BN_is_one(B) ? ret : 0;
      goto end;
    }

    int i = 0;
    while (!BN_is_bit_set(A, i)) {
      i++;
    }
    if (!BN_rshift(A, A, i)) {
      ret = -2;
      goto end;
    }
    if (i & 1) {


      ret = ret * tab[BN_lsw(B) & 7];
    }


    if ((A->neg ? ~BN_lsw(A) : BN_lsw(A)) & BN_lsw(B) & 2) {
      ret = -ret;
    }

    if (!BN_nnmod(B, B, A, ctx)) {
      ret = -2;
      goto end;
    }
    BIGNUM *tmp = A;
    A = B;
    B = tmp;
    tmp->neg = 0;
  }

end:
  BN_CTX_end(ctx);
  return ret;
}
