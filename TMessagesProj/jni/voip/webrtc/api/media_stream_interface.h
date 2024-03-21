/*
 *  Copyright 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// These interfaces are used for implementing MediaStream and MediaTrack as
// defined in http://dev.w3.org/2011/webrtc/editor/webrtc.html#stream-api. These
// interfaces must be used only with PeerConnection.

#ifndef API_MEDIA_STREAM_INTERFACE_H_
#define API_MEDIA_STREAM_INTERFACE_H_

#include <stddef.h>

#include <string>
#include <vector>

#include "absl/types/optional.h"
#include "api/audio_options.h"
#include "api/scoped_refptr.h"
#include "api/video/recordable_encoded_frame.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"
#include "api/video/video_source_interface.h"
#include "api/video_track_source_constraints.h"
#include "modules/audio_processing/include/audio_processing_statistics.h"
#include "rtc_base/ref_count.h"
#include "rtc_base/system/rtc_export.h"

namespace webrtc {

class ObserverInterface {
 public:
  virtual void OnChanged() = 0;

 protected:
  virtual ~ObserverInterface() {}
};

class NotifierInterface {
 public:
  virtual void RegisterObserver(ObserverInterface* observer) = 0;
  virtual void UnregisterObserver(ObserverInterface* observer) = 0;

  virtual ~NotifierInterface() {}
};

// provides media. A source can be shared by multiple tracks.
class RTC_EXPORT MediaSourceInterface : public rtc::RefCountInterface,
                                        public NotifierInterface {
 public:
  enum SourceState { kInitializing, kLive, kEnded, kMuted };

  virtual SourceState state() const = 0;

  virtual bool remote() const = 0;

 protected:
  ~MediaSourceInterface() override = default;
};

// See: https://www.w3.org/TR/mediacapture-streams/#mediastreamtrack
class RTC_EXPORT MediaStreamTrackInterface : public rtc::RefCountInterface,
                                             public NotifierInterface {
 public:
  enum TrackState {
    kLive,
    kEnded,
  };

  static const char* const kAudioKind;
  static const char* const kVideoKind;




  virtual std::string kind() const = 0;

  virtual std::string id() const = 0;


  virtual bool enabled() const = 0;
  virtual bool set_enabled(bool enable) = 0;

  virtual TrackState state() const = 0;

 protected:
  ~MediaStreamTrackInterface() override = default;
};

// VideoTracks. The same source can be used by multiple VideoTracks.
// VideoTrackSourceInterface is designed to be invoked on the signaling thread
// except for rtc::VideoSourceInterface<VideoFrame> methods that will be invoked
// on the worker thread via a VideoTrack. A custom implementation of a source
// can inherit AdaptedVideoTrackSource instead of directly implementing this
// interface.
class VideoTrackSourceInterface : public MediaSourceInterface,
                                  public rtc::VideoSourceInterface<VideoFrame> {
 public:
  struct Stats {

    int input_width;
    int input_height;
  };





  virtual bool is_screencast() const = 0;





  virtual absl::optional<bool> needs_denoising() const = 0;




  virtual bool GetStats(Stats* stats) = 0;

  virtual bool SupportsEncodedOutput() const = 0;


  virtual void GenerateKeyFrame() = 0;



  virtual void AddEncodedSink(
      rtc::VideoSinkInterface<RecordableEncodedFrame>* sink) = 0;

  virtual void RemoveEncodedSink(
      rtc::VideoSinkInterface<RecordableEncodedFrame>* sink) = 0;




  virtual void ProcessConstraints(
      const webrtc::VideoTrackSourceConstraints& constraints) {}

 protected:
  ~VideoTrackSourceInterface() override = default;
};

// for rtc::VideoSourceInterface<VideoFrame> methods that must be invoked
// on the worker thread.
// PeerConnectionFactory::CreateVideoTrack can be used for creating a VideoTrack
// that ensures thread safety and that all methods are called on the right
// thread.
class RTC_EXPORT VideoTrackInterface
    : public MediaStreamTrackInterface,
      public rtc::VideoSourceInterface<VideoFrame> {
 public:



  enum class ContentHint { kNone, kFluid, kDetailed, kText };


  void AddOrUpdateSink(rtc::VideoSinkInterface<VideoFrame>* sink,
                       const rtc::VideoSinkWants& wants) override {}
  void RemoveSink(rtc::VideoSinkInterface<VideoFrame>* sink) override {}

  virtual VideoTrackSourceInterface* GetSource() const = 0;

  virtual ContentHint content_hint() const;
  virtual void set_content_hint(ContentHint hint) {}

 protected:
  ~VideoTrackInterface() override = default;
};

class AudioTrackSinkInterface {
 public:
  virtual void OnData(const void* audio_data,
                      int bits_per_sample,
                      int sample_rate,
                      size_t number_of_channels,
                      size_t number_of_frames) {
    RTC_DCHECK_NOTREACHED() << "This method must be overridden, or not used.";
  }




  virtual void OnData(const void* audio_data,
                      int bits_per_sample,
                      int sample_rate,
                      size_t number_of_channels,
                      size_t number_of_frames,
                      absl::optional<int64_t> absolute_capture_timestamp_ms) {


    return OnData(audio_data, bits_per_sample, sample_rate, number_of_channels,
                  number_of_frames);
  }



  virtual int NumPreferredChannels() const { return -1; }

 protected:
  virtual ~AudioTrackSinkInterface() {}
};

// The same source can be used by multiple AudioTracks.
class RTC_EXPORT AudioSourceInterface : public MediaSourceInterface {
 public:
  class AudioObserver {
   public:
    virtual void OnSetVolume(double volume) = 0;

   protected:
    virtual ~AudioObserver() {}
  };





  virtual void SetVolume(double volume) {}

  virtual void RegisterAudioObserver(AudioObserver* observer) {}
  virtual void UnregisterAudioObserver(AudioObserver* observer) {}

  virtual void AddSink(AudioTrackSinkInterface* sink) {}
  virtual void RemoveSink(AudioTrackSinkInterface* sink) {}



  virtual const cricket::AudioOptions options() const;
};

// statistics.
class AudioProcessorInterface : public rtc::RefCountInterface {
 public:
  struct AudioProcessorStatistics {
    bool typing_noise_detected = false;
    AudioProcessingStats apm_statistics;
  };





  virtual AudioProcessorStatistics GetStats(bool has_remote_tracks) = 0;

 protected:
  ~AudioProcessorInterface() override = default;
};

class RTC_EXPORT AudioTrackInterface : public MediaStreamTrackInterface {
 public:


  virtual AudioSourceInterface* GetSource() const = 0;

  virtual void AddSink(AudioTrackSinkInterface* sink) = 0;
  virtual void RemoveSink(AudioTrackSinkInterface* sink) = 0;




  virtual bool GetSignalLevel(int* level);



  virtual rtc::scoped_refptr<AudioProcessorInterface> GetAudioProcessor();

 protected:
  ~AudioTrackInterface() override = default;
};

typedef std::vector<rtc::scoped_refptr<AudioTrackInterface> > AudioTrackVector;
typedef std::vector<rtc::scoped_refptr<VideoTrackInterface> > VideoTrackVector;

//
// A major difference is that remote audio/video tracks (received by a
// PeerConnection/RtpReceiver) are not synchronized simply by adding them to
// the same stream; a session description with the correct "a=msid" attributes
// must be pushed down.
//
// Thus, this interface acts as simply a container for tracks.
class MediaStreamInterface : public rtc::RefCountInterface,
                             public NotifierInterface {
 public:
  virtual std::string id() const = 0;

  virtual AudioTrackVector GetAudioTracks() = 0;
  virtual VideoTrackVector GetVideoTracks() = 0;
  virtual rtc::scoped_refptr<AudioTrackInterface> FindAudioTrack(
      const std::string& track_id) = 0;
  virtual rtc::scoped_refptr<VideoTrackInterface> FindVideoTrack(
      const std::string& track_id) = 0;




  virtual bool AddTrack(rtc::scoped_refptr<AudioTrackInterface> track) {
    RTC_CHECK_NOTREACHED();
  }
  virtual bool AddTrack(rtc::scoped_refptr<VideoTrackInterface> track) {
    RTC_CHECK_NOTREACHED();
  }
  virtual bool RemoveTrack(rtc::scoped_refptr<AudioTrackInterface> track) {
    RTC_CHECK_NOTREACHED();
  }
  virtual bool RemoveTrack(rtc::scoped_refptr<VideoTrackInterface> track) {
    RTC_CHECK_NOTREACHED();
  }

  [[deprecated("Pass a scoped_refptr")]] virtual bool AddTrack(
      AudioTrackInterface* track) {
    return AddTrack(rtc::scoped_refptr<AudioTrackInterface>(track));
  }
  [[deprecated("Pass a scoped_refptr")]] virtual bool AddTrack(
      VideoTrackInterface* track) {
    return AddTrack(rtc::scoped_refptr<VideoTrackInterface>(track));
  }
  [[deprecated("Pass a scoped_refptr")]] virtual bool RemoveTrack(
      AudioTrackInterface* track) {
    return RemoveTrack(rtc::scoped_refptr<AudioTrackInterface>(track));
  }
  [[deprecated("Pass a scoped_refptr")]] virtual bool RemoveTrack(
      VideoTrackInterface* track) {
    return RemoveTrack(rtc::scoped_refptr<VideoTrackInterface>(track));
  }

 protected:
  ~MediaStreamInterface() override = default;
};

}  // namespace webrtc

#endif  // API_MEDIA_STREAM_INTERFACE_H_
