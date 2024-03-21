/*
 *  Copyright 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_RTC_EVENT_LOG_RTC_EVENT_LOG_FACTORY_INTERFACE_H_
#define API_RTC_EVENT_LOG_RTC_EVENT_LOG_FACTORY_INTERFACE_H_

#include <memory>

#include "api/rtc_event_log/rtc_event_log.h"

namespace webrtc {

// RtcEventLog support. A PeerConnectionFactory is constructed with an
// RtcEventLogFactoryInterface, which may or may not be null.
class RtcEventLogFactoryInterface {
 public:
  virtual ~RtcEventLogFactoryInterface() = default;

  virtual std::unique_ptr<RtcEventLog> Create(
      RtcEventLog::EncodingType encoding_type) const = 0;
  [[deprecated]] virtual std::unique_ptr<RtcEventLog> CreateRtcEventLog(
      RtcEventLog::EncodingType encoding_type) = 0;
};

}  // namespace webrtc

#endif  // API_RTC_EVENT_LOG_RTC_EVENT_LOG_FACTORY_INTERFACE_H_
