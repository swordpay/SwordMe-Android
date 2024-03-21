/*
 *  Copyright 2013 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_ASYNC_RESOLVER_INTERFACE_H_
#define RTC_BASE_ASYNC_RESOLVER_INTERFACE_H_

#include "rtc_base/checks.h"
#include "rtc_base/socket_address.h"
#include "rtc_base/system/rtc_export.h"
#include "rtc_base/third_party/sigslot/sigslot.h"

namespace rtc {

class RTC_EXPORT AsyncResolverInterface {
 public:
  AsyncResolverInterface();
  virtual ~AsyncResolverInterface();

  virtual void Start(const SocketAddress& addr) = 0;

  virtual void Start(const SocketAddress& addr, int family) = 0;




  virtual bool GetResolvedAddress(int family, SocketAddress* addr) const = 0;

  virtual int GetError() const = 0;

  virtual void Destroy(bool wait) = 0;


  SocketAddress address() const {
    SocketAddress addr;
    GetResolvedAddress(AF_INET, &addr);
    return addr;
  }

  sigslot::signal1<AsyncResolverInterface*> SignalDone;
};

}  // namespace rtc

#endif
