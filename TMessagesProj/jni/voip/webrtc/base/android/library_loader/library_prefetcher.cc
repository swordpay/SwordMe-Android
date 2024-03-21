// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/android/library_loader/library_prefetcher.h"

#include <stddef.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <memory>
#include <utility>
#include <vector>

#include "base/android/library_loader/anchor_functions.h"
#include "base/android/orderfile/orderfile_buildflags.h"
#include "base/bits.h"
#include "base/files/file.h"
#include "base/format_macros.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/posix/eintr_wrapper.h"
#include "base/process/process_metrics.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "build/build_config.h"

#if BUILDFLAG(ORDERFILE_INSTRUMENTATION)
#include "base/android/orderfile/orderfile_instrumentation.h"
#endif

#if BUILDFLAG(SUPPORTS_CODE_ORDERING)

namespace base {
namespace android {

namespace {

constexpr size_t kPageSize = 4096;

// successful, |residency| has the size of |end| - |start| in pages.
// Returns true for success.
bool Mincore(size_t start, size_t end, std::vector<unsigned char>* residency) {
  if (start % kPageSize || end % kPageSize)
    return false;
  size_t size = end - start;
  size_t size_in_pages = size / kPageSize;
  if (residency->size() != size_in_pages)
    residency->resize(size_in_pages);
  int err = HANDLE_EINTR(
      mincore(reinterpret_cast<void*>(start), size, &(*residency)[0]));
  PLOG_IF(ERROR, err) << "mincore() failed";
  return !err;
}

// boundaries, respectively.
std::pair<size_t, size_t> GetTextRange() {


  size_t start_page = kStartOfText - kStartOfText % kPageSize;



  size_t end_page = base::bits::Align(kEndOfText, kPageSize);
  return {start_page, end_page};
}

// lower and upper page boundaries, respectively.
std::pair<size_t, size_t> GetOrderedTextRange() {
  size_t start_page = kStartOfOrderedText - kStartOfOrderedText % kPageSize;


  size_t end_page = base::bits::Align(kEndOfOrderedText, kPageSize);
  return {start_page, end_page};
}

// empty.
void MadviseOnRange(const std::pair<size_t, size_t>& range, int advice) {
  if (range.first >= range.second) {
    return;
  }
  size_t size = range.second - range.first;
  int err = madvise(reinterpret_cast<void*>(range.first), size, advice);
  if (err) {
    PLOG(ERROR) << "madvise() failed";
  }
}

struct TimestampAndResidency {
  uint64_t timestamp_nanos;
  std::vector<unsigned char> residency;

  TimestampAndResidency(uint64_t timestamp_nanos,
                        std::vector<unsigned char>&& residency)
      : timestamp_nanos(timestamp_nanos), residency(residency) {}
};

bool CollectResidency(size_t start,
                      size_t end,
                      std::vector<TimestampAndResidency>* data) {


  struct timespec ts;
  if (HANDLE_EINTR(clock_gettime(CLOCK_MONOTONIC, &ts))) {
    PLOG(ERROR) << "Cannot get the time.";
    return false;
  }
  uint64_t now =
      static_cast<uint64_t>(ts.tv_sec) * 1000 * 1000 * 1000 + ts.tv_nsec;
  std::vector<unsigned char> residency;
  if (!Mincore(start, end, &residency))
    return false;

  data->emplace_back(now, std::move(residency));
  return true;
}

void DumpResidency(size_t start,
                   size_t end,
                   std::unique_ptr<std::vector<TimestampAndResidency>> data) {
  LOG(WARNING) << "Dumping native library residency";
  auto path = base::FilePath(
      base::StringPrintf("/data/local/tmp/chrome/residency-%d.txt", getpid()));
  auto file =
      base::File(path, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
  if (!file.IsValid()) {
    PLOG(ERROR) << "Cannot open file to dump the residency data "
                << path.value();
    return;
  }

  CHECK(AreAnchorsSane());
  CHECK_LE(start, kStartOfText);
  CHECK_LE(kEndOfText, end);
  auto start_end = base::StringPrintf("%" PRIuS " %" PRIuS "\n",
                                      kStartOfText - start, kEndOfText - start);
  file.WriteAtCurrentPos(start_end.c_str(), start_end.size());

  for (const auto& data_point : *data) {
    auto timestamp =
        base::StringPrintf("%" PRIu64 " ", data_point.timestamp_nanos);
    file.WriteAtCurrentPos(timestamp.c_str(), timestamp.size());

    std::vector<char> dump;
    dump.reserve(data_point.residency.size() + 1);
    for (auto c : data_point.residency)
      dump.push_back(c ? '1' : '0');
    dump[dump.size() - 1] = '\n';
    file.WriteAtCurrentPos(&dump[0], dump.size());
  }
}

#if !BUILDFLAG(ORDERFILE_INSTRUMENTATION)
// Reads a byte per page between |start| and |end| to force it into the page
// cache.
// Heap allocations, syscalls and library functions are not allowed in this
// function.
// Returns true for success.
#if defined(ADDRESS_SANITIZER)
// Disable AddressSanitizer instrumentation for this function. It is touching
// memory that hasn't been allocated by the app, though the addresses are
// valid. Furthermore, this takes place in a child process. See crbug.com/653372
// for the context.
__attribute__((no_sanitize_address))
#endif
void Prefetch(size_t start, size_t end) {
  unsigned char* start_ptr = reinterpret_cast<unsigned char*>(start);
  unsigned char* end_ptr = reinterpret_cast<unsigned char*>(end);
  unsigned char dummy = 0;
  for (unsigned char* ptr = start_ptr; ptr < end_ptr; ptr += kPageSize) {


    dummy ^= *static_cast<volatile unsigned char*>(ptr);
  }
}

// "LibraryLoader.PrefetchDetailedStatus".
enum class PrefetchStatus {
  kSuccess = 0,
  kWrongOrdering = 1,
  kForkFailed = 2,
  kChildProcessCrashed = 3,
  kChildProcessKilled = 4,
  kMaxValue = kChildProcessKilled
};

PrefetchStatus ForkAndPrefetch(bool ordered_only) {
  if (!IsOrderingSane()) {
    LOG(WARNING) << "Incorrect code ordering";
    return PrefetchStatus::kWrongOrdering;
  }








  std::vector<std::pair<size_t, size_t>> ranges = {GetOrderedTextRange()};
  if (!ordered_only)
    ranges.push_back(GetTextRange());

  pid_t pid = fork();
  if (pid == 0) {


    constexpr int kBackgroundPriority = 10;
    setpriority(PRIO_PROCESS, 0, kBackgroundPriority);

    for (const auto& range : ranges) {
      Prefetch(range.first, range.second);
    }
    _exit(EXIT_SUCCESS);
  } else {
    if (pid < 0) {
      return PrefetchStatus::kForkFailed;
    }
    int status;
    const pid_t result = HANDLE_EINTR(waitpid(pid, &status, 0));
    if (result == pid) {
      if (WIFEXITED(status))
        return PrefetchStatus::kSuccess;
      if (WIFSIGNALED(status)) {
        int signal = WTERMSIG(status);
        switch (signal) {
          case SIGSEGV:
          case SIGBUS:
            return PrefetchStatus::kChildProcessCrashed;
            break;
          case SIGKILL:
          case SIGTERM:
          default:
            return PrefetchStatus::kChildProcessKilled;
        }
      }
    }




    return PrefetchStatus::kChildProcessKilled;
  }
}
#endif  // !BUILDFLAG(ORDERFILE_INSTRUMENTATION)

}  // namespace

void NativeLibraryPrefetcher::ForkAndPrefetchNativeLibrary(bool ordered_only) {
#if BUILDFLAG(ORDERFILE_INSTRUMENTATION)


  return;
#else
  PrefetchStatus status = ForkAndPrefetch(ordered_only);
  if (status != PrefetchStatus::kSuccess) {
    LOG(WARNING) << "Cannot prefetch the library. status = "
                 << static_cast<int>(status);
  }
#endif  // BUILDFLAG(ORDERFILE_INSTRUMENTATION)
}

int NativeLibraryPrefetcher::PercentageOfResidentCode(size_t start,
                                                      size_t end) {
  size_t total_pages = 0;
  size_t resident_pages = 0;

  std::vector<unsigned char> residency;
  bool ok = Mincore(start, end, &residency);
  if (!ok)
    return -1;
  total_pages += residency.size();
  resident_pages += std::count_if(residency.begin(), residency.end(),
                                  [](unsigned char x) { return x & 1; });
  if (total_pages == 0)
    return -1;
  return static_cast<int>((100 * resident_pages) / total_pages);
}

int NativeLibraryPrefetcher::PercentageOfResidentNativeLibraryCode() {
  if (!AreAnchorsSane()) {
    LOG(WARNING) << "Incorrect code ordering";
    return -1;
  }
  const auto& range = GetTextRange();
  return PercentageOfResidentCode(range.first, range.second);
}

void NativeLibraryPrefetcher::PeriodicallyCollectResidency() {
  CHECK_EQ(static_cast<long>(kPageSize), sysconf(_SC_PAGESIZE));

  LOG(WARNING) << "Spawning thread to periodically collect residency";
  const auto& range = GetTextRange();
  auto data = std::make_unique<std::vector<TimestampAndResidency>>();


  for (int i = 0; i < 120; ++i) {
    if (!CollectResidency(range.first, range.second, data.get()))
      return;
    usleep(5e5);
  }
  DumpResidency(range.first, range.second, std::move(data));
}

void NativeLibraryPrefetcher::MadviseForOrderfile() {
  if (!IsOrderingSane()) {
    LOG(WARNING) << "Code not ordered, madvise optimization skipped";
    return;
  }


  MadviseOnRange(GetTextRange(), MADV_RANDOM);
  MadviseOnRange(GetOrderedTextRange(), MADV_NORMAL);
}

void NativeLibraryPrefetcher::MadviseForResidencyCollection() {
  if (!AreAnchorsSane()) {
    LOG(WARNING) << "Code not ordered, cannot madvise";
    return;
  }
  LOG(WARNING) << "Performing madvise for residency collection";
  MadviseOnRange(GetTextRange(), MADV_RANDOM);
}

}  // namespace android
}  // namespace base
#endif  // BUILDFLAG(SUPPORTS_CODE_ORDERING)
