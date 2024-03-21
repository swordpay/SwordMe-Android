/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "rtc_base/openssl_stream_adapter.h"

#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/tls1.h>
#include <openssl/x509v3.h>

#include "absl/strings/string_view.h"
#ifndef OPENSSL_IS_BORINGSSL
#include <openssl/dtls1.h>
#include <openssl/ssl.h>
#endif

#include <atomic>
#include <memory>
#include <utility>
#include <vector>

#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/numerics/safe_conversions.h"
#include "rtc_base/openssl.h"
#include "rtc_base/openssl_adapter.h"
#include "rtc_base/openssl_digest.h"
#ifdef OPENSSL_IS_BORINGSSL
#include "rtc_base/boringssl_identity.h"
#else
#include "rtc_base/openssl_identity.h"
#endif
#include "rtc_base/openssl_utility.h"
#include "rtc_base/ssl_certificate.h"
#include "rtc_base/stream.h"
#include "rtc_base/string_encode.h"
#include "rtc_base/thread.h"
#include "rtc_base/time_utils.h"
#include "system_wrappers/include/field_trial.h"

#if (OPENSSL_VERSION_NUMBER < 0x10100000L)
#error "webrtc requires at least OpenSSL version 1.1.0, to support DTLS-SRTP"
#endif

#define DEFINE_CIPHER_ENTRY_SSL3(name) \
  { SSL3_CK_##name, "TLS_" #name }
#define DEFINE_CIPHER_ENTRY_TLS1(name) \
  { TLS1_CK_##name, "TLS_" #name }

namespace rtc {
namespace {
using ::webrtc::SafeTask;
// SRTP cipher suite table. `internal_name` is used to construct a
// colon-separated profile strings which is needed by
// SSL_CTX_set_tlsext_use_srtp().
struct SrtpCipherMapEntry {
  const char* internal_name;
  const int id;
};

struct SslCipherMapEntry {
  uint32_t openssl_id;
  const char* rfc_name;
};

constexpr SrtpCipherMapEntry kSrtpCipherMap[] = {
    {"SRTP_AES128_CM_SHA1_80", kSrtpAes128CmSha1_80},
    {"SRTP_AES128_CM_SHA1_32", kSrtpAes128CmSha1_32},
    {"SRTP_AEAD_AES_128_GCM", kSrtpAeadAes128Gcm},
    {"SRTP_AEAD_AES_256_GCM", kSrtpAeadAes256Gcm}};

#ifndef OPENSSL_IS_BORINGSSL
// The "SSL_CIPHER_standard_name" function is only available in OpenSSL when
// compiled with tracing, so we need to define the mapping manually here.
constexpr SslCipherMapEntry kSslCipherMap[] = {

    DEFINE_CIPHER_ENTRY_SSL3(RSA_RC4_128_SHA),
    {SSL3_CK_RSA_DES_192_CBC3_SHA, "TLS_RSA_WITH_3DES_EDE_CBC_SHA"},

    {TLS1_CK_RSA_WITH_AES_128_SHA, "TLS_RSA_WITH_AES_128_CBC_SHA"},
    {TLS1_CK_DHE_RSA_WITH_AES_128_SHA, "TLS_DHE_RSA_WITH_AES_128_CBC_SHA"},
    {TLS1_CK_RSA_WITH_AES_256_SHA, "TLS_RSA_WITH_AES_256_CBC_SHA"},
    {TLS1_CK_DHE_RSA_WITH_AES_256_SHA, "TLS_DHE_RSA_WITH_AES_256_CBC_SHA"},

    DEFINE_CIPHER_ENTRY_TLS1(ECDHE_ECDSA_WITH_RC4_128_SHA),
    {TLS1_CK_ECDHE_ECDSA_WITH_DES_192_CBC3_SHA,
     "TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA"},
    DEFINE_CIPHER_ENTRY_TLS1(ECDHE_ECDSA_WITH_AES_128_CBC_SHA),
    DEFINE_CIPHER_ENTRY_TLS1(ECDHE_ECDSA_WITH_AES_256_CBC_SHA),

    DEFINE_CIPHER_ENTRY_TLS1(ECDHE_RSA_WITH_RC4_128_SHA),
    {TLS1_CK_ECDHE_RSA_WITH_DES_192_CBC3_SHA,
     "TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA"},
    DEFINE_CIPHER_ENTRY_TLS1(ECDHE_RSA_WITH_AES_128_CBC_SHA),
    DEFINE_CIPHER_ENTRY_TLS1(ECDHE_RSA_WITH_AES_256_CBC_SHA),

    {TLS1_CK_RSA_WITH_AES_128_SHA256, "TLS_RSA_WITH_AES_128_CBC_SHA256"},
    {TLS1_CK_RSA_WITH_AES_256_SHA256, "TLS_RSA_WITH_AES_256_CBC_SHA256"},
    {TLS1_CK_DHE_RSA_WITH_AES_128_SHA256,
     "TLS_DHE_RSA_WITH_AES_128_CBC_SHA256"},
    {TLS1_CK_DHE_RSA_WITH_AES_256_SHA256,
     "TLS_DHE_RSA_WITH_AES_256_CBC_SHA256"},

    DEFINE_CIPHER_ENTRY_TLS1(RSA_WITH_AES_128_GCM_SHA256),
    DEFINE_CIPHER_ENTRY_TLS1(RSA_WITH_AES_256_GCM_SHA384),
    DEFINE_CIPHER_ENTRY_TLS1(DHE_RSA_WITH_AES_128_GCM_SHA256),
    DEFINE_CIPHER_ENTRY_TLS1(DHE_RSA_WITH_AES_256_GCM_SHA384),
    DEFINE_CIPHER_ENTRY_TLS1(DH_RSA_WITH_AES_128_GCM_SHA256),
    DEFINE_CIPHER_ENTRY_TLS1(DH_RSA_WITH_AES_256_GCM_SHA384),

    {TLS1_CK_ECDHE_ECDSA_WITH_AES_128_SHA256,
     "TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256"},
    {TLS1_CK_ECDHE_ECDSA_WITH_AES_256_SHA384,
     "TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384"},
    {TLS1_CK_ECDHE_RSA_WITH_AES_128_SHA256,
     "TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256"},
    {TLS1_CK_ECDHE_RSA_WITH_AES_256_SHA384,
     "TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384"},

    DEFINE_CIPHER_ENTRY_TLS1(ECDHE_ECDSA_WITH_AES_128_GCM_SHA256),
    DEFINE_CIPHER_ENTRY_TLS1(ECDHE_ECDSA_WITH_AES_256_GCM_SHA384),
    DEFINE_CIPHER_ENTRY_TLS1(ECDHE_RSA_WITH_AES_128_GCM_SHA256),
    DEFINE_CIPHER_ENTRY_TLS1(ECDHE_RSA_WITH_AES_256_GCM_SHA384),

    {0, nullptr}};
#endif  // #ifndef OPENSSL_IS_BORINGSSL

#ifdef OPENSSL_IS_BORINGSSL
// Enabled by EnableTimeCallbackForTesting. Should never be set in production
// code.
bool g_use_time_callback_for_testing = false;
// Not used in production code. Actual time should be relative to Jan 1, 1970.
void TimeCallbackForTesting(const SSL* ssl, struct timeval* out_clock) {
  int64_t time = TimeNanos();
  out_clock->tv_sec = time / kNumNanosecsPerSec;
  out_clock->tv_usec = (time % kNumNanosecsPerSec) / kNumNanosecsPerMicrosec;
}
#endif

}  // namespace

// StreamBIO
//////////////////////////////////////////////////////////////////////

static int stream_write(BIO* h, const char* buf, int num);
static int stream_read(BIO* h, char* buf, int size);
static int stream_puts(BIO* h, const char* str);
static long stream_ctrl(BIO* h, int cmd, long arg1, void* arg2);
static int stream_new(BIO* h);
static int stream_free(BIO* data);

static BIO_METHOD* BIO_stream_method() {
  static BIO_METHOD* method = [] {
    BIO_METHOD* method = BIO_meth_new(BIO_TYPE_BIO, "stream");
    BIO_meth_set_write(method, stream_write);
    BIO_meth_set_read(method, stream_read);
    BIO_meth_set_puts(method, stream_puts);
    BIO_meth_set_ctrl(method, stream_ctrl);
    BIO_meth_set_create(method, stream_new);
    BIO_meth_set_destroy(method, stream_free);
    return method;
  }();
  return method;
}

static BIO* BIO_new_stream(StreamInterface* stream) {
  BIO* ret = BIO_new(BIO_stream_method());
  if (ret == nullptr) {
    return nullptr;
  }
  BIO_set_data(ret, stream);
  return ret;
}


static int stream_new(BIO* b) {
  BIO_set_shutdown(b, 0);
  BIO_set_init(b, 1);
  BIO_set_data(b, 0);
  return 1;
}

static int stream_free(BIO* b) {
  if (b == nullptr) {
    return 0;
  }
  return 1;
}

static int stream_read(BIO* b, char* out, int outl) {
  if (!out) {
    return -1;
  }
  StreamInterface* stream = static_cast<StreamInterface*>(BIO_get_data(b));
  BIO_clear_retry_flags(b);
  size_t read;
  int error;
  StreamResult result = stream->Read(out, outl, &read, &error);
  if (result == SR_SUCCESS) {
    return checked_cast<int>(read);
  } else if (result == SR_BLOCK) {
    BIO_set_retry_read(b);
  }
  return -1;
}

static int stream_write(BIO* b, const char* in, int inl) {
  if (!in) {
    return -1;
  }
  StreamInterface* stream = static_cast<StreamInterface*>(BIO_get_data(b));
  BIO_clear_retry_flags(b);
  size_t written;
  int error;
  StreamResult result = stream->Write(in, inl, &written, &error);
  if (result == SR_SUCCESS) {
    return checked_cast<int>(written);
  } else if (result == SR_BLOCK) {
    BIO_set_retry_write(b);
  }
  return -1;
}

static int stream_puts(BIO* b, const char* str) {
  return stream_write(b, str, checked_cast<int>(strlen(str)));
}

static long stream_ctrl(BIO* b, int cmd, long num, void* ptr) {
  switch (cmd) {
    case BIO_CTRL_RESET:
      return 0;
    case BIO_CTRL_EOF: {
      StreamInterface* stream = static_cast<StreamInterface*>(ptr);

      return (stream->GetState() == SS_CLOSED) ? 1 : 0;
    }
    case BIO_CTRL_WPENDING:
    case BIO_CTRL_PENDING:
      return 0;
    case BIO_CTRL_FLUSH:
      return 1;
    case BIO_CTRL_DGRAM_QUERY_MTU:




      return 1200;
    default:
      return 0;
  }
}

// OpenSSLStreamAdapter
/////////////////////////////////////////////////////////////////////////////

static std::atomic<bool> g_use_legacy_tls_protocols_override(false);
static std::atomic<bool> g_allow_legacy_tls_protocols(false);

void SetAllowLegacyTLSProtocols(const absl::optional<bool>& allow) {
  g_use_legacy_tls_protocols_override.store(allow.has_value());
  if (allow.has_value())
    g_allow_legacy_tls_protocols.store(allow.value());
}

bool ShouldAllowLegacyTLSProtocols() {
  return g_use_legacy_tls_protocols_override.load()
             ? g_allow_legacy_tls_protocols.load()
             : webrtc::field_trial::IsEnabled("WebRTC-LegacyTlsProtocols");
}

OpenSSLStreamAdapter::OpenSSLStreamAdapter(
    std::unique_ptr<StreamInterface> stream)
    : stream_(std::move(stream)),
      owner_(rtc::Thread::Current()),
      state_(SSL_NONE),
      role_(SSL_CLIENT),
      ssl_read_needs_write_(false),
      ssl_write_needs_read_(false),
      ssl_(nullptr),
      ssl_ctx_(nullptr),
      ssl_mode_(SSL_MODE_TLS),
      ssl_max_version_(SSL_PROTOCOL_TLS_12),


      support_legacy_tls_protocols_flag_(ShouldAllowLegacyTLSProtocols()) {
  stream_->SignalEvent.connect(this, &OpenSSLStreamAdapter::OnEvent);
}

OpenSSLStreamAdapter::~OpenSSLStreamAdapter() {
  timeout_task_.Stop();
  Cleanup(0);
}

void OpenSSLStreamAdapter::SetIdentity(std::unique_ptr<SSLIdentity> identity) {
  RTC_DCHECK(!identity_);
#ifdef OPENSSL_IS_BORINGSSL
  identity_.reset(static_cast<BoringSSLIdentity*>(identity.release()));
#else
  identity_.reset(static_cast<OpenSSLIdentity*>(identity.release()));
#endif
}

SSLIdentity* OpenSSLStreamAdapter::GetIdentityForTesting() const {
  return identity_.get();
}

void OpenSSLStreamAdapter::SetServerRole(SSLRole role) {
  role_ = role;
}

bool OpenSSLStreamAdapter::SetPeerCertificateDigest(
    absl::string_view digest_alg,
    const unsigned char* digest_val,
    size_t digest_len,
    SSLPeerCertificateDigestError* error) {
  RTC_DCHECK(!peer_certificate_verified_);
  RTC_DCHECK(!HasPeerCertificateDigest());
  size_t expected_len;
  if (error) {
    *error = SSLPeerCertificateDigestError::NONE;
  }

  if (!OpenSSLDigest::GetDigestSize(digest_alg, &expected_len)) {
    RTC_LOG(LS_WARNING) << "Unknown digest algorithm: " << digest_alg;
    if (error) {
      *error = SSLPeerCertificateDigestError::UNKNOWN_ALGORITHM;
    }
    return false;
  }
  if (expected_len != digest_len) {
    if (error) {
      *error = SSLPeerCertificateDigestError::INVALID_LENGTH;
    }
    return false;
  }

  peer_certificate_digest_value_.SetData(digest_val, digest_len);
  peer_certificate_digest_algorithm_ = std::string(digest_alg);

  if (!peer_cert_chain_) {


    return true;
  }

  if (!VerifyPeerCertificate()) {
    Error("SetPeerCertificateDigest", -1, SSL_AD_BAD_CERTIFICATE, false);
    if (error) {
      *error = SSLPeerCertificateDigestError::VERIFICATION_FAILED;
    }
    return false;
  }

  if (state_ == SSL_CONNECTED) {



    PostEvent(SE_OPEN | SE_READ | SE_WRITE, 0);
  }

  return true;
}

std::string OpenSSLStreamAdapter::SslCipherSuiteToName(int cipher_suite) {
#ifdef OPENSSL_IS_BORINGSSL
  const SSL_CIPHER* ssl_cipher = SSL_get_cipher_by_value(cipher_suite);
  if (!ssl_cipher) {
    return std::string();
  }
  return SSL_CIPHER_standard_name(ssl_cipher);
#else
  const int openssl_cipher_id = 0x03000000L | cipher_suite;
  for (const SslCipherMapEntry* entry = kSslCipherMap; entry->rfc_name;
       ++entry) {
    if (openssl_cipher_id == static_cast<int>(entry->openssl_id)) {
      return entry->rfc_name;
    }
  }
  return std::string();
#endif
}

bool OpenSSLStreamAdapter::GetSslCipherSuite(int* cipher_suite) {
  if (state_ != SSL_CONNECTED) {
    return false;
  }

  const SSL_CIPHER* current_cipher = SSL_get_current_cipher(ssl_);
  if (current_cipher == nullptr) {
    return false;
  }

  *cipher_suite = static_cast<uint16_t>(SSL_CIPHER_get_id(current_cipher));
  return true;
}

SSLProtocolVersion OpenSSLStreamAdapter::GetSslVersion() const {
  if (state_ != SSL_CONNECTED) {
    return SSL_PROTOCOL_NOT_GIVEN;
  }

  int ssl_version = SSL_version(ssl_);
  if (ssl_mode_ == SSL_MODE_DTLS) {
    if (ssl_version == DTLS1_VERSION) {
      return SSL_PROTOCOL_DTLS_10;
    } else if (ssl_version == DTLS1_2_VERSION) {
      return SSL_PROTOCOL_DTLS_12;
    }
  } else {
    if (ssl_version == TLS1_VERSION) {
      return SSL_PROTOCOL_TLS_10;
    } else if (ssl_version == TLS1_1_VERSION) {
      return SSL_PROTOCOL_TLS_11;
    } else if (ssl_version == TLS1_2_VERSION) {
      return SSL_PROTOCOL_TLS_12;
    }
  }

  return SSL_PROTOCOL_NOT_GIVEN;
}

bool OpenSSLStreamAdapter::GetSslVersionBytes(int* version) const {
  if (state_ != SSL_CONNECTED) {
    return false;
  }
  *version = SSL_version(ssl_);
  return true;
}

bool OpenSSLStreamAdapter::ExportKeyingMaterial(absl::string_view label,
                                                const uint8_t* context,
                                                size_t context_len,
                                                bool use_context,
                                                uint8_t* result,
                                                size_t result_len) {
  if (SSL_export_keying_material(ssl_, result, result_len, label.data(),
                                 label.length(), context, context_len,
                                 use_context) != 1) {
    return false;
  }
  return true;
}

bool OpenSSLStreamAdapter::SetDtlsSrtpCryptoSuites(
    const std::vector<int>& ciphers) {
  if (state_ != SSL_NONE) {
    return false;
  }

  std::string internal_ciphers;
  for (const int cipher : ciphers) {
    bool found = false;
    for (const auto& entry : kSrtpCipherMap) {
      if (cipher == entry.id) {
        found = true;
        if (!internal_ciphers.empty()) {
          internal_ciphers += ":";
        }
        internal_ciphers += entry.internal_name;
        break;
      }
    }

    if (!found) {
      RTC_LOG(LS_ERROR) << "Could not find cipher: " << cipher;
      return false;
    }
  }

  if (internal_ciphers.empty()) {
    return false;
  }

  srtp_ciphers_ = internal_ciphers;
  return true;
}

bool OpenSSLStreamAdapter::GetDtlsSrtpCryptoSuite(int* crypto_suite) {
  RTC_DCHECK(state_ == SSL_CONNECTED);
  if (state_ != SSL_CONNECTED) {
    return false;
  }

  const SRTP_PROTECTION_PROFILE* srtp_profile =
      SSL_get_selected_srtp_profile(ssl_);

  if (!srtp_profile) {
    return false;
  }

  *crypto_suite = srtp_profile->id;
  RTC_DCHECK(!SrtpCryptoSuiteToName(*crypto_suite).empty());
  return true;
}

bool OpenSSLStreamAdapter::IsTlsConnected() {
  return state_ == SSL_CONNECTED;
}

int OpenSSLStreamAdapter::StartSSL() {

  if (state_ != SSL_NONE) {
    return -1;
  }

  if (stream_->GetState() != SS_OPEN) {
    state_ = SSL_WAIT;
    return 0;
  }

  state_ = SSL_CONNECTING;
  if (int err = BeginSSL()) {
    Error("BeginSSL", err, 0, false);
    return err;
  }

  return 0;
}

void OpenSSLStreamAdapter::SetMode(SSLMode mode) {
  RTC_DCHECK(state_ == SSL_NONE);
  ssl_mode_ = mode;
}

void OpenSSLStreamAdapter::SetMaxProtocolVersion(SSLProtocolVersion version) {
  RTC_DCHECK(ssl_ctx_ == nullptr);
  ssl_max_version_ = version;
}

void OpenSSLStreamAdapter::SetInitialRetransmissionTimeout(int timeout_ms) {
  RTC_DCHECK(ssl_ctx_ == nullptr);
  dtls_handshake_timeout_ms_ = timeout_ms;
}

// StreamInterface Implementation
//

StreamResult OpenSSLStreamAdapter::Write(const void* data,
                                         size_t data_len,
                                         size_t* written,
                                         int* error) {
  RTC_DLOG(LS_VERBOSE) << "OpenSSLStreamAdapter::Write(" << data_len << ")";

  switch (state_) {
    case SSL_NONE:

      return stream_->Write(data, data_len, written, error);

    case SSL_WAIT:
    case SSL_CONNECTING:
      return SR_BLOCK;

    case SSL_CONNECTED:
      if (WaitingToVerifyPeerCertificate()) {
        return SR_BLOCK;
      }
      break;

    case SSL_ERROR:
    case SSL_CLOSED:
    default:
      if (error) {
        *error = ssl_error_code_;
      }
      return SR_ERROR;
  }

  if (data_len == 0) {
    if (written) {
      *written = 0;
    }
    return SR_SUCCESS;
  }

  ssl_write_needs_read_ = false;

  int code = SSL_write(ssl_, data, checked_cast<int>(data_len));
  int ssl_error = SSL_get_error(ssl_, code);
  switch (ssl_error) {
    case SSL_ERROR_NONE:
      RTC_DLOG(LS_VERBOSE) << " -- success";
      RTC_DCHECK_GT(code, 0);
      RTC_DCHECK_LE(code, data_len);
      if (written)
        *written = code;
      return SR_SUCCESS;
    case SSL_ERROR_WANT_READ:
      RTC_DLOG(LS_VERBOSE) << " -- error want read";
      ssl_write_needs_read_ = true;
      return SR_BLOCK;
    case SSL_ERROR_WANT_WRITE:
      RTC_DLOG(LS_VERBOSE) << " -- error want write";
      return SR_BLOCK;

    case SSL_ERROR_ZERO_RETURN:
    default:
      Error("SSL_write", (ssl_error ? ssl_error : -1), 0, false);
      if (error) {
        *error = ssl_error_code_;
      }
      return SR_ERROR;
  }

}

StreamResult OpenSSLStreamAdapter::Read(void* data,
                                        size_t data_len,
                                        size_t* read,
                                        int* error) {
  RTC_DLOG(LS_VERBOSE) << "OpenSSLStreamAdapter::Read(" << data_len << ")";
  switch (state_) {
    case SSL_NONE:

      return stream_->Read(data, data_len, read, error);
    case SSL_WAIT:
    case SSL_CONNECTING:
      return SR_BLOCK;
    case SSL_CONNECTED:
      if (WaitingToVerifyPeerCertificate()) {
        return SR_BLOCK;
      }
      break;
    case SSL_CLOSED:
      return SR_EOS;
    case SSL_ERROR:
    default:
      if (error) {
        *error = ssl_error_code_;
      }
      return SR_ERROR;
  }

  if (data_len == 0) {
    if (read) {
      *read = 0;
    }
    return SR_SUCCESS;
  }

  ssl_read_needs_write_ = false;

  const int code = SSL_read(ssl_, data, checked_cast<int>(data_len));
  const int ssl_error = SSL_get_error(ssl_, code);

  switch (ssl_error) {
    case SSL_ERROR_NONE:
      RTC_DLOG(LS_VERBOSE) << " -- success";
      RTC_DCHECK_GT(code, 0);
      RTC_DCHECK_LE(code, data_len);
      if (read) {
        *read = code;
      }

      if (ssl_mode_ == SSL_MODE_DTLS) {

        unsigned int pending = SSL_pending(ssl_);

        if (pending) {
          RTC_DLOG(LS_INFO) << " -- short DTLS read. flushing";
          FlushInput(pending);
          if (error) {
            *error = SSE_MSG_TRUNC;
          }
          return SR_ERROR;
        }
      }
      return SR_SUCCESS;
    case SSL_ERROR_WANT_READ:
      RTC_DLOG(LS_VERBOSE) << " -- error want read";
      return SR_BLOCK;
    case SSL_ERROR_WANT_WRITE:
      RTC_DLOG(LS_VERBOSE) << " -- error want write";
      ssl_read_needs_write_ = true;
      return SR_BLOCK;
    case SSL_ERROR_ZERO_RETURN:
      RTC_DLOG(LS_VERBOSE) << " -- remote side closed";
      Close();
      return SR_EOS;
    default:
      Error("SSL_read", (ssl_error ? ssl_error : -1), 0, false);
      if (error) {
        *error = ssl_error_code_;
      }
      return SR_ERROR;
  }

}

void OpenSSLStreamAdapter::FlushInput(unsigned int left) {
  unsigned char buf[2048];

  while (left) {

    const int toread = (sizeof(buf) < left) ? sizeof(buf) : left;
    const int code = SSL_read(ssl_, buf, toread);

    const int ssl_error = SSL_get_error(ssl_, code);
    RTC_DCHECK(ssl_error == SSL_ERROR_NONE);

    if (ssl_error != SSL_ERROR_NONE) {
      RTC_DLOG(LS_VERBOSE) << " -- error " << code;
      Error("SSL_read", (ssl_error ? ssl_error : -1), 0, false);
      return;
    }

    RTC_DLOG(LS_VERBOSE) << " -- flushed " << code << " bytes";
    left -= code;
  }
}

void OpenSSLStreamAdapter::Close() {
  Cleanup(0);
  RTC_DCHECK(state_ == SSL_CLOSED || state_ == SSL_ERROR);



  stream_->Close();
}

StreamState OpenSSLStreamAdapter::GetState() const {
  switch (state_) {
    case SSL_WAIT:
    case SSL_CONNECTING:
      return SS_OPENING;
    case SSL_CONNECTED:
      if (WaitingToVerifyPeerCertificate()) {
        return SS_OPENING;
      }
      return SS_OPEN;
    default:
      return SS_CLOSED;
  }

}

void OpenSSLStreamAdapter::OnEvent(StreamInterface* stream,
                                   int events,
                                   int err) {
  int events_to_signal = 0;
  int signal_error = 0;
  RTC_DCHECK(stream == stream_.get());

  if ((events & SE_OPEN)) {
    RTC_DLOG(LS_VERBOSE) << "OpenSSLStreamAdapter::OnEvent SE_OPEN";
    if (state_ != SSL_WAIT) {
      RTC_DCHECK(state_ == SSL_NONE);
      events_to_signal |= SE_OPEN;
    } else {
      state_ = SSL_CONNECTING;
      if (int err = BeginSSL()) {
        Error("BeginSSL", err, 0, true);
        return;
      }
    }
  }

  if ((events & (SE_READ | SE_WRITE))) {
    RTC_DLOG(LS_VERBOSE) << "OpenSSLStreamAdapter::OnEvent"
                         << ((events & SE_READ) ? " SE_READ" : "")
                         << ((events & SE_WRITE) ? " SE_WRITE" : "");
    if (state_ == SSL_NONE) {
      events_to_signal |= events & (SE_READ | SE_WRITE);
    } else if (state_ == SSL_CONNECTING) {
      if (int err = ContinueSSL()) {
        Error("ContinueSSL", err, 0, true);
        return;
      }
    } else if (state_ == SSL_CONNECTED) {
      if (((events & SE_READ) && ssl_write_needs_read_) ||
          (events & SE_WRITE)) {
        RTC_DLOG(LS_VERBOSE) << " -- onStreamWriteable";
        events_to_signal |= SE_WRITE;
      }
      if (((events & SE_WRITE) && ssl_read_needs_write_) ||
          (events & SE_READ)) {
        RTC_DLOG(LS_VERBOSE) << " -- onStreamReadable";
        events_to_signal |= SE_READ;
      }
    }
  }

  if ((events & SE_CLOSE)) {
    RTC_DLOG(LS_VERBOSE) << "OpenSSLStreamAdapter::OnEvent(SE_CLOSE, " << err
                         << ")";
    Cleanup(0);
    events_to_signal |= SE_CLOSE;

    RTC_DCHECK(signal_error == 0);
    signal_error = err;
  }

  if (events_to_signal) {


    SignalEvent(this, events_to_signal, signal_error);
  }
}

void OpenSSLStreamAdapter::PostEvent(int events, int err) {
  owner_->PostTask(SafeTask(task_safety_.flag(), [this, events, err]() {
    SignalEvent(this, events, err);
  }));
}

void OpenSSLStreamAdapter::SetTimeout(int delay_ms) {


  RTC_DCHECK_GE(delay_ms, 0);
  RTC_DCHECK(!timeout_task_.Running());

  timeout_task_ = webrtc::RepeatingTaskHandle::DelayedStart(
      owner_, webrtc::TimeDelta::Millis(delay_ms),
      [flag = task_safety_.flag(), this]() {
        if (flag->alive()) {
          RTC_DLOG(LS_INFO) << "DTLS timeout expired";
          timeout_task_.Stop();
          int res = DTLSv1_handle_timeout(ssl_);
          if (res > 0) {
            RTC_LOG(LS_INFO) << "DTLS retransmission";
          } else if (res < 0) {
            RTC_LOG(LS_INFO) << "DTLSv1_handle_timeout() return -1";
            Error("DTLSv1_handle_timeout", res, -1, true);
            return webrtc::TimeDelta::PlusInfinity();
          }
          ContinueSSL();
        } else {
          RTC_DCHECK_NOTREACHED();
        }

        return webrtc::TimeDelta::PlusInfinity();
      });
}

int OpenSSLStreamAdapter::BeginSSL() {
  RTC_DCHECK(state_ == SSL_CONNECTING);

  RTC_DLOG(LS_INFO) << "BeginSSL with peer.";

  BIO* bio = nullptr;

  RTC_DCHECK(ssl_ctx_ == nullptr);
  ssl_ctx_ = SetupSSLContext();
  if (!ssl_ctx_) {
    return -1;
  }

  bio = BIO_new_stream(stream_.get());
  if (!bio) {
    return -1;
  }

  ssl_ = SSL_new(ssl_ctx_);
  if (!ssl_) {
    BIO_free(bio);
    return -1;
  }

  SSL_set_app_data(ssl_, this);

  SSL_set_bio(ssl_, bio, bio);  // the SSL object owns the bio now.
  if (ssl_mode_ == SSL_MODE_DTLS) {
#ifdef OPENSSL_IS_BORINGSSL
    DTLSv1_set_initial_timeout_duration(ssl_, dtls_handshake_timeout_ms_);
#else


    SSL_set_read_ahead(ssl_, 1);
#endif
  }

  SSL_set_mode(ssl_, SSL_MODE_ENABLE_PARTIAL_WRITE |
                         SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

  return ContinueSSL();
}

int OpenSSLStreamAdapter::ContinueSSL() {
  RTC_DLOG(LS_VERBOSE) << "ContinueSSL";
  RTC_DCHECK(state_ == SSL_CONNECTING);

  timeout_task_.Stop();

  const int code = (role_ == SSL_CLIENT) ? SSL_connect(ssl_) : SSL_accept(ssl_);
  const int ssl_error = SSL_get_error(ssl_, code);

  switch (ssl_error) {
    case SSL_ERROR_NONE:
      RTC_DLOG(LS_VERBOSE) << " -- success";


      RTC_DCHECK(peer_cert_chain_ || !GetClientAuthEnabled());

      state_ = SSL_CONNECTED;
      if (!WaitingToVerifyPeerCertificate()) {








        SignalEvent(this, SE_OPEN | SE_READ | SE_WRITE, 0);
      }
      break;

    case SSL_ERROR_WANT_READ: {
      RTC_DLOG(LS_VERBOSE) << " -- error want read";
      struct timeval timeout;
      if (DTLSv1_get_timeout(ssl_, &timeout)) {
        int delay = timeout.tv_sec * 1000 + timeout.tv_usec / 1000;
        SetTimeout(delay);
      }
    } break;

    case SSL_ERROR_WANT_WRITE:
      RTC_DLOG(LS_VERBOSE) << " -- error want write";
      break;

    case SSL_ERROR_ZERO_RETURN:
    default:
      SSLHandshakeError ssl_handshake_err = SSLHandshakeError::UNKNOWN;
      int err_code = ERR_peek_last_error();
      if (err_code != 0 && ERR_GET_REASON(err_code) == SSL_R_NO_SHARED_CIPHER) {
        ssl_handshake_err = SSLHandshakeError::INCOMPATIBLE_CIPHERSUITE;
      }
      RTC_DLOG(LS_VERBOSE) << " -- error " << code << ", " << err_code << ", "
                           << ERR_GET_REASON(err_code);
      SignalSSLHandshakeError(ssl_handshake_err);
      return (ssl_error != 0) ? ssl_error : -1;
  }

  return 0;
}

void OpenSSLStreamAdapter::Error(absl::string_view context,
                                 int err,
                                 uint8_t alert,
                                 bool signal) {
  RTC_LOG(LS_WARNING) << "OpenSSLStreamAdapter::Error(" << context << ", "
                      << err << ", " << static_cast<int>(alert) << ")";
  state_ = SSL_ERROR;
  ssl_error_code_ = err;
  Cleanup(alert);
  if (signal) {
    SignalEvent(this, SE_CLOSE, err);
  }
}

void OpenSSLStreamAdapter::Cleanup(uint8_t alert) {
  RTC_DLOG(LS_INFO) << "Cleanup";

  if (state_ != SSL_ERROR) {
    state_ = SSL_CLOSED;
    ssl_error_code_ = 0;
  }

  if (ssl_) {
    int ret;
// SSL_send_fatal_alert is only available in BoringSSL.
#ifdef OPENSSL_IS_BORINGSSL
    if (alert) {
      ret = SSL_send_fatal_alert(ssl_, alert);
      if (ret < 0) {
        RTC_LOG(LS_WARNING) << "SSL_send_fatal_alert failed, error = "
                            << SSL_get_error(ssl_, ret);
      }
    } else {
#endif
      ret = SSL_shutdown(ssl_);
      if (ret < 0) {
        RTC_LOG(LS_WARNING)
            << "SSL_shutdown failed, error = " << SSL_get_error(ssl_, ret);
      }
#ifdef OPENSSL_IS_BORINGSSL
    }
#endif
    SSL_free(ssl_);
    ssl_ = nullptr;
  }
  if (ssl_ctx_) {
    SSL_CTX_free(ssl_ctx_);
    ssl_ctx_ = nullptr;
  }
  identity_.reset();
  peer_cert_chain_.reset();

  timeout_task_.Stop();
}

SSL_CTX* OpenSSLStreamAdapter::SetupSSLContext() {
#ifdef OPENSSL_IS_BORINGSSL


  SSL_CTX* ctx =
      SSL_CTX_new(ssl_mode_ == SSL_MODE_DTLS ? DTLS_with_buffers_method()
                                             : TLS_with_buffers_method());
#else
  SSL_CTX* ctx =
      SSL_CTX_new(ssl_mode_ == SSL_MODE_DTLS ? DTLS_method() : TLS_method());
#endif
  if (ctx == nullptr) {
    return nullptr;
  }

  if (support_legacy_tls_protocols_flag_) {


    SSL_CTX_set_min_proto_version(
        ctx, ssl_mode_ == SSL_MODE_DTLS ? DTLS1_VERSION : TLS1_VERSION);
    switch (ssl_max_version_) {
      case SSL_PROTOCOL_TLS_10:
        SSL_CTX_set_max_proto_version(
            ctx, ssl_mode_ == SSL_MODE_DTLS ? DTLS1_VERSION : TLS1_VERSION);
        break;
      case SSL_PROTOCOL_TLS_11:
        SSL_CTX_set_max_proto_version(
            ctx, ssl_mode_ == SSL_MODE_DTLS ? DTLS1_VERSION : TLS1_1_VERSION);
        break;
      case SSL_PROTOCOL_TLS_12:
      default:
        SSL_CTX_set_max_proto_version(
            ctx, ssl_mode_ == SSL_MODE_DTLS ? DTLS1_2_VERSION : TLS1_2_VERSION);
        break;
    }
  } else {

    SSL_CTX_set_min_proto_version(
        ctx, ssl_mode_ == SSL_MODE_DTLS ? DTLS1_2_VERSION : TLS1_2_VERSION);
    SSL_CTX_set_max_proto_version(
        ctx, ssl_mode_ == SSL_MODE_DTLS ? DTLS1_2_VERSION : TLS1_2_VERSION);
  }

#ifdef OPENSSL_IS_BORINGSSL

  if (g_use_time_callback_for_testing) {
    SSL_CTX_set_current_time_cb(ctx, &TimeCallbackForTesting);
  }
  SSL_CTX_set0_buffer_pool(ctx, openssl::GetBufferPool());
#endif

  if (identity_ && !identity_->ConfigureIdentity(ctx)) {
    SSL_CTX_free(ctx);
    return nullptr;
  }

#if !defined(NDEBUG)
  SSL_CTX_set_info_callback(ctx, OpenSSLAdapter::SSLInfoCallback);
#endif

  int mode = SSL_VERIFY_PEER;
  if (GetClientAuthEnabled()) {



    mode |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
  }


#ifdef OPENSSL_IS_BORINGSSL

  SSL_CTX_set_custom_verify(ctx, mode, SSLVerifyCallback);
#else


  SSL_CTX_set_verify(ctx, mode, nullptr);
  SSL_CTX_set_cert_verify_callback(ctx, SSLVerifyCallback, nullptr);
#endif




  SSL_CTX_set_cipher_list(
      ctx,
      "DEFAULT:!NULL:!aNULL:!SHA256:!SHA384:!aECDH:!AESGCM+AES256:!aPSK:!3DES");

  if (!srtp_ciphers_.empty()) {
    if (SSL_CTX_set_tlsext_use_srtp(ctx, srtp_ciphers_.c_str())) {
      SSL_CTX_free(ctx);
      return nullptr;
    }
  }

  return ctx;
}

bool OpenSSLStreamAdapter::VerifyPeerCertificate() {
  if (!HasPeerCertificateDigest() || !peer_cert_chain_ ||
      !peer_cert_chain_->GetSize()) {
    RTC_LOG(LS_WARNING) << "Missing digest or peer certificate.";
    return false;
  }

  unsigned char digest[EVP_MAX_MD_SIZE];
  size_t digest_length;
  if (!peer_cert_chain_->Get(0).ComputeDigest(
          peer_certificate_digest_algorithm_, digest, sizeof(digest),
          &digest_length)) {
    RTC_LOG(LS_WARNING) << "Failed to compute peer cert digest.";
    return false;
  }

  Buffer computed_digest(digest, digest_length);
  if (computed_digest != peer_certificate_digest_value_) {
    RTC_LOG(LS_WARNING)
        << "Rejected peer certificate due to mismatched digest using "
        << peer_certificate_digest_algorithm_ << ". Expected "
        << rtc::hex_encode_with_delimiter(peer_certificate_digest_value_, ':')
        << " got " << rtc::hex_encode_with_delimiter(computed_digest, ':');
    return false;
  }



  RTC_DLOG(LS_INFO) << "Accepted peer certificate.";
  peer_certificate_verified_ = true;
  return true;
}

std::unique_ptr<SSLCertChain> OpenSSLStreamAdapter::GetPeerSSLCertChain()
    const {
  return peer_cert_chain_ ? peer_cert_chain_->Clone() : nullptr;
}

#ifdef OPENSSL_IS_BORINGSSL
enum ssl_verify_result_t OpenSSLStreamAdapter::SSLVerifyCallback(
    SSL* ssl,
    uint8_t* out_alert) {

  OpenSSLStreamAdapter* stream =
      reinterpret_cast<OpenSSLStreamAdapter*>(SSL_get_app_data(ssl));
  const STACK_OF(CRYPTO_BUFFER)* chain = SSL_get0_peer_certificates(ssl);

  std::vector<std::unique_ptr<SSLCertificate>> cert_chain;
  for (CRYPTO_BUFFER* cert : chain) {
    cert_chain.emplace_back(new BoringSSLCertificate(bssl::UpRef(cert)));
  }
  stream->peer_cert_chain_.reset(new SSLCertChain(std::move(cert_chain)));


  if (stream->peer_certificate_digest_algorithm_.empty()) {
    RTC_LOG(LS_INFO) << "Waiting to verify certificate until digest is known.";

    return ssl_verify_ok;
  }

  if (!stream->VerifyPeerCertificate()) {
    return ssl_verify_invalid;
  }

  return ssl_verify_ok;
}
#else   // OPENSSL_IS_BORINGSSL
int OpenSSLStreamAdapter::SSLVerifyCallback(X509_STORE_CTX* store, void* arg) {

  SSL* ssl = reinterpret_cast<SSL*>(
      X509_STORE_CTX_get_ex_data(store, SSL_get_ex_data_X509_STORE_CTX_idx()));
  OpenSSLStreamAdapter* stream =
      reinterpret_cast<OpenSSLStreamAdapter*>(SSL_get_app_data(ssl));

  X509* cert = X509_STORE_CTX_get0_cert(store);
  stream->peer_cert_chain_.reset(
      new SSLCertChain(std::make_unique<OpenSSLCertificate>(cert)));


  if (stream->peer_certificate_digest_algorithm_.empty()) {
    RTC_DLOG(LS_INFO) << "Waiting to verify certificate until digest is known.";
    return 1;
  }

  if (!stream->VerifyPeerCertificate()) {
    X509_STORE_CTX_set_error(store, X509_V_ERR_CERT_REJECTED);
    return 0;
  }

  return 1;
}
#endif  // !OPENSSL_IS_BORINGSSL

bool OpenSSLStreamAdapter::IsBoringSsl() {
#ifdef OPENSSL_IS_BORINGSSL
  return true;
#else
  return false;
#endif
}

#define CDEF(X) \
  { static_cast<uint16_t>(TLS1_CK_##X & 0xffff), "TLS_" #X }

struct cipher_list {
  uint16_t cipher;
  const char* cipher_str;
};

static const cipher_list OK_RSA_ciphers[] = {
    CDEF(ECDHE_RSA_WITH_AES_128_CBC_SHA),
    CDEF(ECDHE_RSA_WITH_AES_256_CBC_SHA),
    CDEF(ECDHE_RSA_WITH_AES_128_GCM_SHA256),
#ifdef TLS1_CK_ECDHE_RSA_WITH_AES_256_GCM_SHA256
    CDEF(ECDHE_RSA_WITH_AES_256_GCM_SHA256),
#endif
#ifdef TLS1_CK_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256
    CDEF(ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256),
#endif
};

static const cipher_list OK_ECDSA_ciphers[] = {
    CDEF(ECDHE_ECDSA_WITH_AES_128_CBC_SHA),
    CDEF(ECDHE_ECDSA_WITH_AES_256_CBC_SHA),
    CDEF(ECDHE_ECDSA_WITH_AES_128_GCM_SHA256),
#ifdef TLS1_CK_ECDHE_ECDSA_WITH_AES_256_GCM_SHA256
    CDEF(ECDHE_ECDSA_WITH_AES_256_GCM_SHA256),
#endif
#ifdef TLS1_CK_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256
    CDEF(ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256),
#endif
};
#undef CDEF

bool OpenSSLStreamAdapter::IsAcceptableCipher(int cipher, KeyType key_type) {
  if (key_type == KT_RSA) {
    for (const cipher_list& c : OK_RSA_ciphers) {
      if (cipher == c.cipher) {
        return true;
      }
    }
  }

  if (key_type == KT_ECDSA) {
    for (const cipher_list& c : OK_ECDSA_ciphers) {
      if (cipher == c.cipher) {
        return true;
      }
    }
  }

  return false;
}

bool OpenSSLStreamAdapter::IsAcceptableCipher(absl::string_view cipher,
                                              KeyType key_type) {
  if (key_type == KT_RSA) {
    for (const cipher_list& c : OK_RSA_ciphers) {
      if (cipher == c.cipher_str) {
        return true;
      }
    }
  }

  if (key_type == KT_ECDSA) {
    for (const cipher_list& c : OK_ECDSA_ciphers) {
      if (cipher == c.cipher_str) {
        return true;
      }
    }
  }

  return false;
}

void OpenSSLStreamAdapter::EnableTimeCallbackForTesting() {
#ifdef OPENSSL_IS_BORINGSSL
  g_use_time_callback_for_testing = true;
#endif
}

}  // namespace rtc
