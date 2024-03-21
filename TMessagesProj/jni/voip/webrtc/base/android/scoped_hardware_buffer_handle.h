// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ANDROID_SCOPED_HARDWARE_BUFFER_HANDLE_H_
#define BASE_ANDROID_SCOPED_HARDWARE_BUFFER_HANDLE_H_

#include "base/base_export.h"
#include "base/files/scoped_file.h"
#include "base/macros.h"

extern "C" typedef struct AHardwareBuffer AHardwareBuffer;

namespace base {
namespace android {

class BASE_EXPORT ScopedHardwareBufferHandle {
 public:
  ScopedHardwareBufferHandle();

  ScopedHardwareBufferHandle(ScopedHardwareBufferHandle&& other);


  ~ScopedHardwareBufferHandle();


  static ScopedHardwareBufferHandle Adopt(AHardwareBuffer* buffer);

  static ScopedHardwareBufferHandle Create(AHardwareBuffer* buffer);

  ScopedHardwareBufferHandle& operator=(ScopedHardwareBufferHandle&& other);

  bool is_valid() const;

  AHardwareBuffer* get() const;


  void reset();





  AHardwareBuffer* Take() WARN_UNUSED_RESULT;


  ScopedHardwareBufferHandle Clone() const;








  ScopedFD SerializeAsFileDescriptor() const;






  static ScopedHardwareBufferHandle DeserializeFromFileDescriptor(ScopedFD fd)
      WARN_UNUSED_RESULT;

 private:


  explicit ScopedHardwareBufferHandle(AHardwareBuffer* buffer);

  AHardwareBuffer* buffer_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(ScopedHardwareBufferHandle);
};

}  // namespace android
}  // namespace base

#endif  // BASE_ANDROID_SCOPED_HARDWARE_BUFFER_HANDLE_H_
