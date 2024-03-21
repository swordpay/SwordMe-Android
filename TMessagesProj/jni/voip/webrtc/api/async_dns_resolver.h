/*
 *  Copyright 2021 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_ASYNC_DNS_RESOLVER_H_
#define API_ASYNC_DNS_RESOLVER_H_

#include <functional>
#include <memory>

#include "rtc_base/checks.h"
#include "rtc_base/socket_address.h"
#include "rtc_base/system/rtc_export.h"

namespace webrtc {

// The AsyncDnsResolverInterface class encapsulates a single name query.
//
// Usage:
//   std::unique_ptr<AsyncDnsResolverInterface> resolver =
//        factory->Create(address-to-be-resolved, [r = resolver.get()]() {
//     if (r->result.GetResolvedAddress(AF_INET, &addr) {
//       // success
//     } else {
//       // failure
//       error = r->result().GetError();
//     }
//     // Release resolver.
//     resolver_list.erase(std::remove_if(resolver_list.begin(),
//     resolver_list.end(),
//                         [](refptr) { refptr.get() == r; });
//   });
//   resolver_list.push_back(std::move(resolver));

class AsyncDnsResolverResult {
 public:
  virtual ~AsyncDnsResolverResult() = default;




  virtual bool GetResolvedAddress(int family,
                                  rtc::SocketAddress* addr) const = 0;

  virtual int GetError() const = 0;
};

// The constructor, destructor and all functions must be called from
// the same sequence, and the callback will also be called on that sequence.
// The class guarantees that the callback will not be called if the
// resolver's destructor has been called.
class RTC_EXPORT AsyncDnsResolverInterface {
 public:
  virtual ~AsyncDnsResolverInterface() = default;

  virtual void Start(const rtc::SocketAddress& addr,
                     std::function<void()> callback) = 0;

  virtual void Start(const rtc::SocketAddress& addr,
                     int family,
                     std::function<void()> callback) = 0;
  virtual const AsyncDnsResolverResult& result() const = 0;
};

// client applications to provide WebRTC with their own mechanism for
// performing DNS resolution.
class AsyncDnsResolverFactoryInterface {
 public:
  virtual ~AsyncDnsResolverFactoryInterface() = default;



  virtual std::unique_ptr<webrtc::AsyncDnsResolverInterface> CreateAndResolve(
      const rtc::SocketAddress& addr,
      std::function<void()> callback) = 0;




  virtual std::unique_ptr<webrtc::AsyncDnsResolverInterface> CreateAndResolve(
      const rtc::SocketAddress& addr,
      int family,
      std::function<void()> callback) = 0;




  virtual std::unique_ptr<webrtc::AsyncDnsResolverInterface> Create() = 0;
};

}  // namespace webrtc

#endif  // API_ASYNC_DNS_RESOLVER_H_
