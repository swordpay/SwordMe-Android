/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/rtp_rtcp/source/rtp_format_h264.h"

#include <string.h>

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "absl/types/optional.h"
#include "absl/types/variant.h"
#include "common_video/h264/h264_common.h"
#include "common_video/h264/pps_parser.h"
#include "common_video/h264/sps_parser.h"
#include "common_video/h264/sps_vui_rewriter.h"
#include "modules/rtp_rtcp/source/byte_io.h"
#include "modules/rtp_rtcp/source/rtp_packet_to_send.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"

namespace webrtc {
namespace {

static const size_t kNalHeaderSize = 1;
static const size_t kFuAHeaderSize = 2;
static const size_t kLengthFieldSize = 2;

}  // namespace

RtpPacketizerH264::RtpPacketizerH264(rtc::ArrayView<const uint8_t> payload,
                                     PayloadSizeLimits limits,
                                     H264PacketizationMode packetization_mode)
    : limits_(limits), num_packets_left_(0) {

  RTC_CHECK(packetization_mode == H264PacketizationMode::NonInterleaved ||
            packetization_mode == H264PacketizationMode::SingleNalUnit);

  for (const auto& nalu :
       H264::FindNaluIndices(payload.data(), payload.size())) {
    input_fragments_.push_back(
        payload.subview(nalu.payload_start_offset, nalu.payload_size));
  }

  if (!GeneratePackets(packetization_mode)) {



    num_packets_left_ = 0;
    while (!packets_.empty()) {
      packets_.pop();
    }
  }
}

RtpPacketizerH264::~RtpPacketizerH264() = default;

size_t RtpPacketizerH264::NumPackets() const {
  return num_packets_left_;
}

bool RtpPacketizerH264::GeneratePackets(
    H264PacketizationMode packetization_mode) {
  for (size_t i = 0; i < input_fragments_.size();) {
    switch (packetization_mode) {
      case H264PacketizationMode::SingleNalUnit:
        if (!PacketizeSingleNalu(i))
          return false;
        ++i;
        break;
      case H264PacketizationMode::NonInterleaved:
        int fragment_len = input_fragments_[i].size();
        int single_packet_capacity = limits_.max_payload_len;
        if (input_fragments_.size() == 1)
          single_packet_capacity -= limits_.single_packet_reduction_len;
        else if (i == 0)
          single_packet_capacity -= limits_.first_packet_reduction_len;
        else if (i + 1 == input_fragments_.size())
          single_packet_capacity -= limits_.last_packet_reduction_len;

        if (fragment_len > single_packet_capacity) {
          if (!PacketizeFuA(i))
            return false;
          ++i;
        } else {
          i = PacketizeStapA(i);
        }
        break;
    }
  }
  return true;
}

bool RtpPacketizerH264::PacketizeFuA(size_t fragment_index) {

  rtc::ArrayView<const uint8_t> fragment = input_fragments_[fragment_index];

  PayloadSizeLimits limits = limits_;

  limits.max_payload_len -= kFuAHeaderSize;


  if (input_fragments_.size() != 1) {


    if (fragment_index == input_fragments_.size() - 1) {
      limits.single_packet_reduction_len = limits_.last_packet_reduction_len;
    } else if (fragment_index == 0) {
      limits.single_packet_reduction_len = limits_.first_packet_reduction_len;
    } else {
      limits.single_packet_reduction_len = 0;
    }
  }
  if (fragment_index != 0)
    limits.first_packet_reduction_len = 0;
  if (fragment_index != input_fragments_.size() - 1)
    limits.last_packet_reduction_len = 0;

  size_t payload_left = fragment.size() - kNalHeaderSize;
  int offset = kNalHeaderSize;

  std::vector<int> payload_sizes = SplitAboutEqually(payload_left, limits);
  if (payload_sizes.empty())
    return false;

  for (size_t i = 0; i < payload_sizes.size(); ++i) {
    int packet_length = payload_sizes[i];
    RTC_CHECK_GT(packet_length, 0);
    packets_.push(PacketUnit(fragment.subview(offset, packet_length),
                             /*first_fragment=*/i == 0,
                             /*last_fragment=*/i == payload_sizes.size() - 1,
                             false, fragment[0]));
    offset += packet_length;
    payload_left -= packet_length;
  }
  num_packets_left_ += payload_sizes.size();
  RTC_CHECK_EQ(0, payload_left);
  return true;
}

size_t RtpPacketizerH264::PacketizeStapA(size_t fragment_index) {

  size_t payload_size_left = limits_.max_payload_len;
  if (input_fragments_.size() == 1)
    payload_size_left -= limits_.single_packet_reduction_len;
  else if (fragment_index == 0)
    payload_size_left -= limits_.first_packet_reduction_len;
  int aggregated_fragments = 0;
  size_t fragment_headers_length = 0;
  rtc::ArrayView<const uint8_t> fragment = input_fragments_[fragment_index];
  RTC_CHECK_GE(payload_size_left, fragment.size());
  ++num_packets_left_;

  auto payload_size_needed = [&] {
    size_t fragment_size = fragment.size() + fragment_headers_length;
    if (input_fragments_.size() == 1) {


      return fragment_size;
    }
    if (fragment_index == input_fragments_.size() - 1) {

      return fragment_size + limits_.last_packet_reduction_len;
    }
    return fragment_size;
  };

  while (payload_size_left >= payload_size_needed()) {
    RTC_CHECK_GT(fragment.size(), 0);
    packets_.push(PacketUnit(fragment, aggregated_fragments == 0, false, true,
                             fragment[0]));
    payload_size_left -= fragment.size();
    payload_size_left -= fragment_headers_length;

    fragment_headers_length = kLengthFieldSize;



    if (aggregated_fragments == 0)
      fragment_headers_length += kNalHeaderSize + kLengthFieldSize;
    ++aggregated_fragments;

    ++fragment_index;
    if (fragment_index == input_fragments_.size())
      break;
    fragment = input_fragments_[fragment_index];
  }
  RTC_CHECK_GT(aggregated_fragments, 0);
  packets_.back().last_fragment = true;
  return fragment_index;
}

bool RtpPacketizerH264::PacketizeSingleNalu(size_t fragment_index) {

  size_t payload_size_left = limits_.max_payload_len;
  if (input_fragments_.size() == 1)
    payload_size_left -= limits_.single_packet_reduction_len;
  else if (fragment_index == 0)
    payload_size_left -= limits_.first_packet_reduction_len;
  else if (fragment_index + 1 == input_fragments_.size())
    payload_size_left -= limits_.last_packet_reduction_len;
  rtc::ArrayView<const uint8_t> fragment = input_fragments_[fragment_index];
  if (payload_size_left < fragment.size()) {
    RTC_LOG(LS_ERROR) << "Failed to fit a fragment to packet in SingleNalu "
                         "packetization mode. Payload size left "
                      << payload_size_left << ", fragment length "
                      << fragment.size() << ", packet capacity "
                      << limits_.max_payload_len;
    return false;
  }
  RTC_CHECK_GT(fragment.size(), 0u);
  packets_.push(PacketUnit(fragment, true /* first */, true /* last */,
                           false /* aggregated */, fragment[0]));
  ++num_packets_left_;
  return true;
}

bool RtpPacketizerH264::NextPacket(RtpPacketToSend* rtp_packet) {
  RTC_DCHECK(rtp_packet);
  if (packets_.empty()) {
    return false;
  }

  PacketUnit packet = packets_.front();
  if (packet.first_fragment && packet.last_fragment) {

    size_t bytes_to_send = packet.source_fragment.size();
    uint8_t* buffer = rtp_packet->AllocatePayload(bytes_to_send);
    memcpy(buffer, packet.source_fragment.data(), bytes_to_send);
    packets_.pop();
    input_fragments_.pop_front();
  } else if (packet.aggregated) {
    NextAggregatePacket(rtp_packet);
  } else {
    NextFragmentPacket(rtp_packet);
  }
  rtp_packet->SetMarker(packets_.empty());
  --num_packets_left_;
  return true;
}

void RtpPacketizerH264::NextAggregatePacket(RtpPacketToSend* rtp_packet) {

  size_t payload_capacity = rtp_packet->FreeCapacity();
  RTC_CHECK_GE(payload_capacity, kNalHeaderSize);
  uint8_t* buffer = rtp_packet->AllocatePayload(payload_capacity);
  RTC_DCHECK(buffer);
  PacketUnit* packet = &packets_.front();
  RTC_CHECK(packet->first_fragment);

  buffer[0] =
      (packet->header & (kH264FBit | kH264NriMask)) | H264::NaluType::kStapA;
  size_t index = kNalHeaderSize;
  bool is_last_fragment = packet->last_fragment;
  while (packet->aggregated) {
    rtc::ArrayView<const uint8_t> fragment = packet->source_fragment;
    RTC_CHECK_LE(index + kLengthFieldSize + fragment.size(), payload_capacity);

    ByteWriter<uint16_t>::WriteBigEndian(&buffer[index], fragment.size());
    index += kLengthFieldSize;

    memcpy(&buffer[index], fragment.data(), fragment.size());
    index += fragment.size();
    packets_.pop();
    input_fragments_.pop_front();
    if (is_last_fragment)
      break;
    packet = &packets_.front();
    is_last_fragment = packet->last_fragment;
  }
  RTC_CHECK(is_last_fragment);
  rtp_packet->SetPayloadSize(index);
}

void RtpPacketizerH264::NextFragmentPacket(RtpPacketToSend* rtp_packet) {
  PacketUnit* packet = &packets_.front();



  uint8_t fu_indicator =
      (packet->header & (kH264FBit | kH264NriMask)) | H264::NaluType::kFuA;
  uint8_t fu_header = 0;

  fu_header |= (packet->first_fragment ? kH264SBit : 0);
  fu_header |= (packet->last_fragment ? kH264EBit : 0);
  uint8_t type = packet->header & kH264TypeMask;
  fu_header |= type;
  rtc::ArrayView<const uint8_t> fragment = packet->source_fragment;
  uint8_t* buffer =
      rtp_packet->AllocatePayload(kFuAHeaderSize + fragment.size());
  buffer[0] = fu_indicator;
  buffer[1] = fu_header;
  memcpy(buffer + kFuAHeaderSize, fragment.data(), fragment.size());
  if (packet->last_fragment)
    input_fragments_.pop_front();
  packets_.pop();
}

}  // namespace webrtc
