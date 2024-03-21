// Copyright 2018 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// -----------------------------------------------------------------------------
// File: failure_signal_handler.h
// -----------------------------------------------------------------------------
//
// This file configures the Abseil *failure signal handler* to capture and dump
// useful debugging information (such as a stacktrace) upon program failure.
//
// To use the failure signal handler, call `absl::InstallFailureSignalHandler()`
// very early in your program, usually in the first few lines of main():
//
// int main(int argc, char** argv) {
//   // Initialize the symbolizer to get a human-readable stack trace
//   absl::InitializeSymbolizer(argv[0]);
//
//   absl::FailureSignalHandlerOptions options;
//   absl::InstallFailureSignalHandler(options);
//   DoSomethingInteresting();
//   return 0;
// }
//
// Any program that raises a fatal signal (such as `SIGSEGV`, `SIGILL`,
// `SIGFPE`, `SIGABRT`, `SIGTERM`, `SIGBUG`, and `SIGTRAP`) will call the
// installed failure signal handler and provide debugging information to stderr.
//
// Note that you should *not* install the Abseil failure signal handler more
// than once. You may, of course, have another (non-Abseil) failure signal
// handler installed (which would be triggered if Abseil's failure signal
// handler sets `call_previous_handler` to `true`).

#ifndef ABSL_DEBUGGING_FAILURE_SIGNAL_HANDLER_H_
#define ABSL_DEBUGGING_FAILURE_SIGNAL_HANDLER_H_

#include "absl/base/config.h"

namespace absl {
ABSL_NAMESPACE_BEGIN

//
// Struct for holding `absl::InstallFailureSignalHandler()` configuration
// options.
struct FailureSignalHandlerOptions {



  bool symbolize_stacktrace = true;






  bool use_alternate_stack = true;




  int alarm_on_failure_secs = 3;












  bool call_previous_handler = false;










  void (*writerfn)(const char*) = nullptr;
};

//
// Installs a signal handler for the common failure signals `SIGSEGV`, `SIGILL`,
// `SIGFPE`, `SIGABRT`, `SIGTERM`, `SIGBUG`, and `SIGTRAP` (provided they exist
// on the given platform). The failure signal handler dumps program failure data
// useful for debugging in an unspecified format to stderr. This data may
// include the program counter, a stacktrace, and register information on some
// systems; do not rely on an exact format for the output, as it is subject to
// change.
void InstallFailureSignalHandler(const FailureSignalHandlerOptions& options);

namespace debugging_internal {
const char* FailureSignalToString(int signo);
}  // namespace debugging_internal

ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_DEBUGGING_FAILURE_SIGNAL_HANDLER_H_
