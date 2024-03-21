/* Copyright (c) 2014, Google Inc.
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

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include <openssl/asn1.h>
#include <openssl/bytestring.h>
#include <openssl/crypto.h>
#include <openssl/digest.h>
#include <openssl/err.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/nid.h>
#include <openssl/obj.h>
#include <openssl/sha.h>

#include "../internal.h"
#include "../test/test_util.h"


struct MD {

  const char* name;

  const EVP_MD *(*func)(void);


  uint8_t *(*one_shot_func)(const uint8_t *, size_t, uint8_t *);
};

static const MD md4 = { "MD4", &EVP_md4, nullptr };
static const MD md5 = { "MD5", &EVP_md5, &MD5 };
static const MD sha1 = { "SHA1", &EVP_sha1, &SHA1 };
static const MD sha224 = { "SHA224", &EVP_sha224, &SHA224 };
static const MD sha256 = { "SHA256", &EVP_sha256, &SHA256 };
static const MD sha384 = { "SHA384", &EVP_sha384, &SHA384 };
static const MD sha512 = { "SHA512", &EVP_sha512, &SHA512 };
static const MD md5_sha1 = { "MD5-SHA1", &EVP_md5_sha1, nullptr };

struct DigestTestVector {

  const MD &md;

  const char *input;

  size_t repeat;

  const char *expected_hex;
};

static const DigestTestVector kTestVectors[] = {


    { md4, "", 1, "31d6cfe0d16ae931b73c59d7e0c089c0" },
    { md4, "a", 1, "bde52cb31de33e46245e05fbdbd6fb24" },
    { md4, "abc", 1, "a448017aaf21d8525fc10ae87aa6729d" },
    { md4, "message digest", 1, "d9130a8164549fe818874806e1c7014b" },
    { md4, "abcdefghijklmnopqrstuvwxyz", 1,
      "d79e1c308aa5bbcdeea8ed63df412da9" },
    { md4,
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", 1,
      "043f8582f241db351ce627e153e7f0e4" },
    { md4, "1234567890", 8, "e33b4ddc9c38f2199c3e7b164fcc0536" },

    { md5, "", 1, "d41d8cd98f00b204e9800998ecf8427e" },
    { md5, "a", 1, "0cc175b9c0f1b6a831c399e269772661" },
    { md5, "abc", 1, "900150983cd24fb0d6963f7d28e17f72" },
    { md5, "message digest", 1, "f96b697d7cb7938d525a2f31aaf161d0" },
    { md5, "abcdefghijklmnopqrstuvwxyz", 1,
      "c3fcd3d76192e4007dfb496cca67e13b" },
    { md5,
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789", 1,
      "d174ab98d277d9f5a5611c2c9f419d9f" },
    { md5, "1234567890", 8, "57edf4a22be3c955ac49da2e2107b67a" },

    { sha1, "abc", 1, "a9993e364706816aba3e25717850c26c9cd0d89d" },
    { sha1,
      "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 1,
      "84983e441c3bd26ebaae4aa1f95129e5e54670f1" },
    { sha1, "a", 1000000, "34aa973cd4c4daa4f61eeb2bdbad27316534016f" },
    { sha1,
      "0123456701234567012345670123456701234567012345670123456701234567", 10,
      "dea356a2cddd90c7a7ecedc5ebb563934f460452" },

    { sha224, "abc", 1,
      "23097d223405d8228642a477bda255b32aadbce4bda0b3f7e36c9da7" },
    { sha224,
      "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 1,
      "75388b16512776cc5dba5da1fd890150b0c6455cb4f58b1952522525" },
    { sha224,
      "a", 1000000,
      "20794655980c91d8bbb4c1ea97618a4bf03f42581948b2ee4ee7ad67" },

    { sha256, "abc", 1,
      "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad" },
    { sha256,
      "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", 1,
      "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1" },

    { sha384, "abc", 1,
      "cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed"
      "8086072ba1e7cc2358baeca134c825a7" },
    { sha384,
      "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
      "hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu", 1,
      "09330c33f71147e83d192fc782cd1b4753111b173b3b05d22fa08086e3b0f712"
      "fcc7c71a557e2db966c3e9fa91746039" },

    { sha512, "abc", 1,
      "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a"
      "2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f" },
    { sha512,
      "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
      "hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu", 1,
      "8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018"
      "501d289e4900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909" },

    { md5_sha1, "abc", 1,
      "900150983cd24fb0d6963f7d28e17f72a9993e364706816aba3e25717850c26c9cd0d89d" },
};

static void CompareDigest(const DigestTestVector *test,
                          const uint8_t *digest,
                          size_t digest_len) {
  static const char kHexTable[] = "0123456789abcdef";
  char digest_hex[2*EVP_MAX_MD_SIZE + 1];

  for (size_t i = 0; i < digest_len; i++) {
    digest_hex[2*i] = kHexTable[digest[i] >> 4];
    digest_hex[2*i + 1] = kHexTable[digest[i] & 0xf];
  }
  digest_hex[2*digest_len] = '\0';

  EXPECT_STREQ(test->expected_hex, digest_hex);
}

static void TestDigest(const DigestTestVector *test) {
  bssl::ScopedEVP_MD_CTX ctx;

  ASSERT_TRUE(EVP_DigestInit_ex(ctx.get(), test->md.func(), NULL));
  for (size_t i = 0; i < test->repeat; i++) {
    ASSERT_TRUE(EVP_DigestUpdate(ctx.get(), test->input, strlen(test->input)));
  }
  std::unique_ptr<uint8_t[]> digest(new uint8_t[EVP_MD_size(test->md.func())]);
  unsigned digest_len;
  ASSERT_TRUE(EVP_DigestFinal_ex(ctx.get(), digest.get(), &digest_len));
  CompareDigest(test, digest.get(), digest_len);

  ASSERT_TRUE(EVP_DigestInit_ex(ctx.get(), test->md.func(), NULL));
  ASSERT_TRUE(EVP_DigestUpdate(ctx.get(), NULL, 0));
  for (size_t i = 0; i < test->repeat; i++) {
    for (const char *p = test->input; *p; p++) {
      ASSERT_TRUE(EVP_DigestUpdate(ctx.get(), p, 1));
    }
  }
  ASSERT_TRUE(EVP_DigestFinal_ex(ctx.get(), digest.get(), &digest_len));
  EXPECT_EQ(EVP_MD_size(test->md.func()), digest_len);
  CompareDigest(test, digest.get(), digest_len);

  ASSERT_TRUE(EVP_DigestInit_ex(ctx.get(), test->md.func(), NULL));
  std::vector<char> unaligned(strlen(test->input) + 1);
  char *ptr = unaligned.data();
  if ((reinterpret_cast<uintptr_t>(ptr) & 1) == 0) {
    ptr++;
  }
  OPENSSL_memcpy(ptr, test->input, strlen(test->input));
  for (size_t i = 0; i < test->repeat; i++) {
    ASSERT_TRUE(EVP_DigestUpdate(ctx.get(), ptr, strlen(test->input)));
  }
  ASSERT_TRUE(EVP_DigestFinal_ex(ctx.get(), digest.get(), &digest_len));
  CompareDigest(test, digest.get(), digest_len);

  ASSERT_TRUE(EVP_DigestInit_ex(ctx.get(), test->md.func(), NULL));
  bssl::ScopedEVP_MD_CTX copy;
  ASSERT_TRUE(EVP_MD_CTX_copy_ex(copy.get(), ctx.get()));
  for (size_t i = 0; i < test->repeat; i++) {
    ASSERT_TRUE(EVP_DigestUpdate(copy.get(), test->input, strlen(test->input)));
  }
  ASSERT_TRUE(EVP_DigestFinal_ex(copy.get(), digest.get(), &digest_len));
  CompareDigest(test, digest.get(), digest_len);

  size_t half = strlen(test->input) / 2;
  ASSERT_TRUE(EVP_DigestUpdate(ctx.get(), test->input, half));
  ASSERT_TRUE(EVP_MD_CTX_copy_ex(copy.get(), ctx.get()));
  ASSERT_TRUE(EVP_DigestUpdate(copy.get(), test->input + half,
                               strlen(test->input) - half));
  for (size_t i = 1; i < test->repeat; i++) {
    ASSERT_TRUE(EVP_DigestUpdate(copy.get(), test->input, strlen(test->input)));
  }
  ASSERT_TRUE(EVP_DigestFinal_ex(copy.get(), digest.get(), &digest_len));
  CompareDigest(test, digest.get(), digest_len);

  if (test->md.one_shot_func && test->repeat == 1) {
    uint8_t *out = test->md.one_shot_func((const uint8_t *)test->input,
                                          strlen(test->input), digest.get());

    EXPECT_EQ(digest.get(), out);
    CompareDigest(test, digest.get(), EVP_MD_size(test->md.func()));
  }
}

TEST(DigestTest, TestVectors) {
  for (size_t i = 0; i < OPENSSL_ARRAY_SIZE(kTestVectors); i++) {
    SCOPED_TRACE(i);
    TestDigest(&kTestVectors[i]);
  }
}

TEST(DigestTest, Getters) {
  EXPECT_EQ(EVP_sha512(), EVP_get_digestbyname("RSA-SHA512"));
  EXPECT_EQ(EVP_sha512(), EVP_get_digestbyname("sha512WithRSAEncryption"));
  EXPECT_EQ(nullptr, EVP_get_digestbyname("nonsense"));
  EXPECT_EQ(EVP_sha512(), EVP_get_digestbyname("SHA512"));
  EXPECT_EQ(EVP_sha512(), EVP_get_digestbyname("sha512"));

  EXPECT_EQ(EVP_sha512(), EVP_get_digestbynid(NID_sha512));
  EXPECT_EQ(nullptr, EVP_get_digestbynid(NID_sha512WithRSAEncryption));
  EXPECT_EQ(nullptr, EVP_get_digestbynid(NID_undef));

  bssl::UniquePtr<ASN1_OBJECT> obj(OBJ_txt2obj("1.3.14.3.2.26", 0));
  ASSERT_TRUE(obj);
  EXPECT_EQ(EVP_sha1(), EVP_get_digestbyobj(obj.get()));
  EXPECT_EQ(EVP_md5_sha1(), EVP_get_digestbyobj(OBJ_nid2obj(NID_md5_sha1)));
  EXPECT_EQ(EVP_sha1(), EVP_get_digestbyobj(OBJ_nid2obj(NID_sha1)));
}

TEST(DigestTest, ASN1) {
  bssl::ScopedCBB cbb;
  ASSERT_TRUE(CBB_init(cbb.get(), 0));
  EXPECT_FALSE(EVP_marshal_digest_algorithm(cbb.get(), EVP_md5_sha1()));

  static const uint8_t kSHA256[] = {0x30, 0x0d, 0x06, 0x09, 0x60,
                                    0x86, 0x48, 0x01, 0x65, 0x03,
                                    0x04, 0x02, 0x01, 0x05, 0x00};
  static const uint8_t kSHA256NoParam[] = {0x30, 0x0b, 0x06, 0x09, 0x60,
                                           0x86, 0x48, 0x01, 0x65, 0x03,
                                           0x04, 0x02, 0x01};
  static const uint8_t kSHA256GarbageParam[] = {
      0x30, 0x0e, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01,
      0x65, 0x03, 0x04, 0x02, 0x01, 0x02, 0x01, 0x2a};

  cbb.Reset();
  ASSERT_TRUE(CBB_init(cbb.get(), 0));
  ASSERT_TRUE(EVP_marshal_digest_algorithm(cbb.get(), EVP_sha256()));
  uint8_t *der;
  size_t der_len;
  ASSERT_TRUE(CBB_finish(cbb.get(), &der, &der_len));
  bssl::UniquePtr<uint8_t> free_der(der);
  EXPECT_EQ(Bytes(kSHA256), Bytes(der, der_len));

  CBS cbs;
  CBS_init(&cbs, kSHA256, sizeof(kSHA256));
  EXPECT_EQ(EVP_sha256(), EVP_parse_digest_algorithm(&cbs));
  EXPECT_EQ(0u, CBS_len(&cbs));

  CBS_init(&cbs, kSHA256NoParam, sizeof(kSHA256NoParam));
  EXPECT_EQ(EVP_sha256(), EVP_parse_digest_algorithm(&cbs));
  EXPECT_EQ(0u, CBS_len(&cbs));

  CBS_init(&cbs, kSHA256GarbageParam, sizeof(kSHA256GarbageParam));
  EXPECT_FALSE(EVP_parse_digest_algorithm(&cbs));
}

TEST(DigestTest, TransformBlocks) {
  uint8_t blocks[SHA256_CBLOCK * 10];
  for (size_t i = 0; i < sizeof(blocks); i++) {
    blocks[i] = i*3;
  }

  SHA256_CTX ctx1;
  SHA256_Init(&ctx1);
  SHA256_Update(&ctx1, blocks, sizeof(blocks));

  SHA256_CTX ctx2;
  SHA256_Init(&ctx2);
  SHA256_TransformBlocks(ctx2.h, blocks, sizeof(blocks) / SHA256_CBLOCK);

  EXPECT_TRUE(0 == OPENSSL_memcmp(ctx1.h, ctx2.h, sizeof(ctx1.h)));
}
