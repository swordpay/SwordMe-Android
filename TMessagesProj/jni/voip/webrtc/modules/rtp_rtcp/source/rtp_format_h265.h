/*
 *  Copyright (c) 2020 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_RTP_RTCP_SOURCE_RTP_FORMAT_H265_H_
#define WEBRTC_MODULES_RTP_RTCP_SOURCE_RTP_FORMAT_H265_H_

#include <memory>
#include <queue>
#include <string>
#include "api/array_view.h"
#include "modules/include/module_common_types.h"
#include "modules/rtp_rtcp/source/rtp_format.h"
#include "modules/rtp_rtcp/source/rtp_packet_to_send.h"
#include "modules/rtp_rtcp/source/rtp_format.h"
#include "modules/video_coding/codecs/h265/include/h265_globals.h"
#include "rtc_base/buffer.h"
#include "rtc_base/constructor_magic.h"

namespace webrtc {

class RtpPacketizerH265 : public RtpPacketizer {
 public:


  RtpPacketizerH265(rtc::ArrayView<const uint8_t> payload,
                    PayloadSizeLimits limits,
                    H265PacketizationMode packetization_mode);

   ~RtpPacketizerH265() override;

  size_t NumPackets() const override;







  bool NextPacket(RtpPacketToSend* rtp_packet) override;

 private:
  struct Packet {
    Packet(size_t offset,
           size_t size,
           bool first_fragment,
           bool last_fragment,
           bool aggregated,
           uint16_t header)
        : offset(offset),
          size(size),
          first_fragment(first_fragment),
          last_fragment(last_fragment),
          aggregated(aggregated),
          header(header) {}

    size_t offset;
    size_t size;
    bool first_fragment;
    bool last_fragment;
    bool aggregated;
    uint16_t header;  // Different from H264
  };
  struct Fragment {
    Fragment(const uint8_t* buffer, size_t length);
    explicit Fragment(const Fragment& fragment);
    const uint8_t* buffer = nullptr;
    size_t length = 0;
    std::unique_ptr<rtc::Buffer> tmp_buffer;
  };
  struct PacketUnit {
    PacketUnit(const Fragment& source_fragment,
               bool first_fragment,
               bool last_fragment,
               bool aggregated,
               uint16_t header)
        : source_fragment(source_fragment),
          first_fragment(first_fragment),
          last_fragment(last_fragment),
          aggregated(aggregated),
          header(header) {}

    const Fragment source_fragment;
    bool first_fragment;
    bool last_fragment;
    bool aggregated;
    uint16_t header;
  };
  typedef std::queue<Packet> PacketQueue;
  std::deque<Fragment> input_fragments_;
  std::queue<PacketUnit> packets_;

  bool GeneratePackets(H265PacketizationMode packetization_mode);
  bool PacketizeFu(size_t fragment_index);
  int PacketizeAp(size_t fragment_index);
  bool PacketizeSingleNalu(size_t fragment_index);

  void NextAggregatePacket(RtpPacketToSend* rtp_packet, bool last);
  void NextFragmentPacket(RtpPacketToSend* rtp_packet);

  const PayloadSizeLimits limits_;
  size_t num_packets_left_;

  RTC_DISALLOW_COPY_AND_ASSIGN(RtpPacketizerH265);
};
}  // namespace webrtc
#endif  // WEBRTC_MODULES_RTP_RTCP_SOURCE_RTP_FORMAT_H265_H_
