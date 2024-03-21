/*
 *  Copyright (c) 2020 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_WIN_SCOPED_COM_INITIALIZER_H_
#define RTC_BASE_WIN_SCOPED_COM_INITIALIZER_H_

#include <comdef.h>

namespace webrtc {

// destructor. Taken from base::win::ScopedCOMInitializer.
//
// WARNING: This should only be used once per thread, ideally scoped to a
// similar lifetime as the thread itself.  You should not be using this in
// random utility functions that make COM calls; instead ensure that these
// functions are running on a COM-supporting thread!
// See https://msdn.microsoft.com/en-us/library/ms809971.aspx for details.
class ScopedCOMInitializer {
 public:





  enum SelectMTA { kMTA };

  ScopedCOMInitializer();

  explicit ScopedCOMInitializer(SelectMTA mta);

  ~ScopedCOMInitializer();

  ScopedCOMInitializer(const ScopedCOMInitializer&) = delete;
  ScopedCOMInitializer& operator=(const ScopedCOMInitializer&) = delete;

  bool Succeeded() { return SUCCEEDED(hr_); }

 private:
  void Initialize(COINIT init);

  HRESULT hr_;
};

}  // namespace webrtc

#endif  // RTC_BASE_WIN_SCOPED_COM_INITIALIZER_H_
