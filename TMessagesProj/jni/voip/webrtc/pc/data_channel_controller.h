/*
 *  Copyright 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef PC_DATA_CHANNEL_CONTROLLER_H_
#define PC_DATA_CHANNEL_CONTROLLER_H_

#include <string>
#include <vector>

#include "api/data_channel_interface.h"
#include "api/rtc_error.h"
#include "api/scoped_refptr.h"
#include "api/sequence_checker.h"
#include "api/transport/data_channel_transport_interface.h"
#include "media/base/media_channel.h"
#include "pc/data_channel_utils.h"
#include "pc/sctp_data_channel.h"
#include "rtc_base/checks.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "rtc_base/ssl_stream_adapter.h"
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "rtc_base/thread.h"
#include "rtc_base/thread_annotations.h"
#include "rtc_base/weak_ptr.h"

namespace webrtc {

class PeerConnectionInternal;

class DataChannelController : public SctpDataChannelControllerInterface,
                              public DataChannelSink {
 public:
  explicit DataChannelController(PeerConnectionInternal* pc) : pc_(pc) {}
  ~DataChannelController();

  DataChannelController(DataChannelController&) = delete;
  DataChannelController& operator=(const DataChannelController& other) = delete;
  DataChannelController(DataChannelController&&) = delete;
  DataChannelController& operator=(DataChannelController&& other) = delete;


  bool SendData(int sid,
                const SendDataParams& params,
                const rtc::CopyOnWriteBuffer& payload,
                cricket::SendDataResult* result) override;
  bool ConnectDataChannel(SctpDataChannel* webrtc_data_channel) override;
  void DisconnectDataChannel(SctpDataChannel* webrtc_data_channel) override;
  void AddSctpDataStream(int sid) override;
  void RemoveSctpDataStream(int sid) override;
  bool ReadyToSendData() const override;

  void OnDataReceived(int channel_id,
                      DataMessageType type,
                      const rtc::CopyOnWriteBuffer& buffer) override;
  void OnChannelClosing(int channel_id) override;
  void OnChannelClosed(int channel_id) override;
  void OnReadyToSend() override;
  void OnTransportClosed(RTCError error) override;

  void SetupDataChannelTransport_n();

  void TeardownDataChannelTransport_n();


  void OnTransportChanged(
      DataChannelTransportInterface* data_channel_transport);

  std::vector<DataChannelStats> GetDataChannelStats() const;


  rtc::scoped_refptr<DataChannelInterface> InternalCreateDataChannelWithProxy(
      const std::string& label,
      const InternalDataChannelInit*
          config) /* RTC_RUN_ON(signaling_thread()) */;
  void AllocateSctpSids(rtc::SSLRole role);

  bool HasDataChannels() const;
  bool HasSctpDataChannels() const {
    RTC_DCHECK_RUN_ON(signaling_thread());
    return !sctp_data_channels_.empty();
  }

  DataChannelTransportInterface* data_channel_transport() const;
  void set_data_channel_transport(DataChannelTransportInterface* transport);

  sigslot::signal1<SctpDataChannel*>& SignalSctpDataChannelCreated() {
    RTC_DCHECK_RUN_ON(signaling_thread());
    return SignalSctpDataChannelCreated_;
  }

  void OnTransportChannelClosed(RTCError error);

  void OnSctpDataChannelClosed(SctpDataChannel* channel);

 private:
  rtc::scoped_refptr<SctpDataChannel> InternalCreateSctpDataChannel(
      const std::string& label,
      const InternalDataChannelInit*
          config) /* RTC_RUN_ON(signaling_thread()) */;


  bool HandleOpenMessage_s(const cricket::ReceiveDataParams& params,
                           const rtc::CopyOnWriteBuffer& buffer)
      RTC_RUN_ON(signaling_thread());

  void OnDataChannelOpenMessage(const std::string& label,
                                const InternalDataChannelInit& config)
      RTC_RUN_ON(signaling_thread());

  bool DataChannelSendData(int sid,
                           const SendDataParams& params,
                           const rtc::CopyOnWriteBuffer& payload,
                           cricket::SendDataResult* result);


  void NotifyDataChannelsOfTransportCreated();

  rtc::Thread* network_thread() const;
  rtc::Thread* signaling_thread() const;





  DataChannelTransportInterface* data_channel_transport_ = nullptr;

  bool data_channel_transport_ready_to_send_
      RTC_GUARDED_BY(signaling_thread()) = false;

  SctpSidAllocator sid_allocator_ /* RTC_GUARDED_BY(signaling_thread()) */;
  std::vector<rtc::scoped_refptr<SctpDataChannel>> sctp_data_channels_
      RTC_GUARDED_BY(signaling_thread());
  std::vector<rtc::scoped_refptr<SctpDataChannel>> sctp_data_channels_to_free_
      RTC_GUARDED_BY(signaling_thread());




  sigslot::signal1<bool> SignalDataChannelTransportWritable_s
      RTC_GUARDED_BY(signaling_thread());
  sigslot::signal2<const cricket::ReceiveDataParams&,
                   const rtc::CopyOnWriteBuffer&>
      SignalDataChannelTransportReceivedData_s
          RTC_GUARDED_BY(signaling_thread());
  sigslot::signal1<int> SignalDataChannelTransportChannelClosing_s
      RTC_GUARDED_BY(signaling_thread());
  sigslot::signal1<int> SignalDataChannelTransportChannelClosed_s
      RTC_GUARDED_BY(signaling_thread());

  sigslot::signal1<SctpDataChannel*> SignalSctpDataChannelCreated_
      RTC_GUARDED_BY(signaling_thread());

  PeerConnectionInternal* const pc_;


  rtc::WeakPtrFactory<DataChannelController> weak_factory_{this};
};

}  // namespace webrtc

#endif  // PC_DATA_CHANNEL_CONTROLLER_H_
