// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ONE_SHOT_EVENT_H_
#define BASE_ONE_SHOT_EVENT_H_

#include <vector>

#include "base/callback_forward.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/threading/thread_checker.h"

namespace base {

class Location;
class SingleThreadTaskRunner;
class TimeDelta;

// allows clients to guarantee that code is run after the OneShotEvent
// is signaled.  If the OneShotEvent is destroyed before it's
// signaled, the closures are destroyed without being run.
//
// This class is similar to a WaitableEvent combined with several
// WaitableEventWatchers, but using it is simpler.
//
// This class is not thread-safe, and must be used from a single thread.
class BASE_EXPORT OneShotEvent {
 public:
  OneShotEvent();




  explicit OneShotEvent(bool signaled);
  ~OneShotEvent();



  bool is_signaled() const {
    DCHECK(thread_checker_.CalledOnValidThread());
    return signaled_;
  }


  void Signal();





















  void Post(const Location& from_here, OnceClosure task) const;
  void Post(const Location& from_here,
            OnceClosure task,
            const scoped_refptr<SingleThreadTaskRunner>& runner) const;
  void PostDelayed(const Location& from_here,
                   OnceClosure task,
                   const TimeDelta& delay) const;

 private:
  struct TaskInfo;

  void PostImpl(const Location& from_here,
                OnceClosure task,
                const scoped_refptr<SingleThreadTaskRunner>& runner,
                const TimeDelta& delay) const;

  ThreadChecker thread_checker_;

  bool signaled_;









  mutable std::vector<TaskInfo> tasks_;
};

}  // namespace base

#endif  // BASE_ONE_SHOT_EVENT_H_
