// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/task_scheduler/post_task_android.h"

#include "base/android_runtime_jni_headers/Runnable_jni.h"
#include "base/base_jni_headers/PostTask_jni.h"
#include "base/no_destructor.h"
#include "base/run_loop.h"
#include "base/task/post_task.h"
#include "base/task/thread_pool/thread_pool_instance.h"

namespace base {

void PostTaskAndroid::SignalNativeSchedulerReady() {
  Java_PostTask_onNativeSchedulerReady(base::android::AttachCurrentThread());
}

void PostTaskAndroid::SignalNativeSchedulerShutdownForTesting() {
  Java_PostTask_onNativeSchedulerShutdownForTesting(
      base::android::AttachCurrentThread());
}

namespace {
std::array<uint8_t, TaskTraitsExtensionStorage::kStorageSize> GetExtensionData(
    JNIEnv* env,
    const base::android::JavaParamRef<jbyteArray>& array_object) {
  if (env->IsSameObject(array_object, nullptr))
    return std::array<uint8_t, TaskTraitsExtensionStorage::kStorageSize>();

  jbyteArray array = static_cast<jbyteArray>(array_object);
  DCHECK_EQ(env->GetArrayLength(array),
            static_cast<jsize>(TaskTraitsExtensionStorage::kStorageSize));

  std::array<uint8_t, TaskTraitsExtensionStorage::kStorageSize> result;
  jbyte* src_bytes = env->GetByteArrayElements(array, nullptr);
  memcpy(&result[0], src_bytes, TaskTraitsExtensionStorage::kStorageSize);
  env->ReleaseByteArrayElements(array, src_bytes, JNI_ABORT);
  return result;
}
}  // namespace

TaskTraits PostTaskAndroid::CreateTaskTraits(
    JNIEnv* env,
    jint priority,
    jboolean may_block,
    jboolean use_thread_pool,
    jbyte extension_id,
    const base::android::JavaParamRef<jbyteArray>& extension_data) {
  return TaskTraits(static_cast<TaskPriority>(priority), may_block,
                    use_thread_pool,
                    TaskTraitsExtensionStorage(
                        extension_id, GetExtensionData(env, extension_data)));
}

void JNI_PostTask_PostDelayedTask(
    JNIEnv* env,
    jint priority,
    jboolean may_block,
    jboolean use_thread_pool,
    jbyte extension_id,
    const base::android::JavaParamRef<jbyteArray>& extension_data,
    const base::android::JavaParamRef<jobject>& task,
    jlong delay) {


  PostDelayedTask(FROM_HERE,
                  PostTaskAndroid::CreateTaskTraits(
                      env, priority, may_block, use_thread_pool, extension_id,
                      extension_data),
                  BindOnce(&PostTaskAndroid::RunJavaTask,
                           base::android::ScopedJavaGlobalRef<jobject>(task)),
                  TimeDelta::FromMilliseconds(delay));
}

void PostTaskAndroid::RunJavaTask(
    base::android::ScopedJavaGlobalRef<jobject> task) {


  JNI_Runnable::Java_Runnable_run(base::android::AttachCurrentThread(), task);
}

}  // namespace base
