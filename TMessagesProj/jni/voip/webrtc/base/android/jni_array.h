// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ANDROID_JNI_ARRAY_H_
#define BASE_ANDROID_JNI_ARRAY_H_

#include <jni.h>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <vector>

#include "base/android/scoped_java_ref.h"
#include "base/containers/span.h"
#include "base/strings/string16.h"

namespace base {
namespace android {

BASE_EXPORT ScopedJavaLocalRef<jbyteArray> ToJavaByteArray(JNIEnv* env,
                                                           const uint8_t* bytes,
                                                           size_t len);

BASE_EXPORT ScopedJavaLocalRef<jbyteArray> ToJavaByteArray(
    JNIEnv* env,
    base::span<const uint8_t> bytes);

// conversion is performed.
BASE_EXPORT ScopedJavaLocalRef<jbyteArray> ToJavaByteArray(
    JNIEnv* env,
    const std::string& str);

BASE_EXPORT ScopedJavaLocalRef<jbooleanArray>
ToJavaBooleanArray(JNIEnv* env, const bool* bools, size_t len);

BASE_EXPORT ScopedJavaLocalRef<jintArray> ToJavaIntArray(
    JNIEnv* env, const int* ints, size_t len);

BASE_EXPORT ScopedJavaLocalRef<jintArray> ToJavaIntArray(
    JNIEnv* env,
    base::span<const int> ints);

BASE_EXPORT ScopedJavaLocalRef<jlongArray> ToJavaLongArray(JNIEnv* env,
                                                           const int64_t* longs,
                                                           size_t len);

BASE_EXPORT ScopedJavaLocalRef<jlongArray> ToJavaLongArray(
    JNIEnv* env,
    base::span<const int64_t> longs);

BASE_EXPORT ScopedJavaLocalRef<jfloatArray> ToJavaFloatArray(
    JNIEnv* env, const float* floats, size_t len);

BASE_EXPORT ScopedJavaLocalRef<jfloatArray> ToJavaFloatArray(
    JNIEnv* env,
    base::span<const float> floats);

BASE_EXPORT ScopedJavaLocalRef<jdoubleArray>
ToJavaDoubleArray(JNIEnv* env, const double* doubles, size_t len);

BASE_EXPORT ScopedJavaLocalRef<jdoubleArray> ToJavaDoubleArray(
    JNIEnv* env,
    base::span<const double> doubles);

BASE_EXPORT ScopedJavaLocalRef<jobjectArray> ToJavaArrayOfByteArray(
    JNIEnv* env,
    base::span<const std::string> v);

BASE_EXPORT ScopedJavaLocalRef<jobjectArray> ToJavaArrayOfByteArray(
    JNIEnv* env,
    base::span<std::vector<uint8_t>> v);

BASE_EXPORT ScopedJavaLocalRef<jobjectArray> ToJavaArrayOfStrings(
    JNIEnv* env,
    base::span<const std::string> v);

BASE_EXPORT ScopedJavaLocalRef<jobjectArray> ToJavaArrayOfStrings(
    JNIEnv* env,
    base::span<const string16> v);

BASE_EXPORT ScopedJavaLocalRef<jobjectArray> ToJavaArrayOfStringArray(
    JNIEnv* env,
    base::span<const std::vector<string16>> v);

BASE_EXPORT void AppendJavaStringArrayToStringVector(
    JNIEnv* env,
    const JavaRef<jobjectArray>& array,
    std::vector<string16>* out);

BASE_EXPORT void AppendJavaStringArrayToStringVector(
    JNIEnv* env,
    const JavaRef<jobjectArray>& array,
    std::vector<std::string>* out);

BASE_EXPORT void AppendJavaByteArrayToByteVector(
    JNIEnv* env,
    const JavaRef<jbyteArray>& byte_array,
    std::vector<uint8_t>* out);

BASE_EXPORT void JavaByteArrayToByteVector(
    JNIEnv* env,
    const JavaRef<jbyteArray>& byte_array,
    std::vector<uint8_t>* out);

// conversion is performed.
BASE_EXPORT void JavaByteArrayToString(JNIEnv* env,
                                       const JavaRef<jbyteArray>& byte_array,
                                       std::string* out);

BASE_EXPORT void JavaBooleanArrayToBoolVector(
    JNIEnv* env,
    const JavaRef<jbooleanArray>& boolean_array,
    std::vector<bool>* out);

BASE_EXPORT void JavaIntArrayToIntVector(JNIEnv* env,
                                         const JavaRef<jintArray>& int_array,
                                         std::vector<int>* out);

BASE_EXPORT void JavaLongArrayToInt64Vector(
    JNIEnv* env,
    const JavaRef<jlongArray>& long_array,
    std::vector<int64_t>* out);

BASE_EXPORT void JavaLongArrayToLongVector(
    JNIEnv* env,
    const JavaRef<jlongArray>& long_array,
    std::vector<jlong>* out);

BASE_EXPORT void JavaFloatArrayToFloatVector(
    JNIEnv* env,
    const JavaRef<jfloatArray>& float_array,
    std::vector<float>* out);

// content of |out| with the corresponding vector of strings. No UTF-8
// conversion is performed.
BASE_EXPORT void JavaArrayOfByteArrayToStringVector(
    JNIEnv* env,
    const JavaRef<jobjectArray>& array,
    std::vector<std::string>* out);

// content of |out| with the corresponding vector of vector of uint8. No UTF-8
// conversion is performed.
BASE_EXPORT void JavaArrayOfByteArrayToBytesVector(
    JNIEnv* env,
    const JavaRef<jobjectArray>& array,
    std::vector<std::vector<uint8_t>>* out);

// content of |out| with the corresponding vector of string vectors. No UTF-8
// conversion is performed.
BASE_EXPORT void Java2dStringArrayTo2dStringVector(
    JNIEnv* env,
    const JavaRef<jobjectArray>& array,
    std::vector<std::vector<string16>>* out);

// contents of |out| with the corresponding vectors of ints.
BASE_EXPORT void JavaArrayOfIntArrayToIntVector(
    JNIEnv* env,
    const JavaRef<jobjectArray>& array,
    std::vector<std::vector<int>>* out);

}  // namespace android
}  // namespace base

#endif  // BASE_ANDROID_JNI_ARRAY_H_
