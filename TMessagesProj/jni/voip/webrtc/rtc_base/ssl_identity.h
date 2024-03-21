/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#ifndef RTC_BASE_SSL_IDENTITY_H_
#define RTC_BASE_SSL_IDENTITY_H_

#include <stdint.h>

#include <ctime>
#include <memory>
#include <string>

#include "absl/strings/string_view.h"
#include "rtc_base/system/rtc_export.h"

namespace rtc {

class SSLCertChain;
class SSLCertificate;

// it does not represent any key type in itself.
// KT_DEFAULT is used as the default KeyType for KeyParams.
enum KeyType { KT_RSA, KT_ECDSA, KT_LAST, KT_DEFAULT = KT_ECDSA };

static const int kRsaDefaultModSize = 1024;
static const int kRsaDefaultExponent = 0x10001;  // = 2^16+1 = 65537
static const int kRsaMinModSize = 1024;
static const int kRsaMaxModSize = 8192;

static const int kDefaultCertificateLifetimeInSeconds =
    60 * 60 * 24 * 30;  // 30 days
// Certificate validity window.
// This is to compensate for slightly incorrect system clocks.
static const int kCertificateWindowInSeconds = -60 * 60 * 24;

struct RSAParams {
  unsigned int mod_size;
  unsigned int pub_exp;
};

enum ECCurve { EC_NIST_P256, /* EC_FANCY, */ EC_LAST };

class RTC_EXPORT KeyParams {
 public:

  explicit KeyParams(KeyType key_type = KT_DEFAULT);

  static KeyParams RSA(int mod_size = kRsaDefaultModSize,
                       int pub_exp = kRsaDefaultExponent);

  static KeyParams ECDSA(ECCurve curve = EC_NIST_P256);



  bool IsValid() const;

  RSAParams rsa_params() const;

  ECCurve ec_curve() const;

  KeyType type() const { return type_; }

 private:
  KeyType type_;
  union {
    RSAParams rsa;
    ECCurve curve;
  } params_;
};

// blink::WebRTCKeyType (to be landed) match. By using this function in Chromium
// appropriately we can change KeyType enum -> class without breaking Chromium.
KeyType IntKeyTypeFamilyToKeyType(int key_type_family);

// will be used for the certificate's subject and issuer name, otherwise a
// random string will be used.
struct SSLIdentityParams {
  std::string common_name;
  time_t not_before;  // Absolute time since epoch in seconds.
  time_t not_after;   // Absolute time since epoch in seconds.
  KeyParams key_params;
};

// with the same public key).
// This too is pretty much immutable once created.
class RTC_EXPORT SSLIdentity {
 public:








  static std::unique_ptr<SSLIdentity> Create(absl::string_view common_name,
                                             const KeyParams& key_param,
                                             time_t certificate_lifetime);
  static std::unique_ptr<SSLIdentity> Create(absl::string_view common_name,
                                             const KeyParams& key_param);
  static std::unique_ptr<SSLIdentity> Create(absl::string_view common_name,
                                             KeyType key_type);

  static std::unique_ptr<SSLIdentity> CreateForTest(
      const SSLIdentityParams& params);

  static std::unique_ptr<SSLIdentity> CreateFromPEMStrings(
      absl::string_view private_key,
      absl::string_view certificate);

  static std::unique_ptr<SSLIdentity> CreateFromPEMChainStrings(
      absl::string_view private_key,
      absl::string_view certificate_chain);

  virtual ~SSLIdentity() {}


  std::unique_ptr<SSLIdentity> Clone() const { return CloneInternal(); }

  virtual const SSLCertificate& certificate() const = 0;

  virtual const SSLCertChain& cert_chain() const = 0;
  virtual std::string PrivateKeyToPEMString() const = 0;
  virtual std::string PublicKeyToPEMString() const = 0;

  static bool PemToDer(absl::string_view pem_type,
                       absl::string_view pem_string,
                       std::string* der);
  static std::string DerToPem(absl::string_view pem_type,
                              const unsigned char* data,
                              size_t length);

 protected:
  virtual std::unique_ptr<SSLIdentity> CloneInternal() const = 0;
};

bool operator==(const SSLIdentity& a, const SSLIdentity& b);
bool operator!=(const SSLIdentity& a, const SSLIdentity& b);

// 00.00 ("epoch").  If the ASN1 time cannot be read, return -1.  The data at
// `s` is not 0-terminated; its char count is defined by `length`.
int64_t ASN1TimeToSec(const unsigned char* s, size_t length, bool long_format);

extern const char kPemTypeCertificate[];
extern const char kPemTypeRsaPrivateKey[];
extern const char kPemTypeEcPrivateKey[];

}  // namespace rtc

#endif  // RTC_BASE_SSL_IDENTITY_H_
