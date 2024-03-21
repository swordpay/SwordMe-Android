/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef CALL_RTP_STREAM_RECEIVER_CONTROLLER_H_
#define CALL_RTP_STREAM_RECEIVER_CONTROLLER_H_

#include <memory>

#include "api/sequence_checker.h"
#include "call/rtp_demuxer.h"
#include "call/rtp_stream_receiver_controller_interface.h"

namespace webrtc {

class RtpPacketReceived;

// single RTP session.
// TODO(bugs.webrtc.org/7135): Add RTCP processing, we should aim to terminate
// RTCP and not leave any RTCP processing to individual receive streams.
class RtpStreamReceiverController
    : public RtpStreamReceiverControllerInterface {
 public:
  RtpStreamReceiverController();
  ~RtpStreamReceiverController() override;

  std::unique_ptr<RtpStreamReceiverInterface> CreateReceiver(
      uint32_t ssrc,
      RtpPacketSinkInterface* sink) override;

  bool OnRtpPacket(const RtpPacketReceived& packet);

 private:
  class Receiver : public RtpStreamReceiverInterface {
   public:
    Receiver(RtpStreamReceiverController* controller,
             uint32_t ssrc,
             RtpPacketSinkInterface* sink);

    ~Receiver() override;

   private:
    RtpStreamReceiverController* const controller_;
    RtpPacketSinkInterface* const sink_;
  };

  bool AddSink(uint32_t ssrc, RtpPacketSinkInterface* sink);
  bool RemoveSink(const RtpPacketSinkInterface* sink);








  SequenceChecker demuxer_sequence_;


  RtpDemuxer demuxer_ RTC_GUARDED_BY(&demuxer_sequence_){false /*use_mid*/};
};

}  // namespace webrtc

#endif  // CALL_RTP_STREAM_RECEIVER_CONTROLLER_H_
