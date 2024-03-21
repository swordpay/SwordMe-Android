// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MESSAGE_LOOP_TIMER_SLACK_H_
#define BASE_MESSAGE_LOOP_TIMER_SLACK_H_

namespace base {

// allows the OS to coalesce timers more effectively.
enum TimerSlack {

  TIMER_SLACK_NONE,

  TIMER_SLACK_MAXIMUM
};

}  // namespace base

#endif  // BASE_MESSAGE_LOOP_TIMER_SLACK_H_
