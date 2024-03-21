/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_OPENSSL_STREAM_ADAPTER_H_
#define RTC_BASE_OPENSSL_STREAM_ADAPTER_H_

#include <openssl/ossl_typ.h>
#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "rtc_base/buffer.h"
#ifdef OPENSSL_IS_BORINGSSL
#include "rtc_base/boringssl_identity.h"
#else
#include "rtc_base/openssl_identity.h"
#endif
#include "api/task_queue/pending_task_safety_flag.h"
#include "rtc_base/ssl_identity.h"
#include "rtc_base/ssl_stream_adapter.h"
#include "rtc_base/stream.h"
#include "rtc_base/system/rtc_export.h"
#include "rtc_base/task_utils/repeating_task.h"

namespace rtc {

// starting point. It has similar structure and functionality, but uses a
// "peer-to-peer" mode, verifying the peer's certificate using a digest
// sent over a secure signaling channel.
//
// Static methods to initialize and deinit the SSL library are in
// OpenSSLAdapter. These should probably be moved out to a neutral class.
//
// In a few cases I have factored out some OpenSSLAdapter code into static
// methods so it can be reused from this class. Eventually that code should
// probably be moved to a common support class. Unfortunately there remain a
// few duplicated sections of code. I have not done more restructuring because
// I did not want to affect existing code that uses OpenSSLAdapter.
//
// This class does not support the SSL connection restart feature present in
// OpenSSLAdapter. I am not entirely sure how the feature is useful and I am
// not convinced that it works properly.
//
// This implementation is careful to disallow data exchange after an SSL error,
// and it has an explicit SSL_CLOSED state. It should not be possible to send
// any data in clear after one of the StartSSL methods has been called.


class SSLCertChain;


// allowed, overriding the default configuration.
// If `allow` has no value, any previous override is removed and the default
// configuration is restored.
RTC_EXPORT void SetAllowLegacyTLSProtocols(const absl::optional<bool>& allow);

class OpenSSLStreamAdapter final : public SSLStreamAdapter {
 public:
  explicit OpenSSLStreamAdapter(std::unique_ptr<StreamInterface> stream);
  ~OpenSSLStreamAdapter() override;

  void SetIdentity(std::unique_ptr<SSLIdentity> identity) override;
  SSLIdentity* GetIdentityForTesting() const override;

  void SetServerRole(SSLRole role = SSL_SERVER) override;
  bool SetPeerCertificateDigest(
      absl::string_view digest_alg,
      const unsigned char* digest_val,
      size_t digest_len,
      SSLPeerCertificateDigestError* error = nullptr) override;

  std::unique_ptr<SSLCertChain> GetPeerSSLCertChain() const override;


  int StartSSL() override;
  void SetMode(SSLMode mode) override;
  void SetMaxProtocolVersion(SSLProtocolVersion version) override;
  void SetInitialRetransmissionTimeout(int timeout_ms) override;

  StreamResult Read(void* data,
                    size_t data_len,
                    size_t* read,
                    int* error) override;
  StreamResult Write(const void* data,
                     size_t data_len,
                     size_t* written,
                     int* error) override;
  void Close() override;
  StreamState GetState() const override;

  static std::string SslCipherSuiteToName(int crypto_suite);

  bool GetSslCipherSuite(int* cipher) override;

  SSLProtocolVersion GetSslVersion() const override;
  bool GetSslVersionBytes(int* version) const override;

  bool ExportKeyingMaterial(absl::string_view label,
                            const uint8_t* context,
                            size_t context_len,
                            bool use_context,
                            uint8_t* result,
                            size_t result_len) override;

  bool SetDtlsSrtpCryptoSuites(const std::vector<int>& crypto_suites) override;
  bool GetDtlsSrtpCryptoSuite(int* crypto_suite) override;

  bool IsTlsConnected() override;

  static bool IsBoringSsl();

  static bool IsAcceptableCipher(int cipher, KeyType key_type);
  static bool IsAcceptableCipher(absl::string_view cipher, KeyType key_type);


  static void EnableTimeCallbackForTesting();

 private:
  enum SSLState {


    SSL_NONE,
    SSL_WAIT,        // waiting for the stream to open to start SSL negotiation
    SSL_CONNECTING,  // SSL negotiation in progress
    SSL_CONNECTED,   // SSL stream successfully established
    SSL_ERROR,       // some SSL error occurred, stream is closed
    SSL_CLOSED       // Clean close
  };

  void OnEvent(StreamInterface* stream, int events, int err);

  void PostEvent(int events, int err);
  void SetTimeout(int delay_ms);





  int BeginSSL();

  int ContinueSSL();









  void Error(absl::string_view context, int err, uint8_t alert, bool signal);
  void Cleanup(uint8_t alert);

  void FlushInput(unsigned int left);

  SSL_CTX* SetupSSLContext();

  bool VerifyPeerCertificate();

#ifdef OPENSSL_IS_BORINGSSL

  static enum ssl_verify_result_t SSLVerifyCallback(SSL* ssl,
                                                    uint8_t* out_alert);
#else


  static int SSLVerifyCallback(X509_STORE_CTX* store, void* arg);
#endif

  bool WaitingToVerifyPeerCertificate() const {
    return GetClientAuthEnabled() && !peer_certificate_verified_;
  }

  bool HasPeerCertificateDigest() const {
    return !peer_certificate_digest_algorithm_.empty() &&
           !peer_certificate_digest_value_.empty();
  }

  const std::unique_ptr<StreamInterface> stream_;

  rtc::Thread* const owner_;
  webrtc::ScopedTaskSafety task_safety_;
  webrtc::RepeatingTaskHandle timeout_task_;

  SSLState state_;
  SSLRole role_;
  int ssl_error_code_;  // valid when state_ == SSL_ERROR or SSL_CLOSED


  bool ssl_read_needs_write_;
  bool ssl_write_needs_read_;

  SSL* ssl_;
  SSL_CTX* ssl_ctx_;

#ifdef OPENSSL_IS_BORINGSSL
  std::unique_ptr<BoringSSLIdentity> identity_;
#else
  std::unique_ptr<OpenSSLIdentity> identity_;
#endif


  std::unique_ptr<SSLCertChain> peer_cert_chain_;
  bool peer_certificate_verified_ = false;

  Buffer peer_certificate_digest_value_;
  std::string peer_certificate_digest_algorithm_;

  std::string srtp_ciphers_;

  SSLMode ssl_mode_;

  SSLProtocolVersion ssl_max_version_;


  int dtls_handshake_timeout_ms_ = 50;

  const bool support_legacy_tls_protocols_flag_;
};


}  // namespace rtc

#endif  // RTC_BASE_OPENSSL_STREAM_ADAPTER_H_
