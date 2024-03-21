/*
 * Copyright 2013-2016 The OpenSSL Project Authors. All Rights Reserved.
 * Copyright (c) 2012, Intel Corporation. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 *
 * Originally written by Shay Gueron (1, 2), and Vlad Krasnov (1)
 * (1) Intel Corporation, Israel Development Center, Haifa, Israel
 * (2) University of Haifa, Israel
 */

#include "rsaz_exp.h"

#if defined(RSAZ_ENABLED)

#include <openssl/mem.h>

#include "internal.h"
#include "../../internal.h"

alignas(64) static const BN_ULONG one[40] = {
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// two80 is 2^80 in RSAZ's representation. Note RSAZ uses base 2^29, so this is
// 2^(29*2 + 22) = 2^80, not 2^(64*2 + 22).
alignas(64) static const BN_ULONG two80[40] = {
    0, 0, 1 << 22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0,       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void RSAZ_1024_mod_exp_avx2(BN_ULONG result_norm[16],
                            const BN_ULONG base_norm[16],
                            const BN_ULONG exponent[16],
                            const BN_ULONG m_norm[16], const BN_ULONG RR[16],
                            BN_ULONG k0,
                            BN_ULONG storage[MOD_EXP_CTIME_STORAGE_LEN]) {
  OPENSSL_STATIC_ASSERT(MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH % 64 == 0,
                        "MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH is too small");
  assert((uintptr_t)storage % 64 == 0);

  BN_ULONG *a_inv, *m, *result, *table_s = storage + 40 * 3, *R2 = table_s;

  if (((((uintptr_t)storage & 4095) + 320) >> 12) != 0) {
    result = storage;
    a_inv = storage + 40;
    m = storage + 40 * 2;  // should not cross page
  } else {
    m = storage;  // should not cross page
    result = storage + 40;
    a_inv = storage + 40 * 2;
  }

  rsaz_1024_norm2red_avx2(m, m_norm);
  rsaz_1024_norm2red_avx2(a_inv, base_norm);
  rsaz_1024_norm2red_avx2(R2, RR);


  rsaz_1024_mul_avx2(R2, R2, R2, m, k0);

  rsaz_1024_mul_avx2(R2, R2, two80, m, k0);


  rsaz_1024_mul_avx2(result, R2, one, m, k0);

  rsaz_1024_mul_avx2(a_inv, a_inv, R2, m, k0);

  rsaz_1024_scatter5_avx2(table_s, result, 0);
  rsaz_1024_scatter5_avx2(table_s, a_inv, 1);

  rsaz_1024_sqr_avx2(result, a_inv, m, k0, 1);
  rsaz_1024_scatter5_avx2(table_s, result, 2);
#if 0

  for (int index = 3; index < 32; index++) {
    rsaz_1024_mul_avx2(result, result, a_inv, m, k0);
    rsaz_1024_scatter5_avx2(table_s, result, index);
  }
#else

  rsaz_1024_sqr_avx2(result, result, m, k0, 1);
  rsaz_1024_scatter5_avx2(table_s, result, 4);

  rsaz_1024_sqr_avx2(result, result, m, k0, 1);
  rsaz_1024_scatter5_avx2(table_s, result, 8);

  rsaz_1024_sqr_avx2(result, result, m, k0, 1);
  rsaz_1024_scatter5_avx2(table_s, result, 16);

  rsaz_1024_mul_avx2(result, result, a_inv, m, k0);
  rsaz_1024_scatter5_avx2(table_s, result, 17);

  rsaz_1024_gather5_avx2(result, table_s, 2);
  rsaz_1024_mul_avx2(result, result, a_inv, m, k0);
  rsaz_1024_scatter5_avx2(table_s, result, 3);

  rsaz_1024_sqr_avx2(result, result, m, k0, 1);
  rsaz_1024_scatter5_avx2(table_s, result, 6);

  rsaz_1024_sqr_avx2(result, result, m, k0, 1);
  rsaz_1024_scatter5_avx2(table_s, result, 12);

  rsaz_1024_sqr_avx2(result, result, m, k0, 1);
  rsaz_1024_scatter5_avx2(table_s, result, 24);

  rsaz_1024_mul_avx2(result, result, a_inv, m, k0);
  rsaz_1024_scatter5_avx2(table_s, result, 25);

  rsaz_1024_gather5_avx2(result, table_s, 4);
  rsaz_1024_mul_avx2(result, result, a_inv, m, k0);
  rsaz_1024_scatter5_avx2(table_s, result, 5);

  rsaz_1024_sqr_avx2(result, result, m, k0, 1);
  rsaz_1024_scatter5_avx2(table_s, result, 10);

  rsaz_1024_sqr_avx2(result, result, m, k0, 1);
  rsaz_1024_scatter5_avx2(table_s, result, 20);

  rsaz_1024_mul_avx2(result, result, a_inv, m, k0);
  rsaz_1024_scatter5_avx2(table_s, result, 21);

  rsaz_1024_gather5_avx2(result, table_s, 6);
  rsaz_1024_mul_avx2(result, result, a_inv, m, k0);
  rsaz_1024_scatter5_avx2(table_s, result, 7);

  rsaz_1024_sqr_avx2(result, result, m, k0, 1);
  rsaz_1024_scatter5_avx2(table_s, result, 14);

  rsaz_1024_sqr_avx2(result, result, m, k0, 1);
  rsaz_1024_scatter5_avx2(table_s, result, 28);

  rsaz_1024_mul_avx2(result, result, a_inv, m, k0);
  rsaz_1024_scatter5_avx2(table_s, result, 29);

  rsaz_1024_gather5_avx2(result, table_s, 8);
  rsaz_1024_mul_avx2(result, result, a_inv, m, k0);
  rsaz_1024_scatter5_avx2(table_s, result, 9);

  rsaz_1024_sqr_avx2(result, result, m, k0, 1);
  rsaz_1024_scatter5_avx2(table_s, result, 18);

  rsaz_1024_mul_avx2(result, result, a_inv, m, k0);
  rsaz_1024_scatter5_avx2(table_s, result, 19);

  rsaz_1024_gather5_avx2(result, table_s, 10);
  rsaz_1024_mul_avx2(result, result, a_inv, m, k0);
  rsaz_1024_scatter5_avx2(table_s, result, 11);

  rsaz_1024_sqr_avx2(result, result, m, k0, 1);
  rsaz_1024_scatter5_avx2(table_s, result, 22);

  rsaz_1024_mul_avx2(result, result, a_inv, m, k0);
  rsaz_1024_scatter5_avx2(table_s, result, 23);

  rsaz_1024_gather5_avx2(result, table_s, 12);
  rsaz_1024_mul_avx2(result, result, a_inv, m, k0);
  rsaz_1024_scatter5_avx2(table_s, result, 13);

  rsaz_1024_sqr_avx2(result, result, m, k0, 1);
  rsaz_1024_scatter5_avx2(table_s, result, 26);

  rsaz_1024_mul_avx2(result, result, a_inv, m, k0);
  rsaz_1024_scatter5_avx2(table_s, result, 27);

  rsaz_1024_gather5_avx2(result, table_s, 14);
  rsaz_1024_mul_avx2(result, result, a_inv, m, k0);
  rsaz_1024_scatter5_avx2(table_s, result, 15);

  rsaz_1024_sqr_avx2(result, result, m, k0, 1);
  rsaz_1024_scatter5_avx2(table_s, result, 30);

  rsaz_1024_mul_avx2(result, result, a_inv, m, k0);
  rsaz_1024_scatter5_avx2(table_s, result, 31);
#endif

  const uint8_t *p_str = (const uint8_t *)exponent;

  int wvalue = p_str[127] >> 3;
  rsaz_1024_gather5_avx2(result, table_s, wvalue);

  int index = 1014;
  while (index > -1) {  // Loop for the remaining 127 windows.

    rsaz_1024_sqr_avx2(result, result, m, k0, 5);

    uint16_t wvalue_16;
    memcpy(&wvalue_16, &p_str[index / 8], sizeof(wvalue_16));
    wvalue = wvalue_16;
    wvalue = (wvalue >> (index % 8)) & 31;
    index -= 5;

    rsaz_1024_gather5_avx2(a_inv, table_s, wvalue);  // Borrow |a_inv|.
    rsaz_1024_mul_avx2(result, result, a_inv, m, k0);
  }

  rsaz_1024_sqr_avx2(result, result, m, k0, 4);

  wvalue = p_str[0] & 15;

  rsaz_1024_gather5_avx2(a_inv, table_s, wvalue);  // Borrow |a_inv|.
  rsaz_1024_mul_avx2(result, result, a_inv, m, k0);

  rsaz_1024_mul_avx2(result, result, one, m, k0);

  rsaz_1024_red2norm_avx2(result_norm, result);

  OPENSSL_cleanse(storage, MOD_EXP_CTIME_STORAGE_LEN * sizeof(BN_ULONG));
}

#endif  // RSAZ_ENABLED
