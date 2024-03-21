/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_SSL_ADAPTER_H_
#define RTC_BASE_SSL_ADAPTER_H_

#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "rtc_base/async_socket.h"
#include "rtc_base/ssl_certificate.h"
#include "rtc_base/ssl_identity.h"
#include "rtc_base/ssl_stream_adapter.h"
#include "rtc_base/system/rtc_export.h"

namespace rtc {

class SSLAdapter;

// which allows clients to resume SSL sessions to previously-contacted hosts.
// Clients should create the factory using Create(), set up the factory as
// needed using SetMode, and then call CreateAdapter to create adapters when
// needed.
class SSLAdapterFactory {
 public:
  virtual ~SSLAdapterFactory() {}

  virtual void SetMode(SSLMode mode) = 0;

  virtual void SetCertVerifier(SSLCertificateVerifier* ssl_cert_verifier) = 0;


  virtual void SetIdentity(std::unique_ptr<SSLIdentity> identity) = 0;

  virtual void SetRole(SSLRole role) = 0;


  virtual void SetIgnoreBadCert(bool ignore) = 0;

  virtual SSLAdapter* CreateAdapter(Socket* socket) = 0;

  static std::unique_ptr<SSLAdapterFactory> Create();
};

// standalone, via SSLAdapter::Create, or through a factory as described above,
// in which case it will share state with other SSLAdapters created from the
// same factory.
// After creation, call StartSSL to initiate the SSL handshake to the server.
class SSLAdapter : public AsyncSocketAdapter {
 public:
  explicit SSLAdapter(Socket* socket) : AsyncSocketAdapter(socket) {}




  virtual void SetIgnoreBadCert(bool ignore) = 0;

  virtual void SetAlpnProtocols(const std::vector<std::string>& protos) = 0;
  virtual void SetEllipticCurves(const std::vector<std::string>& curves) = 0;

  virtual void SetMode(SSLMode mode) = 0;

  virtual void SetCertVerifier(SSLCertificateVerifier* ssl_cert_verifier) = 0;


  virtual void SetIdentity(std::unique_ptr<SSLIdentity> identity) = 0;

  virtual void SetRole(SSLRole role) = 0;



  virtual int StartSSL(absl::string_view hostname) = 0;





  virtual bool IsResumedSession() = 0;



  static SSLAdapter* Create(Socket* socket);

 private:

  int Listen(int backlog) override { RTC_CHECK(false); }
  Socket* Accept(SocketAddress* paddr) override { RTC_CHECK(false); }
};


// Call CleanupSSL when finished with SSL.
RTC_EXPORT bool InitializeSSL();

RTC_EXPORT bool CleanupSSL();

}  // namespace rtc

#endif  // RTC_BASE_SSL_ADAPTER_H_
