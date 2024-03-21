// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_THREAD_POOL_WORKER_THREAD_OBSERVER_H_
#define BASE_TASK_THREAD_POOL_WORKER_THREAD_OBSERVER_H_

namespace base {

// worker.
class WorkerThreadObserver {
 public:
  virtual ~WorkerThreadObserver() = default;


  virtual void OnWorkerThreadMainEntry() = 0;


  virtual void OnWorkerThreadMainExit() = 0;
};

}  // namespace base

#endif  // BASE_TASK_THREAD_POOL_WORKER_THREAD_OBSERVER_H_
