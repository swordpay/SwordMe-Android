// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/threading/platform_thread.h"

#include <errno.h>
#include <stddef.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>

#include "base/android/jni_android.h"
#include "base/base_jni_headers/ThreadUtils_jni.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/threading/platform_thread_internal_posix.h"
#include "base/threading/thread_id_name_manager.h"

namespace base {

namespace internal {

// result in heavy throttling and force the thread onto a little core on
// big.LITTLE devices.
// - DISPLAY corresponds to Android's PRIORITY_DISPLAY = -4 value.
// - REALTIME_AUDIO corresponds to Android's PRIORITY_AUDIO = -16 value.
const ThreadPriorityToNiceValuePair kThreadPriorityToNiceValueMap[4] = {
    {ThreadPriority::BACKGROUND, 10},
    {ThreadPriority::NORMAL, 0},
    {ThreadPriority::DISPLAY, -4},
    {ThreadPriority::REALTIME_AUDIO, -16},
};

Optional<bool> CanIncreaseCurrentThreadPriorityForPlatform(
    ThreadPriority priority) {
  if (priority == ThreadPriority::REALTIME_AUDIO)
    return base::make_optional(true);
  return base::nullopt;
}

bool SetCurrentThreadPriorityForPlatform(ThreadPriority priority) {


  if (priority == ThreadPriority::REALTIME_AUDIO) {
    JNIEnv* env = base::android::AttachCurrentThread();
    Java_ThreadUtils_setThreadPriorityAudio(env, PlatformThread::CurrentId());
    return true;
  }
  return false;
}

Optional<ThreadPriority> GetCurrentThreadPriorityForPlatform() {
  JNIEnv* env = base::android::AttachCurrentThread();
  if (Java_ThreadUtils_isThreadPriorityAudio(
      env, PlatformThread::CurrentId())) {
    return base::make_optional(ThreadPriority::REALTIME_AUDIO);
  }
  return base::nullopt;
}

}  // namespace internal

void PlatformThread::SetName(const std::string& name) {
  ThreadIdNameManager::GetInstance()->SetName(name);




  if (PlatformThread::CurrentId() == getpid())
    return;

  int err = prctl(PR_SET_NAME, name.c_str());
  if (err < 0 && errno != EPERM)
    DPLOG(ERROR) << "prctl(PR_SET_NAME)";
}


void InitThreading() {
}

void TerminateOnThread() {
  base::android::DetachFromVM();
}

size_t GetDefaultThreadStackSize(const pthread_attr_t& attributes) {
#if !defined(ADDRESS_SANITIZER)
  return 0;
#else


  return 2 * (1 << 20);  // 2Mb
#endif
}

}  // namespace base
