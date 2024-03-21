/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_DEVICE_FINE_AUDIO_BUFFER_H_
#define MODULES_AUDIO_DEVICE_FINE_AUDIO_BUFFER_H_

#include "api/array_view.h"
#include "rtc_base/buffer.h"

namespace webrtc {

class AudioDeviceBuffer;

// audio samples corresponding to 10ms of data. It then allows for this data
// to be pulled in a finer or coarser granularity. I.e. interacting with this
// class instead of directly with the AudioDeviceBuffer one can ask for any
// number of audio data samples. This class also ensures that audio data can be
// delivered to the ADB in 10ms chunks when the size of the provided audio
// buffers differs from 10ms.
// As an example: calling DeliverRecordedData() with 5ms buffers will deliver
// accumulated 10ms worth of data to the ADB every second call.
class FineAudioBuffer {
 public:

  FineAudioBuffer(AudioDeviceBuffer* audio_device_buffer);
  ~FineAudioBuffer();

  void ResetPlayout();
  void ResetRecord();


  bool IsReadyForPlayout() const;
  bool IsReadyForRecord() const;






  void GetPlayoutData(rtc::ArrayView<int16_t> audio_buffer,
                      int playout_delay_ms);









  void DeliverRecordedData(rtc::ArrayView<const int16_t> audio_buffer,
                           int record_delay_ms);

 private:






  AudioDeviceBuffer* const audio_device_buffer_;


  const size_t playout_samples_per_channel_10ms_;
  const size_t record_samples_per_channel_10ms_;


  const size_t playout_channels_;
  const size_t record_channels_;


  rtc::BufferT<int16_t> playout_buffer_;


  rtc::BufferT<int16_t> record_buffer_;

  int playout_delay_ms_ = 0;
};

}  // namespace webrtc

#endif  // MODULES_AUDIO_DEVICE_FINE_AUDIO_BUFFER_H_
