// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// macros in a small executable file that's easy to disassemble.

#include "base/compiler_specific.h"
#include "base/logging.h"

// the CHECK* macros, nor should it have the strings in the
// executable. It is also important that the CHECK() function collapse to the
// same implementation as RELEASE_ASSERT(), in particular on Windows x86.
// Historically, the stream eating caused additional unnecessary instructions.
// See https://crbug.com/672699.

#define BLINK_RELEASE_ASSERT_EQUIVALENT(assertion) \
  (UNLIKELY(!(assertion)) ? (IMMEDIATE_CRASH()) : (void)0)

void DoCheck(bool b) {
  CHECK(b) << "DoCheck " << b;
}

void DoBlinkReleaseAssert(bool b) {
  BLINK_RELEASE_ASSERT_EQUIVALENT(b);
}

void DoCheckEq(int x, int y) {
  CHECK_EQ(x, y);
}

int main(int argc, const char* argv[]) {
  DoCheck(argc > 1);
  DoCheckEq(argc, 1);
  DoBlinkReleaseAssert(argc > 1);
}
