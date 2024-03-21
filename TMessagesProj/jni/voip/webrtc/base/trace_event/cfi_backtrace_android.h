// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_CFI_BACKTRACE_ANDROID_H_
#define BASE_TRACE_EVENT_CFI_BACKTRACE_ANDROID_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>

#include "base/base_export.h"
#include "base/debug/debugging_buildflags.h"
#include "base/files/memory_mapped_file.h"
#include "base/gtest_prod_util.h"
#include "base/threading/thread_local_storage.h"

namespace base {
namespace trace_event {

// information (dwarf debug info) is stripped from the chrome binary and we do
// not build with exception tables (ARM EHABI) in release builds. So, we use a
// custom unwind table which is generated and added to specific android builds,
// when add_unwind_tables_in_apk build option is specified. This unwind table
// contains information for unwinding stack frames when the functions calls are
// from lib[mono]chrome.so. The file is added as an asset to the apk and the
// table is used to unwind stack frames for profiling. This class implements
// methods to read and parse the unwind table and unwind stack frames using this
// data.
class BASE_EXPORT CFIBacktraceAndroid {
 public:


  static CFIBacktraceAndroid* GetInitializedInstance();

  static bool is_chrome_address(uintptr_t pc);

  static uintptr_t executable_start_addr();
  static uintptr_t executable_end_addr();



  bool can_unwind_stack_frames() const { return can_unwind_stack_frames_; }











  size_t Unwind(const void** out_trace, size_t max_depth);




  size_t Unwind(uintptr_t pc,
                uintptr_t sp,
                uintptr_t lr,
                const void** out_trace,
                size_t max_depth);


  void AllocateCacheForCurrentThread();

  struct CFIRow {
    bool operator==(const CFIBacktraceAndroid::CFIRow& o) const {
      return cfa_offset == o.cfa_offset && ra_offset == o.ra_offset;
    }



    uint16_t cfa_offset = 0;


    uint16_t ra_offset = 0;
  };


  bool FindCFIRowForPC(uintptr_t func_addr, CFIRow* out);

 private:
  FRIEND_TEST_ALL_PREFIXES(CFIBacktraceAndroidTest, TestCFICache);
  FRIEND_TEST_ALL_PREFIXES(CFIBacktraceAndroidTest, TestFindCFIRow);
  FRIEND_TEST_ALL_PREFIXES(CFIBacktraceAndroidTest, TestUnwinding);




  class CFICache {
   public:


    void Add(uintptr_t address, CFIRow cfi);


    bool Find(uintptr_t address, CFIRow* cfi);

   private:
    FRIEND_TEST_ALL_PREFIXES(CFIBacktraceAndroidTest, TestCFICache);


    static const int kLimit = 509;

    struct AddrAndCFI {
      uintptr_t address;
      CFIRow cfi;
    };
    AddrAndCFI cache_[kLimit] = {};
  };

  static_assert(sizeof(CFIBacktraceAndroid::CFICache) < 4096,
                "The cache does not fit in a single page.");

  CFIBacktraceAndroid();
  ~CFIBacktraceAndroid();








  void Initialize();

  void ParseCFITables();

  CFICache* GetThreadLocalCFICache();


  std::unique_ptr<MemoryMappedFile> cfi_mmap_;



  const uintptr_t* unw_index_function_col_ = nullptr;


  const uint16_t* unw_index_indices_col_ = nullptr;

  size_t unw_index_row_count_ = 0;

  const uint16_t* unw_data_start_addr_ = nullptr;

  bool can_unwind_stack_frames_ = false;

  ThreadLocalStorage::Slot thread_local_cfi_cache_;
};

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_CFI_BACKTRACE_ANDROID_H_
