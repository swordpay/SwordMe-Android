/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef LOGGING_RTC_EVENT_LOG_RTC_EVENT_PROCESSOR_H_
#define LOGGING_RTC_EVENT_LOG_RTC_EVENT_PROCESSOR_H_

#include <stdint.h>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "api/function_view.h"
#include "rtc_base/checks.h"

namespace webrtc {

// sorted lists in timestamp order. The effect is the same as doing a merge step
// in the merge-sort algorithm but without copying the elements or modifying the
// lists.

namespace event_processor_impl {
// Interface to allow "merging" lists of different types. ProcessNext()
// processes the next unprocesses element in the list. IsEmpty() checks if all
// elements have been processed. GetNextTime returns the timestamp of the next
// unprocessed element.
class ProcessableEventListInterface {
 public:
  virtual ~ProcessableEventListInterface() = default;
  virtual void ProcessNext() = 0;
  virtual bool IsEmpty() const = 0;
  virtual int64_t GetNextTime() const = 0;
  virtual int GetTieBreaker() const = 0;
};

// be applied to each element of the list.
template <typename Iterator, typename T>
class ProcessableEventList : public ProcessableEventListInterface {
 public:
  ProcessableEventList(Iterator begin,
                       Iterator end,
                       std::function<void(const T&)> f,
                       int tie_breaker)
      : begin_(begin), end_(end), f_(f), tie_breaker_(tie_breaker) {}

  void ProcessNext() override {
    RTC_DCHECK(!IsEmpty());
    f_(*begin_);
    ++begin_;
  }

  bool IsEmpty() const override { return begin_ == end_; }

  int64_t GetNextTime() const override {
    RTC_DCHECK(!IsEmpty());
    return begin_->log_time_us();
  }
  int GetTieBreaker() const override { return tie_breaker_; }

 private:
  Iterator begin_;
  Iterator end_;
  std::function<void(const T&)> f_;
  int tie_breaker_;
};
}  // namespace event_processor_impl

// so that they can be treated as a single ordered list. Since the individual
// lists may have different types, we need to access the lists via pointers to
// the common base class.
//
// Usage example:
// ParsedRtcEventLogNew log;
// auto incoming_handler = [] (LoggedRtcpPacketIncoming elem) { ... };
// auto outgoing_handler = [] (LoggedRtcpPacketOutgoing elem) { ... };
//
// RtcEventProcessor processor;
// processor.AddEvents(log.incoming_rtcp_packets(),
//                     incoming_handler);
// processor.AddEvents(log.outgoing_rtcp_packets(),
//                     outgoing_handler);
// processor.ProcessEventsInOrder();
class RtcEventProcessor {
 public:
  RtcEventProcessor();
  ~RtcEventProcessor();






  template <typename Iterable>
  void AddEvents(
      const Iterable& iterable,
      std::function<void(const typename Iterable::value_type&)> handler) {
    if (iterable.begin() == iterable.end())
      return;
    event_lists_.push_back(
        std::make_unique<event_processor_impl::ProcessableEventList<
            typename Iterable::const_iterator, typename Iterable::value_type>>(
            iterable.begin(), iterable.end(), handler,
            insertion_order_index_++));
    std::push_heap(event_lists_.begin(), event_lists_.end(), Cmp);
  }

  void ProcessEventsInOrder();

 private:
  using ListPtrType =
      std::unique_ptr<event_processor_impl::ProcessableEventListInterface>;
  int insertion_order_index_ = 0;
  std::vector<ListPtrType> event_lists_;

  static bool Cmp(const ListPtrType& a, const ListPtrType& b);
};

}  // namespace webrtc

#endif  // LOGGING_RTC_EVENT_LOG_RTC_EVENT_PROCESSOR_H_
