/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/rtp_rtcp/source/rtcp_receiver.h"

#include <string.h>

#include <algorithm>
#include <limits>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "api/video/video_bitrate_allocation.h"
#include "api/video/video_bitrate_allocator.h"
#include "modules/rtp_rtcp/source/rtcp_packet/bye.h"
#include "modules/rtp_rtcp/source/rtcp_packet/common_header.h"
#include "modules/rtp_rtcp/source/rtcp_packet/compound_packet.h"
#include "modules/rtp_rtcp/source/rtcp_packet/extended_reports.h"
#include "modules/rtp_rtcp/source/rtcp_packet/fir.h"
#include "modules/rtp_rtcp/source/rtcp_packet/loss_notification.h"
#include "modules/rtp_rtcp/source/rtcp_packet/nack.h"
#include "modules/rtp_rtcp/source/rtcp_packet/pli.h"
#include "modules/rtp_rtcp/source/rtcp_packet/rapid_resync_request.h"
#include "modules/rtp_rtcp/source/rtcp_packet/receiver_report.h"
#include "modules/rtp_rtcp/source/rtcp_packet/remb.h"
#include "modules/rtp_rtcp/source/rtcp_packet/remote_estimate.h"
#include "modules/rtp_rtcp/source/rtcp_packet/sdes.h"
#include "modules/rtp_rtcp/source/rtcp_packet/sender_report.h"
#include "modules/rtp_rtcp/source/rtcp_packet/tmmbn.h"
#include "modules/rtp_rtcp/source/rtcp_packet/tmmbr.h"
#include "modules/rtp_rtcp/source/rtcp_packet/transport_feedback.h"
#include "modules/rtp_rtcp/source/rtp_rtcp_config.h"
#include "modules/rtp_rtcp/source/rtp_rtcp_impl2.h"
#include "modules/rtp_rtcp/source/time_util.h"
#include "modules/rtp_rtcp/source/tmmbr_help.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/time_utils.h"
#include "rtc_base/trace_event.h"
#include "system_wrappers/include/ntp_time.h"

namespace webrtc {
namespace {

using rtcp::CommonHeader;
using rtcp::ReportBlock;

const int kRrTimeoutIntervals = 3;

const int64_t kTmmbrTimeoutIntervalMs = 5 * 5000;

const int64_t kMaxWarningLogIntervalMs = 10000;
const int64_t kRtcpMinFrameLengthMs = 17;

const size_t kMaxNumberOfStoredRrtrs = 300;

constexpr TimeDelta kDefaultVideoReportInterval = TimeDelta::Seconds(1);
constexpr TimeDelta kDefaultAudioReportInterval = TimeDelta::Seconds(5);

// kRrTimeoutIntervals| period and was reset (set to PlusInfinity()). Returns
// false if the timer was either already reset or if it has not expired.
bool ResetTimestampIfExpired(const Timestamp now,
                             Timestamp& timestamp,
                             TimeDelta interval) {
  if (timestamp.IsInfinite() ||
      now <= timestamp + interval * kRrTimeoutIntervals) {
    return false;
  }

  timestamp = Timestamp::PlusInfinity();
  return true;
}

}  // namespace

constexpr size_t RTCPReceiver::RegisteredSsrcs::kMediaSsrcIndex;
constexpr size_t RTCPReceiver::RegisteredSsrcs::kMaxSsrcs;

RTCPReceiver::RegisteredSsrcs::RegisteredSsrcs(
    bool disable_sequence_checker,
    const RtpRtcpInterface::Configuration& config)
    : packet_sequence_checker_(disable_sequence_checker) {
  packet_sequence_checker_.Detach();
  ssrcs_.push_back(config.local_media_ssrc);
  if (config.rtx_send_ssrc) {
    ssrcs_.push_back(*config.rtx_send_ssrc);
  }
  if (config.fec_generator) {
    absl::optional<uint32_t> flexfec_ssrc = config.fec_generator->FecSsrc();
    if (flexfec_ssrc) {
      ssrcs_.push_back(*flexfec_ssrc);
    }
  }

  RTC_DCHECK_LE(ssrcs_.size(), RTCPReceiver::RegisteredSsrcs::kMaxSsrcs);
}

bool RTCPReceiver::RegisteredSsrcs::contains(uint32_t ssrc) const {
  RTC_DCHECK_RUN_ON(&packet_sequence_checker_);
  return absl::c_linear_search(ssrcs_, ssrc);
}

uint32_t RTCPReceiver::RegisteredSsrcs::media_ssrc() const {
  RTC_DCHECK_RUN_ON(&packet_sequence_checker_);
  return ssrcs_[kMediaSsrcIndex];
}

void RTCPReceiver::RegisteredSsrcs::set_media_ssrc(uint32_t ssrc) {
  RTC_DCHECK_RUN_ON(&packet_sequence_checker_);
  ssrcs_[kMediaSsrcIndex] = ssrc;
}

struct RTCPReceiver::PacketInformation {
  uint32_t packet_type_flags = 0;  // RTCPPacketTypeFlags bit field.

  uint32_t remote_ssrc = 0;
  std::vector<uint16_t> nack_sequence_numbers;

  ReportBlockList report_blocks;
  std::vector<ReportBlockData> report_block_datas;
  int64_t rtt_ms = 0;
  uint32_t receiver_estimated_max_bitrate_bps = 0;
  std::unique_ptr<rtcp::TransportFeedback> transport_feedback;
  absl::optional<VideoBitrateAllocation> target_bitrate_allocation;
  absl::optional<NetworkStateEstimate> network_state_estimate;
  std::unique_ptr<rtcp::LossNotification> loss_notification;
};

RTCPReceiver::RTCPReceiver(const RtpRtcpInterface::Configuration& config,
                           ModuleRtpRtcpImpl2* owner)
    : clock_(config.clock),
      receiver_only_(config.receiver_only),
      rtp_rtcp_(owner),
      registered_ssrcs_(false, config),
      rtcp_bandwidth_observer_(config.bandwidth_callback),
      rtcp_intra_frame_observer_(config.intra_frame_callback),
      rtcp_loss_notification_observer_(config.rtcp_loss_notification_observer),
      network_state_estimate_observer_(config.network_state_estimate_observer),
      transport_feedback_observer_(config.transport_feedback_callback),
      bitrate_allocation_observer_(config.bitrate_allocation_observer),
      report_interval_(config.rtcp_report_interval_ms > 0
                           ? TimeDelta::Millis(config.rtcp_report_interval_ms)
                           : (config.audio ? kDefaultAudioReportInterval
                                           : kDefaultVideoReportInterval)),

      remote_ssrc_(0),
      remote_sender_rtp_time_(0),
      remote_sender_packet_count_(0),
      remote_sender_octet_count_(0),
      remote_sender_reports_count_(0),
      xr_rrtr_status_(config.non_sender_rtt_measurement),
      xr_rr_rtt_ms_(0),
      oldest_tmmbr_info_ms_(0),
      cname_callback_(config.rtcp_cname_callback),
      report_block_data_observer_(config.report_block_data_observer),
      packet_type_counter_observer_(config.rtcp_packet_type_counter_observer),
      num_skipped_packets_(0),
      last_skipped_packets_warning_ms_(clock_->TimeInMilliseconds()) {
  RTC_DCHECK(owner);
}

RTCPReceiver::RTCPReceiver(const RtpRtcpInterface::Configuration& config,
                           ModuleRtpRtcp* owner)
    : clock_(config.clock),
      receiver_only_(config.receiver_only),
      rtp_rtcp_(owner),
      registered_ssrcs_(true, config),
      rtcp_bandwidth_observer_(config.bandwidth_callback),
      rtcp_intra_frame_observer_(config.intra_frame_callback),
      rtcp_loss_notification_observer_(config.rtcp_loss_notification_observer),
      network_state_estimate_observer_(config.network_state_estimate_observer),
      transport_feedback_observer_(config.transport_feedback_callback),
      bitrate_allocation_observer_(config.bitrate_allocation_observer),
      report_interval_(config.rtcp_report_interval_ms > 0
                           ? TimeDelta::Millis(config.rtcp_report_interval_ms)
                           : (config.audio ? kDefaultAudioReportInterval
                                           : kDefaultVideoReportInterval)),

      remote_ssrc_(0),
      remote_sender_rtp_time_(0),
      remote_sender_packet_count_(0),
      remote_sender_octet_count_(0),
      remote_sender_reports_count_(0),
      xr_rrtr_status_(config.non_sender_rtt_measurement),
      xr_rr_rtt_ms_(0),
      oldest_tmmbr_info_ms_(0),
      cname_callback_(config.rtcp_cname_callback),
      report_block_data_observer_(config.report_block_data_observer),
      packet_type_counter_observer_(config.rtcp_packet_type_counter_observer),
      num_skipped_packets_(0),
      last_skipped_packets_warning_ms_(clock_->TimeInMilliseconds()) {
  RTC_DCHECK(owner);












  RTC_LOG(LS_INFO) << "************** !!!DEPRECATION WARNING!! **************";
}

RTCPReceiver::~RTCPReceiver() {}

void RTCPReceiver::IncomingPacket(rtc::ArrayView<const uint8_t> packet) {
  if (packet.empty()) {
    RTC_LOG(LS_WARNING) << "Incoming empty RTCP packet";
    return;
  }

  PacketInformation packet_information;
  if (!ParseCompoundPacket(packet, &packet_information))
    return;
  TriggerCallbacksFromRtcpPacket(packet_information);
}

// remove it soon.
int64_t RTCPReceiver::LastReceivedReportBlockMs() const {
  MutexLock lock(&rtcp_receiver_lock_);
  return last_received_rb_.IsFinite() ? last_received_rb_.ms() : 0;
}

void RTCPReceiver::SetRemoteSSRC(uint32_t ssrc) {
  MutexLock lock(&rtcp_receiver_lock_);

  last_received_sr_ntp_.Reset();
  remote_ssrc_ = ssrc;
}

void RTCPReceiver::set_local_media_ssrc(uint32_t ssrc) {
  registered_ssrcs_.set_media_ssrc(ssrc);
}

uint32_t RTCPReceiver::local_media_ssrc() const {
  return registered_ssrcs_.media_ssrc();
}

uint32_t RTCPReceiver::RemoteSSRC() const {
  MutexLock lock(&rtcp_receiver_lock_);
  return remote_ssrc_;
}

void RTCPReceiver::RttStats::AddRtt(TimeDelta rtt) {
  last_rtt_ = rtt;
  if (rtt < min_rtt_) {
    min_rtt_ = rtt;
  }
  if (rtt > max_rtt_) {
    max_rtt_ = rtt;
  }
  sum_rtt_ += rtt;
  ++num_rtts_;
}

int32_t RTCPReceiver::RTT(uint32_t remote_ssrc,
                          int64_t* last_rtt_ms,
                          int64_t* avg_rtt_ms,
                          int64_t* min_rtt_ms,
                          int64_t* max_rtt_ms) const {
  MutexLock lock(&rtcp_receiver_lock_);

  auto it = rtts_.find(remote_ssrc);
  if (it == rtts_.end()) {
    return -1;
  }

  if (last_rtt_ms) {
    *last_rtt_ms = it->second.last_rtt().ms();
  }

  if (avg_rtt_ms) {
    *avg_rtt_ms = it->second.average_rtt().ms();
  }

  if (min_rtt_ms) {
    *min_rtt_ms = it->second.min_rtt().ms();
  }

  if (max_rtt_ms) {
    *max_rtt_ms = it->second.max_rtt().ms();
  }

  return 0;
}

RTCPReceiver::NonSenderRttStats RTCPReceiver::GetNonSenderRTT() const {
  MutexLock lock(&rtcp_receiver_lock_);
  auto it = non_sender_rtts_.find(remote_ssrc_);
  if (it == non_sender_rtts_.end()) {
    return {};
  }
  return it->second;
}

void RTCPReceiver::SetNonSenderRttMeasurement(bool enabled) {
  MutexLock lock(&rtcp_receiver_lock_);
  xr_rrtr_status_ = enabled;
}

bool RTCPReceiver::GetAndResetXrRrRtt(int64_t* rtt_ms) {
  RTC_DCHECK(rtt_ms);
  MutexLock lock(&rtcp_receiver_lock_);
  if (xr_rr_rtt_ms_ == 0) {
    return false;
  }
  *rtt_ms = xr_rr_rtt_ms_;
  xr_rr_rtt_ms_ = 0;
  return true;
}

absl::optional<TimeDelta> RTCPReceiver::OnPeriodicRttUpdate(
    Timestamp newer_than,
    bool sending) {

  absl::optional<TimeDelta> rtt;

  if (sending) {


    MutexLock lock(&rtcp_receiver_lock_);
    if (last_received_rb_.IsInfinite() || last_received_rb_ > newer_than) {
      TimeDelta max_rtt = TimeDelta::MinusInfinity();
      for (const auto& rtt_stats : rtts_) {
        if (rtt_stats.second.last_rtt() > max_rtt) {
          max_rtt = rtt_stats.second.last_rtt();
        }
      }
      if (max_rtt.IsFinite()) {
        rtt = max_rtt;
      }
    }

    auto now = clock_->CurrentTime();
    if (RtcpRrTimeoutLocked(now)) {
      RTC_LOG_F(LS_WARNING) << "Timeout: No RTCP RR received.";
    } else if (RtcpRrSequenceNumberTimeoutLocked(now)) {
      RTC_LOG_F(LS_WARNING) << "Timeout: No increase in RTCP RR extended "
                               "highest sequence number.";
    }
  } else {

    int64_t rtt_ms;
    if (GetAndResetXrRrRtt(&rtt_ms)) {
      rtt.emplace(TimeDelta::Millis(rtt_ms));
    }
  }

  return rtt;
}

bool RTCPReceiver::NTP(uint32_t* received_ntp_secs,
                       uint32_t* received_ntp_frac,
                       uint32_t* rtcp_arrival_time_secs,
                       uint32_t* rtcp_arrival_time_frac,
                       uint32_t* rtcp_timestamp,
                       uint32_t* remote_sender_packet_count,
                       uint64_t* remote_sender_octet_count,
                       uint64_t* remote_sender_reports_count) const {
  MutexLock lock(&rtcp_receiver_lock_);
  if (!last_received_sr_ntp_.Valid())
    return false;

  if (received_ntp_secs)
    *received_ntp_secs = remote_sender_ntp_time_.seconds();
  if (received_ntp_frac)
    *received_ntp_frac = remote_sender_ntp_time_.fractions();

  if (rtcp_timestamp)
    *rtcp_timestamp = remote_sender_rtp_time_;

  if (rtcp_arrival_time_secs)
    *rtcp_arrival_time_secs = last_received_sr_ntp_.seconds();
  if (rtcp_arrival_time_frac)
    *rtcp_arrival_time_frac = last_received_sr_ntp_.fractions();

  if (remote_sender_packet_count)
    *remote_sender_packet_count = remote_sender_packet_count_;
  if (remote_sender_octet_count)
    *remote_sender_octet_count = remote_sender_octet_count_;
  if (remote_sender_reports_count)
    *remote_sender_reports_count = remote_sender_reports_count_;

  return true;
}

std::vector<rtcp::ReceiveTimeInfo>
RTCPReceiver::ConsumeReceivedXrReferenceTimeInfo() {
  MutexLock lock(&rtcp_receiver_lock_);

  const size_t last_xr_rtis_size = std::min(
      received_rrtrs_.size(), rtcp::ExtendedReports::kMaxNumberOfDlrrItems);
  std::vector<rtcp::ReceiveTimeInfo> last_xr_rtis;
  last_xr_rtis.reserve(last_xr_rtis_size);

  const uint32_t now_ntp = CompactNtp(clock_->CurrentNtpTime());

  for (size_t i = 0; i < last_xr_rtis_size; ++i) {
    RrtrInformation& rrtr = received_rrtrs_.front();
    last_xr_rtis.emplace_back(rrtr.ssrc, rrtr.received_remote_mid_ntp_time,
                              now_ntp - rrtr.local_receive_mid_ntp_time);
    received_rrtrs_ssrc_it_.erase(rrtr.ssrc);
    received_rrtrs_.pop_front();
  }

  return last_xr_rtis;
}

std::vector<ReportBlockData> RTCPReceiver::GetLatestReportBlockData() const {
  std::vector<ReportBlockData> result;
  MutexLock lock(&rtcp_receiver_lock_);
  for (const auto& report : received_report_blocks_) {
    result.push_back(report.second);
  }
  return result;
}

bool RTCPReceiver::ParseCompoundPacket(rtc::ArrayView<const uint8_t> packet,
                                       PacketInformation* packet_information) {
  MutexLock lock(&rtcp_receiver_lock_);

  CommonHeader rtcp_block;



  struct RtcpReceivedBlock {
    bool sender_report = false;
    bool dlrr = false;
  };


  flat_map<uint32_t, RtcpReceivedBlock> received_blocks;
  for (const uint8_t* next_block = packet.begin(); next_block != packet.end();
       next_block = rtcp_block.NextPacket()) {
    ptrdiff_t remaining_blocks_size = packet.end() - next_block;
    RTC_DCHECK_GT(remaining_blocks_size, 0);
    if (!rtcp_block.Parse(next_block, remaining_blocks_size)) {
      if (next_block == packet.begin()) {

        RTC_LOG(LS_WARNING) << "Incoming invalid RTCP packet";
        return false;
      }
      ++num_skipped_packets_;
      break;
    }

    switch (rtcp_block.type()) {
      case rtcp::SenderReport::kPacketType:
        HandleSenderReport(rtcp_block, packet_information);
        received_blocks[packet_information->remote_ssrc].sender_report = true;
        break;
      case rtcp::ReceiverReport::kPacketType:
        HandleReceiverReport(rtcp_block, packet_information);
        break;
      case rtcp::Sdes::kPacketType:
        HandleSdes(rtcp_block, packet_information);
        break;
      case rtcp::ExtendedReports::kPacketType: {
        bool contains_dlrr = false;
        uint32_t ssrc = 0;
        HandleXr(rtcp_block, packet_information, contains_dlrr, ssrc);
        if (contains_dlrr) {
          received_blocks[ssrc].dlrr = true;
        }
        break;
      }
      case rtcp::Bye::kPacketType:
        HandleBye(rtcp_block);
        break;
      case rtcp::App::kPacketType:
        HandleApp(rtcp_block, packet_information);
        break;
      case rtcp::Rtpfb::kPacketType:
        switch (rtcp_block.fmt()) {
          case rtcp::Nack::kFeedbackMessageType:
            HandleNack(rtcp_block, packet_information);
            break;
          case rtcp::Tmmbr::kFeedbackMessageType:
            HandleTmmbr(rtcp_block, packet_information);
            break;
          case rtcp::Tmmbn::kFeedbackMessageType:
            HandleTmmbn(rtcp_block, packet_information);
            break;
          case rtcp::RapidResyncRequest::kFeedbackMessageType:
            HandleSrReq(rtcp_block, packet_information);
            break;
          case rtcp::TransportFeedback::kFeedbackMessageType:
            HandleTransportFeedback(rtcp_block, packet_information);
            break;
          default:
            ++num_skipped_packets_;
            break;
        }
        break;
      case rtcp::Psfb::kPacketType:
        switch (rtcp_block.fmt()) {
          case rtcp::Pli::kFeedbackMessageType:
            HandlePli(rtcp_block, packet_information);
            break;
          case rtcp::Fir::kFeedbackMessageType:
            HandleFir(rtcp_block, packet_information);
            break;
          case rtcp::Psfb::kAfbMessageType:
            HandlePsfbApp(rtcp_block, packet_information);
            break;
          default:
            ++num_skipped_packets_;
            break;
        }
        break;
      default:
        ++num_skipped_packets_;
        break;
    }
  }

  for (const auto& rb : received_blocks) {
    if (rb.second.sender_report && !rb.second.dlrr) {
      auto rtt_stats = non_sender_rtts_.find(rb.first);
      if (rtt_stats != non_sender_rtts_.end()) {
        rtt_stats->second.Invalidate();
      }
    }
  }

  if (packet_type_counter_observer_) {
    packet_type_counter_observer_->RtcpPacketTypesCounterUpdated(
        local_media_ssrc(), packet_type_counter_);
  }

  if (num_skipped_packets_ > 0) {
    const int64_t now_ms = clock_->TimeInMilliseconds();
    if (now_ms - last_skipped_packets_warning_ms_ >= kMaxWarningLogIntervalMs) {
      last_skipped_packets_warning_ms_ = now_ms;
      RTC_LOG(LS_WARNING)
          << num_skipped_packets_
          << " RTCP blocks were skipped due to being malformed or of "
             "unrecognized/unsupported type, during the past "
          << (kMaxWarningLogIntervalMs / 1000) << " second period.";
    }
  }

  return true;
}

void RTCPReceiver::HandleSenderReport(const CommonHeader& rtcp_block,
                                      PacketInformation* packet_information) {
  rtcp::SenderReport sender_report;
  if (!sender_report.Parse(rtcp_block)) {
    ++num_skipped_packets_;
    return;
  }

  const uint32_t remote_ssrc = sender_report.sender_ssrc();

  packet_information->remote_ssrc = remote_ssrc;

  UpdateTmmbrRemoteIsAlive(remote_ssrc);

  if (remote_ssrc_ == remote_ssrc) {

    packet_information->packet_type_flags |= kRtcpSr;

    remote_sender_ntp_time_ = sender_report.ntp();
    remote_sender_rtp_time_ = sender_report.rtp_timestamp();
    last_received_sr_ntp_ = clock_->CurrentNtpTime();
    remote_sender_packet_count_ = sender_report.sender_packet_count();
    remote_sender_octet_count_ = sender_report.sender_octet_count();
    remote_sender_reports_count_++;
  } else {


    packet_information->packet_type_flags |= kRtcpRr;
  }

  for (const rtcp::ReportBlock& report_block : sender_report.report_blocks())
    HandleReportBlock(report_block, packet_information, remote_ssrc);
}

void RTCPReceiver::HandleReceiverReport(const CommonHeader& rtcp_block,
                                        PacketInformation* packet_information) {
  rtcp::ReceiverReport receiver_report;
  if (!receiver_report.Parse(rtcp_block)) {
    ++num_skipped_packets_;
    return;
  }

  const uint32_t remote_ssrc = receiver_report.sender_ssrc();

  packet_information->remote_ssrc = remote_ssrc;

  UpdateTmmbrRemoteIsAlive(remote_ssrc);

  packet_information->packet_type_flags |= kRtcpRr;

  for (const ReportBlock& report_block : receiver_report.report_blocks())
    HandleReportBlock(report_block, packet_information, remote_ssrc);
}

void RTCPReceiver::HandleReportBlock(const ReportBlock& report_block,
                                     PacketInformation* packet_information,
                                     uint32_t remote_ssrc) {








  if (!registered_ssrcs_.contains(report_block.source_ssrc()))
    return;

  last_received_rb_ = clock_->CurrentTime();

  ReportBlockData* report_block_data =
      &received_report_blocks_[report_block.source_ssrc()];
  RTCPReportBlock rtcp_report_block;
  rtcp_report_block.sender_ssrc = remote_ssrc;
  rtcp_report_block.source_ssrc = report_block.source_ssrc();
  rtcp_report_block.fraction_lost = report_block.fraction_lost();
  rtcp_report_block.packets_lost = report_block.cumulative_lost_signed();
  if (report_block.extended_high_seq_num() >
      report_block_data->report_block().extended_highest_sequence_number) {


    last_increased_sequence_number_ = last_received_rb_;
  }
  rtcp_report_block.extended_highest_sequence_number =
      report_block.extended_high_seq_num();
  rtcp_report_block.jitter = report_block.jitter();
  rtcp_report_block.delay_since_last_sender_report =
      report_block.delay_since_last_sr();
  rtcp_report_block.last_sender_report_timestamp = report_block.last_sr();


  report_block_data->SetReportBlock(
      rtcp_report_block,
      (clock_->CurrentNtpInMilliseconds() - rtc::kNtpJan1970Millisecs) *
          rtc::kNumMicrosecsPerMillisec);

  uint32_t send_time_ntp = report_block.last_sr();




  if (send_time_ntp != 0) {
    uint32_t delay_ntp = report_block.delay_since_last_sr();

    uint32_t receive_time_ntp =
        CompactNtp(clock_->ConvertTimestampToNtpTime(last_received_rb_));

    uint32_t rtt_ntp = receive_time_ntp - delay_ntp - send_time_ntp;

    TimeDelta rtt = CompactNtpRttToTimeDelta(rtt_ntp);
    report_block_data->AddRoundTripTimeSample(rtt.ms());
    if (report_block.source_ssrc() == local_media_ssrc()) {
      rtts_[remote_ssrc].AddRtt(rtt);
    }

    packet_information->rtt_ms = rtt.ms();
  }

  packet_information->report_blocks.push_back(
      report_block_data->report_block());
  packet_information->report_block_datas.push_back(*report_block_data);
}

RTCPReceiver::TmmbrInformation* RTCPReceiver::FindOrCreateTmmbrInfo(
    uint32_t remote_ssrc) {

  TmmbrInformation* tmmbr_info = &tmmbr_infos_[remote_ssrc];

  tmmbr_info->last_time_received_ms = clock_->TimeInMilliseconds();
  return tmmbr_info;
}

void RTCPReceiver::UpdateTmmbrRemoteIsAlive(uint32_t remote_ssrc) {
  auto tmmbr_it = tmmbr_infos_.find(remote_ssrc);
  if (tmmbr_it != tmmbr_infos_.end())
    tmmbr_it->second.last_time_received_ms = clock_->TimeInMilliseconds();
}

RTCPReceiver::TmmbrInformation* RTCPReceiver::GetTmmbrInformation(
    uint32_t remote_ssrc) {
  auto it = tmmbr_infos_.find(remote_ssrc);
  if (it == tmmbr_infos_.end())
    return nullptr;
  return &it->second;
}

// for tests and legacy code (rtp_rtcp_impl.cc). We should be able to to delete
// the methods and require that access to the locked variables only happens on
// the worker thread and thus no locking is needed.
bool RTCPReceiver::RtcpRrTimeout() {
  MutexLock lock(&rtcp_receiver_lock_);
  return RtcpRrTimeoutLocked(clock_->CurrentTime());
}

bool RTCPReceiver::RtcpRrSequenceNumberTimeout() {
  MutexLock lock(&rtcp_receiver_lock_);
  return RtcpRrSequenceNumberTimeoutLocked(clock_->CurrentTime());
}

bool RTCPReceiver::UpdateTmmbrTimers() {
  MutexLock lock(&rtcp_receiver_lock_);

  int64_t now_ms = clock_->TimeInMilliseconds();
  int64_t timeout_ms = now_ms - kTmmbrTimeoutIntervalMs;

  if (oldest_tmmbr_info_ms_ >= timeout_ms)
    return false;

  bool update_bounding_set = false;
  oldest_tmmbr_info_ms_ = -1;
  for (auto tmmbr_it = tmmbr_infos_.begin(); tmmbr_it != tmmbr_infos_.end();) {
    TmmbrInformation* tmmbr_info = &tmmbr_it->second;
    if (tmmbr_info->last_time_received_ms > 0) {
      if (tmmbr_info->last_time_received_ms < timeout_ms) {

        tmmbr_info->tmmbr.clear();

        tmmbr_info->last_time_received_ms = 0;

        update_bounding_set = true;
      } else if (oldest_tmmbr_info_ms_ == -1 ||
                 tmmbr_info->last_time_received_ms < oldest_tmmbr_info_ms_) {
        oldest_tmmbr_info_ms_ = tmmbr_info->last_time_received_ms;
      }
      ++tmmbr_it;
    } else if (tmmbr_info->ready_for_delete) {


      tmmbr_it = tmmbr_infos_.erase(tmmbr_it);
    } else {
      ++tmmbr_it;
    }
  }
  return update_bounding_set;
}

std::vector<rtcp::TmmbItem> RTCPReceiver::BoundingSet(bool* tmmbr_owner) {
  MutexLock lock(&rtcp_receiver_lock_);
  TmmbrInformation* tmmbr_info = GetTmmbrInformation(remote_ssrc_);
  if (!tmmbr_info)
    return std::vector<rtcp::TmmbItem>();

  *tmmbr_owner = TMMBRHelp::IsOwner(tmmbr_info->tmmbn, local_media_ssrc());
  return tmmbr_info->tmmbn;
}

void RTCPReceiver::HandleSdes(const CommonHeader& rtcp_block,
                              PacketInformation* packet_information) {
  rtcp::Sdes sdes;
  if (!sdes.Parse(rtcp_block)) {
    ++num_skipped_packets_;
    return;
  }

  for (const rtcp::Sdes::Chunk& chunk : sdes.chunks()) {
    if (cname_callback_)
      cname_callback_->OnCname(chunk.ssrc, chunk.cname);
  }
  packet_information->packet_type_flags |= kRtcpSdes;
}

void RTCPReceiver::HandleNack(const CommonHeader& rtcp_block,
                              PacketInformation* packet_information) {
  rtcp::Nack nack;
  if (!nack.Parse(rtcp_block)) {
    ++num_skipped_packets_;
    return;
  }

  if (receiver_only_ || local_media_ssrc() != nack.media_ssrc())  // Not to us.
    return;

  packet_information->nack_sequence_numbers.insert(
      packet_information->nack_sequence_numbers.end(),
      nack.packet_ids().begin(), nack.packet_ids().end());
  for (uint16_t packet_id : nack.packet_ids())
    nack_stats_.ReportRequest(packet_id);

  if (!nack.packet_ids().empty()) {
    packet_information->packet_type_flags |= kRtcpNack;
    ++packet_type_counter_.nack_packets;
    packet_type_counter_.nack_requests = nack_stats_.requests();
    packet_type_counter_.unique_nack_requests = nack_stats_.unique_requests();
  }
}

void RTCPReceiver::HandleApp(const rtcp::CommonHeader& rtcp_block,
                             PacketInformation* packet_information) {
  rtcp::App app;
  if (app.Parse(rtcp_block)) {
    if (app.name() == rtcp::RemoteEstimate::kName &&
        app.sub_type() == rtcp::RemoteEstimate::kSubType) {
      rtcp::RemoteEstimate estimate(std::move(app));
      if (estimate.ParseData()) {
        packet_information->network_state_estimate = estimate.estimate();
        return;
      }
    }
  }
  ++num_skipped_packets_;
}

void RTCPReceiver::HandleBye(const CommonHeader& rtcp_block) {
  rtcp::Bye bye;
  if (!bye.Parse(rtcp_block)) {
    ++num_skipped_packets_;
    return;
  }

  rtts_.erase(bye.sender_ssrc());
  EraseIf(received_report_blocks_, [&](const auto& elem) {
    return elem.second.report_block().sender_ssrc == bye.sender_ssrc();
  });

  TmmbrInformation* tmmbr_info = GetTmmbrInformation(bye.sender_ssrc());
  if (tmmbr_info)
    tmmbr_info->ready_for_delete = true;

  last_fir_.erase(bye.sender_ssrc());
  auto it = received_rrtrs_ssrc_it_.find(bye.sender_ssrc());
  if (it != received_rrtrs_ssrc_it_.end()) {
    received_rrtrs_.erase(it->second);
    received_rrtrs_ssrc_it_.erase(it);
  }
  xr_rr_rtt_ms_ = 0;
}

void RTCPReceiver::HandleXr(const CommonHeader& rtcp_block,
                            PacketInformation* packet_information,
                            bool& contains_dlrr,
                            uint32_t& ssrc) {
  rtcp::ExtendedReports xr;
  if (!xr.Parse(rtcp_block)) {
    ++num_skipped_packets_;
    return;
  }
  ssrc = xr.sender_ssrc();
  contains_dlrr = !xr.dlrr().sub_blocks().empty();

  if (xr.rrtr())
    HandleXrReceiveReferenceTime(xr.sender_ssrc(), *xr.rrtr());

  for (const rtcp::ReceiveTimeInfo& time_info : xr.dlrr().sub_blocks())
    HandleXrDlrrReportBlock(xr.sender_ssrc(), time_info);

  if (xr.target_bitrate()) {
    HandleXrTargetBitrate(xr.sender_ssrc(), *xr.target_bitrate(),
                          packet_information);
  }
}

void RTCPReceiver::HandleXrReceiveReferenceTime(uint32_t sender_ssrc,
                                                const rtcp::Rrtr& rrtr) {
  uint32_t received_remote_mid_ntp_time = CompactNtp(rrtr.ntp());
  uint32_t local_receive_mid_ntp_time = CompactNtp(clock_->CurrentNtpTime());

  auto it = received_rrtrs_ssrc_it_.find(sender_ssrc);
  if (it != received_rrtrs_ssrc_it_.end()) {
    it->second->received_remote_mid_ntp_time = received_remote_mid_ntp_time;
    it->second->local_receive_mid_ntp_time = local_receive_mid_ntp_time;
  } else {
    if (received_rrtrs_.size() < kMaxNumberOfStoredRrtrs) {
      received_rrtrs_.emplace_back(sender_ssrc, received_remote_mid_ntp_time,
                                   local_receive_mid_ntp_time);
      received_rrtrs_ssrc_it_[sender_ssrc] = std::prev(received_rrtrs_.end());
    } else {
      RTC_LOG(LS_WARNING) << "Discarding received RRTR for ssrc " << sender_ssrc
                          << ", reached maximum number of stored RRTRs.";
    }
  }
}

void RTCPReceiver::HandleXrDlrrReportBlock(uint32_t sender_ssrc,
                                           const rtcp::ReceiveTimeInfo& rti) {
  if (!registered_ssrcs_.contains(rti.ssrc))  // Not to us.
    return;

  if (!xr_rrtr_status_)
    return;

  uint32_t send_time_ntp = rti.last_rr;


  if (send_time_ntp == 0) {
    auto rtt_stats = non_sender_rtts_.find(sender_ssrc);
    if (rtt_stats != non_sender_rtts_.end()) {
      rtt_stats->second.Invalidate();
    }
    return;
  }

  uint32_t delay_ntp = rti.delay_since_last_rr;
  uint32_t now_ntp = CompactNtp(clock_->CurrentNtpTime());

  uint32_t rtt_ntp = now_ntp - delay_ntp - send_time_ntp;
  TimeDelta rtt = CompactNtpRttToTimeDelta(rtt_ntp);
  xr_rr_rtt_ms_ = rtt.ms();

  non_sender_rtts_[sender_ssrc].Update(rtt);
}

void RTCPReceiver::HandleXrTargetBitrate(
    uint32_t ssrc,
    const rtcp::TargetBitrate& target_bitrate,
    PacketInformation* packet_information) {
  if (ssrc != remote_ssrc_) {
    return;  // Not for us.
  }

  VideoBitrateAllocation bitrate_allocation;
  for (const auto& item : target_bitrate.GetTargetBitrates()) {
    if (item.spatial_layer >= kMaxSpatialLayers ||
        item.temporal_layer >= kMaxTemporalStreams) {
      RTC_LOG(LS_WARNING)
          << "Invalid layer in XR target bitrate pack: spatial index "
          << item.spatial_layer << ", temporal index " << item.temporal_layer
          << ", dropping.";
    } else {
      bitrate_allocation.SetBitrate(item.spatial_layer, item.temporal_layer,
                                    item.target_bitrate_kbps * 1000);
    }
  }
  packet_information->target_bitrate_allocation.emplace(bitrate_allocation);
}

void RTCPReceiver::HandlePli(const CommonHeader& rtcp_block,
                             PacketInformation* packet_information) {
  rtcp::Pli pli;
  if (!pli.Parse(rtcp_block)) {
    ++num_skipped_packets_;
    return;
  }

  if (local_media_ssrc() == pli.media_ssrc()) {
    ++packet_type_counter_.pli_packets;

    packet_information->packet_type_flags |= kRtcpPli;
  }
}

void RTCPReceiver::HandleTmmbr(const CommonHeader& rtcp_block,
                               PacketInformation* packet_information) {
  rtcp::Tmmbr tmmbr;
  if (!tmmbr.Parse(rtcp_block)) {
    ++num_skipped_packets_;
    return;
  }

  uint32_t sender_ssrc = tmmbr.sender_ssrc();
  if (tmmbr.media_ssrc()) {


    sender_ssrc = tmmbr.media_ssrc();
  }

  for (const rtcp::TmmbItem& request : tmmbr.requests()) {
    if (local_media_ssrc() != request.ssrc() || request.bitrate_bps() == 0)
      continue;

    TmmbrInformation* tmmbr_info = FindOrCreateTmmbrInfo(tmmbr.sender_ssrc());
    auto* entry = &tmmbr_info->tmmbr[sender_ssrc];
    entry->tmmbr_item = rtcp::TmmbItem(sender_ssrc, request.bitrate_bps(),
                                       request.packet_overhead());


    entry->last_updated_ms = tmmbr_info->last_time_received_ms;

    packet_information->packet_type_flags |= kRtcpTmmbr;
    break;
  }
}

void RTCPReceiver::HandleTmmbn(const CommonHeader& rtcp_block,
                               PacketInformation* packet_information) {
  rtcp::Tmmbn tmmbn;
  if (!tmmbn.Parse(rtcp_block)) {
    ++num_skipped_packets_;
    return;
  }

  TmmbrInformation* tmmbr_info = FindOrCreateTmmbrInfo(tmmbn.sender_ssrc());

  packet_information->packet_type_flags |= kRtcpTmmbn;

  tmmbr_info->tmmbn = tmmbn.items();
}

void RTCPReceiver::HandleSrReq(const CommonHeader& rtcp_block,
                               PacketInformation* packet_information) {
  rtcp::RapidResyncRequest sr_req;
  if (!sr_req.Parse(rtcp_block)) {
    ++num_skipped_packets_;
    return;
  }

  packet_information->packet_type_flags |= kRtcpSrReq;
}

void RTCPReceiver::HandlePsfbApp(const CommonHeader& rtcp_block,
                                 PacketInformation* packet_information) {
  {
    rtcp::Remb remb;
    if (remb.Parse(rtcp_block)) {
      packet_information->packet_type_flags |= kRtcpRemb;
      packet_information->receiver_estimated_max_bitrate_bps =
          remb.bitrate_bps();
      return;
    }
  }

  {
    auto loss_notification = std::make_unique<rtcp::LossNotification>();
    if (loss_notification->Parse(rtcp_block)) {
      packet_information->packet_type_flags |= kRtcpLossNotification;
      packet_information->loss_notification = std::move(loss_notification);
      return;
    }
  }

  RTC_LOG(LS_WARNING) << "Unknown PSFB-APP packet.";

  ++num_skipped_packets_;
}

void RTCPReceiver::HandleFir(const CommonHeader& rtcp_block,
                             PacketInformation* packet_information) {
  rtcp::Fir fir;
  if (!fir.Parse(rtcp_block)) {
    ++num_skipped_packets_;
    return;
  }

  if (fir.requests().empty())
    return;

  const int64_t now_ms = clock_->TimeInMilliseconds();
  for (const rtcp::Fir::Request& fir_request : fir.requests()) {

    if (local_media_ssrc() != fir_request.ssrc)
      continue;

    ++packet_type_counter_.fir_packets;

    auto inserted = last_fir_.insert(std::make_pair(
        fir.sender_ssrc(), LastFirStatus(now_ms, fir_request.seq_nr)));
    if (!inserted.second) {  // There was already an entry.
      LastFirStatus* last_fir = &inserted.first->second;

      if (fir_request.seq_nr == last_fir->sequence_number)
        continue;

      if (now_ms - last_fir->request_ms < kRtcpMinFrameLengthMs)
        continue;

      last_fir->request_ms = now_ms;
      last_fir->sequence_number = fir_request.seq_nr;
    }

    packet_information->packet_type_flags |= kRtcpFir;
  }
}

void RTCPReceiver::HandleTransportFeedback(
    const CommonHeader& rtcp_block,
    PacketInformation* packet_information) {
  std::unique_ptr<rtcp::TransportFeedback> transport_feedback(
      new rtcp::TransportFeedback());
  if (!transport_feedback->Parse(rtcp_block)) {
    ++num_skipped_packets_;
    return;
  }

  packet_information->packet_type_flags |= kRtcpTransportFeedback;
  packet_information->transport_feedback = std::move(transport_feedback);
}

void RTCPReceiver::NotifyTmmbrUpdated() {

  std::vector<rtcp::TmmbItem> bounding =
      TMMBRHelp::FindBoundingSet(TmmbrReceived());

  if (!bounding.empty() && rtcp_bandwidth_observer_) {

    uint64_t bitrate_bps = TMMBRHelp::CalcMinBitrateBps(bounding);
    if (bitrate_bps <= std::numeric_limits<uint32_t>::max())
      rtcp_bandwidth_observer_->OnReceivedEstimatedBitrate(bitrate_bps);
  }

  rtp_rtcp_->SetTmmbn(std::move(bounding));
}

void RTCPReceiver::TriggerCallbacksFromRtcpPacket(
    const PacketInformation& packet_information) {


  if (packet_information.packet_type_flags & kRtcpTmmbr) {

    NotifyTmmbrUpdated();
  }

  if (!receiver_only_ && (packet_information.packet_type_flags & kRtcpSrReq)) {
    rtp_rtcp_->OnRequestSendReport();
  }
  if (!receiver_only_ && (packet_information.packet_type_flags & kRtcpNack)) {
    if (!packet_information.nack_sequence_numbers.empty()) {
      RTC_LOG(LS_VERBOSE) << "Incoming NACK length: "
                          << packet_information.nack_sequence_numbers.size();
      rtp_rtcp_->OnReceivedNack(packet_information.nack_sequence_numbers);
    }
  }




  if (rtcp_intra_frame_observer_) {
    RTC_DCHECK(!receiver_only_);
    if ((packet_information.packet_type_flags & kRtcpPli) ||
        (packet_information.packet_type_flags & kRtcpFir)) {
      if (packet_information.packet_type_flags & kRtcpPli) {
        RTC_LOG(LS_VERBOSE)
            << "Incoming PLI from SSRC " << packet_information.remote_ssrc;
      } else {
        RTC_LOG(LS_VERBOSE)
            << "Incoming FIR from SSRC " << packet_information.remote_ssrc;
      }
      rtcp_intra_frame_observer_->OnReceivedIntraFrameRequest(
          local_media_ssrc());
    }
  }
  if (rtcp_loss_notification_observer_ &&
      (packet_information.packet_type_flags & kRtcpLossNotification)) {
    rtcp::LossNotification* loss_notification =
        packet_information.loss_notification.get();
    RTC_DCHECK(loss_notification);
    if (loss_notification->media_ssrc() == local_media_ssrc()) {
      rtcp_loss_notification_observer_->OnReceivedLossNotification(
          loss_notification->media_ssrc(), loss_notification->last_decoded(),
          loss_notification->last_received(),
          loss_notification->decodability_flag());
    }
  }
  if (rtcp_bandwidth_observer_) {
    RTC_DCHECK(!receiver_only_);
    if (packet_information.packet_type_flags & kRtcpRemb) {
      RTC_LOG(LS_VERBOSE)
          << "Incoming REMB: "
          << packet_information.receiver_estimated_max_bitrate_bps;
      rtcp_bandwidth_observer_->OnReceivedEstimatedBitrate(
          packet_information.receiver_estimated_max_bitrate_bps);
    }
    if ((packet_information.packet_type_flags & kRtcpSr) ||
        (packet_information.packet_type_flags & kRtcpRr)) {
      int64_t now_ms = clock_->TimeInMilliseconds();
      rtcp_bandwidth_observer_->OnReceivedRtcpReceiverReport(
          packet_information.report_blocks, packet_information.rtt_ms, now_ms);
    }
  }
  if ((packet_information.packet_type_flags & kRtcpSr) ||
      (packet_information.packet_type_flags & kRtcpRr)) {
    rtp_rtcp_->OnReceivedRtcpReportBlocks(packet_information.report_blocks);
  }

  if (transport_feedback_observer_ &&
      (packet_information.packet_type_flags & kRtcpTransportFeedback)) {
    uint32_t media_source_ssrc =
        packet_information.transport_feedback->media_ssrc();
    if (media_source_ssrc == local_media_ssrc() ||
        registered_ssrcs_.contains(media_source_ssrc)) {
      transport_feedback_observer_->OnTransportFeedback(
          *packet_information.transport_feedback);
    }
  }

  if (network_state_estimate_observer_ &&
      packet_information.network_state_estimate) {
    network_state_estimate_observer_->OnRemoteNetworkEstimate(
        *packet_information.network_state_estimate);
  }

  if (bitrate_allocation_observer_ &&
      packet_information.target_bitrate_allocation) {
    bitrate_allocation_observer_->OnBitrateAllocationUpdated(
        *packet_information.target_bitrate_allocation);
  }

  if (!receiver_only_) {
    if (report_block_data_observer_) {
      for (const auto& report_block_data :
           packet_information.report_block_datas) {
        report_block_data_observer_->OnReportBlockDataUpdated(
            report_block_data);
      }
    }
  }
}

std::vector<rtcp::TmmbItem> RTCPReceiver::TmmbrReceived() {
  MutexLock lock(&rtcp_receiver_lock_);
  std::vector<rtcp::TmmbItem> candidates;

  int64_t now_ms = clock_->TimeInMilliseconds();
  int64_t timeout_ms = now_ms - kTmmbrTimeoutIntervalMs;

  for (auto& kv : tmmbr_infos_) {
    for (auto it = kv.second.tmmbr.begin(); it != kv.second.tmmbr.end();) {
      if (it->second.last_updated_ms < timeout_ms) {

        it = kv.second.tmmbr.erase(it);
      } else {
        candidates.push_back(it->second.tmmbr_item);
        ++it;
      }
    }
  }
  return candidates;
}

bool RTCPReceiver::RtcpRrTimeoutLocked(Timestamp now) {
  return ResetTimestampIfExpired(now, last_received_rb_, report_interval_);
}

bool RTCPReceiver::RtcpRrSequenceNumberTimeoutLocked(Timestamp now) {
  return ResetTimestampIfExpired(now, last_increased_sequence_number_,
                                 report_interval_);
}

}  // namespace webrtc
