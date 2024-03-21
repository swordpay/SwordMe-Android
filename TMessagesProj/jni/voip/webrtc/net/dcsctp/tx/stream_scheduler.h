/*
 *  Copyright (c) 2022 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef NET_DCSCTP_TX_STREAM_SCHEDULER_H_
#define NET_DCSCTP_TX_STREAM_SCHEDULER_H_

#include <algorithm>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <utility>

#include "absl/algorithm/container.h"
#include "absl/memory/memory.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "api/array_view.h"
#include "net/dcsctp/packet/chunk/idata_chunk.h"
#include "net/dcsctp/packet/sctp_packet.h"
#include "net/dcsctp/public/dcsctp_message.h"
#include "net/dcsctp/public/dcsctp_socket.h"
#include "net/dcsctp/public/types.h"
#include "net/dcsctp/tx/send_queue.h"
#include "rtc_base/containers/flat_set.h"
#include "rtc_base/strong_alias.h"

namespace dcsctp {

// scheduling algorithm using virtual finish time. It is to be used as a part of
// a send queue and will track all active streams (streams that have any data
// that can be sent).
//
// The stream scheduler works with the concept of associating active streams
// with a "virtual finish time", which is the time when a stream is allowed to
// produce data. Streams are ordered by their virtual finish time, and the
// "current virtual time" will advance to the next following virtual finish time
// whenever a chunk is to be produced.
//
// When message interleaving is enabled, the WFQ - Weighted Fair Queueing -
// scheduling algorithm will be used. And when it's not, round-robin scheduling
// will be used instead.
//
// In the round robin scheduling algorithm, a stream's virtual finish time will
// just increment by one (1) after having produced a chunk, which results in a
// round-robin scheduling.
//
// In WFQ scheduling algorithm, a stream's virtual finish time will be defined
// as the number of bytes in the next fragment to be sent, multiplied by the
// inverse of the stream's priority, meaning that a high priority - or a smaller
// fragment - results in a closer virtual finish time, compared to a stream with
// either a lower priority or a larger fragment to be sent.
class StreamScheduler {
 private:
  class VirtualTime : public webrtc::StrongAlias<class VirtualTimeTag, double> {
   public:
    constexpr explicit VirtualTime(const UnderlyingType& v)
        : webrtc::StrongAlias<class VirtualTimeTag, double>(v) {}

    static constexpr VirtualTime Zero() { return VirtualTime(0); }
  };
  class InverseWeight
      : public webrtc::StrongAlias<class InverseWeightTag, double> {
   public:
    constexpr explicit InverseWeight(StreamPriority priority)
        : webrtc::StrongAlias<class InverseWeightTag, double>(
              1.0 / std::max(static_cast<double>(*priority), 0.000001)) {}
  };

 public:
  class StreamProducer {
   public:
    virtual ~StreamProducer() = default;





    virtual absl::optional<SendQueue::DataToSend> Produce(TimeMs now,
                                                          size_t max_size) = 0;



    virtual size_t bytes_to_send_in_next_message() const = 0;
  };

  class Stream {
   public:
    StreamID stream_id() const { return stream_id_; }

    StreamPriority priority() const { return priority_; }
    void SetPriority(StreamPriority priority);



    void MaybeMakeActive();


    void MakeInactive();



    void ForceReschedule() { parent_.ForceReschedule(); }

   private:
    friend class StreamScheduler;

    Stream(StreamScheduler* parent,
           StreamProducer* producer,
           StreamID stream_id,
           StreamPriority priority)
        : parent_(*parent),
          producer_(*producer),
          stream_id_(stream_id),
          priority_(priority),
          inverse_weight_(priority) {}


    absl::optional<SendQueue::DataToSend> Produce(TimeMs now, size_t max_size);

    void MakeActive(size_t bytes_to_send_next);
    void ForceMarkInactive();

    VirtualTime current_time() const { return current_virtual_time_; }
    VirtualTime next_finish_time() const { return next_finish_time_; }
    size_t bytes_to_send_in_next_message() const {
      return producer_.bytes_to_send_in_next_message();
    }

    VirtualTime CalculateFinishTime(size_t bytes_to_send_next) const;

    StreamScheduler& parent_;
    StreamProducer& producer_;
    const StreamID stream_id_;
    StreamPriority priority_;
    InverseWeight inverse_weight_;

    VirtualTime current_virtual_time_ = VirtualTime::Zero();
    VirtualTime next_finish_time_ = VirtualTime::Zero();
  };


  explicit StreamScheduler(size_t mtu)
      : max_payload_bytes_(mtu - SctpPacket::kHeaderSize -
                           IDataChunk::kHeaderSize) {}

  std::unique_ptr<Stream> CreateStream(StreamProducer* producer,
                                       StreamID stream_id,
                                       StreamPriority priority) {
    return absl::WrapUnique(new Stream(this, producer, stream_id, priority));
  }

  void EnableMessageInterleaving(bool enabled) {
    enable_message_interleaving_ = enabled;
  }


  void ForceReschedule() { currently_sending_a_message_ = false; }




  absl::optional<SendQueue::DataToSend> Produce(TimeMs now, size_t max_size);

  std::set<StreamID> ActiveStreamsForTesting() const;

 private:
  struct ActiveStreamComparator {

    bool operator()(Stream* a, Stream* b) const {
      VirtualTime a_vft = a->next_finish_time();
      VirtualTime b_vft = b->next_finish_time();
      if (a_vft == b_vft) {
        return a->stream_id() < b->stream_id();
      }
      return a_vft < b_vft;
    }
  };

  bool IsConsistent() const;

  const size_t max_payload_bytes_;

  VirtualTime virtual_time_ = VirtualTime::Zero();

  Stream* current_stream_ = nullptr;

  bool enable_message_interleaving_ = false;



  bool currently_sending_a_message_ = false;

  webrtc::flat_set<Stream*, ActiveStreamComparator> active_streams_;
};

}  // namespace dcsctp

#endif  // NET_DCSCTP_TX_STREAM_SCHEDULER_H_
