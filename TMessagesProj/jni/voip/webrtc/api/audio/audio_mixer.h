/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_AUDIO_AUDIO_MIXER_H_
#define API_AUDIO_AUDIO_MIXER_H_

#include <memory>

#include "api/audio/audio_frame.h"
#include "rtc_base/ref_count.h"

namespace webrtc {

// This class is under development and is not yet intended for for use outside
// of WebRtc/Libjingle.
class AudioMixer : public rtc::RefCountInterface {
 public:

  class Source {
   public:
    enum class AudioFrameInfo {
      kNormal,  // The samples in audio_frame are valid and should be used.
      kMuted,   // The samples in audio_frame should not be used, but



      kError,   // The audio_frame will not be used.
    };



    virtual AudioFrameInfo GetAudioFrameWithInfo(int sample_rate_hz,
                                                 AudioFrame* audio_frame) = 0;

    virtual int Ssrc() const = 0;


    virtual int PreferredSampleRate() const = 0;

    virtual ~Source() {}
  };


  virtual bool AddSource(Source* audio_source) = 0;


  virtual void RemoveSource(Source* audio_source) = 0;








  virtual void Mix(size_t number_of_channels,
                   AudioFrame* audio_frame_for_mixing) = 0;

 protected:


  ~AudioMixer() override {}
};
}  // namespace webrtc

#endif  // API_AUDIO_AUDIO_MIXER_H_
