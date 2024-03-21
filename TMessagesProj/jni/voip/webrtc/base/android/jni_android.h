// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ANDROID_JNI_ANDROID_H_
#define BASE_ANDROID_JNI_ANDROID_H_

#include <jni.h>
#include <sys/types.h>

#include <atomic>
#include <string>

#include "base/android/scoped_java_ref.h"
#include "base/atomicops.h"
#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "base/debug/debugging_buildflags.h"
#include "base/debug/stack_trace.h"
#include "base/macros.h"

#if BUILDFLAG(CAN_UNWIND_WITH_FRAME_POINTERS)

// all generated JNI stubs so that it becomes the last thing that runs before
// control goes into Java.
//
// This macro saves stack frame pointer of the current function. Saved value
// used later by JNI_LINK_SAVED_FRAME_POINTER.
#define JNI_SAVE_FRAME_POINTER \
  base::android::JNIStackFrameSaver jni_frame_saver(__builtin_frame_address(0))

// all generated JNI callbacks so that it becomes the first thing that runs
// after control returns from Java.
//
// This macro links stack frame of the current function to the stack frame
// saved by JNI_SAVE_FRAME_POINTER, allowing frame-based unwinding
// (used by the heap profiler) to produce complete traces.
#define JNI_LINK_SAVED_FRAME_POINTER                    \
  base::debug::ScopedStackFrameLinker jni_frame_linker( \
      __builtin_frame_address(0),                       \
      base::android::JNIStackFrameSaver::SavedFrame())

#else

#define JNI_SAVE_FRAME_POINTER
#define JNI_LINK_SAVED_FRAME_POINTER

#endif  // BUILDFLAG(CAN_UNWIND_WITH_FRAME_POINTERS)

namespace base {
namespace android {

#define JNI_EXPORT __attribute__ ((visibility("default")))

struct RegistrationMethod {
  const char* name;
  bool (*func)(JNIEnv* env);
};

BASE_EXPORT JNIEnv* AttachCurrentThread();

// |thread_name| if it is the first call. Otherwise, thread_name won't be
// changed. AttachCurrentThread() doesn't regard underlying platform thread
// name, but just resets it to "Thread-???". This function should be called
// right after new thread is created if it is important to keep thread name.
BASE_EXPORT JNIEnv* AttachCurrentThreadWithName(const std::string& thread_name);

BASE_EXPORT void DetachFromVM();

BASE_EXPORT void InitVM(JavaVM* vm);

BASE_EXPORT bool IsVMInitialized();

// methods. This is needed because JNI will use the base ClassLoader when there
// is no Java code on the stack. The base ClassLoader doesn't know about any of
// the application classes and will fail to lookup anything other than system
// classes.
BASE_EXPORT void InitReplacementClassLoader(
    JNIEnv* env,
    const JavaRef<jobject>& class_loader);

// Use this method instead of invoking directly the JNI FindClass method (to
// prevent leaking local references).
// This method triggers a fatal assertion if the class could not be found.
// Use HasClass if you need to check whether the class exists.
BASE_EXPORT ScopedJavaLocalRef<jclass> GetClass(JNIEnv* env,
                                                const char* class_name);

// class. And will return that ref on subsequent calls.  It's the caller's
// responsibility to release the ref when it is no longer needed.
// The caller is responsible to zero-initialize |atomic_method_id|.
// It's fine to simultaneously call this on multiple threads referencing the
// same |atomic_method_id|.
BASE_EXPORT jclass LazyGetClass(
    JNIEnv* env,
    const char* class_name,
    std::atomic<jclass>* atomic_class_id);

class BASE_EXPORT MethodID {
 public:
  enum Type {
    TYPE_STATIC,
    TYPE_INSTANCE,
  };


  template<Type type>
  static jmethodID Get(JNIEnv* env,
                       jclass clazz,
                       const char* method_name,
                       const char* jni_signature);



  template<Type type>
  static jmethodID LazyGet(JNIEnv* env,
                           jclass clazz,
                           const char* method_name,
                           const char* jni_signature,
                           std::atomic<jmethodID>* atomic_method_id);
};

BASE_EXPORT bool HasException(JNIEnv* env);

// and returns true.
BASE_EXPORT bool ClearException(JNIEnv* env);

BASE_EXPORT void CheckException(JNIEnv* env);

BASE_EXPORT std::string GetJavaExceptionInfo(JNIEnv* env,
                                             jthrowable java_throwable);

#if BUILDFLAG(CAN_UNWIND_WITH_FRAME_POINTERS)

// Implemented only when profiling is enabled (enable_profiling=true).
class BASE_EXPORT JNIStackFrameSaver {
 public:
  JNIStackFrameSaver(void* current_fp);
  ~JNIStackFrameSaver();
  static void* SavedFrame();

 private:
  void* previous_fp_;

  DISALLOW_COPY_AND_ASSIGN(JNIStackFrameSaver);
};

#endif  // BUILDFLAG(CAN_UNWIND_WITH_FRAME_POINTERS)

}  // namespace android
}  // namespace base

#endif  // BASE_ANDROID_JNI_ANDROID_H_
