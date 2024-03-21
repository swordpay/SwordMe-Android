/*
 *  Copyright (c) 2022 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_PACING_PRIORITIZED_PACKET_QUEUE_H_
#define MODULES_PACING_PRIORITIZED_PACKET_QUEUE_H_

#include <stddef.h>

#include <deque>
#include <list>
#include <memory>
#include <unordered_map>

#include "api/units/data_size.h"
#include "api/units/time_delta.h"
#include "api/units/timestamp.h"
#include "modules/rtp_rtcp/source/rtp_packet_to_send.h"

namespace webrtc {

class PrioritizedPacketQueue {
 public:
  explicit PrioritizedPacketQueue(Timestamp creation_time);
  PrioritizedPacketQueue(const PrioritizedPacketQueue&) = delete;
  PrioritizedPacketQueue& operator=(const PrioritizedPacketQueue&) = delete;


  void Push(Timestamp enqueue_time, std::unique_ptr<RtpPacketToSend> packet);





  std::unique_ptr<RtpPacketToSend> Pop();

  int SizeInPackets() const;


  DataSize SizeInPayloadBytes() const;

  bool Empty() const;


  const std::array<int, kNumMediaTypes>& SizeInPacketsPerRtpPacketMediaType()
      const;



  Timestamp LeadingPacketEnqueueTime(RtpPacketMediaType type) const;


  Timestamp OldestEnqueueTime() const;




  TimeDelta AverageQueueTime() const;



  void UpdateAverageQueueTime(Timestamp now);

  void SetPauseState(bool paused, Timestamp now);

 private:
  static constexpr int kNumPriorityLevels = 4;

  class QueuedPacket {
   public:
    DataSize PacketSize() const;

    std::unique_ptr<RtpPacketToSend> packet;
    Timestamp enqueue_time;
    std::list<Timestamp>::iterator enqueue_time_iterator;
  };


  class StreamQueue {
   public:
    explicit StreamQueue(Timestamp creation_time);
    StreamQueue(StreamQueue&&) = default;
    StreamQueue& operator=(StreamQueue&&) = default;

    StreamQueue(const StreamQueue&) = delete;
    StreamQueue& operator=(const StreamQueue&) = delete;


    bool EnqueuePacket(QueuedPacket packet, int priority_level);

    QueuedPacket DequePacket(int priority_level);

    bool HasPacketsAtPrio(int priority_level) const;
    bool IsEmpty() const;
    Timestamp LeadingPacketEnqueueTime(int priority_level) const;
    Timestamp LastEnqueueTime() const;

   private:
    std::deque<QueuedPacket> packets_[kNumPriorityLevels];
    Timestamp last_enqueue_time_;
  };

  TimeDelta queue_time_sum_;

  TimeDelta pause_time_sum_;

  int size_packets_;

  std::array<int, kNumMediaTypes> size_packets_per_media_type_;

  DataSize size_payload_;

  Timestamp last_update_time_;
  bool paused_;

  Timestamp last_culling_time_;

  std::unordered_map<uint32_t, std::unique_ptr<StreamQueue>> streams_;


  std::deque<StreamQueue*> streams_by_prio_[kNumPriorityLevels];

  int top_active_prio_level_;



  std::list<Timestamp> enqueue_times_;
};

}  // namespace webrtc

#endif  // MODULES_PACING_PRIORITIZED_PACKET_QUEUE_H_
