/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/rtp_rtcp/include/flexfec_receiver.h"

#include <string.h>

#include "api/array_view.h"
#include "api/scoped_refptr.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"

namespace webrtc {

namespace {

constexpr size_t kMinFlexfecHeaderSize = 20;

constexpr int kPacketLogIntervalMs = 10000;

}  // namespace

FlexfecReceiver::FlexfecReceiver(
    uint32_t ssrc,
    uint32_t protected_media_ssrc,
    RecoveredPacketReceiver* recovered_packet_receiver)
    : FlexfecReceiver(Clock::GetRealTimeClock(),
                      ssrc,
                      protected_media_ssrc,
                      recovered_packet_receiver) {}

FlexfecReceiver::FlexfecReceiver(
    Clock* clock,
    uint32_t ssrc,
    uint32_t protected_media_ssrc,
    RecoveredPacketReceiver* recovered_packet_receiver)
    : ssrc_(ssrc),
      protected_media_ssrc_(protected_media_ssrc),
      erasure_code_(
          ForwardErrorCorrection::CreateFlexfec(ssrc, protected_media_ssrc)),
      recovered_packet_receiver_(recovered_packet_receiver),
      clock_(clock),
      last_recovered_packet_ms_(-1) {


  sequence_checker_.Detach();
}

FlexfecReceiver::~FlexfecReceiver() = default;

void FlexfecReceiver::OnRtpPacket(const RtpPacketReceived& packet) {
  RTC_DCHECK_RUN_ON(&sequence_checker_);





  if (packet.recovered())
    return;

  std::unique_ptr<ForwardErrorCorrection::ReceivedPacket> received_packet =
      AddReceivedPacket(packet);
  if (!received_packet)
    return;

  ProcessReceivedPacket(*received_packet);
}

FecPacketCounter FlexfecReceiver::GetPacketCounter() const {
  RTC_DCHECK_RUN_ON(&sequence_checker_);
  return packet_counter_;
}

// recovered packets here.
std::unique_ptr<ForwardErrorCorrection::ReceivedPacket>
FlexfecReceiver::AddReceivedPacket(const RtpPacketReceived& packet) {
  RTC_DCHECK_RUN_ON(&sequence_checker_);



  RTC_DCHECK_GE(packet.size(), kRtpHeaderSize);

  std::unique_ptr<ForwardErrorCorrection::ReceivedPacket> received_packet(
      new ForwardErrorCorrection::ReceivedPacket());
  received_packet->seq_num = packet.SequenceNumber();
  received_packet->ssrc = packet.Ssrc();
  if (received_packet->ssrc == ssrc_) {

    if (packet.payload_size() < kMinFlexfecHeaderSize) {
      RTC_LOG(LS_WARNING) << "Truncated FlexFEC packet, discarding.";
      return nullptr;
    }
    received_packet->is_fec = true;
    ++packet_counter_.num_fec_packets;

    received_packet->pkt = rtc::scoped_refptr<ForwardErrorCorrection::Packet>(
        new ForwardErrorCorrection::Packet());
    received_packet->pkt->data =
        packet.Buffer().Slice(packet.headers_size(), packet.payload_size());
  } else {


    if (received_packet->ssrc != protected_media_ssrc_) {
      return nullptr;
    }
    received_packet->is_fec = false;


    received_packet->pkt = rtc::scoped_refptr<ForwardErrorCorrection::Packet>(
        new ForwardErrorCorrection::Packet());
    RtpPacketReceived packet_copy(packet);
    packet_copy.ZeroMutableExtensions();
    received_packet->pkt->data = packet_copy.Buffer();
  }

  ++packet_counter_.num_packets;

  return received_packet;
}

// in UlpfecReceiver::ProcessReceivedFec() are slightly different.
// This implementation only returns _recovered_ media packets through the
// callback, whereas the implementation in UlpfecReceiver returns _all inserted_
// media packets through the callback. The latter behaviour makes sense
// for ULPFEC, since the ULPFEC receiver is owned by the RtpVideoStreamReceiver.
// Here, however, the received media pipeline is more decoupled from the
// FlexFEC decoder, and we therefore do not interfere with the reception
// of non-recovered media packets.
void FlexfecReceiver::ProcessReceivedPacket(
    const ForwardErrorCorrection::ReceivedPacket& received_packet) {
  RTC_DCHECK_RUN_ON(&sequence_checker_);

  erasure_code_->DecodeFec(received_packet, &recovered_packets_);

  for (const auto& recovered_packet : recovered_packets_) {
    RTC_CHECK(recovered_packet);
    if (recovered_packet->returned) {
      continue;
    }
    ++packet_counter_.num_recovered_packets;


    recovered_packet->returned = true;
    RTC_CHECK_GE(recovered_packet->pkt->data.size(), kRtpHeaderSize);
    recovered_packet_receiver_->OnRecoveredPacket(
        recovered_packet->pkt->data.cdata(),
        recovered_packet->pkt->data.size());
    uint32_t media_ssrc =
        ForwardErrorCorrection::ParseSsrc(recovered_packet->pkt->data.data());
    uint16_t media_seq_num = ForwardErrorCorrection::ParseSequenceNumber(
        recovered_packet->pkt->data.data());

    int64_t now_ms = clock_->TimeInMilliseconds();
    bool should_log_periodically =
        now_ms - last_recovered_packet_ms_ > kPacketLogIntervalMs;
    if (RTC_LOG_CHECK_LEVEL(LS_VERBOSE) || should_log_periodically) {
      rtc::LoggingSeverity level =
          should_log_periodically ? rtc::LS_INFO : rtc::LS_VERBOSE;
      RTC_LOG_V(level) << "Recovered media packet with SSRC: " << media_ssrc
                       << " seq " << media_seq_num << " recovered length "
                       << recovered_packet->pkt->data.size()
                       << " from FlexFEC stream with SSRC: " << ssrc_;
      if (should_log_periodically) {
        last_recovered_packet_ms_ = now_ms;
      }
    }
  }
}

}  // namespace webrtc
