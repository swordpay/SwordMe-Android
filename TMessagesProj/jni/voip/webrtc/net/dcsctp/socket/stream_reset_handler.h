/*
 *  Copyright (c) 2021 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef NET_DCSCTP_SOCKET_STREAM_RESET_HANDLER_H_
#define NET_DCSCTP_SOCKET_STREAM_RESET_HANDLER_H_

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/functional/bind_front.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "api/array_view.h"
#include "net/dcsctp/common/internal_types.h"
#include "net/dcsctp/packet/chunk/reconfig_chunk.h"
#include "net/dcsctp/packet/parameter/incoming_ssn_reset_request_parameter.h"
#include "net/dcsctp/packet/parameter/outgoing_ssn_reset_request_parameter.h"
#include "net/dcsctp/packet/parameter/reconfiguration_response_parameter.h"
#include "net/dcsctp/packet/sctp_packet.h"
#include "net/dcsctp/public/dcsctp_socket.h"
#include "net/dcsctp/rx/data_tracker.h"
#include "net/dcsctp/rx/reassembly_queue.h"
#include "net/dcsctp/socket/context.h"
#include "net/dcsctp/timer/timer.h"
#include "net/dcsctp/tx/retransmission_queue.h"
#include "rtc_base/containers/flat_set.h"

namespace dcsctp {

// an SCTP stream, which translates to closing a data channel).
//
// It also handles incoming "outgoing stream reset requests", when the peer
// wants to close its data channel.
//
// Resetting streams is an asynchronous operation where the client will request
// a request a stream to be reset, but then it might not be performed exactly at
// this point. First, the sender might need to discard all messages that have
// been enqueued for this stream, or it may select to wait until all have been
// sent. At least, it must wait for the currently sending fragmented message to
// be fully sent, because a stream can't be reset while having received half a
// message. In the stream reset request, the "sender's last assigned TSN" is
// provided, which is simply the TSN for which the receiver should've received
// all messages before this value, before the stream can be reset. Since
// fragments can get lost or sent out-of-order, the receiver of a request may
// not have received all the data just yet, and then it will respond to the
// sender: "In progress". In other words, try again. The sender will then need
// to start a timer and try the very same request again (but with a new sequence
// number) until the receiver successfully performs the operation.
//
// All this can take some time, and may be driven by timers, so the client will
// ultimately be notified using callbacks.
//
// In this implementation, when a stream is reset, the queued but not-yet-sent
// messages will be discarded, but that may change in the future. RFC8831 allows
// both behaviors.
class StreamResetHandler {
 public:
  StreamResetHandler(absl::string_view log_prefix,
                     Context* context,
                     TimerManager* timer_manager,
                     DataTracker* data_tracker,
                     ReassemblyQueue* reassembly_queue,
                     RetransmissionQueue* retransmission_queue,
                     const DcSctpSocketHandoverState* handover_state = nullptr)
      : log_prefix_(std::string(log_prefix) + "reset: "),
        ctx_(context),
        data_tracker_(data_tracker),
        reassembly_queue_(reassembly_queue),
        retransmission_queue_(retransmission_queue),
        reconfig_timer_(timer_manager->CreateTimer(
            "re-config",
            absl::bind_front(&StreamResetHandler::OnReconfigTimerExpiry, this),
            TimerOptions(DurationMs(0)))),
        next_outgoing_req_seq_nbr_(
            handover_state
                ? ReconfigRequestSN(handover_state->tx.next_reset_req_sn)
                : ReconfigRequestSN(*ctx_->my_initial_tsn())),
        last_processed_req_seq_nbr_(
            handover_state ? ReconfigRequestSN(
                                 handover_state->rx.last_completed_reset_req_sn)
                           : ReconfigRequestSN(*ctx_->peer_initial_tsn() - 1)),
        last_processed_req_result_(
            ReconfigurationResponseParameter::Result::kSuccessNothingToDo) {}





  void ResetStreams(rtc::ArrayView<const StreamID> outgoing_streams);




  absl::optional<ReConfigChunk> MakeStreamResetRequest();

  void HandleReConfig(ReConfigChunk chunk);

  HandoverReadinessStatus GetHandoverReadiness() const;

  void AddHandoverState(DcSctpSocketHandoverState& state);

 private:




  class CurrentRequest {
   public:
    CurrentRequest(TSN sender_last_assigned_tsn, std::vector<StreamID> streams)
        : req_seq_nbr_(absl::nullopt),
          sender_last_assigned_tsn_(sender_last_assigned_tsn),
          streams_(std::move(streams)) {}



    ReconfigRequestSN req_seq_nbr() const {
      return req_seq_nbr_.value_or(ReconfigRequestSN(0));
    }



    TSN sender_last_assigned_tsn() const { return sender_last_assigned_tsn_; }

    const std::vector<StreamID>& streams() const { return streams_; }




    bool has_been_sent() const { return req_seq_nbr_.has_value(); }



    void PrepareRetransmission() { req_seq_nbr_ = absl::nullopt; }

    void PrepareToSend(ReconfigRequestSN new_req_seq_nbr) {
      req_seq_nbr_ = new_req_seq_nbr;
    }

   private:




    absl::optional<ReconfigRequestSN> req_seq_nbr_;


    TSN sender_last_assigned_tsn_;

    const std::vector<StreamID> streams_;
  };

  bool Validate(const ReConfigChunk& chunk);



  absl::optional<std::vector<ReconfigurationResponseParameter>> Process(
      const ReConfigChunk& chunk);


  ReConfigChunk MakeReconfigChunk();



  bool ValidateReqSeqNbr(
      ReconfigRequestSN req_seq_nbr,
      std::vector<ReconfigurationResponseParameter>& responses);



  void HandleResetOutgoing(
      const ParameterDescriptor& descriptor,
      std::vector<ReconfigurationResponseParameter>& responses);


  void HandleResetIncoming(
      const ParameterDescriptor& descriptor,
      std::vector<ReconfigurationResponseParameter>& responses);




  void HandleResponse(const ParameterDescriptor& descriptor);

  absl::optional<DurationMs> OnReconfigTimerExpiry();

  const std::string log_prefix_;
  Context* ctx_;
  DataTracker* data_tracker_;
  ReassemblyQueue* reassembly_queue_;
  RetransmissionQueue* retransmission_queue_;
  const std::unique_ptr<Timer> reconfig_timer_;

  ReconfigRequestSN next_outgoing_req_seq_nbr_;

  absl::optional<CurrentRequest> current_request_;

  ReconfigRequestSN last_processed_req_seq_nbr_;

  ReconfigurationResponseParameter::Result last_processed_req_result_;
};
}  // namespace dcsctp

#endif  // NET_DCSCTP_SOCKET_STREAM_RESET_HANDLER_H_
