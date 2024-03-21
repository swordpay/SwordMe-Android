/*
 *  Copyright (c) 2021 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef NET_DCSCTP_TX_OUTSTANDING_DATA_H_
#define NET_DCSCTP_TX_OUTSTANDING_DATA_H_

#include <map>
#include <set>
#include <utility>
#include <vector>

#include "absl/types/optional.h"
#include "net/dcsctp/common/sequence_numbers.h"
#include "net/dcsctp/packet/chunk/forward_tsn_chunk.h"
#include "net/dcsctp/packet/chunk/iforward_tsn_chunk.h"
#include "net/dcsctp/packet/chunk/sack_chunk.h"
#include "net/dcsctp/packet/data.h"
#include "net/dcsctp/public/types.h"

namespace dcsctp {

// handles acking, nacking, rescheduling and abandoning.
class OutstandingData {
 public:

  enum class State {


    kInFlight,


    kNacked,

    kToBeRetransmitted,

    kAcked,


    kAbandoned,
  };

  struct AckInfo {
    explicit AckInfo(UnwrappedTSN cumulative_tsn_ack)
        : highest_tsn_acked(cumulative_tsn_ack) {}

    size_t bytes_acked = 0;





    bool has_packet_loss = false;

    UnwrappedTSN highest_tsn_acked;

    std::vector<LifecycleId> acked_lifecycle_ids;

    std::vector<LifecycleId> abandoned_lifecycle_ids;
  };

  OutstandingData(
      size_t data_chunk_header_size,
      UnwrappedTSN next_tsn,
      UnwrappedTSN last_cumulative_tsn_ack,
      std::function<bool(IsUnordered, StreamID, MID)> discard_from_send_queue)
      : data_chunk_header_size_(data_chunk_header_size),
        next_tsn_(next_tsn),
        last_cumulative_tsn_ack_(last_cumulative_tsn_ack),
        discard_from_send_queue_(std::move(discard_from_send_queue)) {}

  AckInfo HandleSack(
      UnwrappedTSN cumulative_tsn_ack,
      rtc::ArrayView<const SackChunk::GapAckBlock> gap_ack_blocks,
      bool is_in_fast_recovery);




  std::vector<std::pair<TSN, Data>> GetChunksToBeFastRetransmitted(
      size_t max_size);


  std::vector<std::pair<TSN, Data>> GetChunksToBeRetransmitted(size_t max_size);

  size_t outstanding_bytes() const { return outstanding_bytes_; }

  size_t outstanding_items() const { return outstanding_items_; }


  void ExpireOutstandingChunks(TimeMs now);

  bool empty() const { return outstanding_data_.empty(); }

  bool has_data_to_be_fast_retransmitted() const {
    return !to_be_fast_retransmitted_.empty();
  }

  bool has_data_to_be_retransmitted() const {
    return !to_be_retransmitted_.empty() || !to_be_fast_retransmitted_.empty();
  }

  UnwrappedTSN last_cumulative_tsn_ack() const {
    return last_cumulative_tsn_ack_;
  }

  UnwrappedTSN next_tsn() const { return next_tsn_; }

  UnwrappedTSN highest_outstanding_tsn() const;



  absl::optional<UnwrappedTSN> Insert(
      const Data& data,
      TimeMs time_sent,
      MaxRetransmits max_retransmissions = MaxRetransmits::NoLimit(),
      TimeMs expires_at = TimeMs::InfiniteFuture(),
      LifecycleId lifecycle_id = LifecycleId::NotSet());

  void NackAll();

  ForwardTsnChunk CreateForwardTsn() const;

  IForwardTsnChunk CreateIForwardTsn() const;



  absl::optional<DurationMs> MeasureRTT(TimeMs now, UnwrappedTSN tsn) const;


  std::vector<std::pair<TSN, State>> GetChunkStatesForTesting() const;


  bool ShouldSendForwardTsn() const;

  void ResetSequenceNumbers(UnwrappedTSN next_tsn,
                            UnwrappedTSN last_cumulative_tsn);

 private:


  class Item {
   public:
    enum class NackAction {
      kNothing,
      kRetransmit,
      kAbandon,
    };

    Item(Data data,
         TimeMs time_sent,
         MaxRetransmits max_retransmissions,
         TimeMs expires_at,
         LifecycleId lifecycle_id)
        : time_sent_(time_sent),
          max_retransmissions_(max_retransmissions),
          expires_at_(expires_at),
          lifecycle_id_(lifecycle_id),
          data_(std::move(data)) {}

    Item(const Item&) = delete;
    Item& operator=(const Item&) = delete;

    TimeMs time_sent() const { return time_sent_; }

    const Data& data() const { return data_; }

    void Ack();




    NackAction Nack(bool retransmit_now);


    void MarkAsRetransmitted();

    void Abandon();

    bool is_outstanding() const { return ack_state_ == AckState::kUnacked; }
    bool is_acked() const { return ack_state_ == AckState::kAcked; }
    bool is_nacked() const { return ack_state_ == AckState::kNacked; }
    bool is_abandoned() const { return lifecycle_ == Lifecycle::kAbandoned; }

    bool should_be_retransmitted() const {
      return lifecycle_ == Lifecycle::kToBeRetransmitted;
    }

    bool has_been_retransmitted() const { return num_retransmissions_ > 0; }


    bool has_expired(TimeMs now) const;

    LifecycleId lifecycle_id() const { return lifecycle_id_; }

   private:
    enum class Lifecycle : uint8_t {

      kActive,


      kToBeRetransmitted,

      kAbandoned
    };
    enum class AckState : uint8_t {

      kUnacked,

      kAcked,

      kNacked
    };



    const TimeMs time_sent_;



    const MaxRetransmits max_retransmissions_;

    Lifecycle lifecycle_ = Lifecycle::kActive;


    AckState ack_state_ = AckState::kUnacked;


    uint8_t nack_count_ = 0;

    uint16_t num_retransmissions_ = 0;


    const TimeMs expires_at_;

    const LifecycleId lifecycle_id_;

    const Data data_;
  };

  size_t GetSerializedChunkSize(const Data& data) const;



  void RemoveAcked(UnwrappedTSN cumulative_tsn_ack, AckInfo& ack_info);


  void AckGapBlocks(UnwrappedTSN cumulative_tsn_ack,
                    rtc::ArrayView<const SackChunk::GapAckBlock> gap_ack_blocks,
                    AckInfo& ack_info);




  void NackBetweenAckBlocks(
      UnwrappedTSN cumulative_tsn_ack,
      rtc::ArrayView<const SackChunk::GapAckBlock> gap_ack_blocks,
      bool is_in_fast_recovery,
      OutstandingData::AckInfo& ack_info);


  void AckChunk(AckInfo& ack_info, std::map<UnwrappedTSN, Item>::iterator iter);








  bool NackItem(UnwrappedTSN tsn,
                Item& item,
                bool retransmit_now,
                bool do_fast_retransmit);



  void AbandonAllFor(const OutstandingData::Item& item);

  std::vector<std::pair<TSN, Data>> ExtractChunksThatCanFit(
      std::set<UnwrappedTSN>& chunks,
      size_t max_size);

  bool IsConsistent() const;

  const size_t data_chunk_header_size_;

  UnwrappedTSN next_tsn_;

  UnwrappedTSN last_cumulative_tsn_ack_;

  std::function<bool(IsUnordered, StreamID, MID)> discard_from_send_queue_;

  std::map<UnwrappedTSN, Item> outstanding_data_;

  size_t outstanding_bytes_ = 0;


  size_t outstanding_items_ = 0;

  std::set<UnwrappedTSN> to_be_fast_retransmitted_;

  std::set<UnwrappedTSN> to_be_retransmitted_;
};
}  // namespace dcsctp
#endif  // NET_DCSCTP_TX_OUTSTANDING_DATA_H_
