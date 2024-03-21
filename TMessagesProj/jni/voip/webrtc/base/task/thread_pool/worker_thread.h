// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_WORKER_THREAD_H_
#define BASE_TASK_THREAD_POOL_WORKER_THREAD_H_

#include <memory>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/synchronization/atomic_flag.h"
#include "base/synchronization/waitable_event.h"
#include "base/task/common/checked_lock.h"
#include "base/task/thread_pool/task_source.h"
#include "base/task/thread_pool/tracked_ref.h"
#include "base/thread_annotations.h"
#include "base/threading/platform_thread.h"
#include "base/time/time.h"
#include "build/build_config.h"

namespace base {

class WorkerThreadObserver;

namespace internal {

class TaskTracker;

// by a delegate.
//
// A WorkerThread starts out sleeping. It is woken up by a call to WakeUp().
// After a wake-up, a WorkerThread runs Tasks from TaskSources returned by
// the GetWork() method of its delegate as long as it doesn't return nullptr. It
// also periodically checks with its TaskTracker whether shutdown has completed
// and exits when it has.
//
// This class is thread-safe.
class BASE_EXPORT WorkerThread : public RefCountedThreadSafe<WorkerThread>,
                                 public PlatformThread::Delegate {
 public:



  enum class ThreadLabel {
    POOLED,
    SHARED,
    DEDICATED,
#if defined(OS_WIN)
    SHARED_COM,
    DEDICATED_COM,
#endif  // defined(OS_WIN)
  };


  class BASE_EXPORT Delegate {
   public:
    virtual ~Delegate() = default;


    virtual ThreadLabel GetThreadLabel() const = 0;

    virtual void OnMainEntry(const WorkerThread* worker) = 0;

    virtual RegisteredTaskSource GetWork(WorkerThread* worker) = 0;



    virtual void DidProcessTask(RegisteredTaskSource task_source) = 0;



    virtual TimeDelta GetSleepTimeout() = 0;




    virtual void WaitForWork(WaitableEvent* wake_up_event);




    virtual void OnMainExit(WorkerThread* worker) {}
  };









  WorkerThread(ThreadPriority priority_hint,
               std::unique_ptr<Delegate> delegate,
               TrackedRef<TaskTracker> task_tracker,
               const CheckedLock* predecessor_lock = nullptr);






  bool Start(WorkerThreadObserver* worker_thread_observer = nullptr);





  void WakeUp();

  WorkerThread::Delegate* delegate() { return delegate_.get(); }






  void JoinForTesting();

  bool ThreadAliveForTesting() const;









  void Cleanup();


  void BeginUnusedPeriod();
  void EndUnusedPeriod();


  TimeTicks GetLastUsedTime() const;

 private:
  friend class RefCountedThreadSafe<WorkerThread>;
  class Thread;

  ~WorkerThread() override;

  bool ShouldExit() const;


  ThreadPriority GetDesiredThreadPriority() const;


  void UpdateThreadPriority(ThreadPriority desired_thread_priority);

  void ThreadMain() override;


  void RunPooledWorker();
  void RunBackgroundPooledWorker();
  void RunSharedWorker();
  void RunBackgroundSharedWorker();
  void RunDedicatedWorker();
  void RunBackgroundDedicatedWorker();
#if defined(OS_WIN)
  void RunSharedCOMWorker();
  void RunBackgroundSharedCOMWorker();
  void RunDedicatedCOMWorker();
  void RunBackgroundDedicatedCOMWorker();
#endif  // defined(OS_WIN)




  void RunWorker();




  scoped_refptr<WorkerThread> self_;

  mutable CheckedLock thread_lock_;

  PlatformThreadHandle thread_handle_ GUARDED_BY(thread_lock_);


  TimeTicks last_used_time_ GUARDED_BY(thread_lock_);

  WaitableEvent wake_up_event_{WaitableEvent::ResetPolicy::AUTOMATIC,
                               WaitableEvent::InitialState::NOT_SIGNALED};

  AtomicFlag should_exit_;

  const std::unique_ptr<Delegate> delegate_;
  const TrackedRef<TaskTracker> task_tracker_;


  WorkerThreadObserver* worker_thread_observer_ = nullptr;

  const ThreadPriority priority_hint_;



  ThreadPriority current_thread_priority_;

  AtomicFlag join_called_for_testing_;

  DISALLOW_COPY_AND_ASSIGN(WorkerThread);
};

}  // namespace internal
}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_WORKER_THREAD_H_
