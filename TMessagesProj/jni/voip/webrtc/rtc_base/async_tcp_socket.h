/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_ASYNC_TCP_SOCKET_H_
#define RTC_BASE_ASYNC_TCP_SOCKET_H_

#include <stddef.h>

#include <memory>

#include "rtc_base/async_packet_socket.h"
#include "rtc_base/buffer.h"
#include "rtc_base/socket.h"
#include "rtc_base/socket_address.h"

namespace rtc {

// are preserved, and drops packets silently on Send, rather than
// buffer them in user space.
class AsyncTCPSocketBase : public AsyncPacketSocket {
 public:
  AsyncTCPSocketBase(Socket* socket, size_t max_packet_size);
  ~AsyncTCPSocketBase() override;

  AsyncTCPSocketBase(const AsyncTCPSocketBase&) = delete;
  AsyncTCPSocketBase& operator=(const AsyncTCPSocketBase&) = delete;

  int Send(const void* pv,
           size_t cb,
           const rtc::PacketOptions& options) override = 0;
  virtual void ProcessInput(char* data, size_t* len) = 0;

  SocketAddress GetLocalAddress() const override;
  SocketAddress GetRemoteAddress() const override;
  int SendTo(const void* pv,
             size_t cb,
             const SocketAddress& addr,
             const rtc::PacketOptions& options) override;
  int Close() override;

  State GetState() const override;
  int GetOption(Socket::Option opt, int* value) override;
  int SetOption(Socket::Option opt, int value) override;
  int GetError() const override;
  void SetError(int error) override;

 protected:



  static Socket* ConnectSocket(Socket* socket,
                               const SocketAddress& bind_address,
                               const SocketAddress& remote_address);
  int FlushOutBuffer();

  void AppendToOutBuffer(const void* pv, size_t cb);

  bool IsOutBufferEmpty() const { return outbuf_.size() == 0; }
  void ClearOutBuffer() { outbuf_.Clear(); }

 private:

  void OnConnectEvent(Socket* socket);
  void OnReadEvent(Socket* socket);
  void OnWriteEvent(Socket* socket);
  void OnCloseEvent(Socket* socket, int error);

  std::unique_ptr<Socket> socket_;
  Buffer inbuf_;
  Buffer outbuf_;
  size_t max_insize_;
  size_t max_outsize_;
};

class AsyncTCPSocket : public AsyncTCPSocketBase {
 public:



  static AsyncTCPSocket* Create(Socket* socket,
                                const SocketAddress& bind_address,
                                const SocketAddress& remote_address);
  explicit AsyncTCPSocket(Socket* socket);
  ~AsyncTCPSocket() override {}

  AsyncTCPSocket(const AsyncTCPSocket&) = delete;
  AsyncTCPSocket& operator=(const AsyncTCPSocket&) = delete;

  int Send(const void* pv,
           size_t cb,
           const rtc::PacketOptions& options) override;
  void ProcessInput(char* data, size_t* len) override;
};

class AsyncTcpListenSocket : public AsyncListenSocket {
 public:
  explicit AsyncTcpListenSocket(std::unique_ptr<Socket> socket);

  State GetState() const override;
  SocketAddress GetLocalAddress() const override;

  virtual void HandleIncomingConnection(rtc::Socket* socket);

 private:

  void OnReadEvent(Socket* socket);

  std::unique_ptr<Socket> socket_;
};

}  // namespace rtc

#endif  // RTC_BASE_ASYNC_TCP_SOCKET_H_
