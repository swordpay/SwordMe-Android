/*
 *  Copyright (c) 2021 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef SYSTEM_WRAPPERS_INCLUDE_DENORMAL_DISABLER_H_
#define SYSTEM_WRAPPERS_INCLUDE_DENORMAL_DISABLER_H_

#include "rtc_base/system/arch.h"

namespace webrtc {

// can very seriously impact performance. At destruction time restores the
// denormals handling state read by the ctor; hence, supports nested calls.
// Equals a no-op if the architecture is not x86 or ARM or if the compiler is
// not CLANG.
// [1] https://en.wikipedia.org/wiki/Denormal_number
//
// Example usage:
//
// void Foo() {
//   DenormalDisabler d;
//   ...
// }
class DenormalDisabler {
 public:




  explicit DenormalDisabler(bool enabled);
  DenormalDisabler(const DenormalDisabler&) = delete;
  DenormalDisabler& operator=(const DenormalDisabler&) = delete;


  ~DenormalDisabler();

  static bool IsSupported();

 private:
  const int status_word_;
  const bool disabling_activated_;
};

}  // namespace webrtc

#endif  // SYSTEM_WRAPPERS_INCLUDE_DENORMAL_DISABLER_H_
