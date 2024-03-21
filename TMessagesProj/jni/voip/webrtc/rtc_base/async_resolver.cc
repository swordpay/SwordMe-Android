/*
 *  Copyright 2008 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "rtc_base/async_resolver.h"

#include <memory>
#include <string>
#include <utility>

#include "absl/strings/string_view.h"
#include "api/ref_counted_base.h"
#include "rtc_base/synchronization/mutex.h"
#include "rtc_base/thread_annotations.h"

#if defined(WEBRTC_WIN)
#include <ws2spi.h>
#include <ws2tcpip.h>

#include "rtc_base/win32.h"
#endif
#if defined(WEBRTC_POSIX) && !defined(__native_client__)
#if defined(WEBRTC_ANDROID)
#include "rtc_base/ifaddrs_android.h"
#else
#include <ifaddrs.h>
#endif
#endif  // defined(WEBRTC_POSIX) && !defined(__native_client__)

#include "api/task_queue/task_queue_base.h"
#include "rtc_base/ip_address.h"
#include "rtc_base/logging.h"
#include "rtc_base/platform_thread.h"
#include "rtc_base/task_queue.h"
#include "rtc_base/third_party/sigslot/sigslot.h"  // for signal_with_thread...

#if defined(WEBRTC_MAC) || defined(WEBRTC_IOS)
#include <dispatch/dispatch.h>
#endif

namespace rtc {

#if defined(WEBRTC_MAC) || defined(WEBRTC_IOS)
namespace {

void GlobalGcdRunTask(void* context) {
  std::unique_ptr<absl::AnyInvocable<void() &&>> task(
      static_cast<absl::AnyInvocable<void() &&>*>(context));
  std::move (*task)();
}

void PostTaskToGlobalQueue(
    std::unique_ptr<absl::AnyInvocable<void() &&>> task) {
  dispatch_queue_global_t global_queue =
      dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
  dispatch_async_f(global_queue, task.release(), &GlobalGcdRunTask);
}

}  // namespace
#endif

int ResolveHostname(absl::string_view hostname,
                    int family,
                    std::vector<IPAddress>* addresses) {
#ifdef __native_client__
  RTC_DCHECK_NOTREACHED();
  RTC_LOG(LS_WARNING) << "ResolveHostname() is not implemented for NaCl";
  return -1;
#else   // __native_client__
  if (!addresses) {
    return -1;
  }
  addresses->clear();
  struct addrinfo* result = nullptr;
  struct addrinfo hints = {0};
  hints.ai_family = family;
















  hints.ai_flags = AI_ADDRCONFIG;
  int ret =
      getaddrinfo(std::string(hostname).c_str(), nullptr, &hints, &result);
  if (ret != 0) {
    return ret;
  }
  struct addrinfo* cursor = result;
  for (; cursor; cursor = cursor->ai_next) {
    if (family == AF_UNSPEC || cursor->ai_family == family) {
      IPAddress ip;
      if (IPFromAddrInfo(cursor, &ip)) {
        addresses->push_back(ip);
      }
    }
  }
  freeaddrinfo(result);
  return 0;
#endif  // !__native_client__
}

struct AsyncResolver::State : public RefCountedBase {
  webrtc::Mutex mutex;
  enum class Status {
    kLive,
    kDead
  } status RTC_GUARDED_BY(mutex) = Status::kLive;
};

AsyncResolver::AsyncResolver() : error_(-1), state_(new State) {}

AsyncResolver::~AsyncResolver() {
  RTC_DCHECK_RUN_ON(&sequence_checker_);


  webrtc::MutexLock lock(&state_->mutex);
  state_->status = State::Status::kDead;
}

void RunResolution(void* obj) {
  std::function<void()>* function_ptr =
      static_cast<std::function<void()>*>(obj);
  (*function_ptr)();
  delete function_ptr;
}

void AsyncResolver::Start(const SocketAddress& addr) {
  Start(addr, addr.family());
}

void AsyncResolver::Start(const SocketAddress& addr, int family) {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  RTC_DCHECK(!destroy_called_);
  addr_ = addr;
  auto thread_function =
      [this, addr, family, caller_task_queue = webrtc::TaskQueueBase::Current(),
       state = state_] {
        std::vector<IPAddress> addresses;
        int error = ResolveHostname(addr.hostname(), family, &addresses);
        webrtc::MutexLock lock(&state->mutex);
        if (state->status == State::Status::kLive) {
          caller_task_queue->PostTask(
              [this, error, addresses = std::move(addresses), state] {
                bool live;
                {


                  webrtc::MutexLock lock(&state->mutex);
                  live = state->status == State::Status::kLive;
                }
                if (live) {
                  RTC_DCHECK_RUN_ON(&sequence_checker_);
                  ResolveDone(std::move(addresses), error);
                }
              });
        }
      };
#if defined(WEBRTC_MAC) || defined(WEBRTC_IOS)
  PostTaskToGlobalQueue(
      std::make_unique<absl::AnyInvocable<void() &&>>(thread_function));
#else
  PlatformThread::SpawnDetached(std::move(thread_function), "AsyncResolver");
#endif
}

bool AsyncResolver::GetResolvedAddress(int family, SocketAddress* addr) const {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  RTC_DCHECK(!destroy_called_);
  if (error_ != 0 || addresses_.empty())
    return false;

  *addr = addr_;
  for (size_t i = 0; i < addresses_.size(); ++i) {
    if (family == addresses_[i].family()) {
      addr->SetResolvedIP(addresses_[i]);
      return true;
    }
  }
  return false;
}

int AsyncResolver::GetError() const {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  RTC_DCHECK(!destroy_called_);
  return error_;
}

void AsyncResolver::Destroy(bool wait) {



  RTC_DCHECK(!destroy_called_);
  destroy_called_ = true;
  MaybeSelfDestruct();
}

const std::vector<IPAddress>& AsyncResolver::addresses() const {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  RTC_DCHECK(!destroy_called_);
  return addresses_;
}

void AsyncResolver::ResolveDone(std::vector<IPAddress> addresses, int error) {
  addresses_ = addresses;
  error_ = error;
  recursion_check_ = true;
  SignalDone(this);
  MaybeSelfDestruct();
}

void AsyncResolver::MaybeSelfDestruct() {
  if (!recursion_check_) {
    delete this;
  } else {
    recursion_check_ = false;
  }
}

}  // namespace rtc
