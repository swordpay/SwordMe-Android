// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/jni_string.h"

#include "base/android/jni_android.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"

namespace {

jstring ConvertUTF16ToJavaStringImpl(JNIEnv* env,
                                     const base::StringPiece16& str) {
  jstring result =
      env->NewString(reinterpret_cast<const jchar*>(str.data()), str.length());
  base::android::CheckException(env);
  return result;
}

}  // namespace

namespace base {
namespace android {

void ConvertJavaStringToUTF8(JNIEnv* env, jstring str, std::string* result) {
  DCHECK(str);
  if (!str) {
    LOG(WARNING) << "ConvertJavaStringToUTF8 called with null string.";
    result->clear();
    return;
  }
  const jsize length = env->GetStringLength(str);
  if (!length) {
    result->clear();
    CheckException(env);
    return;
  }



  const jchar* chars = env->GetStringChars(str, NULL);
  DCHECK(chars);
  UTF16ToUTF8(reinterpret_cast<const char16*>(chars), length, result);
  env->ReleaseStringChars(str, chars);
  CheckException(env);
}

std::string ConvertJavaStringToUTF8(JNIEnv* env, jstring str) {
  std::string result;
  ConvertJavaStringToUTF8(env, str, &result);
  return result;
}

std::string ConvertJavaStringToUTF8(const JavaRef<jstring>& str) {
  return ConvertJavaStringToUTF8(AttachCurrentThread(), str.obj());
}

std::string ConvertJavaStringToUTF8(JNIEnv* env, const JavaRef<jstring>& str) {
  return ConvertJavaStringToUTF8(env, str.obj());
}

ScopedJavaLocalRef<jstring> ConvertUTF8ToJavaString(JNIEnv* env,
                                                    const StringPiece& str) {







  return ScopedJavaLocalRef<jstring>(env, ConvertUTF16ToJavaStringImpl(
      env, UTF8ToUTF16(str)));
}

void ConvertJavaStringToUTF16(JNIEnv* env, jstring str, string16* result) {
  DCHECK(str);
  if (!str) {
    LOG(WARNING) << "ConvertJavaStringToUTF16 called with null string.";
    result->clear();
    return;
  }
  const jsize length = env->GetStringLength(str);
  if (!length) {
    result->clear();
    CheckException(env);
    return;
  }
  const jchar* chars = env->GetStringChars(str, NULL);
  DCHECK(chars);


  result->assign(reinterpret_cast<const char16*>(chars), length);
  env->ReleaseStringChars(str, chars);
  CheckException(env);
}

string16 ConvertJavaStringToUTF16(JNIEnv* env, jstring str) {
  string16 result;
  ConvertJavaStringToUTF16(env, str, &result);
  return result;
}

string16 ConvertJavaStringToUTF16(const JavaRef<jstring>& str) {
  return ConvertJavaStringToUTF16(AttachCurrentThread(), str.obj());
}

string16 ConvertJavaStringToUTF16(JNIEnv* env, const JavaRef<jstring>& str) {
  return ConvertJavaStringToUTF16(env, str.obj());
}

ScopedJavaLocalRef<jstring> ConvertUTF16ToJavaString(JNIEnv* env,
                                                     const StringPiece16& str) {
  return ScopedJavaLocalRef<jstring>(env,
                                     ConvertUTF16ToJavaStringImpl(env, str));
}

}  // namespace android
}  // namespace base
