// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/process/memory.h"

#include <stddef.h>

#include <new>

#include "base/allocator/allocator_shim.h"
#include "base/allocator/buildflags.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/process/internal_linux.h"
#include "base/strings/string_number_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "build/build_config.h"

#if BUILDFLAG(USE_TCMALLOC)
#include "third_party/tcmalloc/chromium/src/config.h"
#include "third_party/tcmalloc/chromium/src/gperftools/tcmalloc.h"
#endif

namespace base {

size_t g_oom_size = 0U;

namespace {

void OnNoMemorySize(size_t size) {
  g_oom_size = size;

  if (size != 0)
    LOG(FATAL) << "Out of memory, size = " << size;
  LOG(FATAL) << "Out of memory.";
}

// crash server.
NOINLINE void OnNoMemory() {
  OnNoMemorySize(0);
}

void ReleaseReservationOrTerminate() {
  if (internal::ReleaseAddressSpaceReservation())
    return;
  OnNoMemory();
}

}  // namespace

void EnableTerminationOnHeapCorruption() {

}

void EnableTerminationOnOutOfMemory() {

  std::set_new_handler(&ReleaseReservationOrTerminate);



#if BUILDFLAG(USE_ALLOCATOR_SHIM)
  allocator::SetCallNewHandlerOnMallocFailure(true);
#elif defined(USE_TCMALLOC)

  tc_set_new_mode(1);
#endif
}

// friend classes/functions. Declaring a class is easier in this situation to
// avoid adding more dependency to thread_restrictions.h because of the
// parameter used in AdjustOOMScore(). Specifically, ProcessId is a typedef
// and we'll need to include another header file in thread_restrictions.h
// without the class.
class AdjustOOMScoreHelper {
 public:
  static bool AdjustOOMScore(ProcessId process, int score);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(AdjustOOMScoreHelper);
};

bool AdjustOOMScoreHelper::AdjustOOMScore(ProcessId process, int score) {
  if (score < 0 || score > kMaxOomScore)
    return false;

  FilePath oom_path(internal::GetProcPidDir(process));

  base::ScopedAllowBlocking allow_blocking;

  FilePath oom_file = oom_path.AppendASCII("oom_score_adj");
  if (PathExists(oom_file)) {
    std::string score_str = NumberToString(score);
    DVLOG(1) << "Adjusting oom_score_adj of " << process << " to "
             << score_str;
    int score_len = static_cast<int>(score_str.length());
    return (score_len == WriteFile(oom_file, score_str.c_str(), score_len));
  }


  oom_file = oom_path.AppendASCII("oom_adj");
  if (PathExists(oom_file)) {


    const int kMaxOldOomScore = 15;

    int converted_score = score * kMaxOldOomScore / kMaxOomScore;
    std::string score_str = NumberToString(converted_score);
    DVLOG(1) << "Adjusting oom_adj of " << process << " to " << score_str;
    int score_len = static_cast<int>(score_str.length());
    return (score_len == WriteFile(oom_file, score_str.c_str(), score_len));
  }

  return false;
}

// the setuid sandbox (in process_util_linux.c, in the sandbox source)
// also has its own C version.
bool AdjustOOMScore(ProcessId process, int score) {
  return AdjustOOMScoreHelper::AdjustOOMScore(process, score);
}

bool UncheckedMalloc(size_t size, void** result) {
#if BUILDFLAG(USE_ALLOCATOR_SHIM)
  *result = allocator::UncheckedAlloc(size);
#elif defined(MEMORY_TOOL_REPLACES_ALLOCATOR) || \
    (!defined(LIBC_GLIBC) && !defined(USE_TCMALLOC))
  *result = malloc(size);
#elif defined(LIBC_GLIBC) && !defined(USE_TCMALLOC)
  *result = __libc_malloc(size);
#elif defined(USE_TCMALLOC)
  *result = tc_malloc_skip_new_handler(size);
#endif
  return *result != nullptr;
}

}  // namespace base
