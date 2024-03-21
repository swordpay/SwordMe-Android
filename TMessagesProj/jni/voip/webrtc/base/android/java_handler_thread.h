// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ANDROID_JAVA_HANDLER_THREAD_H_
#define BASE_ANDROID_JAVA_HANDLER_THREAD_H_

#include <jni.h>

#include <memory>

#include "base/android/scoped_java_ref.h"
#include "base/single_thread_task_runner.h"
#include "base/task/sequence_manager/sequence_manager.h"
#include "base/task/sequence_manager/task_queue.h"
#include "base/threading/thread_task_runner_handle.h"

namespace base {

class MessagePumpForUI;

namespace android {

// to the message loop and they will be scheduled along with Java tasks
// on the thread.
// This is useful for callbacks where the receiver expects a thread
// with a prepared Looper.
class BASE_EXPORT JavaHandlerThread {
 public:

  explicit JavaHandlerThread(
      const char* name,
      base::ThreadPriority priority = base::ThreadPriority::NORMAL);


  explicit JavaHandlerThread(
      const char* name,
      const base::android::ScopedJavaLocalRef<jobject>& obj);
  virtual ~JavaHandlerThread();


  scoped_refptr<SingleThreadTaskRunner> task_runner() const {
    return state_ ? state_->default_task_queue->task_runner() : nullptr;
  }

  void Start();
  void Stop();


  void InitializeThread(JNIEnv* env,
                        jlong event);

  void OnLooperStopped(JNIEnv* env);

  void StopSequenceManagerForTesting();

  void JoinForTesting();


  void ListenForUncaughtExceptionsForTesting();

  ScopedJavaLocalRef<jthrowable> GetUncaughtExceptionIfAny();

 protected:


  struct State {
    State();
    ~State();

    std::unique_ptr<sequence_manager::SequenceManager> sequence_manager;
    scoped_refptr<sequence_manager::TaskQueue> default_task_queue;
    MessagePumpForUI* pump = nullptr;
  };

  State* state() const { return state_.get(); }



  virtual void Init() {}


  virtual void CleanUp() {}

  std::unique_ptr<State> state_;

 private:
  void StartMessageLoop();

  void StopOnThread();
  void QuitThreadSafely();

  const char* name_;
  ScopedJavaGlobalRef<jobject> java_thread_;
};

}  // namespace android
}  // namespace base

#endif  // BASE_ANDROID_JAVA_HANDLER_THREAD_H_
