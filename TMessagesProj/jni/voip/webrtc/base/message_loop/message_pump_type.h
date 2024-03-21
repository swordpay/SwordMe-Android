// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MESSAGE_LOOP_MESSAGE_PUMP_TYPE_H_
#define BASE_MESSAGE_LOOP_MESSAGE_PUMP_TYPE_H_

#include "build/build_config.h"

namespace base {

// asynchronous events it may process in addition to tasks and timers.

enum class MessagePumpType {

  DEFAULT,


  UI,

  CUSTOM,

  IO,

#if defined(OS_ANDROID)




  JAVA,
#endif  // defined(OS_ANDROID)

#if defined(OS_MACOSX)


  NS_RUNLOOP,
#endif  // defined(OS_MACOSX)

#if defined(OS_WIN)


  UI_WITH_WM_QUIT_SUPPORT,
#endif  // defined(OS_WIN)
};

}  // namespace base

#endif  // BASE_MESSAGE_LOOP_MESSAGE_PUMP_TYPE_H_
