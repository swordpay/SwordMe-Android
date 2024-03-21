/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_MESSAGE_DIGEST_H_
#define RTC_BASE_MESSAGE_DIGEST_H_

#include <stddef.h>

#include <string>

#include "absl/strings/string_view.h"

namespace rtc {

extern const char DIGEST_MD5[];
extern const char DIGEST_SHA_1[];
extern const char DIGEST_SHA_224[];
extern const char DIGEST_SHA_256[];
extern const char DIGEST_SHA_384[];
extern const char DIGEST_SHA_512[];

class MessageDigest {
 public:
  enum { kMaxSize = 64 };  // Maximum known size (SHA-512)
  virtual ~MessageDigest() {}

  virtual size_t Size() const = 0;

  virtual void Update(const void* buf, size_t len) = 0;


  virtual size_t Finish(void* buf, size_t len) = 0;
};

class MessageDigestFactory {
 public:
  static MessageDigest* Create(absl::string_view alg);
};

// from RFC 4572 (FIPS 180).
bool IsFips180DigestAlgorithm(absl::string_view alg);


// implementation, and outputs the hash to the buffer `output`, which is
// `out_len` bytes long. Returns the number of bytes written to `output` if
// successful, or 0 if `out_len` was too small.
size_t ComputeDigest(MessageDigest* digest,
                     const void* input,
                     size_t in_len,
                     void* output,
                     size_t out_len);
// Like the previous function, but creates a digest implementation based on
// the desired digest name `alg`, e.g. DIGEST_SHA_1. Returns 0 if there is no
// digest with the given name.
size_t ComputeDigest(absl::string_view alg,
                     const void* input,
                     size_t in_len,
                     void* output,
                     size_t out_len);
// Computes the hash of `input` using the `digest` hash implementation, and
// returns it as a hex-encoded string.
std::string ComputeDigest(MessageDigest* digest, absl::string_view input);
// Like the previous function, but creates a digest implementation based on
// the desired digest name `alg`, e.g. DIGEST_SHA_1. Returns empty string if
// there is no digest with the given name.
std::string ComputeDigest(absl::string_view alg, absl::string_view input);
// Like the previous function, but returns an explicit result code.
bool ComputeDigest(absl::string_view alg,
                   absl::string_view input,
                   std::string* output);

inline std::string MD5(absl::string_view input) {
  return ComputeDigest(DIGEST_MD5, input);
}


// implementation and `key_len` bytes of `key` to key the HMAC, and outputs
// the HMAC to the buffer `output`, which is `out_len` bytes long. Returns the
// number of bytes written to `output` if successful, or 0 if `out_len` was too
// small.
size_t ComputeHmac(MessageDigest* digest,
                   const void* key,
                   size_t key_len,
                   const void* input,
                   size_t in_len,
                   void* output,
                   size_t out_len);
// Like the previous function, but creates a digest implementation based on
// the desired digest name `alg`, e.g. DIGEST_SHA_1. Returns 0 if there is no
// digest with the given name.
size_t ComputeHmac(absl::string_view alg,
                   const void* key,
                   size_t key_len,
                   const void* input,
                   size_t in_len,
                   void* output,
                   size_t out_len);
// Computes the HMAC of `input` using the `digest` hash implementation and `key`
// to key the HMAC, and returns it as a hex-encoded string.
std::string ComputeHmac(MessageDigest* digest,
                        absl::string_view key,
                        absl::string_view input);
// Like the previous function, but creates a digest implementation based on
// the desired digest name `alg`, e.g. DIGEST_SHA_1. Returns empty string if
// there is no digest with the given name.
std::string ComputeHmac(absl::string_view alg,
                        absl::string_view key,
                        absl::string_view input);
// Like the previous function, but returns an explicit result code.
bool ComputeHmac(absl::string_view alg,
                 absl::string_view key,
                 absl::string_view input,
                 std::string* output);

}  // namespace rtc

#endif  // RTC_BASE_MESSAGE_DIGEST_H_
