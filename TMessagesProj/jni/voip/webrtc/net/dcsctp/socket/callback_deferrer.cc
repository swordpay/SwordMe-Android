/*
 *  Copyright (c) 2021 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "net/dcsctp/socket/callback_deferrer.h"

#include "api/make_ref_counted.h"

namespace dcsctp {
namespace {
// A wrapper around the move-only DcSctpMessage, to let it be captured in a
// lambda.
class MessageDeliverer {
 public:
  explicit MessageDeliverer(DcSctpMessage&& message)
      : state_(rtc::make_ref_counted<State>(std::move(message))) {}

  void Deliver(DcSctpSocketCallbacks& c) {

    RTC_DCHECK(!state_->has_delivered);
    state_->has_delivered = true;
    c.OnMessageReceived(std::move(state_->message));
  }

 private:
  struct State : public rtc::RefCountInterface {
    explicit State(DcSctpMessage&& m)
        : has_delivered(false), message(std::move(m)) {}
    bool has_delivered;
    DcSctpMessage message;
  };
  rtc::scoped_refptr<State> state_;
};
}  // namespace

void CallbackDeferrer::Prepare() {
  RTC_DCHECK(!prepared_);
  prepared_ = true;
}

void CallbackDeferrer::TriggerDeferred() {



  RTC_DCHECK(prepared_);
  std::vector<std::function<void(DcSctpSocketCallbacks & cb)>> deferred;
  deferred.swap(deferred_);
  prepared_ = false;

  for (auto& cb : deferred) {
    cb(underlying_);
  }
}

SendPacketStatus CallbackDeferrer::SendPacketWithStatus(
    rtc::ArrayView<const uint8_t> data) {

  return underlying_.SendPacketWithStatus(data);
}

std::unique_ptr<Timeout> CallbackDeferrer::CreateTimeout(
    webrtc::TaskQueueBase::DelayPrecision precision) {

  return underlying_.CreateTimeout(precision);
}

TimeMs CallbackDeferrer::TimeMillis() {

  return underlying_.TimeMillis();
}

uint32_t CallbackDeferrer::GetRandomInt(uint32_t low, uint32_t high) {

  return underlying_.GetRandomInt(low, high);
}

void CallbackDeferrer::OnMessageReceived(DcSctpMessage message) {
  RTC_DCHECK(prepared_);
  deferred_.emplace_back(
      [deliverer = MessageDeliverer(std::move(message))](
          DcSctpSocketCallbacks& cb) mutable { deliverer.Deliver(cb); });
}

void CallbackDeferrer::OnError(ErrorKind error, absl::string_view message) {
  RTC_DCHECK(prepared_);
  deferred_.emplace_back(
      [error, message = std::string(message)](DcSctpSocketCallbacks& cb) {
        cb.OnError(error, message);
      });
}

void CallbackDeferrer::OnAborted(ErrorKind error, absl::string_view message) {
  RTC_DCHECK(prepared_);
  deferred_.emplace_back(
      [error, message = std::string(message)](DcSctpSocketCallbacks& cb) {
        cb.OnAborted(error, message);
      });
}

void CallbackDeferrer::OnConnected() {
  RTC_DCHECK(prepared_);
  deferred_.emplace_back([](DcSctpSocketCallbacks& cb) { cb.OnConnected(); });
}

void CallbackDeferrer::OnClosed() {
  RTC_DCHECK(prepared_);
  deferred_.emplace_back([](DcSctpSocketCallbacks& cb) { cb.OnClosed(); });
}

void CallbackDeferrer::OnConnectionRestarted() {
  RTC_DCHECK(prepared_);
  deferred_.emplace_back(
      [](DcSctpSocketCallbacks& cb) { cb.OnConnectionRestarted(); });
}

void CallbackDeferrer::OnStreamsResetFailed(
    rtc::ArrayView<const StreamID> outgoing_streams,
    absl::string_view reason) {
  RTC_DCHECK(prepared_);
  deferred_.emplace_back(
      [streams = std::vector<StreamID>(outgoing_streams.begin(),
                                       outgoing_streams.end()),
       reason = std::string(reason)](DcSctpSocketCallbacks& cb) {
        cb.OnStreamsResetFailed(streams, reason);
      });
}

void CallbackDeferrer::OnStreamsResetPerformed(
    rtc::ArrayView<const StreamID> outgoing_streams) {
  RTC_DCHECK(prepared_);
  deferred_.emplace_back(
      [streams = std::vector<StreamID>(outgoing_streams.begin(),
                                       outgoing_streams.end())](
          DcSctpSocketCallbacks& cb) { cb.OnStreamsResetPerformed(streams); });
}

void CallbackDeferrer::OnIncomingStreamsReset(
    rtc::ArrayView<const StreamID> incoming_streams) {
  RTC_DCHECK(prepared_);
  deferred_.emplace_back(
      [streams = std::vector<StreamID>(incoming_streams.begin(),
                                       incoming_streams.end())](
          DcSctpSocketCallbacks& cb) { cb.OnIncomingStreamsReset(streams); });
}

void CallbackDeferrer::OnBufferedAmountLow(StreamID stream_id) {
  RTC_DCHECK(prepared_);
  deferred_.emplace_back([stream_id](DcSctpSocketCallbacks& cb) {
    cb.OnBufferedAmountLow(stream_id);
  });
}

void CallbackDeferrer::OnTotalBufferedAmountLow() {
  RTC_DCHECK(prepared_);
  deferred_.emplace_back(
      [](DcSctpSocketCallbacks& cb) { cb.OnTotalBufferedAmountLow(); });
}

void CallbackDeferrer::OnLifecycleMessageExpired(LifecycleId lifecycle_id,
                                                 bool maybe_delivered) {

  underlying_.OnLifecycleMessageExpired(lifecycle_id, maybe_delivered);
}
void CallbackDeferrer::OnLifecycleMessageFullySent(LifecycleId lifecycle_id) {

  underlying_.OnLifecycleMessageFullySent(lifecycle_id);
}
void CallbackDeferrer::OnLifecycleMessageDelivered(LifecycleId lifecycle_id) {

  underlying_.OnLifecycleMessageDelivered(lifecycle_id);
}
void CallbackDeferrer::OnLifecycleEnd(LifecycleId lifecycle_id) {

  underlying_.OnLifecycleEnd(lifecycle_id);
}
}  // namespace dcsctp
