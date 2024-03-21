// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//          Chrome's message-loop based Thread abstraction, and if you are a
//          thread running in the browser, there will likely be assumptions
//          that your thread will have an associated message loop.
//
// This is a simple thread interface that backs to a native operating system
// thread.  You should use this only when you want a thread that does not have
// an associated MessageLoop.  Unittesting is the best example of this.
//
// The simplest interface to use is DelegateSimpleThread, which will create
// a new thread, and execute the Delegate's virtual Run() in this new thread
// until it has completed, exiting the thread.
//
// NOTE: You *MUST* call Join on the thread to clean up the underlying thread
// resources.  You are also responsible for destructing the SimpleThread object.
// It is invalid to destroy a SimpleThread while it is running, or without
// Start() having been called (and a thread never created).  The Delegate
// object should live as long as a DelegateSimpleThread.
//
// Thread Safety: A SimpleThread is not completely thread safe.  It is safe to
// access it from the creating thread or from the newly created thread.  This
// implies that the creator thread should be the thread that calls Join.
//
// Example:
//   class MyThreadRunner : public DelegateSimpleThread::Delegate { ... };
//   MyThreadRunner runner;
//   DelegateSimpleThread thread(&runner, "good_name_here");
//   thread.Start();
//   // Start will return after the Thread has been successfully started and
//   // initialized.  The newly created thread will invoke runner->Run(), and
//   // run until it returns.
//   thread.Join();  // Wait until the thread has exited.  You *MUST* Join!
//   // The SimpleThread object is still valid, however you may not call Join
//   // or Start again.

#ifndef BASE_THREADING_SIMPLE_THREAD_H_
#define BASE_THREADING_SIMPLE_THREAD_H_

#include <stddef.h>

#include <string>
#include <vector>

#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "base/containers/queue.h"
#include "base/macros.h"
#include "base/synchronization/lock.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/platform_thread.h"

namespace base {

// virtual Run method, or you can use the DelegateSimpleThread interface.
class BASE_EXPORT SimpleThread : public PlatformThread::Delegate {
 public:
  struct BASE_EXPORT Options {
   public:
    Options() = default;
    explicit Options(ThreadPriority priority_in) : priority(priority_in) {}
    ~Options() = default;

    Options(const Options& other) = default;
    Options& operator=(const Options& other) = default;

    size_t stack_size = 0;

    ThreadPriority priority = ThreadPriority::NORMAL;




    bool joinable = true;
  };




  explicit SimpleThread(const std::string& name);
  SimpleThread(const std::string& name, const Options& options);

  ~SimpleThread() override;


  void Start();


  void Join();



  void StartAsync();

  virtual void Run() = 0;




  PlatformThreadId tid();




  bool HasBeenStarted();

  bool HasBeenJoined() const { return joined_; }

  bool HasStartBeenAttempted() { return start_called_; }

  void ThreadMain() override;

 private:


  virtual void BeforeStart() {}


  virtual void BeforeRun() {}


  virtual void BeforeJoin() {}

  const std::string name_;
  const Options options_;
  PlatformThreadHandle thread_;  // PlatformThread handle, reset after Join.
  WaitableEvent event_;          // Signaled if Start() was ever called.
  PlatformThreadId tid_ = kInvalidThreadId;  // The backing thread's id.
  bool joined_ = false;                      // True if Join has been called.

  bool start_called_ = false;

  DISALLOW_COPY_AND_ASSIGN(SimpleThread);
};

// DelegateSimpleThread are safe to delete after Run() was invoked, their
// Delegates are also safe to delete after that point from this class' point of
// view (although implementations must of course make sure that Run() will not
// use their Delegate's member state after its deletion).
class BASE_EXPORT DelegateSimpleThread : public SimpleThread {
 public:
  class BASE_EXPORT Delegate {
   public:
    virtual ~Delegate() = default;
    virtual void Run() = 0;
  };

  DelegateSimpleThread(Delegate* delegate,
                       const std::string& name_prefix);
  DelegateSimpleThread(Delegate* delegate,
                       const std::string& name_prefix,
                       const Options& options);

  ~DelegateSimpleThread() override;
  void Run() override;

 private:
  Delegate* delegate_;

  DISALLOW_COPY_AND_ASSIGN(DelegateSimpleThread);
};

// and then add jobs which will be dispatched to the threads.  This is
// convenient when you have a lot of small work that you want done
// multi-threaded, but don't want to spawn a thread for each small bit of work.
//
// You just call AddWork() to add a delegate to the list of work to be done.
// JoinAll() will make sure that all outstanding work is processed, and wait
// for everything to finish.  You can reuse a pool, so you can call Start()
// again after you've called JoinAll().
class BASE_EXPORT DelegateSimpleThreadPool
    : public DelegateSimpleThread::Delegate {
 public:
  typedef DelegateSimpleThread::Delegate Delegate;

  DelegateSimpleThreadPool(const std::string& name_prefix, int num_threads);
  ~DelegateSimpleThreadPool() override;


  void Start();


  void JoinAll();


  void AddWork(Delegate* work, int repeat_count);
  void AddWork(Delegate* work) {
    AddWork(work, 1);
  }

  void Run() override;

 private:
  const std::string name_prefix_;
  int num_threads_;
  std::vector<DelegateSimpleThread*> threads_;
  base::queue<Delegate*> delegates_;
  base::Lock lock_;            // Locks delegates_
  WaitableEvent dry_;    // Not signaled when there is no work to do.

  DISALLOW_COPY_AND_ASSIGN(DelegateSimpleThreadPool);
};

}  // namespace base

#endif  // BASE_THREADING_SIMPLE_THREAD_H_
