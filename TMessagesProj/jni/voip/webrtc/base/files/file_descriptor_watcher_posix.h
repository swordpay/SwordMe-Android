// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_FILES_FILE_DESCRIPTOR_WATCHER_POSIX_H_
#define BASE_FILES_FILE_DESCRIPTOR_WATCHER_POSIX_H_

#include <memory>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "base/message_loop/message_pump_for_io.h"
#include "base/sequence_checker.h"
#include "base/single_thread_task_runner.h"

namespace base {

class SingleThreadTaskRunner;

// descriptors are readable or writable without blocking.
//
// To enable this API in unit tests, use a TaskEnvironment with
// MainThreadType::IO.
//
// Note: Prefer FileDescriptorWatcher to MessageLoopForIO::WatchFileDescriptor()
// for non-critical IO. FileDescriptorWatcher works on threads/sequences without
// MessagePumps but involves going through the task queue after being notified
// by the OS (a desirablable property for non-critical IO that shouldn't preempt
// the main queue).
class BASE_EXPORT FileDescriptorWatcher {
 public:



  class Controller {
   public:

    ~Controller();

   private:
    friend class FileDescriptorWatcher;
    class Watcher;


    Controller(MessagePumpForIO::Mode mode,
               int fd,
               const RepeatingClosure& callback);

    void StartWatching();

    void RunCallback();


    RepeatingClosure callback_;


    const scoped_refptr<SingleThreadTaskRunner> io_thread_task_runner_;







    std::unique_ptr<Watcher> watcher_;


    SequenceChecker sequence_checker_;

    WeakPtrFactory<Controller> weak_factory_{this};

    DISALLOW_COPY_AND_ASSIGN(Controller);
  };






  explicit FileDescriptorWatcher(
      scoped_refptr<SingleThreadTaskRunner> io_thread_task_runner);
  ~FileDescriptorWatcher();













  static std::unique_ptr<Controller> WatchReadable(
      int fd,
      const RepeatingClosure& callback);
  static std::unique_ptr<Controller> WatchWritable(
      int fd,
      const RepeatingClosure& callback);

  static void AssertAllowed()
#if DCHECK_IS_ON()
      ;
#else
  {
  }
#endif

 private:
  scoped_refptr<SingleThreadTaskRunner> io_thread_task_runner() const {
    return io_thread_task_runner_;
  }

  const scoped_refptr<SingleThreadTaskRunner> io_thread_task_runner_;

  DISALLOW_COPY_AND_ASSIGN(FileDescriptorWatcher);
};

}  // namespace base

#endif  // BASE_FILES_FILE_DESCRIPTOR_WATCHER_POSIX_H_
