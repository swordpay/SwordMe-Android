/*
 *  Copyright 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "pc/data_channel_controller.h"

#include <utility>

#include "api/peer_connection_interface.h"
#include "api/rtc_error.h"
#include "pc/peer_connection_internal.h"
#include "pc/sctp_utils.h"
#include "rtc_base/logging.h"

namespace webrtc {

DataChannelController::~DataChannelController() {



  for (auto channel : sctp_data_channels_) {
    channel->DetachFromController();
  }
}

bool DataChannelController::HasDataChannels() const {
  RTC_DCHECK_RUN_ON(signaling_thread());
  return !sctp_data_channels_.empty();
}

bool DataChannelController::SendData(int sid,
                                     const SendDataParams& params,
                                     const rtc::CopyOnWriteBuffer& payload,
                                     cricket::SendDataResult* result) {
  if (data_channel_transport())
    return DataChannelSendData(sid, params, payload, result);
  RTC_LOG(LS_ERROR) << "SendData called before transport is ready";
  return false;
}

bool DataChannelController::ConnectDataChannel(
    SctpDataChannel* webrtc_data_channel) {
  RTC_DCHECK_RUN_ON(signaling_thread());
  if (!data_channel_transport()) {



    return false;
  }
  SignalDataChannelTransportWritable_s.connect(
      webrtc_data_channel, &SctpDataChannel::OnTransportReady);
  SignalDataChannelTransportReceivedData_s.connect(
      webrtc_data_channel, &SctpDataChannel::OnDataReceived);
  SignalDataChannelTransportChannelClosing_s.connect(
      webrtc_data_channel, &SctpDataChannel::OnClosingProcedureStartedRemotely);
  SignalDataChannelTransportChannelClosed_s.connect(
      webrtc_data_channel, &SctpDataChannel::OnClosingProcedureComplete);
  return true;
}

void DataChannelController::DisconnectDataChannel(
    SctpDataChannel* webrtc_data_channel) {
  RTC_DCHECK_RUN_ON(signaling_thread());
  if (!data_channel_transport()) {
    RTC_LOG(LS_ERROR)
        << "DisconnectDataChannel called when sctp_transport_ is NULL.";
    return;
  }
  SignalDataChannelTransportWritable_s.disconnect(webrtc_data_channel);
  SignalDataChannelTransportReceivedData_s.disconnect(webrtc_data_channel);
  SignalDataChannelTransportChannelClosing_s.disconnect(webrtc_data_channel);
  SignalDataChannelTransportChannelClosed_s.disconnect(webrtc_data_channel);
}

void DataChannelController::AddSctpDataStream(int sid) {
  if (data_channel_transport()) {
    network_thread()->BlockingCall([this, sid] {
      if (data_channel_transport()) {
        data_channel_transport()->OpenChannel(sid);
      }
    });
  }
}

void DataChannelController::RemoveSctpDataStream(int sid) {
  if (data_channel_transport()) {
    network_thread()->BlockingCall([this, sid] {
      if (data_channel_transport()) {
        data_channel_transport()->CloseChannel(sid);
      }
    });
  }
}

bool DataChannelController::ReadyToSendData() const {
  RTC_DCHECK_RUN_ON(signaling_thread());
  return (data_channel_transport() && data_channel_transport_ready_to_send_);
}

void DataChannelController::OnDataReceived(
    int channel_id,
    DataMessageType type,
    const rtc::CopyOnWriteBuffer& buffer) {
  RTC_DCHECK_RUN_ON(network_thread());
  cricket::ReceiveDataParams params;
  params.sid = channel_id;
  params.type = type;
  signaling_thread()->PostTask(
      [self = weak_factory_.GetWeakPtr(), params, buffer] {
        if (self) {
          RTC_DCHECK_RUN_ON(self->signaling_thread());








          if (!self->HandleOpenMessage_s(params, buffer)) {
            self->SignalDataChannelTransportReceivedData_s(params, buffer);
          }
        }
      });
}

void DataChannelController::OnChannelClosing(int channel_id) {
  RTC_DCHECK_RUN_ON(network_thread());
  signaling_thread()->PostTask([self = weak_factory_.GetWeakPtr(), channel_id] {
    if (self) {
      RTC_DCHECK_RUN_ON(self->signaling_thread());
      self->SignalDataChannelTransportChannelClosing_s(channel_id);
    }
  });
}

void DataChannelController::OnChannelClosed(int channel_id) {
  RTC_DCHECK_RUN_ON(network_thread());
  signaling_thread()->PostTask([self = weak_factory_.GetWeakPtr(), channel_id] {
    if (self) {
      RTC_DCHECK_RUN_ON(self->signaling_thread());
      self->SignalDataChannelTransportChannelClosed_s(channel_id);
    }
  });
}

void DataChannelController::OnReadyToSend() {
  RTC_DCHECK_RUN_ON(network_thread());
  signaling_thread()->PostTask([self = weak_factory_.GetWeakPtr()] {
    if (self) {
      RTC_DCHECK_RUN_ON(self->signaling_thread());
      self->data_channel_transport_ready_to_send_ = true;
      self->SignalDataChannelTransportWritable_s(
          self->data_channel_transport_ready_to_send_);
    }
  });
}

void DataChannelController::OnTransportClosed(RTCError error) {
  RTC_DCHECK_RUN_ON(network_thread());
  signaling_thread()->PostTask([self = weak_factory_.GetWeakPtr(), error] {
    if (self) {
      RTC_DCHECK_RUN_ON(self->signaling_thread());
      self->OnTransportChannelClosed(error);
    }
  });
}

void DataChannelController::SetupDataChannelTransport_n() {
  RTC_DCHECK_RUN_ON(network_thread());



  NotifyDataChannelsOfTransportCreated();
}

void DataChannelController::TeardownDataChannelTransport_n() {
  RTC_DCHECK_RUN_ON(network_thread());
  if (data_channel_transport()) {
    data_channel_transport()->SetDataSink(nullptr);
  }
  set_data_channel_transport(nullptr);
}

void DataChannelController::OnTransportChanged(
    DataChannelTransportInterface* new_data_channel_transport) {
  RTC_DCHECK_RUN_ON(network_thread());
  if (data_channel_transport() &&
      data_channel_transport() != new_data_channel_transport) {


    data_channel_transport()->SetDataSink(nullptr);
    set_data_channel_transport(new_data_channel_transport);
    if (new_data_channel_transport) {
      new_data_channel_transport->SetDataSink(this);



      NotifyDataChannelsOfTransportCreated();
    }
  }
}

std::vector<DataChannelStats> DataChannelController::GetDataChannelStats()
    const {
  RTC_DCHECK_RUN_ON(signaling_thread());
  std::vector<DataChannelStats> stats;
  stats.reserve(sctp_data_channels_.size());
  for (const auto& channel : sctp_data_channels_)
    stats.push_back(channel->GetStats());
  return stats;
}

bool DataChannelController::HandleOpenMessage_s(
    const cricket::ReceiveDataParams& params,
    const rtc::CopyOnWriteBuffer& buffer) {
  if (params.type == DataMessageType::kControl && IsOpenMessage(buffer)) {


    std::string label;
    InternalDataChannelInit config;
    config.id = params.sid;
    if (!ParseDataChannelOpenMessage(buffer, &label, &config)) {
      RTC_LOG(LS_WARNING) << "Failed to parse the OPEN message for sid "
                          << params.sid;
      return true;
    }
    config.open_handshake_role = InternalDataChannelInit::kAcker;
    OnDataChannelOpenMessage(label, config);
    return true;
  }
  return false;
}

void DataChannelController::OnDataChannelOpenMessage(
    const std::string& label,
    const InternalDataChannelInit& config) {
  rtc::scoped_refptr<DataChannelInterface> channel(
      InternalCreateDataChannelWithProxy(label, &config));
  if (!channel.get()) {
    RTC_LOG(LS_ERROR) << "Failed to create DataChannel from the OPEN message.";
    return;
  }

  pc_->Observer()->OnDataChannel(std::move(channel));
  pc_->NoteDataAddedEvent();
}

rtc::scoped_refptr<DataChannelInterface>
DataChannelController::InternalCreateDataChannelWithProxy(
    const std::string& label,
    const InternalDataChannelInit* config) {
  RTC_DCHECK_RUN_ON(signaling_thread());
  if (pc_->IsClosed()) {
    return nullptr;
  }

  rtc::scoped_refptr<SctpDataChannel> channel =
      InternalCreateSctpDataChannel(label, config);
  if (channel) {
    return SctpDataChannel::CreateProxy(channel);
  }

  return nullptr;
}

rtc::scoped_refptr<SctpDataChannel>
DataChannelController::InternalCreateSctpDataChannel(
    const std::string& label,
    const InternalDataChannelInit* config) {
  RTC_DCHECK_RUN_ON(signaling_thread());
  InternalDataChannelInit new_config =
      config ? (*config) : InternalDataChannelInit();
  if (new_config.id < 0) {
    rtc::SSLRole role;
    if ((pc_->GetSctpSslRole(&role)) &&
        !sid_allocator_.AllocateSid(role, &new_config.id)) {
      RTC_LOG(LS_ERROR) << "No id can be allocated for the SCTP data channel.";
      return nullptr;
    }
  } else if (!sid_allocator_.ReserveSid(new_config.id)) {
    RTC_LOG(LS_ERROR) << "Failed to create a SCTP data channel "
                         "because the id is already in use or out of range.";
    return nullptr;
  }
  rtc::scoped_refptr<SctpDataChannel> channel(SctpDataChannel::Create(
      this, label, new_config, signaling_thread(), network_thread()));
  if (!channel) {
    sid_allocator_.ReleaseSid(new_config.id);
    return nullptr;
  }
  sctp_data_channels_.push_back(channel);
  channel->SignalClosed.connect(
      pc_, &PeerConnectionInternal::OnSctpDataChannelClosed);
  SignalSctpDataChannelCreated_(channel.get());
  return channel;
}

void DataChannelController::AllocateSctpSids(rtc::SSLRole role) {
  RTC_DCHECK_RUN_ON(signaling_thread());
  std::vector<rtc::scoped_refptr<SctpDataChannel>> channels_to_close;
  for (const auto& channel : sctp_data_channels_) {
    if (channel->id() < 0) {
      int sid;
      if (!sid_allocator_.AllocateSid(role, &sid)) {
        RTC_LOG(LS_ERROR) << "Failed to allocate SCTP sid, closing channel.";
        channels_to_close.push_back(channel);
        continue;
      }
      channel->SetSctpSid(sid);
    }
  }


  for (const auto& channel : channels_to_close) {
    channel->CloseAbruptlyWithDataChannelFailure("Failed to allocate SCTP SID");
  }
}

void DataChannelController::OnSctpDataChannelClosed(SctpDataChannel* channel) {
  RTC_DCHECK_RUN_ON(signaling_thread());
  for (auto it = sctp_data_channels_.begin(); it != sctp_data_channels_.end();
       ++it) {
    if (it->get() == channel) {
      if (channel->id() >= 0) {


        sid_allocator_.ReleaseSid(channel->id());
      }


      sctp_data_channels_to_free_.push_back(*it);
      sctp_data_channels_.erase(it);
      signaling_thread()->PostTask([self = weak_factory_.GetWeakPtr()] {
        if (self) {
          RTC_DCHECK_RUN_ON(self->signaling_thread());
          self->sctp_data_channels_to_free_.clear();
        }
      });
      return;
    }
  }
}

void DataChannelController::OnTransportChannelClosed(RTCError error) {
  RTC_DCHECK_RUN_ON(signaling_thread());


  std::vector<rtc::scoped_refptr<SctpDataChannel>> temp_sctp_dcs;
  temp_sctp_dcs.swap(sctp_data_channels_);
  for (const auto& channel : temp_sctp_dcs) {
    channel->OnTransportChannelClosed(error);
  }
}

DataChannelTransportInterface* DataChannelController::data_channel_transport()
    const {



  return data_channel_transport_;
}

void DataChannelController::set_data_channel_transport(
    DataChannelTransportInterface* transport) {
  RTC_DCHECK_RUN_ON(network_thread());
  data_channel_transport_ = transport;
}

bool DataChannelController::DataChannelSendData(
    int sid,
    const SendDataParams& params,
    const rtc::CopyOnWriteBuffer& payload,
    cricket::SendDataResult* result) {



  RTC_DCHECK_RUN_ON(signaling_thread());
  RTC_DCHECK(data_channel_transport());

  RTCError error = network_thread()->BlockingCall([this, sid, params, payload] {
    return data_channel_transport()->SendData(sid, params, payload);
  });

  if (error.ok()) {
    *result = cricket::SendDataResult::SDR_SUCCESS;
    return true;
  } else if (error.type() == RTCErrorType::RESOURCE_EXHAUSTED) {


    *result = cricket::SendDataResult::SDR_BLOCK;
    return false;
  }
  *result = cricket::SendDataResult::SDR_ERROR;
  return false;
}

void DataChannelController::NotifyDataChannelsOfTransportCreated() {
  RTC_DCHECK_RUN_ON(network_thread());
  signaling_thread()->PostTask([self = weak_factory_.GetWeakPtr()] {
    if (self) {
      RTC_DCHECK_RUN_ON(self->signaling_thread());
      for (const auto& channel : self->sctp_data_channels_) {
        channel->OnTransportChannelCreated();
      }
    }
  });
}

rtc::Thread* DataChannelController::network_thread() const {
  return pc_->network_thread();
}
rtc::Thread* DataChannelController::signaling_thread() const {
  return pc_->signaling_thread();
}

}  // namespace webrtc
