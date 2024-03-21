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

#include <algorithm>
#include <string>

#include <gtest/gtest.h>

#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/mem.h>

#include "../internal.h"
#include "../test/test_util.h"

#if !defined(OPENSSL_WINDOWS)
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#else
#include <io.h>
OPENSSL_MSVC_PRAGMA(warning(push, 3))
#include <winsock2.h>
#include <ws2tcpip.h>
OPENSSL_MSVC_PRAGMA(warning(pop))
#endif


#if !defined(OPENSSL_WINDOWS)
static int closesocket(int sock) { return close(sock); }
static std::string LastSocketError() { return strerror(errno); }
#else
static std::string LastSocketError() {
  char buf[DECIMAL_SIZE(int) + 1];
  BIO_snprintf(buf, sizeof(buf), "%d", WSAGetLastError());
  return buf;
}
#endif

class ScopedSocket {
 public:
  explicit ScopedSocket(int sock) : sock_(sock) {}
  ~ScopedSocket() {
    closesocket(sock_);
  }

 private:
  const int sock_;
};

TEST(BIOTest, SocketConnect) {
  static const char kTestMessage[] = "test";
  int listening_sock = -1;
  socklen_t len = 0;
  sockaddr_storage ss;
  struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) &ss;
  struct sockaddr_in *sin = (struct sockaddr_in *) &ss;
  OPENSSL_memset(&ss, 0, sizeof(ss));

  ss.ss_family = AF_INET6;
  listening_sock = socket(AF_INET6, SOCK_STREAM, 0);
  ASSERT_NE(-1, listening_sock) << LastSocketError();
  len = sizeof(*sin6);
  ASSERT_EQ(1, inet_pton(AF_INET6, "::1", &sin6->sin6_addr))
      << LastSocketError();
  if (bind(listening_sock, (struct sockaddr *)sin6, sizeof(*sin6)) == -1) {
    closesocket(listening_sock);

    ss.ss_family = AF_INET;
    listening_sock = socket(AF_INET, SOCK_STREAM, 0);
    ASSERT_NE(-1, listening_sock) << LastSocketError();
    len = sizeof(*sin);
    ASSERT_EQ(1, inet_pton(AF_INET, "127.0.0.1", &sin->sin_addr))
        << LastSocketError();
    ASSERT_EQ(0, bind(listening_sock, (struct sockaddr *)sin, sizeof(*sin)))
        << LastSocketError();
  }

  ScopedSocket listening_sock_closer(listening_sock);
  ASSERT_EQ(0, listen(listening_sock, 1)) << LastSocketError();
  ASSERT_EQ(0, getsockname(listening_sock, (struct sockaddr *)&ss, &len))
        << LastSocketError();

  char hostname[80];
  if (ss.ss_family == AF_INET6) {
    BIO_snprintf(hostname, sizeof(hostname), "[::1]:%d",
                 ntohs(sin6->sin6_port));
  } else if (ss.ss_family == AF_INET) {
    BIO_snprintf(hostname, sizeof(hostname), "127.0.0.1:%d",
                 ntohs(sin->sin_port));
  }

  bssl::UniquePtr<BIO> bio(BIO_new_connect(hostname));
  ASSERT_TRUE(bio);

  ASSERT_EQ(static_cast<int>(sizeof(kTestMessage)),
            BIO_write(bio.get(), kTestMessage, sizeof(kTestMessage)));

  int sock = accept(listening_sock, (struct sockaddr *) &ss, &len);
  ASSERT_NE(-1, sock) << LastSocketError();
  ScopedSocket sock_closer(sock);

  char buf[sizeof(kTestMessage)];
  ASSERT_EQ(static_cast<int>(sizeof(kTestMessage)),
            recv(sock, buf, sizeof(buf), 0))
      << LastSocketError();
  EXPECT_EQ(Bytes(kTestMessage, sizeof(kTestMessage)), Bytes(buf, sizeof(buf)));
}

TEST(BIOTest, Printf) {


  static const size_t kLengths[] = {5, 250, 251, 252, 253, 254, 1023};

  bssl::UniquePtr<BIO> bio(BIO_new(BIO_s_mem()));
  ASSERT_TRUE(bio);

  for (size_t length : kLengths) {
    SCOPED_TRACE(length);

    std::string in(length, 'a');

    int ret = BIO_printf(bio.get(), "test %s", in.c_str());
    ASSERT_GE(ret, 0);
    EXPECT_EQ(5 + length, static_cast<size_t>(ret));

    const uint8_t *contents;
    size_t len;
    ASSERT_TRUE(BIO_mem_contents(bio.get(), &contents, &len));
    EXPECT_EQ("test " + in,
              std::string(reinterpret_cast<const char *>(contents), len));

    ASSERT_TRUE(BIO_reset(bio.get()));
  }
}

static const size_t kLargeASN1PayloadLen = 8000;

struct ASN1TestParam {
  bool should_succeed;
  std::vector<uint8_t> input;

  size_t suffix_len;


  size_t expected_len;
  size_t max_len;
} kASN1TestParams[] = {
    {true, {0x30, 2, 1, 2, 0, 0}, 0, 4, 100},
    {false /* truncated */, {0x30, 3, 1, 2}, 0, 0, 100},
    {false /* should be short len */, {0x30, 0x81, 1, 1}, 0, 0, 100},
    {false /* zero padded */, {0x30, 0x82, 0, 1, 1}, 0, 0, 100},

    {true,
     {0x30, 0x82, kLargeASN1PayloadLen >> 8, kLargeASN1PayloadLen & 0xff},
     kLargeASN1PayloadLen,
     4 + kLargeASN1PayloadLen,
     kLargeASN1PayloadLen * 2},
    {false /* max_len too short */,
     {0x30, 0x82, kLargeASN1PayloadLen >> 8, kLargeASN1PayloadLen & 0xff},
     kLargeASN1PayloadLen,
     4 + kLargeASN1PayloadLen,
     3 + kLargeASN1PayloadLen},

    {true,
     {0x30, 0x80},
     kLargeASN1PayloadLen + 2,
     2 + kLargeASN1PayloadLen + 2,
     kLargeASN1PayloadLen * 2},
    {false /* max_len too short */,
     {0x30, 0x80},
     kLargeASN1PayloadLen + 2,
     2 + kLargeASN1PayloadLen + 2,
     2 + kLargeASN1PayloadLen + 1},
};

class BIOASN1Test : public testing::TestWithParam<ASN1TestParam> {};

TEST_P(BIOASN1Test, ReadASN1) {
  const ASN1TestParam& param = GetParam();
  std::vector<uint8_t> input = param.input;
  input.resize(input.size() + param.suffix_len, 0);

  bssl::UniquePtr<BIO> bio(BIO_new_mem_buf(input.data(), input.size()));
  ASSERT_TRUE(bio);

  uint8_t *out;
  size_t out_len;
  int ok = BIO_read_asn1(bio.get(), &out, &out_len, param.max_len);
  if (!ok) {
    out = nullptr;
  }
  bssl::UniquePtr<uint8_t> out_storage(out);

  ASSERT_EQ(param.should_succeed, (ok == 1));
  if (param.should_succeed) {
    EXPECT_EQ(Bytes(input.data(), param.expected_len), Bytes(out, out_len));
  }
}

INSTANTIATE_TEST_SUITE_P(, BIOASN1Test, testing::ValuesIn(kASN1TestParams));

class BIOPairTest : public testing::TestWithParam<bool> {};

TEST_P(BIOPairTest, TestPair) {
  BIO *bio1, *bio2;
  ASSERT_TRUE(BIO_new_bio_pair(&bio1, 10, &bio2, 10));
  bssl::UniquePtr<BIO> free_bio1(bio1), free_bio2(bio2);

  if (GetParam()) {
    std::swap(bio1, bio2);
  }

  EXPECT_EQ(10u, BIO_ctrl_get_write_guarantee(bio1));
  EXPECT_EQ(0u, BIO_ctrl_get_read_request(bio1));

  uint8_t buf[20];
  EXPECT_EQ(5, BIO_write(bio1, "12345", 5));
  EXPECT_EQ(5u, BIO_ctrl_get_write_guarantee(bio1));
  ASSERT_EQ(5, BIO_read(bio2, buf, sizeof(buf)));
  EXPECT_EQ(Bytes("12345"), Bytes(buf, 5));
  EXPECT_EQ(10u, BIO_ctrl_get_write_guarantee(bio1));

  EXPECT_EQ(10, BIO_write(bio1, "1234567890___", 13));
  EXPECT_EQ(0u, BIO_ctrl_get_write_guarantee(bio1));
  EXPECT_EQ(-1, BIO_write(bio1, "z", 1));
  EXPECT_TRUE(BIO_should_write(bio1));
  ASSERT_EQ(10, BIO_read(bio2, buf, sizeof(buf)));
  EXPECT_EQ(Bytes("1234567890"), Bytes(buf, 10));
  EXPECT_EQ(10u, BIO_ctrl_get_write_guarantee(bio1));

  EXPECT_EQ(-1, BIO_read(bio2, buf, 5));
  EXPECT_TRUE(BIO_should_read(bio2));
  EXPECT_EQ(5u, BIO_ctrl_get_read_request(bio1));

  EXPECT_EQ(-1, BIO_read(bio2, buf, 20));
  EXPECT_TRUE(BIO_should_read(bio2));
  EXPECT_EQ(10u, BIO_ctrl_get_read_request(bio1));

  EXPECT_EQ(5, BIO_write(bio1, "12345", 5));
  EXPECT_EQ(5u, BIO_ctrl_get_write_guarantee(bio1));
  EXPECT_EQ(5, BIO_write(bio1, "67890___", 8));
  EXPECT_EQ(0u, BIO_ctrl_get_write_guarantee(bio1));
  ASSERT_EQ(3, BIO_read(bio2, buf, 3));
  EXPECT_EQ(Bytes("123"), Bytes(buf, 3));
  EXPECT_EQ(3u, BIO_ctrl_get_write_guarantee(bio1));
  ASSERT_EQ(7, BIO_read(bio2, buf, sizeof(buf)));
  EXPECT_EQ(Bytes("4567890"), Bytes(buf, 7));
  EXPECT_EQ(10u, BIO_ctrl_get_write_guarantee(bio1));

  EXPECT_EQ(0u, BIO_ctrl_get_read_request(bio1));


  EXPECT_EQ(8, BIO_write(bio1, "abcdefgh", 8));
  EXPECT_EQ(2u, BIO_ctrl_get_write_guarantee(bio1));
  ASSERT_EQ(3, BIO_read(bio2, buf, 3));
  EXPECT_EQ(Bytes("abc"), Bytes(buf, 3));
  EXPECT_EQ(5u, BIO_ctrl_get_write_guarantee(bio1));
  EXPECT_EQ(5, BIO_write(bio1, "ijklm___", 8));
  EXPECT_EQ(0u, BIO_ctrl_get_write_guarantee(bio1));
  ASSERT_EQ(10, BIO_read(bio2, buf, sizeof(buf)));
  EXPECT_EQ(Bytes("defghijklm"), Bytes(buf, 10));
  EXPECT_EQ(10u, BIO_ctrl_get_write_guarantee(bio1));

  EXPECT_EQ(5, BIO_write(bio1, "12345", 5));
  EXPECT_EQ(5, BIO_write(bio2, "67890", 5));
  ASSERT_EQ(5, BIO_read(bio2, buf, sizeof(buf)));
  EXPECT_EQ(Bytes("12345"), Bytes(buf, 5));
  ASSERT_EQ(5, BIO_read(bio1, buf, sizeof(buf)));
  EXPECT_EQ(Bytes("67890"), Bytes(buf, 5));

  EXPECT_EQ(5, BIO_write(bio1, "12345", 5));
  EXPECT_TRUE(BIO_shutdown_wr(bio1));
  ASSERT_EQ(5, BIO_read(bio2, buf, sizeof(buf)));
  EXPECT_EQ(Bytes("12345"), Bytes(buf, 5));
  EXPECT_EQ(0, BIO_read(bio2, buf, sizeof(buf)));

  EXPECT_EQ(0u, BIO_ctrl_get_write_guarantee(bio1));
  EXPECT_EQ(-1, BIO_write(bio1, "_____", 5));

  uint32_t err = ERR_get_error();
  EXPECT_EQ(ERR_LIB_BIO, ERR_GET_LIB(err));
  EXPECT_EQ(BIO_R_BROKEN_PIPE, ERR_GET_REASON(err));

  EXPECT_EQ(5, BIO_write(bio2, "12345", 5));
  ASSERT_EQ(5, BIO_read(bio1, buf, sizeof(buf)));
  EXPECT_EQ(Bytes("12345"), Bytes(buf, 5));
}

INSTANTIATE_TEST_SUITE_P(, BIOPairTest, testing::Values(false, true));
