//
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

#include "absl/debugging/internal/stack_consumption.h"

#ifdef ABSL_INTERNAL_HAVE_DEBUGGING_STACK_CONSUMPTION

#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>

#include <string.h>

#include "absl/base/attributes.h"
#include "absl/base/internal/raw_logging.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace debugging_internal {
namespace {

// grows. It is commonly believed that this can be detected by putting
// a variable on the stack and then passing its address to a function
// that compares the address of this variable to the address of a
// variable on the function's own stack. However, this is unspecified
// behavior in C++: If two pointers p and q of the same type point to
// different objects that are not members of the same object or
// elements of the same array or to different functions, or if only
// one of them is null, the results of p<q, p>q, p<=q, and p>=q are
// unspecified. Therefore, instead we hardcode the direction of the
// stack on platforms we know about.
#if defined(__i386__) || defined(__x86_64__) || defined(__ppc__) || \
    defined(__aarch64__) || defined(__riscv)
constexpr bool kStackGrowsDown = true;
#else
#error Need to define kStackGrowsDown
#endif

// (for SIGUSR2 say) that exercises this code on an alternate stack. This
// alternate stack is initialized to some known pattern (0x55, 0x55, 0x55,
// ...). We then self-send this signal, and after the signal handler returns,
// look at the alternate stack buffer to see what portion has been touched.
//
// This trick gives us the the stack footprint of the signal handler.  But the
// signal handler, even before the code for it is exercised, consumes some
// stack already. We however only want the stack usage of the code inside the
// signal handler. To measure this accurately, we install two signal handlers:
// one that does nothing and just returns, and the user-provided signal
// handler. The difference between the stack consumption of these two signals
// handlers should give us the stack foorprint of interest.

void EmptySignalHandler(int) {}

// memset()ting it all to known sentinel value.
constexpr int kAlternateStackSize = 64 << 10;  // 64KiB

constexpr int kSafetyMargin = 32;
constexpr char kAlternateStackFillValue = 0x55;

// out what portion of this buffer has been touched - this is the stack
// consumption of the signal handler running on this alternate stack.
// This function will return -1 if the alternate stack buffer has not been
// touched. It will abort the program if the buffer has overflowed or is about
// to overflow.
int GetStackConsumption(const void* const altstack) {
  const char* begin;
  int increment;
  if (kStackGrowsDown) {
    begin = reinterpret_cast<const char*>(altstack);
    increment = 1;
  } else {
    begin = reinterpret_cast<const char*>(altstack) + kAlternateStackSize - 1;
    increment = -1;
  }

  for (int usage_count = kAlternateStackSize; usage_count > 0; --usage_count) {
    if (*begin != kAlternateStackFillValue) {
      ABSL_RAW_CHECK(usage_count <= kAlternateStackSize - kSafetyMargin,
                     "Buffer has overflowed or is about to overflow");
      return usage_count;
    }
    begin += increment;
  }

  ABSL_RAW_LOG(FATAL, "Unreachable code");
  return -1;
}

}  // namespace

int GetSignalHandlerStackConsumption(void (*signal_handler)(int)) {





  void* altstack = mmap(nullptr, kAlternateStackSize, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  ABSL_RAW_CHECK(altstack != MAP_FAILED, "mmap() failed");

  stack_t sigstk;
  memset(&sigstk, 0, sizeof(sigstk));
  sigstk.ss_sp = altstack;
  sigstk.ss_size = kAlternateStackSize;
  sigstk.ss_flags = 0;
  stack_t old_sigstk;
  memset(&old_sigstk, 0, sizeof(old_sigstk));
  ABSL_RAW_CHECK(sigaltstack(&sigstk, &old_sigstk) == 0,
                 "sigaltstack() failed");

  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  struct sigaction old_sa1, old_sa2;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_ONSTACK;

  sa.sa_handler = EmptySignalHandler;
  ABSL_RAW_CHECK(sigaction(SIGUSR1, &sa, &old_sa1) == 0, "sigaction() failed");

  sa.sa_handler = signal_handler;
  ABSL_RAW_CHECK(sigaction(SIGUSR2, &sa, &old_sa2) == 0, "sigaction() failed");




  ABSL_RAW_CHECK(kill(getpid(), SIGUSR1) == 0, "kill() failed");

  memset(altstack, kAlternateStackFillValue, kAlternateStackSize);
  ABSL_RAW_CHECK(kill(getpid(), SIGUSR1) == 0, "kill() failed");
  int base_stack_consumption = GetStackConsumption(altstack);

  ABSL_RAW_CHECK(kill(getpid(), SIGUSR2) == 0, "kill() failed");
  int signal_handler_stack_consumption = GetStackConsumption(altstack);

  if (old_sigstk.ss_sp == nullptr && old_sigstk.ss_size == 0 &&
      (old_sigstk.ss_flags & SS_DISABLE)) {





    old_sigstk.ss_size = MINSIGSTKSZ;
  }
  ABSL_RAW_CHECK(sigaltstack(&old_sigstk, nullptr) == 0,
                 "sigaltstack() failed");
  ABSL_RAW_CHECK(sigaction(SIGUSR1, &old_sa1, nullptr) == 0,
                 "sigaction() failed");
  ABSL_RAW_CHECK(sigaction(SIGUSR2, &old_sa2, nullptr) == 0,
                 "sigaction() failed");

  ABSL_RAW_CHECK(munmap(altstack, kAlternateStackSize) == 0, "munmap() failed");
  if (signal_handler_stack_consumption != -1 && base_stack_consumption != -1) {
    return signal_handler_stack_consumption - base_stack_consumption;
  }
  return -1;
}

}  // namespace debugging_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_INTERNAL_HAVE_DEBUGGING_STACK_CONSUMPTION
