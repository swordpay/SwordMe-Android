// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_SYNC_SOCKET_H_
#define BASE_SYNC_SOCKET_H_

// data.  Because the receiving is blocking, they can be used to perform
// rudimentary cross-process synchronization with low latency.

#include <stddef.h>

#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "base/files/platform_file.h"
#include "base/macros.h"
#include "base/synchronization/waitable_event.h"
#include "base/time/time.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include <windows.h>
#endif
#include <sys/types.h>

#if defined(OS_POSIX) || defined(OS_FUCHSIA)
#include "base/file_descriptor_posix.h"
#endif

namespace base {

class BASE_EXPORT SyncSocket {
 public:
  using Handle = PlatformFile;
  using ScopedHandle = ScopedPlatformFile;
  static const Handle kInvalidHandle;

  SyncSocket();

  explicit SyncSocket(Handle handle);
  explicit SyncSocket(ScopedHandle handle);
  virtual ~SyncSocket();



  static bool CreatePair(SyncSocket* socket_a, SyncSocket* socket_b);

  virtual void Close();






  virtual size_t Send(const void* buffer, size_t length);




  virtual size_t Receive(void* buffer, size_t length);



  virtual size_t ReceiveWithTimeout(void* buffer,
                                    size_t length,
                                    TimeDelta timeout);


  virtual size_t Peek();

  bool IsValid() const;


  Handle handle() const;

  Handle Release();
  ScopedHandle Take();

 protected:
  ScopedHandle handle_;

 private:
  DISALLOW_COPY_AND_ASSIGN(SyncSocket);
};

// another thread while a blocking Receive or Send is being done from the
// thread that owns the socket.
class BASE_EXPORT CancelableSyncSocket : public SyncSocket {
 public:
  CancelableSyncSocket();
  explicit CancelableSyncSocket(Handle handle);
  explicit CancelableSyncSocket(ScopedHandle handle);
  ~CancelableSyncSocket() override = default;


  static bool CreatePair(CancelableSyncSocket* socket_a,
                         CancelableSyncSocket* socket_b);


  bool Shutdown();

#if defined(OS_WIN)






  void Close() override;
  size_t Receive(void* buffer, size_t length) override;
  size_t ReceiveWithTimeout(void* buffer,
                            size_t length,
                            TimeDelta timeout) override;
#endif





  size_t Send(const void* buffer, size_t length) override;

 private:
#if defined(OS_WIN)
  WaitableEvent shutdown_event_;
  WaitableEvent file_operation_;
#endif
  DISALLOW_COPY_AND_ASSIGN(CancelableSyncSocket);
};

}  // namespace base

#endif  // BASE_SYNC_SOCKET_H_
