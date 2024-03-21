// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ANDROID_TASK_SCHEDULER_POST_TASK_ANDROID_H_
#define BASE_ANDROID_TASK_SCHEDULER_POST_TASK_ANDROID_H_

#include "base/android/jni_weak_ref.h"
#include "base/base_export.h"
#include "base/task/task_traits.h"

namespace base {

class BASE_EXPORT PostTaskAndroid {
 public:


  static void SignalNativeSchedulerReady();


  static void SignalNativeSchedulerShutdownForTesting();

  static TaskTraits CreateTaskTraits(
      JNIEnv* env,
      jint priority,
      jboolean may_block,
      jboolean use_thread_pool,
      jbyte extension_id,
      const base::android::JavaParamRef<jbyteArray>& extension_data);


  static void RunJavaTask(base::android::ScopedJavaGlobalRef<jobject> task);

  DISALLOW_COPY_AND_ASSIGN(PostTaskAndroid);
};

}  // namespace base

#endif  // BASE_ANDROID_TASK_SCHEDULER_POST_TASK_ANDROID_H_
