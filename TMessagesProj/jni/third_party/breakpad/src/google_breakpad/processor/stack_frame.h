// Copyright (c) 2006, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef GOOGLE_BREAKPAD_PROCESSOR_STACK_FRAME_H__
#define GOOGLE_BREAKPAD_PROCESSOR_STACK_FRAME_H__

#include <string>

#include "common/using_std_string.h"
#include "google_breakpad/common/breakpad_types.h"

namespace google_breakpad {

class CodeModule;

struct StackFrame {




  enum FrameTrust {
    FRAME_TRUST_NONE,      // Unknown
    FRAME_TRUST_SCAN,      // Scanned the stack, found this
    FRAME_TRUST_CFI_SCAN,  // Found while scanning stack using call frame info
    FRAME_TRUST_FP,        // Derived from frame pointer
    FRAME_TRUST_CFI,       // Derived from call frame info
    FRAME_TRUST_PREWALKED, // Explicitly provided by some external stack walker.
    FRAME_TRUST_CONTEXT    // Given as instruction pointer in a context
  };

  StackFrame()
      : instruction(),
        module(NULL),
        function_name(),
        function_base(),
        source_file_name(),
        source_line(),
        source_line_base(),
        trust(FRAME_TRUST_NONE) {}
  virtual ~StackFrame() {}


  string trust_description() const {
    switch (trust) {
      case StackFrame::FRAME_TRUST_CONTEXT:
        return "given as instruction pointer in context";
      case StackFrame::FRAME_TRUST_PREWALKED:
        return "recovered by external stack walker";
      case StackFrame::FRAME_TRUST_CFI:
        return "call frame info";
      case StackFrame::FRAME_TRUST_CFI_SCAN:
        return "call frame info with scanning";
      case StackFrame::FRAME_TRUST_FP:
        return "previous frame's frame pointer";
      case StackFrame::FRAME_TRUST_SCAN:
        return "stack scanning";
      default:
        return "unknown";
    }
  };


  virtual uint64_t ReturnAddress() const { return instruction; }





















  uint64_t instruction;

  const CodeModule *module;

  string function_name;


  uint64_t function_base;

  string source_file_name;


  int source_line;


  uint64_t source_line_base;


  FrameTrust trust;
};

}  // namespace google_breakpad

#endif  // GOOGLE_BREAKPAD_PROCESSOR_STACK_FRAME_H__
