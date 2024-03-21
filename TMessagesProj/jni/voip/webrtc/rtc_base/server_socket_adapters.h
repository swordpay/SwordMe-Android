/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_SERVER_SOCKET_ADAPTERS_H_
#define RTC_BASE_SERVER_SOCKET_ADAPTERS_H_

#include "rtc_base/socket_adapters.h"

namespace rtc {

class AsyncProxyServerSocket : public BufferedReadAdapter {
 public:
  AsyncProxyServerSocket(Socket* socket, size_t buffer_size);
  ~AsyncProxyServerSocket() override;
  sigslot::signal2<AsyncProxyServerSocket*, const SocketAddress&>
      SignalConnectRequest;
  virtual void SendConnectResult(int err, const SocketAddress& addr) = 0;
};

// fake SSL handshake. Used when implementing a relay server that does "ssltcp".
class AsyncSSLServerSocket : public BufferedReadAdapter {
 public:
  explicit AsyncSSLServerSocket(Socket* socket);

  AsyncSSLServerSocket(const AsyncSSLServerSocket&) = delete;
  AsyncSSLServerSocket& operator=(const AsyncSSLServerSocket&) = delete;

 protected:
  void ProcessInput(char* data, size_t* len) override;
};

class AsyncSocksProxyServerSocket : public AsyncProxyServerSocket {
 public:
  explicit AsyncSocksProxyServerSocket(Socket* socket);

  AsyncSocksProxyServerSocket(const AsyncSocksProxyServerSocket&) = delete;
  AsyncSocksProxyServerSocket& operator=(const AsyncSocksProxyServerSocket&) =
      delete;

 private:
  void ProcessInput(char* data, size_t* len) override;
  void DirectSend(const ByteBufferWriter& buf);

  void HandleHello(ByteBufferReader* request);
  void SendHelloReply(uint8_t method);
  void HandleAuth(ByteBufferReader* request);
  void SendAuthReply(uint8_t result);
  void HandleConnect(ByteBufferReader* request);
  void SendConnectResult(int result, const SocketAddress& addr) override;

  void Error(int error);

  static const int kBufferSize = 1024;
  enum State {
    SS_HELLO,
    SS_AUTH,
    SS_CONNECT,
    SS_CONNECT_PENDING,
    SS_TUNNEL,
    SS_ERROR
  };
  State state_;
};

}  // namespace rtc

#endif  // RTC_BASE_SERVER_SOCKET_ADAPTERS_H_
