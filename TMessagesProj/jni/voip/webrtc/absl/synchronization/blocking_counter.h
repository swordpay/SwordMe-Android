//
// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// -----------------------------------------------------------------------------
// blocking_counter.h
// -----------------------------------------------------------------------------

#ifndef ABSL_SYNCHRONIZATION_BLOCKING_COUNTER_H_
#define ABSL_SYNCHRONIZATION_BLOCKING_COUNTER_H_

#include <atomic>

#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"

namespace absl {
ABSL_NAMESPACE_BEGIN

//
// This class allows a thread to block for a pre-specified number of actions.
// `BlockingCounter` maintains a single non-negative abstract integer "count"
// with an initial value `initial_count`. A thread can then call `Wait()` on
// this blocking counter to block until the specified number of events occur;
// worker threads then call 'DecrementCount()` on the counter upon completion of
// their work. Once the counter's internal "count" reaches zero, the blocked
// thread unblocks.
//
// A `BlockingCounter` requires the following:
//     - its `initial_count` is non-negative.
//     - the number of calls to `DecrementCount()` on it is at most
//       `initial_count`.
//     - `Wait()` is called at most once on it.
//
// Given the above requirements, a `BlockingCounter` provides the following
// guarantees:
//     - Once its internal "count" reaches zero, no legal action on the object
//       can further change the value of "count".
//     - When `Wait()` returns, it is legal to destroy the `BlockingCounter`.
//     - When `Wait()` returns, the number of calls to `DecrementCount()` on
//       this blocking counter exactly equals `initial_count`.
//
// Example:
//     BlockingCounter bcount(N);         // there are N items of work
//     ... Allow worker threads to start.
//     ... On completing each work item, workers do:
//     ... bcount.DecrementCount();      // an item of work has been completed
//
//     bcount.Wait();                    // wait for all work to be complete
//
class BlockingCounter {
 public:
  explicit BlockingCounter(int initial_count);

  BlockingCounter(const BlockingCounter&) = delete;
  BlockingCounter& operator=(const BlockingCounter&) = delete;








  bool DecrementCount();









  void Wait();

 private:
  Mutex lock_;
  std::atomic<int> count_;
  int num_waiting_ ABSL_GUARDED_BY(lock_);
  bool done_ ABSL_GUARDED_BY(lock_);
};

ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_SYNCHRONIZATION_BLOCKING_COUNTER_H_
