/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 *
 */
#ifndef MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_H_
#define MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_H_

#include <stddef.h>
#include <stdint.h>

#include "api/array_view.h"
#include "api/function_view.h"
#include "rtc_base/buffer.h"

namespace webrtc {
namespace rtcp {
// Class for building RTCP packets.
//
//  Example:
//  ReportBlock report_block;
//  report_block.SetMediaSsrc(234);
//  report_block.SetFractionLost(10);
//
//  ReceiverReport rr;
//  rr.SetSenderSsrc(123);
//  rr.AddReportBlock(report_block);
//
//  Fir fir;
//  fir.SetSenderSsrc(123);
//  fir.AddRequestTo(234, 56);
//
//  size_t length = 0;                     // Builds an intra frame request
//  uint8_t packet[kPacketSize];           // with sequence number 56.
//  fir.Build(packet, &length, kPacketSize);
//
//  rtc::Buffer packet = fir.Build();      // Returns a RawPacket holding
//                                         // the built rtcp packet.
//
//  CompoundPacket compound;               // Builds a compound RTCP packet with
//  compound.Append(&rr);                  // a receiver report, report block
//  compound.Append(&fir);                 // and fir message.
//  rtc::Buffer packet = compound.Build();

class RtcpPacket {
 public:




  using PacketReadyCallback =
      rtc::FunctionView<void(rtc::ArrayView<const uint8_t> packet)>;

  virtual ~RtcpPacket() = default;

  void SetSenderSsrc(uint32_t ssrc) { sender_ssrc_ = ssrc; }
  uint32_t sender_ssrc() const { return sender_ssrc_; }


  rtc::Buffer Build() const;

  bool Build(size_t max_length, PacketReadyCallback callback) const;

  virtual size_t BlockLength() const = 0;



  virtual bool Create(uint8_t* packet,
                      size_t* index,
                      size_t max_length,
                      PacketReadyCallback callback) const = 0;

 protected:

  static constexpr size_t kHeaderLength = 4;
  RtcpPacket() {}

  static void CreateHeader(size_t count_or_format,
                           uint8_t packet_type,
                           size_t block_length,  // Payload size in 32bit words.
                           uint8_t* buffer,
                           size_t* pos);

  static void CreateHeader(size_t count_or_format,
                           uint8_t packet_type,
                           size_t block_length,  // Payload size in 32bit words.
                           bool padding,  // True if there are padding bytes.
                           uint8_t* buffer,
                           size_t* pos);

  bool OnBufferFull(uint8_t* packet,
                    size_t* index,
                    PacketReadyCallback callback) const;

  size_t HeaderLength() const;

 private:
  uint32_t sender_ssrc_ = 0;
};
}  // namespace rtcp
}  // namespace webrtc
#endif  // MODULES_RTP_RTCP_SOURCE_RTCP_PACKET_H_
