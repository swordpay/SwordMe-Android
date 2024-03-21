// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ANDROID_UNGUESSABLE_TOKEN_ANDROID_H_
#define BASE_ANDROID_UNGUESSABLE_TOKEN_ANDROID_H_

#include <jni.h>

#include "base/android/scoped_java_ref.h"
#include "base/base_export.h"
#include "base/unguessable_token.h"

namespace base {
namespace android {

class BASE_EXPORT UnguessableTokenAndroid {
 public:

  static ScopedJavaLocalRef<jobject> Create(
      JNIEnv* env,
      const base::UnguessableToken& token);

  static base::UnguessableToken FromJavaUnguessableToken(
      JNIEnv* env,
      const JavaRef<jobject>& token);



  static ScopedJavaLocalRef<jobject> ParcelAndUnparcelForTesting(
      JNIEnv* env,
      const JavaRef<jobject>& token);

 private:
  DISALLOW_COPY_AND_ASSIGN(UnguessableTokenAndroid);
};

}  // namespace android
}  // namespace base

#endif  // BASE_ANDROID_UNGUESSABLE_TOKEN_ANDROID_H_
