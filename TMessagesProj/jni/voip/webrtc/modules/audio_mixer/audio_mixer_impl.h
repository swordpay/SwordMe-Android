/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_MIXER_AUDIO_MIXER_IMPL_H_
#define MODULES_AUDIO_MIXER_AUDIO_MIXER_IMPL_H_

#include <stddef.h>

#include <memory>
#include <vector>

#include "api/array_view.h"
#include "api/audio/audio_frame.h"
#include "api/audio/audio_mixer.h"
#include "api/scoped_refptr.h"
#include "modules/audio_mixer/frame_combiner.h"
#include "modules/audio_mixer/output_rate_calculator.h"
#include "rtc_base/race_checker.h"
#include "rtc_base/synchronization/mutex.h"
#include "rtc_base/thread_annotations.h"

namespace webrtc {

class AudioMixerImpl : public AudioMixer {
 public:
  struct SourceStatus;

  static const int kFrameDurationInMs = 10;

  static const int kDefaultNumberOfMixedAudioSources = 3;

  static rtc::scoped_refptr<AudioMixerImpl> Create(
      int max_sources_to_mix = kDefaultNumberOfMixedAudioSources);

  static rtc::scoped_refptr<AudioMixerImpl> Create(
      std::unique_ptr<OutputRateCalculator> output_rate_calculator,
      bool use_limiter,
      int max_sources_to_mix = kDefaultNumberOfMixedAudioSources);

  ~AudioMixerImpl() override;

  AudioMixerImpl(const AudioMixerImpl&) = delete;
  AudioMixerImpl& operator=(const AudioMixerImpl&) = delete;

  bool AddSource(Source* audio_source) override;
  void RemoveSource(Source* audio_source) override;

  void Mix(size_t number_of_channels,
           AudioFrame* audio_frame_for_mixing) override
      RTC_LOCKS_EXCLUDED(mutex_);



  bool GetAudioSourceMixabilityStatusForTest(Source* audio_source) const;

 protected:
  AudioMixerImpl(std::unique_ptr<OutputRateCalculator> output_rate_calculator,
                 bool use_limiter,
                 int max_sources_to_mix);

 private:
  struct HelperContainers;



  rtc::ArrayView<AudioFrame* const> GetAudioFromSources(int output_frequency)
      RTC_EXCLUSIVE_LOCKS_REQUIRED(mutex_);



  mutable Mutex mutex_;

  const int max_sources_to_mix_;

  std::unique_ptr<OutputRateCalculator> output_rate_calculator_;

  std::vector<std::unique_ptr<SourceStatus>> audio_source_list_
      RTC_GUARDED_BY(mutex_);
  const std::unique_ptr<HelperContainers> helper_containers_
      RTC_GUARDED_BY(mutex_);

  FrameCombiner frame_combiner_;
};
}  // namespace webrtc

#endif  // MODULES_AUDIO_MIXER_AUDIO_MIXER_IMPL_H_
