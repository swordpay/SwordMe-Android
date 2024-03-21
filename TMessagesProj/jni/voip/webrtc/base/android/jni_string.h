// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ANDROID_JNI_STRING_H_
#define BASE_ANDROID_JNI_STRING_H_

#include <jni.h>
#include <string>

#include "base/android/scoped_java_ref.h"
#include "base/base_export.h"
#include "base/strings/string_piece.h"

namespace base {
namespace android {

BASE_EXPORT void ConvertJavaStringToUTF8(JNIEnv* env,
                                         jstring str,
                                         std::string* result);
BASE_EXPORT std::string ConvertJavaStringToUTF8(JNIEnv* env, jstring str);
BASE_EXPORT std::string ConvertJavaStringToUTF8(const JavaRef<jstring>& str);
BASE_EXPORT std::string ConvertJavaStringToUTF8(JNIEnv* env,
                                                const JavaRef<jstring>& str);

BASE_EXPORT ScopedJavaLocalRef<jstring> ConvertUTF8ToJavaString(
    JNIEnv* env,
    const base::StringPiece& str);

BASE_EXPORT void ConvertJavaStringToUTF16(JNIEnv* env,
                                          jstring str,
                                          string16* result);
BASE_EXPORT string16 ConvertJavaStringToUTF16(JNIEnv* env, jstring str);
BASE_EXPORT string16 ConvertJavaStringToUTF16(const JavaRef<jstring>& str);
BASE_EXPORT string16 ConvertJavaStringToUTF16(JNIEnv* env,
                                              const JavaRef<jstring>& str);

BASE_EXPORT ScopedJavaLocalRef<jstring> ConvertUTF16ToJavaString(
    JNIEnv* env,
    const base::StringPiece16& str);

}  // namespace android
}  // namespace base

#endif  // BASE_ANDROID_JNI_STRING_H_
