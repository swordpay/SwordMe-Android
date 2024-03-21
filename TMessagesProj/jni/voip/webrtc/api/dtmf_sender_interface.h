/*
 *  Copyright 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_DTMF_SENDER_INTERFACE_H_
#define API_DTMF_SENDER_INTERFACE_H_

#include <string>

#include "api/media_stream_interface.h"
#include "rtc_base/ref_count.h"

namespace webrtc {

// Applications should implement this interface to get notifications from the
// DtmfSender.
class DtmfSenderObserverInterface {
 public:





  virtual void OnToneChange(const std::string& tone,
                            const std::string& tone_buffer) {}



  virtual void OnToneChange(const std::string& tone) {}

 protected:
  virtual ~DtmfSenderObserverInterface() = default;
};

// WebRTC W3C Editor's Draft.
// See: https://www.w3.org/TR/webrtc/#peer-to-peer-dtmf
class DtmfSenderInterface : public rtc::RefCountInterface {
 public:

  static const int kDtmfDefaultCommaDelayMs = 2000;



  virtual void RegisterObserver(DtmfSenderObserverInterface* observer) = 0;
  virtual void UnregisterObserver() = 0;



  virtual bool CanInsertDtmf() = 0;

























  virtual bool InsertDtmf(const std::string& tones,
                          int duration,
                          int inter_tone_gap) {
    return InsertDtmf(tones, duration, inter_tone_gap,
                      kDtmfDefaultCommaDelayMs);
  }
  virtual bool InsertDtmf(const std::string& tones,
                          int duration,
                          int inter_tone_gap,
                          int comma_delay) {


    return InsertDtmf(tones, duration, inter_tone_gap);
  }

  virtual std::string tones() const = 0;



  virtual int duration() const = 0;



  virtual int inter_tone_gap() const = 0;



  virtual int comma_delay() const { return kDtmfDefaultCommaDelayMs; }

 protected:
  ~DtmfSenderInterface() override = default;
};

}  // namespace webrtc

#endif  // API_DTMF_SENDER_INTERFACE_H_
