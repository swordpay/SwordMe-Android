/*
 *  Copyright 2020 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef PC_SCTP_DATA_CHANNEL_H_
#define PC_SCTP_DATA_CHANNEL_H_

#include <stdint.h>

#include <memory>
#include <set>
#include <string>

#include "absl/types/optional.h"
#include "api/data_channel_interface.h"
#include "api/priority.h"
#include "api/rtc_error.h"
#include "api/scoped_refptr.h"
#include "api/transport/data_channel_transport_interface.h"
#include "media/base/media_channel.h"
#include "pc/data_channel_utils.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "rtc_base/ssl_stream_adapter.h"  // For SSLRole
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "rtc_base/thread.h"
#include "rtc_base/thread_annotations.h"

namespace webrtc {

class SctpDataChannel;

// SctpTransportInternal (pure virtual SctpTransport interface) instead.
class SctpDataChannelControllerInterface {
 public:

  virtual bool SendData(int sid,
                        const SendDataParams& params,
                        const rtc::CopyOnWriteBuffer& payload,
                        cricket::SendDataResult* result) = 0;

  virtual bool ConnectDataChannel(SctpDataChannel* data_channel) = 0;

  virtual void DisconnectDataChannel(SctpDataChannel* data_channel) = 0;

  virtual void AddSctpDataStream(int sid) = 0;


  virtual void RemoveSctpDataStream(int sid) = 0;

  virtual bool ReadyToSendData() const = 0;

 protected:
  virtual ~SctpDataChannelControllerInterface() {}
};

// a const member. Block access to the 'id' member since it cannot be const.
struct InternalDataChannelInit : public DataChannelInit {
  enum OpenHandshakeRole { kOpener, kAcker, kNone };

  InternalDataChannelInit() : open_handshake_role(kOpener) {}
  explicit InternalDataChannelInit(const DataChannelInit& base);
  OpenHandshakeRole open_handshake_role;
};

class SctpSidAllocator {
 public:




  bool AllocateSid(rtc::SSLRole role, int* sid);

  bool ReserveSid(int sid);

  void ReleaseSid(int sid);

 private:

  bool IsSidAvailable(int sid) const;

  std::set<int> used_sids_;
};

// SctpTransport. It provides an implementation of unreliable or
// reliabledata channels.

// kConnecting: The channel has been created the transport might not yet be
//              ready.
// kOpen: The open handshake has been performed (if relevant) and the data
//        channel is able to send messages.
// kClosing: DataChannelInterface::Close has been called, or the remote side
//           initiated the closing procedure, but the closing procedure has not
//           yet finished.
// kClosed: The closing handshake is finished (possibly initiated from this,
//          side, possibly from the peer).
//
// How the closing procedure works for SCTP:
// 1. Alice calls Close(), state changes to kClosing.
// 2. Alice finishes sending any queued data.
// 3. Alice calls RemoveSctpDataStream, sends outgoing stream reset.
// 4. Bob receives incoming stream reset; OnClosingProcedureStartedRemotely
//    called.
// 5. Bob sends outgoing stream reset.
// 6. Alice receives incoming reset, Bob receives acknowledgement. Both receive
//    OnClosingProcedureComplete callback and transition to kClosed.
class SctpDataChannel : public DataChannelInterface,
                        public sigslot::has_slots<> {
 public:
  static rtc::scoped_refptr<SctpDataChannel> Create(
      SctpDataChannelControllerInterface* controller,
      const std::string& label,
      const InternalDataChannelInit& config,
      rtc::Thread* signaling_thread,
      rtc::Thread* network_thread);


  static rtc::scoped_refptr<DataChannelInterface> CreateProxy(
      rtc::scoped_refptr<SctpDataChannel> channel);

  void DetachFromController();

  void RegisterObserver(DataChannelObserver* observer) override;
  void UnregisterObserver() override;

  std::string label() const override { return label_; }
  bool reliable() const override;
  bool ordered() const override { return config_.ordered; }

  uint16_t maxRetransmitTime() const override {
    return config_.maxRetransmitTime ? *config_.maxRetransmitTime
                                     : static_cast<uint16_t>(-1);
  }
  uint16_t maxRetransmits() const override {
    return config_.maxRetransmits ? *config_.maxRetransmits
                                  : static_cast<uint16_t>(-1);
  }
  absl::optional<int> maxPacketLifeTime() const override {
    return config_.maxRetransmitTime;
  }
  absl::optional<int> maxRetransmitsOpt() const override {
    return config_.maxRetransmits;
  }
  std::string protocol() const override { return config_.protocol; }
  bool negotiated() const override { return config_.negotiated; }
  int id() const override { return config_.id; }
  Priority priority() const override {
    return config_.priority ? *config_.priority : Priority::kLow;
  }

  virtual int internal_id() const { return internal_id_; }

  uint64_t buffered_amount() const override;
  void Close() override;
  DataState state() const override;
  RTCError error() const override;
  uint32_t messages_sent() const override;
  uint64_t bytes_sent() const override;
  uint32_t messages_received() const override;
  uint64_t bytes_received() const override;
  bool Send(const DataBuffer& buffer) override;



  void CloseAbruptlyWithError(RTCError error);

  void CloseAbruptlyWithDataChannelFailure(const std::string& message);







  void OnTransportReady(bool writable);

  void OnDataReceived(const cricket::ReceiveDataParams& params,
                      const rtc::CopyOnWriteBuffer& payload);


  void SetSctpSid(int sid);


  void OnClosingProcedureStartedRemotely(int sid);



  void OnClosingProcedureComplete(int sid);


  void OnTransportChannelCreated();



  void OnTransportChannelClosed(RTCError error);

  DataChannelStats GetStats() const;

  sigslot::signal1<DataChannelInterface*> SignalOpened;


  sigslot::signal1<DataChannelInterface*> SignalClosed;


  static void ResetInternalIdAllocatorForTesting(int new_value);

 protected:
  SctpDataChannel(const InternalDataChannelInit& config,
                  SctpDataChannelControllerInterface* client,
                  const std::string& label,
                  rtc::Thread* signaling_thread,
                  rtc::Thread* network_thread);
  ~SctpDataChannel() override;

 private:

  enum HandshakeState {
    kHandshakeInit,
    kHandshakeShouldSendOpen,
    kHandshakeShouldSendAck,
    kHandshakeWaitingForAck,
    kHandshakeReady
  };

  bool Init();
  void UpdateState();
  void SetState(DataState state);
  void DisconnectFromTransport();

  void DeliverQueuedReceivedData();

  void SendQueuedDataMessages();
  bool SendDataMessage(const DataBuffer& buffer, bool queue_if_blocked);
  bool QueueSendDataMessage(const DataBuffer& buffer);

  void SendQueuedControlMessages();
  void QueueControlMessage(const rtc::CopyOnWriteBuffer& buffer);
  bool SendControlMessage(const rtc::CopyOnWriteBuffer& buffer);

  rtc::Thread* const signaling_thread_;
  rtc::Thread* const network_thread_;
  const int internal_id_;
  const std::string label_;
  const InternalDataChannelInit config_;
  DataChannelObserver* observer_ RTC_GUARDED_BY(signaling_thread_) = nullptr;
  DataState state_ RTC_GUARDED_BY(signaling_thread_) = kConnecting;
  RTCError error_ RTC_GUARDED_BY(signaling_thread_);
  uint32_t messages_sent_ RTC_GUARDED_BY(signaling_thread_) = 0;
  uint64_t bytes_sent_ RTC_GUARDED_BY(signaling_thread_) = 0;
  uint32_t messages_received_ RTC_GUARDED_BY(signaling_thread_) = 0;
  uint64_t bytes_received_ RTC_GUARDED_BY(signaling_thread_) = 0;
  SctpDataChannelControllerInterface* const controller_
      RTC_GUARDED_BY(signaling_thread_);
  bool controller_detached_ RTC_GUARDED_BY(signaling_thread_) = false;
  HandshakeState handshake_state_ RTC_GUARDED_BY(signaling_thread_) =
      kHandshakeInit;
  bool connected_to_transport_ RTC_GUARDED_BY(signaling_thread_) = false;
  bool writable_ RTC_GUARDED_BY(signaling_thread_) = false;

  bool started_closing_procedure_ RTC_GUARDED_BY(signaling_thread_) = false;


  PacketQueue queued_control_data_ RTC_GUARDED_BY(signaling_thread_);
  PacketQueue queued_received_data_ RTC_GUARDED_BY(signaling_thread_);
  PacketQueue queued_send_data_ RTC_GUARDED_BY(signaling_thread_);
};

// to its underlying SctpDataChannel object. For testing only.
SctpDataChannel* DowncastProxiedDataChannelInterfaceToSctpDataChannelForTesting(
    DataChannelInterface* channel);

}  // namespace webrtc

#endif  // PC_SCTP_DATA_CHANNEL_H_
