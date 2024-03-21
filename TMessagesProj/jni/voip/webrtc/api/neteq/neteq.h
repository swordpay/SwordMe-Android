/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_NETEQ_NETEQ_H_
#define API_NETEQ_NETEQ_H_

#include <stddef.h>  // Provide access to size_t.

#include <map>
#include <string>
#include <vector>

#include "absl/types/optional.h"
#include "api/audio_codecs/audio_codec_pair_id.h"
#include "api/audio_codecs/audio_decoder.h"
#include "api/audio_codecs/audio_format.h"
#include "api/rtp_headers.h"
#include "api/scoped_refptr.h"

namespace webrtc {

class AudioFrame;
class AudioDecoderFactory;
class Clock;

struct NetEqNetworkStatistics {
  uint16_t current_buffer_size_ms;    // Current jitter buffer size in ms.
  uint16_t preferred_buffer_size_ms;  // Target buffer size in ms.
  uint16_t jitter_peaks_found;        // 1 if adding extra delay due to peaky

  uint16_t expand_rate;         // Fraction (of original stream) of synthesized

  uint16_t speech_expand_rate;  // Fraction (of original stream) of synthesized

  uint16_t preemptive_rate;     // Fraction of data inserted through pre-emptive

  uint16_t accelerate_rate;     // Fraction of data removed through acceleration

  uint16_t secondary_decoded_rate;    // Fraction of data coming from FEC/RED

  uint16_t secondary_discarded_rate;  // Fraction of discarded FEC/RED data (in



  int mean_waiting_time_ms;
  int median_waiting_time_ms;
  int min_waiting_time_ms;
  int max_waiting_time_ms;
};

// These metrics are never reset.
struct NetEqLifetimeStatistics {


  uint64_t total_samples_received = 0;
  uint64_t concealed_samples = 0;
  uint64_t concealment_events = 0;
  uint64_t jitter_buffer_delay_ms = 0;
  uint64_t jitter_buffer_emitted_count = 0;
  uint64_t jitter_buffer_target_delay_ms = 0;
  uint64_t jitter_buffer_minimum_delay_ms = 0;
  uint64_t inserted_samples_for_deceleration = 0;
  uint64_t removed_samples_for_acceleration = 0;
  uint64_t silent_concealed_samples = 0;
  uint64_t fec_packets_received = 0;
  uint64_t fec_packets_discarded = 0;
  uint64_t packets_discarded = 0;

  uint64_t delayed_packet_outage_samples = 0;







  uint64_t relative_packet_arrival_delay_ms = 0;
  uint64_t jitter_buffer_packets_received = 0;



  int32_t interruption_count = 0;
  int32_t total_interruption_duration_ms = 0;

  uint64_t generated_noise_samples = 0;
};

// state.
struct NetEqOperationsAndState {



  uint64_t preemptive_samples = 0;
  uint64_t accelerate_samples = 0;

  uint64_t packet_buffer_flushes = 0;


  uint64_t last_waiting_time_ms = 0;

  uint64_t current_buffer_size_ms = 0;

  uint64_t current_frame_size_ms = 0;

  bool next_packet_available = false;
};

class NetEq {
 public:
  struct Config {
    Config();
    Config(const Config&);
    Config(Config&&);
    ~Config();
    Config& operator=(const Config&);
    Config& operator=(Config&&);

    std::string ToString() const;

    int sample_rate_hz = 16000;  // Initial value. Will change with input data.
    bool enable_post_decode_vad = false;
    size_t max_packets_in_buffer = 200;
    int max_delay_ms = 0;
    int min_delay_ms = 0;
    bool enable_fast_accelerate = false;
    bool enable_muted_state = false;
    bool enable_rtx_handling = false;
    absl::optional<AudioCodecPairId> codec_pair_id;
    bool for_test_no_time_stretching = false;  // Use only for testing.
  };

  enum ReturnCodes { kOK = 0, kFail = -1 };

  enum class Operation {
    kNormal,
    kMerge,
    kExpand,
    kAccelerate,
    kFastAccelerate,
    kPreemptiveExpand,
    kRfc3389Cng,
    kRfc3389CngNoPacket,
    kCodecInternalCng,
    kDtmf,
    kUndefined,
  };

  enum class Mode {
    kNormal,
    kExpand,
    kMerge,
    kAccelerateSuccess,
    kAccelerateLowEnergy,
    kAccelerateFail,
    kPreemptiveExpandSuccess,
    kPreemptiveExpandLowEnergy,
    kPreemptiveExpandFail,
    kRfc3389Cng,
    kCodecInternalCng,
    kCodecPlc,
    kDtmf,
    kError,
    kUndefined,
  };

  struct DecoderFormat {
    int sample_rate_hz;
    int num_channels;
    SdpAudioFormat sdp_format;
  };

  virtual ~NetEq() {}


  virtual int InsertPacket(const RTPHeader& rtp_header,
                           rtc::ArrayView<const uint8_t> payload) = 0;




  virtual void InsertEmptyPacket(const RTPHeader& rtp_header) = 0;















  virtual int GetAudio(
      AudioFrame* audio_frame,
      bool* muted,
      int* current_sample_rate_hz = nullptr,
      absl::optional<Operation> action_override = absl::nullopt) = 0;

  virtual void SetCodecs(const std::map<int, SdpAudioFormat>& codecs) = 0;


  virtual bool RegisterPayloadType(int rtp_payload_type,
                                   const SdpAudioFormat& audio_format) = 0;



  virtual int RemovePayloadType(uint8_t rtp_payload_type) = 0;

  virtual void RemoveAllPayloadTypes() = 0;




  virtual bool SetMinimumDelay(int delay_ms) = 0;




  virtual bool SetMaximumDelay(int delay_ms) = 0;





  virtual bool SetBaseMinimumDelayMs(int delay_ms) = 0;

  virtual int GetBaseMinimumDelayMs() const = 0;


  virtual int TargetDelayMs() const = 0;



  virtual int FilteredCurrentDelayMs() const = 0;


  virtual int NetworkStatistics(NetEqNetworkStatistics* stats) = 0;

  virtual NetEqNetworkStatistics CurrentNetworkStatistics() const = 0;


  virtual NetEqLifetimeStatistics GetLifetimeStatistics() const = 0;


  virtual NetEqOperationsAndState GetOperationsAndState() const = 0;


  virtual void EnableVad() = 0;

  virtual void DisableVad() = 0;


  virtual absl::optional<uint32_t> GetPlayoutTimestamp() const = 0;



  virtual int last_output_sample_rate_hz() const = 0;


  virtual absl::optional<DecoderFormat> GetDecoderFormat(
      int payload_type) const = 0;

  virtual void FlushBuffers() = 0;



  virtual void EnableNack(size_t max_nack_list_size) = 0;

  virtual void DisableNack() = 0;


  virtual std::vector<uint16_t> GetNackList(
      int64_t round_trip_time_ms) const = 0;


  virtual int SyncBufferSizeMs() const = 0;
};

}  // namespace webrtc
#endif  // API_NETEQ_NETEQ_H_
