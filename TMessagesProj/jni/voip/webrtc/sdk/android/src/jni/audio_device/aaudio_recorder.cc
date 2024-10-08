/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "sdk/android/src/jni/audio_device/aaudio_recorder.h"

#include <memory>

#include "api/array_view.h"
#include "modules/audio_device/fine_audio_buffer.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/time_utils.h"

namespace webrtc {

namespace jni {

AAudioRecorder::AAudioRecorder(const AudioParameters& audio_parameters)
    : main_thread_(TaskQueueBase::Current()),
      aaudio_(audio_parameters, AAUDIO_DIRECTION_INPUT, this) {
  RTC_LOG(LS_INFO) << "ctor";
  thread_checker_aaudio_.Detach();
}

AAudioRecorder::~AAudioRecorder() {
  RTC_LOG(LS_INFO) << "dtor";
  RTC_DCHECK(thread_checker_.IsCurrent());
  Terminate();
  RTC_LOG(LS_INFO) << "detected owerflows: " << overflow_count_;
}

int AAudioRecorder::Init() {
  RTC_LOG(LS_INFO) << "Init";
  RTC_DCHECK(thread_checker_.IsCurrent());
  if (aaudio_.audio_parameters().channels() == 2) {
    RTC_DLOG(LS_WARNING) << "Stereo mode is enabled";
  }
  return 0;
}

int AAudioRecorder::Terminate() {
  RTC_LOG(LS_INFO) << "Terminate";
  RTC_DCHECK(thread_checker_.IsCurrent());
  StopRecording();
  return 0;
}

int AAudioRecorder::InitRecording() {
  RTC_LOG(LS_INFO) << "InitRecording";
  RTC_DCHECK(thread_checker_.IsCurrent());
  RTC_DCHECK(!initialized_);
  RTC_DCHECK(!recording_);
  if (!aaudio_.Init()) {
    return -1;
  }
  initialized_ = true;
  return 0;
}

bool AAudioRecorder::RecordingIsInitialized() const {
  return initialized_;
}

int AAudioRecorder::StartRecording() {
  RTC_LOG(LS_INFO) << "StartRecording";
  RTC_DCHECK(thread_checker_.IsCurrent());
  RTC_DCHECK(initialized_);
  RTC_DCHECK(!recording_);
  if (fine_audio_buffer_) {
    fine_audio_buffer_->ResetPlayout();
  }
  if (!aaudio_.Start()) {
    return -1;
  }
  overflow_count_ = aaudio_.xrun_count();
  first_data_callback_ = true;
  recording_ = true;
  return 0;
}

int AAudioRecorder::StopRecording() {
  RTC_LOG(LS_INFO) << "StopRecording";
  RTC_DCHECK(thread_checker_.IsCurrent());
  if (!initialized_ || !recording_) {
    return 0;
  }
  if (!aaudio_.Stop()) {
    return -1;
  }
  thread_checker_aaudio_.Detach();
  initialized_ = false;
  recording_ = false;
  return 0;
}

bool AAudioRecorder::Recording() const {
  return recording_;
}

void AAudioRecorder::AttachAudioBuffer(AudioDeviceBuffer* audioBuffer) {
  RTC_LOG(LS_INFO) << "AttachAudioBuffer";
  RTC_DCHECK(thread_checker_.IsCurrent());
  audio_device_buffer_ = audioBuffer;
  const AudioParameters audio_parameters = aaudio_.audio_parameters();
  audio_device_buffer_->SetRecordingSampleRate(audio_parameters.sample_rate());
  audio_device_buffer_->SetRecordingChannels(audio_parameters.channels());
  RTC_CHECK(audio_device_buffer_);



  fine_audio_buffer_ = std::make_unique<FineAudioBuffer>(audio_device_buffer_);
}

bool AAudioRecorder::IsAcousticEchoCancelerSupported() const {
  return false;
}

bool AAudioRecorder::IsNoiseSuppressorSupported() const {
  return false;
}

int AAudioRecorder::EnableBuiltInAEC(bool enable) {
  RTC_LOG(LS_INFO) << "EnableBuiltInAEC: " << enable;
  RTC_LOG(LS_ERROR) << "Not implemented";
  return -1;
}

int AAudioRecorder::EnableBuiltInNS(bool enable) {
  RTC_LOG(LS_INFO) << "EnableBuiltInNS: " << enable;
  RTC_LOG(LS_ERROR) << "Not implemented";
  return -1;
}

void AAudioRecorder::OnErrorCallback(aaudio_result_t error) {
  RTC_LOG(LS_ERROR) << "OnErrorCallback: " << AAudio_convertResultToText(error);

  if (aaudio_.stream_state() == AAUDIO_STREAM_STATE_DISCONNECTED) {


    RTC_LOG(LS_WARNING) << "Input stream disconnected => restart is required";



    RTC_DCHECK(main_thread_);
    main_thread_->PostTask([this] { HandleStreamDisconnected(); });
  }
}

// TODO(henrika): possibly add trace here to be included in systrace.
// See https://developer.android.com/studio/profile/systrace-commandline.html.
aaudio_data_callback_result_t AAudioRecorder::OnDataCallback(
    void* audio_data,
    int32_t num_frames) {






  if (first_data_callback_) {
    RTC_LOG(LS_INFO) << "--- First input data callback: "
                        "device id="
                     << aaudio_.device_id();
    aaudio_.ClearInputStream(audio_data, num_frames);
    first_data_callback_ = false;
  }


  const int32_t overflow_count = aaudio_.xrun_count();
  if (overflow_count > overflow_count_) {
    RTC_LOG(LS_ERROR) << "Overflow detected: " << overflow_count;
    overflow_count_ = overflow_count;
  }


  latency_millis_ = aaudio_.EstimateLatencyMillis();

  if (aaudio_.frames_read() % (1000 * aaudio_.frames_per_burst()) == 0) {
    RTC_DLOG(LS_INFO) << "input latency: " << latency_millis_
                      << ", num_frames: " << num_frames;
  }


  fine_audio_buffer_->DeliverRecordedData(
      rtc::MakeArrayView(static_cast<const int16_t*>(audio_data),
                         aaudio_.samples_per_frame() * num_frames),
      static_cast<int>(latency_millis_ + 0.5));

  return AAUDIO_CALLBACK_RESULT_CONTINUE;
}

void AAudioRecorder::HandleStreamDisconnected() {
  RTC_DCHECK_RUN_ON(&thread_checker_);
  RTC_LOG(LS_INFO) << "HandleStreamDisconnected";
  if (!initialized_ || !recording_) {
    return;
  }





  StopRecording();
  InitRecording();
  StartRecording();
}

}  // namespace jni

}  // namespace webrtc
