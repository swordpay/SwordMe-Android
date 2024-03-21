/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef API_TEST_PEERCONNECTION_QUALITY_TEST_FIXTURE_H_
#define API_TEST_PEERCONNECTION_QUALITY_TEST_FIXTURE_H_

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/memory/memory.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "api/array_view.h"
#include "api/async_resolver_factory.h"
#include "api/audio/audio_mixer.h"
#include "api/call/call_factory_interface.h"
#include "api/fec_controller.h"
#include "api/function_view.h"
#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "api/rtc_event_log/rtc_event_log_factory_interface.h"
#include "api/rtp_parameters.h"
#include "api/task_queue/task_queue_factory.h"
#include "api/test/audio_quality_analyzer_interface.h"
#include "api/test/frame_generator_interface.h"
#include "api/test/peer_network_dependencies.h"
#include "api/test/simulated_network.h"
#include "api/test/stats_observer_interface.h"
#include "api/test/track_id_stream_info_map.h"
#include "api/test/video/video_frame_writer.h"
#include "api/test/video_quality_analyzer_interface.h"
#include "api/transport/network_control.h"
#include "api/units/time_delta.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_encoder.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "media/base/media_constants.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "rtc_base/network.h"
#include "rtc_base/rtc_certificate_generator.h"
#include "rtc_base/ssl_certificate.h"
#include "rtc_base/thread.h"

namespace webrtc {
namespace webrtc_pc_e2e {

constexpr size_t kDefaultSlidesWidth = 1850;
constexpr size_t kDefaultSlidesHeight = 1110;

class PeerConnectionE2EQualityTestFixture {
 public:




  enum class CapturingDeviceIndex : size_t {};


















  struct ScrollingParams {
    ScrollingParams(TimeDelta duration,
                    size_t source_width,
                    size_t source_height)
        : duration(duration),
          source_width(source_width),
          source_height(source_height) {
      RTC_CHECK_GT(duration.ms(), 0);
    }

    TimeDelta duration;

    size_t source_width;

    size_t source_height;
  };

  struct ScreenShareConfig {
    explicit ScreenShareConfig(TimeDelta slide_change_interval)
        : slide_change_interval(slide_change_interval) {
      RTC_CHECK_GT(slide_change_interval.ms(), 0);
    }


    TimeDelta slide_change_interval;


    bool generate_slides = false;


    absl::optional<ScrollingParams> scrolling_params;








    std::vector<std::string> slides_yuv_file_names;
  };

















  struct VideoSimulcastConfig {
    explicit VideoSimulcastConfig(int simulcast_streams_count)
        : simulcast_streams_count(simulcast_streams_count) {
      RTC_CHECK_GT(simulcast_streams_count, 1);
    }


    int simulcast_streams_count;
  };











  struct EmulatedSFUConfig {
    EmulatedSFUConfig() {}
    explicit EmulatedSFUConfig(int target_layer_index)
        : target_layer_index(target_layer_index) {
      RTC_CHECK_GE(target_layer_index, 0);
    }

    EmulatedSFUConfig(absl::optional<int> target_layer_index,
                      absl::optional<int> target_temporal_index)
        : target_layer_index(target_layer_index),
          target_temporal_index(target_temporal_index) {
      RTC_CHECK_GE(target_temporal_index.value_or(0), 0);
      if (target_temporal_index)
        RTC_CHECK_GE(*target_temporal_index, 0);
    }













    absl::optional<int> target_layer_index;




    absl::optional<int> target_temporal_index;
  };

  class VideoResolution {
   public:


    enum class Spec {


      kNone,


      kMaxFromSender
    };

    VideoResolution(size_t width, size_t height, int32_t fps);
    explicit VideoResolution(Spec spec = Spec::kNone);

    bool operator==(const VideoResolution& other) const;
    bool operator!=(const VideoResolution& other) const {
      return !(*this == other);
    }

    size_t width() const { return width_; }
    void set_width(size_t width) { width_ = width; }
    size_t height() const { return height_; }
    void set_height(size_t height) { height_ = height; }
    int32_t fps() const { return fps_; }
    void set_fps(int32_t fps) { fps_ = fps; }


    bool IsRegular() const { return spec_ == Spec::kNone; }

    std::string ToString() const;

   private:
    size_t width_ = 0;
    size_t height_ = 0;
    int32_t fps_ = 0;
    Spec spec_ = Spec::kNone;
  };

  class VideoDumpOptions {
   public:
    static constexpr int kDefaultSamplingModulo = 1;


















    explicit VideoDumpOptions(
        absl::string_view output_directory,
        int sampling_modulo = kDefaultSamplingModulo,
        bool export_frame_ids = false,
        std::function<std::unique_ptr<test::VideoFrameWriter>(
            absl::string_view file_name_prefix,
            const VideoResolution& resolution)> video_frame_writer_factory =
            Y4mVideoFrameWriterFactory);
    VideoDumpOptions(absl::string_view output_directory, bool export_frame_ids);

    VideoDumpOptions(const VideoDumpOptions&) = default;
    VideoDumpOptions& operator=(const VideoDumpOptions&) = default;
    VideoDumpOptions(VideoDumpOptions&&) = default;
    VideoDumpOptions& operator=(VideoDumpOptions&&) = default;

    std::string output_directory() const { return output_directory_; }
    int sampling_modulo() const { return sampling_modulo_; }
    bool export_frame_ids() const { return export_frame_ids_; }

    std::unique_ptr<test::VideoFrameWriter> CreateInputDumpVideoFrameWriter(
        absl::string_view stream_label,
        const VideoResolution& resolution) const;

    std::unique_ptr<test::VideoFrameWriter> CreateOutputDumpVideoFrameWriter(
        absl::string_view stream_label,
        absl::string_view receiver,
        const VideoResolution& resolution) const;

    std::string ToString() const;

   private:
    static std::unique_ptr<test::VideoFrameWriter> Y4mVideoFrameWriterFactory(
        absl::string_view file_name_prefix,
        const VideoResolution& resolution);
    std::string GetInputDumpFileName(absl::string_view stream_label,
                                     const VideoResolution& resolution) const;


    absl::optional<std::string> GetInputFrameIdsDumpFileName(
        absl::string_view stream_label,
        const VideoResolution& resolution) const;
    std::string GetOutputDumpFileName(absl::string_view stream_label,
                                      absl::string_view receiver,
                                      const VideoResolution& resolution) const;


    absl::optional<std::string> GetOutputFrameIdsDumpFileName(
        absl::string_view stream_label,
        absl::string_view receiver,
        const VideoResolution& resolution) const;

    std::string output_directory_;
    int sampling_modulo_ = 1;
    bool export_frame_ids_ = false;
    std::function<std::unique_ptr<test::VideoFrameWriter>(
        absl::string_view file_name_prefix,
        const VideoResolution& resolution)>
        video_frame_writer_factory_;
  };

  struct VideoConfig {
    explicit VideoConfig(const VideoResolution& resolution);
    VideoConfig(size_t width, size_t height, int32_t fps)
        : width(width), height(height), fps(fps) {}
    VideoConfig(std::string stream_label,
                size_t width,
                size_t height,
                int32_t fps)
        : width(width),
          height(height),
          fps(fps),
          stream_label(std::move(stream_label)) {}

    size_t width;

    size_t height;
    int32_t fps;
    VideoResolution GetResolution() const {
      return VideoResolution(width, height, fps);
    }


    absl::optional<std::string> stream_label;


    absl::optional<VideoTrackInterface::ContentHint> content_hint;








    absl::optional<VideoSimulcastConfig> simulcast_config;

    absl::optional<EmulatedSFUConfig> emulated_sfu_config;







    std::vector<RtpEncodingParameters> encoding_params;



    absl::optional<int> temporal_layers_count;




    absl::optional<VideoDumpOptions> input_dump_options;



    absl::optional<VideoDumpOptions> output_dump_options;


    bool output_dump_use_fixed_framerate = false;

    bool show_on_screen = false;



    absl::optional<std::string> sync_group;



    absl::optional<DegradationPreference> degradation_preference;
  };

  struct AudioConfig {
    enum Mode {
      kGenerated,
      kFile,
    };

    AudioConfig() = default;
    explicit AudioConfig(std::string stream_label)
        : stream_label(std::move(stream_label)) {}


    absl::optional<std::string> stream_label;
    Mode mode = kGenerated;

    absl::optional<std::string> input_file_name;

    absl::optional<std::string> input_dump_file_name;

    absl::optional<std::string> output_dump_file_name;

    cricket::AudioOptions audio_options;

    int sampling_frequency_in_hz = 48000;



    absl::optional<std::string> sync_group;
  };

  struct VideoCodecConfig {
    explicit VideoCodecConfig(std::string name)
        : name(std::move(name)), required_params() {}
    VideoCodecConfig(std::string name,
                     std::map<std::string, std::string> required_params)
        : name(std::move(name)), required_params(std::move(required_params)) {}





    std::string name = cricket::kVp8CodecName;






    std::map<std::string, std::string> required_params;
  };


  class VideoSubscription {
   public:


    static absl::optional<VideoResolution> GetMaxResolution(
        rtc::ArrayView<const VideoConfig> video_configs);
    static absl::optional<VideoResolution> GetMaxResolution(
        rtc::ArrayView<const VideoResolution> resolutions);

    bool operator==(const VideoSubscription& other) const;
    bool operator!=(const VideoSubscription& other) const {
      return !(*this == other);
    }



    VideoSubscription& SubscribeToPeer(
        absl::string_view peer_name,
        VideoResolution resolution =
            VideoResolution(VideoResolution::Spec::kMaxFromSender)) {
      peers_resolution_[std::string(peer_name)] = resolution;
      return *this;
    }




    VideoSubscription& SubscribeToAllPeers(
        VideoResolution resolution =
            VideoResolution(VideoResolution::Spec::kMaxFromSender)) {
      default_resolution_ = resolution;
      return *this;
    }




    absl::optional<VideoResolution> GetResolutionForPeer(
        absl::string_view peer_name) const {
      auto it = peers_resolution_.find(std::string(peer_name));
      if (it == peers_resolution_.end()) {
        return default_resolution_;
      }
      return it->second;
    }


    std::vector<std::string> GetSubscribedPeers() const {
      std::vector<std::string> subscribed_streams;
      subscribed_streams.reserve(peers_resolution_.size());
      for (const auto& entry : peers_resolution_) {
        subscribed_streams.push_back(entry.first);
      }
      return subscribed_streams;
    }

    std::string ToString() const;

   private:
    absl::optional<VideoResolution> default_resolution_ = absl::nullopt;
    std::map<std::string, VideoResolution> peers_resolution_;
  };

  class PeerConfigurer {
   public:
    virtual ~PeerConfigurer() = default;



    virtual PeerConfigurer* SetName(absl::string_view name) = 0;



    virtual PeerConfigurer* SetTaskQueueFactory(
        std::unique_ptr<TaskQueueFactory> task_queue_factory) = 0;
    virtual PeerConfigurer* SetCallFactory(
        std::unique_ptr<CallFactoryInterface> call_factory) = 0;
    virtual PeerConfigurer* SetEventLogFactory(
        std::unique_ptr<RtcEventLogFactoryInterface> event_log_factory) = 0;
    virtual PeerConfigurer* SetFecControllerFactory(
        std::unique_ptr<FecControllerFactoryInterface>
            fec_controller_factory) = 0;
    virtual PeerConfigurer* SetNetworkControllerFactory(
        std::unique_ptr<NetworkControllerFactoryInterface>
            network_controller_factory) = 0;
    virtual PeerConfigurer* SetVideoEncoderFactory(
        std::unique_ptr<VideoEncoderFactory> video_encoder_factory) = 0;
    virtual PeerConfigurer* SetVideoDecoderFactory(
        std::unique_ptr<VideoDecoderFactory> video_decoder_factory) = 0;

    virtual PeerConfigurer* SetNetEqFactory(
        std::unique_ptr<NetEqFactory> neteq_factory) = 0;
    virtual PeerConfigurer* SetAudioProcessing(
        rtc::scoped_refptr<webrtc::AudioProcessing> audio_processing) = 0;
    virtual PeerConfigurer* SetAudioMixer(
        rtc::scoped_refptr<webrtc::AudioMixer> audio_mixer) = 0;


    virtual PeerConfigurer* SetUseNetworkThreadAsWorkerThread() = 0;



    virtual PeerConfigurer* SetAsyncResolverFactory(
        std::unique_ptr<webrtc::AsyncResolverFactory>
            async_resolver_factory) = 0;
    virtual PeerConfigurer* SetRTCCertificateGenerator(
        std::unique_ptr<rtc::RTCCertificateGeneratorInterface>
            cert_generator) = 0;
    virtual PeerConfigurer* SetSSLCertificateVerifier(
        std::unique_ptr<rtc::SSLCertificateVerifier> tls_cert_verifier) = 0;
    virtual PeerConfigurer* SetIceTransportFactory(
        std::unique_ptr<IceTransportFactory> factory) = 0;



    virtual PeerConfigurer* SetPortAllocatorExtraFlags(
        uint32_t extra_flags) = 0;


    virtual PeerConfigurer* AddVideoConfig(VideoConfig config) = 0;


    virtual PeerConfigurer* AddVideoConfig(
        VideoConfig config,
        std::unique_ptr<test::FrameGeneratorInterface> generator) = 0;


    virtual PeerConfigurer* AddVideoConfig(
        VideoConfig config,
        CapturingDeviceIndex capturing_device_index) = 0;



    virtual PeerConfigurer* SetVideoSubscription(
        VideoSubscription subscription) = 0;






    virtual PeerConfigurer* SetVideoCodecs(
        std::vector<VideoCodecConfig> video_codecs) = 0;


    virtual PeerConfigurer* SetAudioConfig(AudioConfig config) = 0;

    virtual PeerConfigurer* SetUseUlpFEC(bool value) = 0;



    virtual PeerConfigurer* SetUseFlexFEC(bool value) = 0;






    virtual PeerConfigurer* SetVideoEncoderBitrateMultiplier(
        double multiplier) = 0;


    virtual PeerConfigurer* SetRtcEventLogPath(std::string path) = 0;


    virtual PeerConfigurer* SetAecDumpPath(std::string path) = 0;
    virtual PeerConfigurer* SetRTCConfiguration(
        PeerConnectionInterface::RTCConfiguration configuration) = 0;
    virtual PeerConfigurer* SetRTCOfferAnswerOptions(
        PeerConnectionInterface::RTCOfferAnswerOptions options) = 0;


    virtual PeerConfigurer* SetBitrateSettings(
        BitrateSettings bitrate_settings) = 0;
  };

  struct EchoEmulationConfig {


    TimeDelta echo_delay = TimeDelta::Millis(50);
  };


  struct RunParams {
    explicit RunParams(TimeDelta run_duration) : run_duration(run_duration) {}



    TimeDelta run_duration;


    bool enable_flex_fec_support = false;


    bool use_conference_mode = false;



    absl::optional<EchoEmulationConfig> echo_emulation_config;
  };

  class QualityMetricsReporter : public StatsObserverInterface {
   public:
    virtual ~QualityMetricsReporter() = default;








    virtual void Start(absl::string_view test_case_name,
                       const TrackIdStreamInfoMap* reporter_helper) = 0;


    virtual void StopAndReportResults() = 0;
  };


  class PeerHandle {
   public:
    virtual ~PeerHandle() = default;
  };

  virtual ~PeerConnectionE2EQualityTestFixture() = default;




  virtual void ExecuteAt(TimeDelta target_time_since_start,
                         std::function<void(TimeDelta)> func) = 0;




  virtual void ExecuteEvery(TimeDelta initial_delay_since_start,
                            TimeDelta interval,
                            std::function<void(TimeDelta)> func) = 0;

  virtual void AddQualityMetricsReporter(
      std::unique_ptr<QualityMetricsReporter> quality_metrics_reporter) = 0;





  virtual PeerHandle* AddPeer(
      const PeerNetworkDependencies& network_dependencies,
      rtc::FunctionView<void(PeerConfigurer*)> configurer) = 0;






  virtual void Run(RunParams run_params) = 0;





  virtual TimeDelta GetRealTestDuration() const = 0;
};

}  // namespace webrtc_pc_e2e
}  // namespace webrtc

#endif  // API_TEST_PEERCONNECTION_QUALITY_TEST_FIXTURE_H_
