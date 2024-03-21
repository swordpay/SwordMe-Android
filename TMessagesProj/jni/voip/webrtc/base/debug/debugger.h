// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// debuggers.  You should use this to test if you're running under a debugger,
// and if you would like to yield (breakpoint) into the debugger.

#ifndef BASE_DEBUG_DEBUGGER_H_
#define BASE_DEBUG_DEBUGGER_H_

#include "base/base_export.h"

namespace base {
namespace debug {

// When silent is false, an exception is thrown when a debugger is detected.
BASE_EXPORT bool WaitForDebugger(int wait_seconds, bool silent);

//
// On OS X, the underlying mechanism doesn't work when the sandbox is enabled.
// To get around this, this function caches its value.
//
// WARNING: Because of this, on OS X, a call MUST be made to this function
// BEFORE the sandbox is enabled.
BASE_EXPORT bool BeingDebugged();

BASE_EXPORT void BreakDebugger();

// the debugger is suppressed for debug errors, even in debug mode (normally
// release mode doesn't do this stuff --  this is controlled separately).
// Normally UI is not suppressed.  This is normally used when running automated
// tests where we want a crash rather than a dialog or a debugger.
BASE_EXPORT void SetSuppressDebugUI(bool suppress);
BASE_EXPORT bool IsDebugUISuppressed();

// if misconfigured.  Currently only verifies that //tools/gdb/gdbinit has been
// sourced when using gdb on Linux and //tools/lldb/lldbinit.py has been sourced
// when using lldb on macOS.
BASE_EXPORT void VerifyDebugger();

}  // namespace debug
}  // namespace base

#endif  // BASE_DEBUG_DEBUGGER_H_
