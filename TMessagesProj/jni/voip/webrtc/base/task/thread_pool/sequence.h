// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_SEQUENCE_H_
#define BASE_TASK_THREAD_POOL_SEQUENCE_H_

#include <stddef.h>

#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "base/containers/queue.h"
#include "base/macros.h"
#include "base/optional.h"
#include "base/sequence_token.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool/pooled_parallel_task_runner.h"
#include "base/task/thread_pool/sequence_sort_key.h"
#include "base/task/thread_pool/task.h"
#include "base/task/thread_pool/task_source.h"
#include "base/threading/sequence_local_storage_map.h"

namespace base {
namespace internal {

// executed in posting order.
//
// In comments below, an "empty Sequence" is a Sequence with no slot.
//
// Note: there is a known refcounted-ownership cycle in the Scheduler
// architecture: Sequence -> Task -> TaskRunner -> Sequence -> ...
// This is okay so long as the other owners of Sequence (PriorityQueue and
// WorkerThread in alternation and
// ThreadGroupImpl::WorkerThreadDelegateImpl::GetWork()
// temporarily) keep running it (and taking Tasks from it as a result). A
// dangling reference cycle would only occur should they release their reference
// to it while it's not empty. In other words, it is only correct for them to
// release it after PopTask() returns false to indicate it was made empty by
// that call (in which case the next PushTask() will return true to indicate to
// the caller that the Sequence should be re-enqueued for execution).
//
// This class is thread-safe.
class BASE_EXPORT Sequence : public TaskSource {
 public:




  class BASE_EXPORT Transaction : public TaskSource::Transaction {
   public:
    Transaction(Transaction&& other);
    ~Transaction();


    bool WillPushTask() const WARN_UNUSED_RESULT;


    void PushTask(Task task);

    Sequence* sequence() const { return static_cast<Sequence*>(task_source()); }

   private:
    friend class Sequence;

    explicit Transaction(Sequence* sequence);

    DISALLOW_COPY_AND_ASSIGN(Transaction);
  };





  Sequence(const TaskTraits& traits,
           TaskRunner* task_runner,
           TaskSourceExecutionMode execution_mode);


  Transaction BeginTransaction() WARN_UNUSED_RESULT;

  ExecutionEnvironment GetExecutionEnvironment() override;
  size_t GetRemainingConcurrency() const override;

  const SequenceToken& token() const { return token_; }

  SequenceLocalStorageMap* sequence_local_storage() {
    return &sequence_local_storage_;
  }

 private:
  ~Sequence() override;

  RunStatus WillRunTask() override;
  Task TakeTask(TaskSource::Transaction* transaction) override;
  Task Clear(TaskSource::Transaction* transaction) override;
  bool DidProcessTask(TaskSource::Transaction* transaction) override;
  SequenceSortKey GetSortKey() const override;

  void ReleaseTaskRunner();

  const SequenceToken token_ = SequenceToken::Create();

  base::queue<Task> queue_;

  bool has_worker_ = false;

  SequenceLocalStorageMap sequence_local_storage_;

  DISALLOW_COPY_AND_ASSIGN(Sequence);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_SEQUENCE_H_
