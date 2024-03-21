// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// running on the system.

#ifndef BASE_PROCESS_PROCESS_METRICS_H_
#define BASE_PROCESS_PROCESS_METRICS_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <string>

#include "base/base_export.h"
#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/process/process_handle.h"
#include "base/time/time.h"
#include "base/values.h"
#include "build/build_config.h"

#if defined(OS_MACOSX)
#include <mach/mach.h>
#include "base/process/port_provider_mac.h"

#if !defined(OS_IOS)
#include <mach/mach_vm.h>
#endif
#endif

#if defined(OS_WIN)
#include "base/win/scoped_handle.h"
#include "base/win/windows_types.h"
#endif

namespace base {

struct IoCounters;

#if defined(OS_LINUX) || defined(OS_ANDROID)
// Minor and major page fault counts since the process creation.
// Both counts are process-wide, and exclude child processes.
//
// minor: Number of page faults that didn't require disk IO.
// major: Number of page faults that required disk IO.
struct PageFaultCounts {
  int64_t minor;
  int64_t major;
};
#endif  // defined(OS_LINUX) || defined(OS_ANDROID)

BASE_EXPORT int64_t TimeValToMicroseconds(const struct timeval& tv);

// counters). Use CreateCurrentProcessMetrics() to get an instance for the
// current process, or CreateProcessMetrics() to get an instance for an
// arbitrary process. Then, access the information with the different get
// methods.
//
// This class exposes a few platform-specific APIs for parsing memory usage, but
// these are not intended to generalize to other platforms, since the memory
// models differ substantially.
//
// To obtain consistent memory metrics, use the memory_instrumentation service.
//
// For further documentation on memory, see
// https://chromium.googlesource.com/chromium/src/+/HEAD/docs/README.md
class BASE_EXPORT ProcessMetrics {
 public:
  ~ProcessMetrics();

#if !defined(OS_MACOSX) || defined(OS_IOS)
  static std::unique_ptr<ProcessMetrics> CreateProcessMetrics(
      ProcessHandle process);
#else



  static std::unique_ptr<ProcessMetrics> CreateProcessMetrics(
      ProcessHandle process,
      PortProvider* port_provider);
#endif  // !defined(OS_MACOSX) || defined(OS_IOS)


  static std::unique_ptr<ProcessMetrics> CreateCurrentProcessMetrics();

#if defined(OS_LINUX) || defined(OS_ANDROID)


  BASE_EXPORT size_t GetResidentSetSize() const;
#endif












  double GetPlatformIndependentCPUUsage();




  TimeDelta GetCumulativeCPUUsage();


  int GetIdleWakeupsPerSecond();

#if defined(OS_MACOSX)













  int GetPackageIdleWakeupsPerSecond();


  int GetEnergyImpact();
#endif





  bool GetIOCounters(IoCounters* io_counters) const;






  uint64_t GetDiskUsageBytesPerSecond();


  uint64_t GetCumulativeDiskUsageInBytes();

#if defined(OS_POSIX)


  int GetOpenFdCount() const;


  int GetOpenFdSoftLimit() const;
#endif  // defined(OS_POSIX)

#if defined(OS_LINUX) || defined(OS_ANDROID)

  uint64_t GetVmSwapBytes() const;


  bool GetPageFaultCounts(PageFaultCounts* counts) const;
#endif  // defined(OS_LINUX) || defined(OS_ANDROID)

  size_t GetMallocUsage();

 private:
#if !defined(OS_MACOSX) || defined(OS_IOS)
  explicit ProcessMetrics(ProcessHandle process);
#else
  ProcessMetrics(ProcessHandle process, PortProvider* port_provider);
#endif  // !defined(OS_MACOSX) || defined(OS_IOS)

#if defined(OS_MACOSX) || defined(OS_LINUX) || defined(OS_AIX)
  int CalculateIdleWakeupsPerSecond(uint64_t absolute_idle_wakeups);
#endif
#if defined(OS_MACOSX)


  int CalculatePackageIdleWakeupsPerSecond(
      uint64_t absolute_package_idle_wakeups);
#endif

#if defined(OS_WIN)
  win::ScopedHandle process_;
#else
  ProcessHandle process_;
#endif


  TimeTicks last_cpu_time_;
#if !defined(OS_FREEBSD) || !defined(OS_POSIX)
  TimeDelta last_cumulative_cpu_;
#endif


  TimeTicks last_disk_usage_time_;

  uint64_t last_cumulative_disk_usage_ = 0;

#if defined(OS_MACOSX) || defined(OS_LINUX) || defined(OS_AIX)

  TimeTicks last_idle_wakeups_time_;
  uint64_t last_absolute_idle_wakeups_;
#endif

#if defined(OS_MACOSX)

  TimeTicks last_package_idle_wakeups_time_;
  uint64_t last_absolute_package_idle_wakeups_;
  double last_energy_impact_;

  uint64_t last_energy_impact_time_;
#endif

#if !defined(OS_IOS)
#if defined(OS_MACOSX)

  mach_port_t TaskForPid(ProcessHandle process) const;

  PortProvider* port_provider_;
#endif  // defined(OS_MACOSX)
#endif  // !defined(OS_IOS)

  DISALLOW_COPY_AND_ASSIGN(ProcessMetrics);
};

// Returns 0 if it can't compute the commit charge.
BASE_EXPORT size_t GetSystemCommitCharge();

// the number of pages in a block of memory for calling mincore(). On some
// platforms, e.g. iOS, mincore() uses a different page size from what is
// returned by GetPageSize().
BASE_EXPORT size_t GetPageSize();

// at once. If the number is unavailable, a conservative best guess is returned.
BASE_EXPORT size_t GetMaxFds();

BASE_EXPORT size_t GetHandleLimit();

#if defined(OS_POSIX)
// Increases the file descriptor soft limit to |max_descriptors| or the OS hard
// limit, whichever is lower. If the limit is already higher than
// |max_descriptors|, then nothing happens.
BASE_EXPORT void IncreaseFdLimitTo(unsigned int max_descriptors);
#endif  // defined(OS_POSIX)

#if defined(OS_WIN) || defined(OS_MACOSX) || defined(OS_LINUX) || \
    defined(OS_ANDROID) || defined(OS_AIX) || defined(OS_FUCHSIA)
// Data about system-wide memory consumption. Values are in KB. Available on
// Windows, Mac, Linux, Android and Chrome OS.
//
// Total memory are available on all platforms that implement
// GetSystemMemoryInfo(). Total/free swap memory are available on all platforms
// except on Mac. Buffers/cached/active_anon/inactive_anon/active_file/
// inactive_file/dirty/reclaimable/pswpin/pswpout/pgmajfault are available on
// Linux/Android/Chrome OS. Shmem/slab/gem_objects/gem_size are Chrome OS only.
// Speculative/file_backed/purgeable are Mac and iOS only.
// Free is absent on Windows (see "avail_phys" below).
struct BASE_EXPORT SystemMemoryInfoKB {
  SystemMemoryInfoKB();
  SystemMemoryInfoKB(const SystemMemoryInfoKB& other);

  std::unique_ptr<DictionaryValue> ToValue() const;

  int total = 0;

#if !defined(OS_WIN)
  int free = 0;
#endif

#if defined(OS_WIN)





  int avail_phys = 0;
#endif

#if defined(OS_LINUX) || defined(OS_ANDROID) || defined(OS_AIX)





  int available = 0;
#endif

#if !defined(OS_MACOSX)
  int swap_total = 0;
  int swap_free = 0;
#endif

#if defined(OS_ANDROID) || defined(OS_LINUX) || defined(OS_AIX) || \
    defined(OS_FUCHSIA)
  int buffers = 0;
  int cached = 0;
  int active_anon = 0;
  int inactive_anon = 0;
  int active_file = 0;
  int inactive_file = 0;
  int dirty = 0;
  int reclaimable = 0;
#endif  // defined(OS_ANDROID) || defined(OS_LINUX) || defined(OS_AIX) ||


#if defined(OS_CHROMEOS)
  int shmem = 0;
  int slab = 0;

  int gem_objects = -1;
  long long gem_size = -1;
#endif  // defined(OS_CHROMEOS)

#if defined(OS_MACOSX)
  int speculative = 0;
  int file_backed = 0;
  int purgeable = 0;
#endif  // defined(OS_MACOSX)
};

// from /proc/meminfo and /proc/vmstat. On Windows/Mac, it is obtained using
// system API calls.
//
// Fills in the provided |meminfo| structure. Returns true on success.
// Exposed for memory debugging widget.
BASE_EXPORT bool GetSystemMemoryInfo(SystemMemoryInfoKB* meminfo);

#endif  // defined(OS_WIN) || defined(OS_MACOSX) || defined(OS_LINUX) ||


#if defined(OS_LINUX) || defined(OS_ANDROID) || defined(OS_AIX)
// Parse the data found in /proc/<pid>/stat and return the sum of the
// CPU-related ticks.  Returns -1 on parse error.
// Exposed for testing.
BASE_EXPORT int ParseProcStatCPU(StringPiece input);

// This should be used with care as no synchronization with running threads is
// done. This is mostly useful to guarantee being single-threaded.
// Returns 0 on failure.
BASE_EXPORT int GetNumberOfThreads(ProcessHandle process);

BASE_EXPORT extern const char kProcSelfExe[];

// returns true on success or false for a parsing error
// Exposed for testing.
BASE_EXPORT bool ParseProcMeminfo(StringPiece input,
                                  SystemMemoryInfoKB* meminfo);

struct BASE_EXPORT VmStatInfo {

  std::unique_ptr<DictionaryValue> ToValue() const;

  unsigned long pswpin = 0;
  unsigned long pswpout = 0;
  unsigned long pgmajfault = 0;
};

// Fills in the provided |vmstat| structure. Returns true on success.
BASE_EXPORT bool GetVmStatInfo(VmStatInfo* vmstat);

// returns true on success or false for a parsing error
// Exposed for testing.
BASE_EXPORT bool ParseProcVmstat(StringPiece input, VmStatInfo* vmstat);

struct BASE_EXPORT SystemDiskInfo {
  SystemDiskInfo();
  SystemDiskInfo(const SystemDiskInfo& other);

  std::unique_ptr<Value> ToValue() const;

  uint64_t reads = 0;
  uint64_t reads_merged = 0;
  uint64_t sectors_read = 0;
  uint64_t read_time = 0;
  uint64_t writes = 0;
  uint64_t writes_merged = 0;
  uint64_t sectors_written = 0;
  uint64_t write_time = 0;
  uint64_t io = 0;
  uint64_t io_time = 0;
  uint64_t weighted_io_time = 0;
};

// for a generic disk or mmcblk[0-9]+ for the MMC case.
// Names of disk partitions (e.g. sda1) are not valid.
BASE_EXPORT bool IsValidDiskName(StringPiece candidate);

// Fills in the provided |diskinfo| structure. Returns true on success.
BASE_EXPORT bool GetSystemDiskInfo(SystemDiskInfo* diskinfo);

BASE_EXPORT TimeDelta GetUserCpuTimeSinceBoot();

#endif  // defined(OS_LINUX) || defined(OS_ANDROID)

#if defined(OS_CHROMEOS)
// Data from files in directory /sys/block/zram0 about ZRAM usage.
struct BASE_EXPORT SwapInfo {
  SwapInfo()
      : num_reads(0),
        num_writes(0),
        compr_data_size(0),
        orig_data_size(0),
        mem_used_total(0) {
  }

  std::unique_ptr<Value> ToValue() const;

  uint64_t num_reads = 0;
  uint64_t num_writes = 0;
  uint64_t compr_data_size = 0;
  uint64_t orig_data_size = 0;
  uint64_t mem_used_total = 0;
};

// This should be used for the new ZRAM sysfs interfaces.
// Returns true on success or false for a parsing error.
// Exposed for testing.
BASE_EXPORT bool ParseZramMmStat(StringPiece mm_stat_data, SwapInfo* swap_info);

// This should be used for the new ZRAM sysfs interfaces.
// Returns true on success or false for a parsing error.
// Exposed for testing.
BASE_EXPORT bool ParseZramStat(StringPiece stat_data, SwapInfo* swap_info);

// Fills in the provided |swap_data| structure.
// Returns true on success or false for a parsing error.
BASE_EXPORT bool GetSwapInfo(SwapInfo* swap_info);
#endif  // defined(OS_CHROMEOS)

struct BASE_EXPORT SystemPerformanceInfo {
  SystemPerformanceInfo();
  SystemPerformanceInfo(const SystemPerformanceInfo& other);

  std::unique_ptr<Value> ToValue() const;

  uint64_t idle_time = 0;

  uint64_t read_transfer_count = 0;

  uint64_t write_transfer_count = 0;

  uint64_t other_transfer_count = 0;

  uint64_t read_operation_count = 0;

  uint64_t write_operation_count = 0;

  uint64_t other_operation_count = 0;

  uint64_t pagefile_pages_written = 0;

  uint64_t pagefile_pages_write_ios = 0;


  uint64_t available_pages = 0;

  uint64_t pages_read = 0;

  uint64_t page_read_ios = 0;
};

// Fills in the provided |info| structure. Returns true on success.
BASE_EXPORT bool GetSystemPerformanceInfo(SystemPerformanceInfo* info);

// Provides functionality to retrieve the data on various platforms and
// to serialize the stored data.
class BASE_EXPORT SystemMetrics {
 public:
  SystemMetrics();

  static SystemMetrics Sample();

  std::unique_ptr<Value> ToValue() const;

 private:
  FRIEND_TEST_ALL_PREFIXES(SystemMetricsTest, SystemMetrics);

  size_t committed_memory_;
#if defined(OS_LINUX) || defined(OS_ANDROID)
  SystemMemoryInfoKB memory_info_;
  VmStatInfo vmstat_info_;
  SystemDiskInfo disk_info_;
#endif
#if defined(OS_CHROMEOS)
  SwapInfo swap_info_;
#endif
#if defined(OS_WIN)
  SystemPerformanceInfo performance_;
#endif
};

#if defined(OS_MACOSX) && !defined(OS_IOS)
enum class MachVMRegionResult {


  Finished,

  Error,

  Success
};

// resident memory and share mode. On Success, |size| reflects the size of the
// memory region.
// |size| and |info| are output parameters, only valid on Success.
// |address| is an in-out parameter, than represents both the address to start
// looking, and the start address of the memory region.
BASE_EXPORT MachVMRegionResult GetTopInfo(mach_port_t task,
                                          mach_vm_size_t* size,
                                          mach_vm_address_t* address,
                                          vm_region_top_info_data_t* info);

// protection values. On Success, |size| reflects the size of the
// memory region.
// Returns info on the first memory region at or after |address|, including
// resident memory and share mode.
// |size| and |info| are output parameters, only valid on Success.
BASE_EXPORT MachVMRegionResult GetBasicInfo(mach_port_t task,
                                            mach_vm_size_t* size,
                                            mach_vm_address_t* address,
                                            vm_region_basic_info_64* info);
#endif  // defined(OS_MACOSX) && !defined(OS_IOS)

}  // namespace base

#endif  // BASE_PROCESS_PROCESS_METRICS_H_
