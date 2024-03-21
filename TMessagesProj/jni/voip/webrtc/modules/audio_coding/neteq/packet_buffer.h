/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_CODING_NETEQ_PACKET_BUFFER_H_
#define MODULES_AUDIO_CODING_NETEQ_PACKET_BUFFER_H_

#include "absl/types/optional.h"
#include "modules/audio_coding/neteq/decoder_database.h"
#include "modules/audio_coding/neteq/packet.h"
#include "modules/include/module_common_types_public.h"  // IsNewerTimestamp

namespace webrtc {

class DecoderDatabase;
class StatisticsCalculator;
class TickTimer;
struct SmartFlushingConfig {


  int target_level_threshold_ms = 500;


  int target_level_multiplier = 3;
};

class PacketBuffer {
 public:
  enum BufferReturnCodes {
    kOK = 0,
    kFlushed,
    kPartialFlush,
    kNotFound,
    kBufferEmpty,
    kInvalidPacket,
    kInvalidPointer
  };


  PacketBuffer(size_t max_number_of_packets, const TickTimer* tick_timer);

  virtual ~PacketBuffer();

  PacketBuffer(const PacketBuffer&) = delete;
  PacketBuffer& operator=(const PacketBuffer&) = delete;

  virtual void Flush(StatisticsCalculator* stats);

  virtual void PartialFlush(int target_level_ms,
                            size_t sample_rate,
                            size_t last_decoded_length,
                            StatisticsCalculator* stats);

  virtual bool Empty() const;




  virtual int InsertPacket(Packet&& packet,
                           StatisticsCalculator* stats,
                           size_t last_decoded_length,
                           size_t sample_rate,
                           int target_level_ms,
                           const DecoderDatabase& decoder_database);








  virtual int InsertPacketList(
      PacketList* packet_list,
      const DecoderDatabase& decoder_database,
      absl::optional<uint8_t>* current_rtp_payload_type,
      absl::optional<uint8_t>* current_cng_rtp_payload_type,
      StatisticsCalculator* stats,
      size_t last_decoded_length,
      size_t sample_rate,
      int target_level_ms);




  virtual int NextTimestamp(uint32_t* next_timestamp) const;





  virtual int NextHigherTimestamp(uint32_t timestamp,
                                  uint32_t* next_timestamp) const;


  virtual const Packet* PeekNextPacket() const;


  virtual absl::optional<Packet> GetNextPacket();



  virtual int DiscardNextPacket(StatisticsCalculator* stats);





  virtual void DiscardOldPackets(uint32_t timestamp_limit,
                                 uint32_t horizon_samples,
                                 StatisticsCalculator* stats);

  virtual void DiscardAllOldPackets(uint32_t timestamp_limit,
                                    StatisticsCalculator* stats);

  virtual void DiscardPacketsWithPayloadType(uint8_t payload_type,
                                             StatisticsCalculator* stats);


  virtual size_t NumPacketsInBuffer() const;


  virtual size_t NumSamplesInBuffer(size_t last_decoded_length) const;


  virtual size_t GetSpanSamples(size_t last_decoded_length,
                                size_t sample_rate,
                                bool count_dtx_waiting_time) const;

  virtual bool ContainsDtxOrCngPacket(
      const DecoderDatabase* decoder_database) const;






  static bool IsObsoleteTimestamp(uint32_t timestamp,
                                  uint32_t timestamp_limit,
                                  uint32_t horizon_samples) {
    return IsNewerTimestamp(timestamp_limit, timestamp) &&
           (horizon_samples == 0 ||
            IsNewerTimestamp(timestamp, timestamp_limit - horizon_samples));
  }

 private:
  absl::optional<SmartFlushingConfig> smart_flushing_config_;
  size_t max_number_of_packets_;
  PacketList buffer_;
  const TickTimer* tick_timer_;
};

}  // namespace webrtc
#endif  // MODULES_AUDIO_CODING_NETEQ_PACKET_BUFFER_H_
