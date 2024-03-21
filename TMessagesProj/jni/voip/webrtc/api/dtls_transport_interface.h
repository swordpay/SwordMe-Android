/*
 *  Copyright 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_DTLS_TRANSPORT_INTERFACE_H_
#define API_DTLS_TRANSPORT_INTERFACE_H_

#include <memory>
#include <utility>

#include "absl/types/optional.h"
#include "api/ice_transport_interface.h"
#include "api/rtc_error.h"
#include "api/scoped_refptr.h"
#include "rtc_base/ref_count.h"
#include "rtc_base/ssl_certificate.h"
#include "rtc_base/system/rtc_export.h"

namespace webrtc {

// http://w3c.github.io/webrtc-pc/#dom-rtcdtlstransportstate
enum class DtlsTransportState {
  kNew,         // Has not started negotiating yet.
  kConnecting,  // In the process of negotiating a secure connection.
  kConnected,   // Completed negotiation and verified fingerprints.
  kClosed,      // Intentionally closed.
  kFailed,      // Failure due to an error or failing to verify a remote

  kNumValues
};

enum class DtlsTransportTlsRole {
  kServer,  // Other end sends CLIENT_HELLO
  kClient   // This end sends CLIENT_HELLO
};

// DTLSTransport.
class RTC_EXPORT DtlsTransportInformation {
 public:
  DtlsTransportInformation();
  explicit DtlsTransportInformation(DtlsTransportState state);
  DtlsTransportInformation(
      DtlsTransportState state,
      absl::optional<DtlsTransportTlsRole> role,
      absl::optional<int> tls_version,
      absl::optional<int> ssl_cipher_suite,
      absl::optional<int> srtp_cipher_suite,
      std::unique_ptr<rtc::SSLCertChain> remote_ssl_certificates);
  ABSL_DEPRECATED("Use version with role parameter")
  DtlsTransportInformation(
      DtlsTransportState state,
      absl::optional<int> tls_version,
      absl::optional<int> ssl_cipher_suite,
      absl::optional<int> srtp_cipher_suite,
      std::unique_ptr<rtc::SSLCertChain> remote_ssl_certificates);

  DtlsTransportInformation(const DtlsTransportInformation& c);
  DtlsTransportInformation& operator=(const DtlsTransportInformation& c);

  DtlsTransportInformation(DtlsTransportInformation&& other) = default;
  DtlsTransportInformation& operator=(DtlsTransportInformation&& other) =
      default;

  DtlsTransportState state() const { return state_; }
  absl::optional<DtlsTransportTlsRole> role() const { return role_; }
  absl::optional<int> tls_version() const { return tls_version_; }
  absl::optional<int> ssl_cipher_suite() const { return ssl_cipher_suite_; }
  absl::optional<int> srtp_cipher_suite() const { return srtp_cipher_suite_; }

  const rtc::SSLCertChain* remote_ssl_certificates() const {
    return remote_ssl_certificates_.get();
  }

 private:
  DtlsTransportState state_;
  absl::optional<DtlsTransportTlsRole> role_;
  absl::optional<int> tls_version_;
  absl::optional<int> ssl_cipher_suite_;
  absl::optional<int> srtp_cipher_suite_;
  std::unique_ptr<rtc::SSLCertChain> remote_ssl_certificates_;
};

class DtlsTransportObserverInterface {
 public:


  virtual void OnStateChange(DtlsTransportInformation info) = 0;


  virtual void OnError(RTCError error) = 0;

 protected:
  virtual ~DtlsTransportObserverInterface() = default;
};

// This object is created on the network thread, and can only be
// accessed on that thread, except for functions explicitly marked otherwise.
// References can be held by other threads, and destruction can therefore
// be initiated by other threads.
class DtlsTransportInterface : public rtc::RefCountInterface {
 public:

  virtual rtc::scoped_refptr<IceTransportInterface> ice_transport() = 0;


  virtual DtlsTransportInformation Information() = 0;

  virtual void RegisterObserver(DtlsTransportObserverInterface* observer) = 0;
  virtual void UnregisterObserver() = 0;
};

}  // namespace webrtc

#endif  // API_DTLS_TRANSPORT_INTERFACE_H_
