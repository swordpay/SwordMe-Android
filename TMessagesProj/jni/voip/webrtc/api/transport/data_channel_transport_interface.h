/* Copyright 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#ifndef API_TRANSPORT_DATA_CHANNEL_TRANSPORT_INTERFACE_H_
#define API_TRANSPORT_DATA_CHANNEL_TRANSPORT_INTERFACE_H_

#include "absl/types/optional.h"
#include "api/rtc_error.h"
#include "rtc_base/copy_on_write_buffer.h"

namespace webrtc {

enum class DataMessageType {

  kText,

  kBinary,

  kControl,
};

// message, even within a single channel.  For example, control messages may be
// sent reliably and in-order, even if the data channel is configured for
// unreliable delivery.
struct SendDataParams {
  DataMessageType type = DataMessageType::kText;


  bool ordered = false;





  absl::optional<int> max_rtx_count;





  absl::optional<int> max_rtx_ms;
};

class DataChannelSink {
 public:
  virtual ~DataChannelSink() = default;

  virtual void OnDataReceived(int channel_id,
                              DataMessageType type,
                              const rtc::CopyOnWriteBuffer& buffer) = 0;


  virtual void OnChannelClosing(int channel_id) = 0;



  virtual void OnChannelClosed(int channel_id) = 0;





  virtual void OnReadyToSend() = 0;



  virtual void OnTransportClosed(RTCError error) {}
};

class DataChannelTransportInterface {
 public:
  virtual ~DataChannelTransportInterface() = default;


  virtual RTCError OpenChannel(int channel_id) = 0;



  virtual RTCError SendData(int channel_id,
                            const SendDataParams& params,
                            const rtc::CopyOnWriteBuffer& buffer) = 0;



  virtual RTCError CloseChannel(int channel_id) = 0;



  virtual void SetDataSink(DataChannelSink* sink) = 0;



  virtual bool IsReadyToSend() const = 0;
};

}  // namespace webrtc

#endif  // API_TRANSPORT_DATA_CHANNEL_TRANSPORT_INTERFACE_H_
