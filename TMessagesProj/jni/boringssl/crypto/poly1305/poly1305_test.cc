/* Copyright (c) 2015, Google Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

#include <stdio.h>
#include <string.h>

#include <vector>

#include <gtest/gtest.h>

#include <openssl/poly1305.h>

#include "../internal.h"
#include "../test/file_test.h"
#include "../test/test_util.h"


static void TestSIMD(unsigned excess, const std::vector<uint8_t> &key,
                     const std::vector<uint8_t> &in,
                     const std::vector<uint8_t> &mac) {
  poly1305_state state;
  CRYPTO_poly1305_init(&state, key.data());

  size_t done = 0;


  size_t todo = 16;
  if (todo > in.size()) {
    todo = in.size();
  }
  CRYPTO_poly1305_update(&state, in.data(), todo);
  done += todo;

  for (;;) {

    if (done + 128 + excess > in.size()) {
      break;
    }
    CRYPTO_poly1305_update(&state, in.data() + done, 128 + excess);
    done += 128 + excess;

    if (done + excess > in.size()) {
      break;
    }
    CRYPTO_poly1305_update(&state, in.data() + done, excess);
    done += excess;
  }

  CRYPTO_poly1305_update(&state, in.data() + done, in.size() - done);

  alignas(16) uint8_t out[16];
  CRYPTO_poly1305_finish(&state, out);
  EXPECT_EQ(Bytes(out), Bytes(mac)) << "SIMD pattern " << excess << " failed.";
}

TEST(Poly1305Test, TestVectors) {
  FileTestGTest("crypto/poly1305/poly1305_tests.txt", [](FileTest *t) {
    std::vector<uint8_t> key, in, mac;
    ASSERT_TRUE(t->GetBytes(&key, "Key"));
    ASSERT_TRUE(t->GetBytes(&in, "Input"));
    ASSERT_TRUE(t->GetBytes(&mac, "MAC"));
    ASSERT_EQ(32u, key.size());
    ASSERT_EQ(16u, mac.size());

    poly1305_state state;
    CRYPTO_poly1305_init(&state, key.data());
    CRYPTO_poly1305_update(&state, in.data(), in.size());

    alignas(16) uint8_t out[16];
    CRYPTO_poly1305_finish(&state, out);
    EXPECT_EQ(Bytes(out), Bytes(mac)) << "Single-shot Poly1305 failed.";

    CRYPTO_poly1305_init(&state, key.data());
    for (size_t i = 0; i < in.size(); i++) {
      CRYPTO_poly1305_update(&state, &in[i], 1);
    }
    CRYPTO_poly1305_finish(&state, out);
    EXPECT_EQ(Bytes(out), Bytes(mac)) << "Streaming Poly1305 failed.";


    TestSIMD(0, key, in, mac);
    TestSIMD(16, key, in, mac);
    TestSIMD(32, key, in, mac);
    TestSIMD(48, key, in, mac);
  });
}
