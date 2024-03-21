// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_DEBUG_ACTIVITY_ANALYZER_H_
#define BASE_DEBUG_ACTIVITY_ANALYZER_H_

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "base/base_export.h"
#include "base/debug/activity_tracker.h"
#include "base/memory/shared_memory_mapping.h"

namespace base {
namespace debug {

class GlobalActivityAnalyzer;

// When created, it takes a snapshot of the data held by the tracker and
// makes that information available to other code.
class BASE_EXPORT ThreadActivityAnalyzer {
 public:
  struct BASE_EXPORT Snapshot : ThreadActivityTracker::Snapshot {
    Snapshot();
    ~Snapshot();


    std::vector<ActivityUserData::Snapshot> user_data_stack;
  };


  class ThreadKey {
   public:
    ThreadKey(int64_t pid, int64_t tid) : pid_(pid), tid_(tid) {}

    bool operator<(const ThreadKey& rhs) const {
      if (pid_ != rhs.pid_)
        return pid_ < rhs.pid_;
      return tid_ < rhs.tid_;
    }

    bool operator==(const ThreadKey& rhs) const {
      return (pid_ == rhs.pid_ && tid_ == rhs.tid_);
    }

   private:
    int64_t pid_;
    int64_t tid_;
  };


  explicit ThreadActivityAnalyzer(const ThreadActivityTracker& tracker);



  ThreadActivityAnalyzer(void* base, size_t size);



  ThreadActivityAnalyzer(PersistentMemoryAllocator* allocator,
                         PersistentMemoryAllocator::Reference reference);

  ~ThreadActivityAnalyzer();

  void AddGlobalInformation(GlobalActivityAnalyzer* global);


  bool IsValid() { return activity_snapshot_valid_; }

  int64_t GetProcessId(int64_t* out_stamp = nullptr) {
    if (out_stamp)
      *out_stamp = activity_snapshot_.create_stamp;
    return activity_snapshot_.process_id;
  }

  const std::string& GetThreadName() {
    return activity_snapshot_.thread_name;
  }

  ThreadKey GetThreadKey() {
    return ThreadKey(activity_snapshot_.process_id,
                     activity_snapshot_.thread_id);
  }

  const Snapshot& activity_snapshot() { return activity_snapshot_; }

 private:
  friend class GlobalActivityAnalyzer;

  Snapshot activity_snapshot_;

  bool activity_snapshot_valid_;


  PersistentMemoryAllocator::Reference allocator_reference_ = 0;

  DISALLOW_COPY_AND_ASSIGN(ThreadActivityAnalyzer);
};

// in a persistent memory allocator. It supports retrieval of them through
// iteration and directly using a ThreadKey, which allows for cross-references
// to be resolved.
// Note that though atomic snapshots are used and everything has its snapshot
// taken at the same time, the multi-snapshot itself is not atomic and thus may
// show small inconsistencies between threads if attempted on a live system.
class BASE_EXPORT GlobalActivityAnalyzer {
 public:
  struct ProgramLocation {
    int module;
    uintptr_t offset;
  };

  using ThreadKey = ThreadActivityAnalyzer::ThreadKey;

  explicit GlobalActivityAnalyzer(
      std::unique_ptr<PersistentMemoryAllocator> allocator);

  ~GlobalActivityAnalyzer();

  static std::unique_ptr<GlobalActivityAnalyzer> CreateWithAllocator(
      std::unique_ptr<PersistentMemoryAllocator> allocator);

#if !defined(OS_NACL)


  static std::unique_ptr<GlobalActivityAnalyzer> CreateWithFile(
      const FilePath& file_path);
#endif  // !defined(OS_NACL)

  static std::unique_ptr<GlobalActivityAnalyzer> CreateWithSharedMemory(
      base::ReadOnlySharedMemoryMapping mapping);







  int64_t GetFirstProcess();
  int64_t GetNextProcess();






  ThreadActivityAnalyzer* GetFirstAnalyzer(int64_t pid);
  ThreadActivityAnalyzer* GetNextAnalyzer();


  ThreadActivityAnalyzer* GetAnalyzerForThread(const ThreadKey& key);

  ActivityUserData::Snapshot GetUserDataSnapshot(int64_t pid,
                                                 uint32_t ref,
                                                 uint32_t id);


  const ActivityUserData::Snapshot& GetProcessDataSnapshot(int64_t pid);

  std::vector<std::string> GetLogMessages();



  std::vector<GlobalActivityTracker::ModuleInfo> GetModules(int64_t pid);


  ProgramLocation GetProgramLocationFromAddress(uint64_t address);


  bool IsDataComplete() const;

 private:
  using AnalyzerMap =
      std::map<ThreadKey, std::unique_ptr<ThreadActivityAnalyzer>>;

  struct UserDataSnapshot {

    UserDataSnapshot();
    UserDataSnapshot(const UserDataSnapshot& rhs);
    UserDataSnapshot(UserDataSnapshot&& rhs);
    ~UserDataSnapshot();

    int64_t process_id;
    int64_t create_stamp;
    ActivityUserData::Snapshot data;
  };

  void PrepareAllAnalyzers();

  std::unique_ptr<PersistentMemoryAllocator> allocator_;


  int64_t analysis_stamp_;

  PersistentMemoryAllocator::Iterator allocator_iterator_;

  std::set<PersistentMemoryAllocator::Reference> memory_references_;

  std::map<int64_t, UserDataSnapshot> process_data_;


  std::vector<int64_t> process_ids_;

  AnalyzerMap analyzers_;


  AnalyzerMap::iterator analyzers_iterator_;
  int64_t analyzers_iterator_pid_;

  DISALLOW_COPY_AND_ASSIGN(GlobalActivityAnalyzer);
};

}  // namespace debug
}  // namespace base

#endif  // BASE_DEBUG_ACTIVITY_ANALYZER_H_
