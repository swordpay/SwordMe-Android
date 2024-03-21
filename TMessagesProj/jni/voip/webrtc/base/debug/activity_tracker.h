// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// about the state of the application for analysis both while it is running
// and after it has terminated unexpectedly. Its primary purpose is to help
// locate reasons the browser becomes unresponsive by providing insight into
// what all the various threads and processes are (or were) doing.

#ifndef BASE_DEBUG_ACTIVITY_TRACKER_H_
#define BASE_DEBUG_ACTIVITY_TRACKER_H_

// variables. There are no such instances here. This module uses the
// PersistentMemoryAllocator which also uses std::atomic and is written
// by the same author.
#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/atomicops.h"
#include "base/base_export.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/gtest_prod_util.h"
#include "base/location.h"
#include "base/memory/shared_memory_mapping.h"
#include "base/metrics/persistent_memory_allocator.h"
#include "base/process/process_handle.h"
#include "base/sequenced_task_runner.h"
#include "base/strings/string_piece.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/platform_thread.h"
#include "base/threading/thread_local.h"

namespace base {

struct PendingTask;

class FilePath;
class Lock;
class PlatformThreadHandle;
class Process;
class WaitableEvent;

namespace debug {

class ThreadActivityTracker;


enum : int {



  kActivityCallStackSize = 10,
};

// associated with a given process.
struct OwningProcess {
  OwningProcess();
  ~OwningProcess();



  void Release_Initialize(int64_t pid = 0);

  void SetOwningProcessIdForTesting(int64_t pid, int64_t stamp);



  static bool GetOwningProcessId(const void* memory,
                                 int64_t* out_id,
                                 int64_t* out_stamp);

  static constexpr uint32_t kPersistentTypeId = 0xB1179672 + 1;

  static constexpr size_t kExpectedInstanceSize = 24;

  std::atomic<uint32_t> data_id;
  uint32_t padding;
  int64_t process_id;
  int64_t create_stamp;
};

// This union defines all of the various fields. All fields must be explicitly
// sized types to ensure no interoperability problems between 32-bit and
// 64-bit systems.
union ActivityData {




  struct {
    uint32_t id;   // An arbitrary identifier used for association.
    int32_t info;  // An arbitrary value used for information purposes.
  } generic;
  struct {
    uint64_t sequence_id;  // The sequence identifier of the posted task.
  } task;
  struct {
    uint64_t lock_address;  // The memory address of the lock object.
  } lock;
  struct {
    uint64_t event_address;  // The memory address of the event object.
  } event;
  struct {
    int64_t thread_id;  // A unique identifier for a thread within a process.
  } thread;
  struct {
    int64_t process_id;  // A unique identifier for a process.
  } process;
  struct {
    uint32_t code;  // An "exception code" number.
  } exception;







  static ActivityData ForGeneric(uint32_t id, int32_t info) {
    ActivityData data;
    data.generic.id = id;
    data.generic.info = info;
    return data;
  }

  static ActivityData ForTask(uint64_t sequence) {
    ActivityData data;
    data.task.sequence_id = sequence;
    return data;
  }

  static ActivityData ForLock(const void* lock) {
    ActivityData data;
    data.lock.lock_address = reinterpret_cast<uintptr_t>(lock);
    return data;
  }

  static ActivityData ForEvent(const void* event) {
    ActivityData data;
    data.event.event_address = reinterpret_cast<uintptr_t>(event);
    return data;
  }

  static ActivityData ForThread(const PlatformThreadHandle& handle);
  static ActivityData ForThread(const int64_t id) {
    ActivityData data;
    data.thread.thread_id = id;
    return data;
  }

  static ActivityData ForProcess(const int64_t id) {
    ActivityData data;
    data.process.process_id = id;
    return data;
  }

  static ActivityData ForException(const uint32_t code) {
    ActivityData data;
    data.exception.code = code;
    return data;
  }
};

extern const ActivityData kNullActivityData;

// persistent memory allocator. Instances of this class are NOT thread-safe.
// Use from a single thread or protect access with a lock.
class BASE_EXPORT ActivityTrackerMemoryAllocator {
 public:
  using Reference = PersistentMemoryAllocator::Reference;





  ActivityTrackerMemoryAllocator(PersistentMemoryAllocator* allocator,
                                 uint32_t object_type,
                                 uint32_t object_free_type,
                                 size_t object_size,
                                 size_t cache_size,
                                 bool make_iterable);
  ~ActivityTrackerMemoryAllocator();


  Reference GetObjectReference();

  void ReleaseObjectReference(Reference ref);

  template <typename T>
  T* GetAsObject(Reference ref) {
    return allocator_->GetAsObject<T>(ref);
  }

  template <typename T>
  T* GetAsArray(Reference ref, size_t count) {
    return allocator_->GetAsArray<T>(ref, object_type_, count);
  }

  size_t cache_used() const { return cache_used_; }

 private:
  PersistentMemoryAllocator* const allocator_;
  const uint32_t object_type_;
  const uint32_t object_free_type_;
  const size_t object_size_;
  const size_t cache_size_;
  const bool make_iterable_;

  PersistentMemoryAllocator::Iterator iterator_;

  std::unique_ptr<Reference[]> cache_values_;
  size_t cache_used_;

  DISALLOW_COPY_AND_ASSIGN(ActivityTrackerMemoryAllocator);
};

// onto the stack. The |activity_type| indicates what is actually stored in
// the |data| field. All fields must be explicitly sized types to ensure no
// interoperability problems between 32-bit and 64-bit systems.
struct Activity {

  static constexpr uint32_t kPersistentTypeId = 0x99425159 + 1;

  static constexpr size_t kExpectedInstanceSize =
      48 + 8 * kActivityCallStackSize;




  enum Type : uint8_t {

    ACT_NULL = 0,


    ACT_TASK = 1 << 4,
    ACT_TASK_RUN = ACT_TASK,

    ACT_LOCK = 2 << 4,
    ACT_LOCK_ACQUIRE = ACT_LOCK,
    ACT_LOCK_RELEASE,

    ACT_EVENT = 3 << 4,
    ACT_EVENT_WAIT = ACT_EVENT,
    ACT_EVENT_SIGNAL,

    ACT_THREAD = 4 << 4,
    ACT_THREAD_START = ACT_THREAD,
    ACT_THREAD_JOIN,

    ACT_PROCESS = 5 << 4,
    ACT_PROCESS_START = ACT_PROCESS,
    ACT_PROCESS_WAIT,

    ACT_EXCEPTION = 14 << 4,

    ACT_GENERIC = 15 << 4,


    ACT_CATEGORY_MASK = 0xF << 4,
    ACT_ACTION_MASK = 0xF
  };


  int64_t time_internal;

  uint64_t calling_address;




  uint64_t origin_address;





  uint64_t call_stack[kActivityCallStackSize];


  uint32_t user_data_ref;
  uint32_t user_data_id;


  uint8_t activity_type;



  uint8_t padding[7];

  ActivityData data;

  static void FillFrom(Activity* activity,
                       const void* program_counter,
                       const void* origin,
                       Type type,
                       const ActivityData& data);
};

// done by a thread by supporting key/value pairs of any type. This can provide
// additional information during debugging. It is also used to store arbitrary
// global data. All updates must be done from the same thread though other
// threads can read it concurrently if they create new objects using the same
// memory. For a thread-safe version, see ThreadSafeUserData later on.
class BASE_EXPORT ActivityUserData {
 public:


  enum ValueType : uint8_t {
    END_OF_VALUES = 0,
    RAW_VALUE,
    RAW_VALUE_REFERENCE,
    STRING_VALUE,
    STRING_VALUE_REFERENCE,
    CHAR_VALUE,
    BOOL_VALUE,
    SIGNED_VALUE,
    UNSIGNED_VALUE,
  };

  class BASE_EXPORT TypedValue {
   public:
    TypedValue();
    TypedValue(const TypedValue& other);
    ~TypedValue();

    ValueType type() const { return type_; }

    StringPiece Get() const;
    StringPiece GetString() const;
    bool GetBool() const;
    char GetChar() const;
    int64_t GetInt() const;
    uint64_t GetUint() const;








    StringPiece GetReference() const;
    StringPiece GetStringReference() const;

   private:
    friend class ActivityUserData;

    ValueType type_ = END_OF_VALUES;
    uint64_t short_value_;    // Used to hold copy of numbers, etc.
    std::string long_value_;  // Used to hold copy of raw/string data.
    StringPiece ref_value_;   // Used to hold reference to external data.
  };

  using Snapshot = std::map<std::string, TypedValue>;


  ActivityUserData();
  ActivityUserData(void* memory, size_t size, int64_t pid = 0);
  virtual ~ActivityUserData();



  uint32_t id() const {
    return header_ ? header_->owner.data_id.load(std::memory_order_relaxed) : 0;
  }
















  void Set(StringPiece name, const void* memory, size_t size) {
    Set(name, RAW_VALUE, memory, size);
  }
  void SetString(StringPiece name, StringPiece value) {
    Set(name, STRING_VALUE, value.data(), value.length());
  }
  void SetString(StringPiece name, StringPiece16 value) {
    SetString(name, UTF16ToUTF8(value));
  }
  std::atomic<bool>* SetBool(StringPiece name, bool value) {
    char cvalue = value ? 1 : 0;
    void* addr = Set(name, BOOL_VALUE, &cvalue, sizeof(cvalue));
    return reinterpret_cast<std::atomic<bool>*>(addr);
  }
  std::atomic<char>* SetChar(StringPiece name, char value) {
    void* addr = Set(name, CHAR_VALUE, &value, sizeof(value));
    return reinterpret_cast<std::atomic<char>*>(addr);
  }
  std::atomic<int64_t>* SetInt(StringPiece name, int64_t value) {
    void* addr = Set(name, SIGNED_VALUE, &value, sizeof(value));
    return reinterpret_cast<std::atomic<int64_t>*>(addr);
  }
  std::atomic<uint64_t>* SetUint(StringPiece name, uint64_t value) {
    void* addr = Set(name, UNSIGNED_VALUE, &value, sizeof(value));
    return reinterpret_cast<std::atomic<uint64_t>*>(addr);
  }




  void SetReference(StringPiece name, const void* memory, size_t size) {
    SetReference(name, RAW_VALUE_REFERENCE, memory, size);
  }
  void SetStringReference(StringPiece name, StringPiece value) {
    SetReference(name, STRING_VALUE_REFERENCE, value.data(), value.length());
  }





  bool CreateSnapshot(Snapshot* output_snapshot) const;

  const void* GetBaseAddress() const;

  void SetOwningProcessIdForTesting(int64_t pid, int64_t stamp);



  static bool GetOwningProcessId(const void* memory,
                                 int64_t* out_id,
                                 int64_t* out_stamp);

 protected:
  virtual void* Set(StringPiece name,
                    ValueType type,
                    const void* memory,
                    size_t size);

 private:
  FRIEND_TEST_ALL_PREFIXES(ActivityTrackerTest, UserDataTest);

  enum : size_t { kMemoryAlignment = sizeof(uint64_t) };

  struct MemoryHeader {
    MemoryHeader();
    ~MemoryHeader();

    OwningProcess owner;  // Information about the creating process.
  };

  struct FieldHeader {
    FieldHeader();
    ~FieldHeader();

    std::atomic<uint8_t> type;         // Encoded ValueType
    uint8_t name_size;                 // Length of "name" key.
    std::atomic<uint16_t> value_size;  // Actual size of of the stored value.
    uint16_t record_size;              // Total storage of name, value, header.
  };

  struct ReferenceRecord {
    uint64_t address;
    uint64_t size;
  };


  struct ValueInfo {
    ValueInfo();
    ValueInfo(ValueInfo&&);
    ~ValueInfo();

    StringPiece name;                 // The "key" of the record.
    ValueType type;                   // The type of the value.
    void* memory;                     // Where the "value" is held.
    std::atomic<uint16_t>* size_ptr;  // Address of the actual size of value.
    size_t extent;                    // The total storage of the value,
  };                                  // typically rounded up for alignment.

  void SetReference(StringPiece name,
                    ValueType type,
                    const void* memory,
                    size_t size);





  void ImportExistingData() const;



  mutable std::map<StringPiece, ValueInfo> values_;



  mutable char* memory_;
  mutable size_t available_;

  MemoryHeader* const header_;


  const uint32_t orig_data_id;
  const int64_t orig_process_id;
  const int64_t orig_create_stamp;

  DISALLOW_COPY_AND_ASSIGN(ActivityUserData);
};

// a persistent manner, implementing a bounded-size stack in a fixed-size
// memory allocation. In order to support an operational mode where another
// thread is analyzing this data in real-time, atomic operations are used
// where necessary to guarantee a consistent view from the outside.
//
// This class is not generally used directly but instead managed by the
// GlobalActivityTracker instance and updated using Scoped*Activity local
// objects.
class BASE_EXPORT ThreadActivityTracker {
 public:
  using ActivityId = uint32_t;



  struct Header;



  struct BASE_EXPORT Snapshot {


    Snapshot();
    ~Snapshot();


    std::string thread_name;

    int64_t create_stamp;




    int64_t process_id = 0;
    int64_t thread_id = 0;


    std::vector<Activity> activity_stack;


    uint32_t activity_stack_depth = 0;

    Activity last_exception;
  };




  class BASE_EXPORT ScopedActivity {
   public:
    ScopedActivity(ThreadActivityTracker* tracker,
                   const void* program_counter,
                   const void* origin,
                   Activity::Type type,
                   const ActivityData& data);
    ~ScopedActivity();



    bool IsRecorded();

    void ChangeTypeAndData(Activity::Type type, const ActivityData& data);

   protected:


    ThreadActivityTracker* const tracker_;

    ActivityId activity_id_;

   private:
    DISALLOW_COPY_AND_ASSIGN(ScopedActivity);
  };



  ThreadActivityTracker(void* base, size_t size);
  virtual ~ThreadActivityTracker();





  ActivityId PushActivity(const void* program_counter,
                          const void* origin,
                          Activity::Type type,
                          const ActivityData& data);


  ALWAYS_INLINE
  ActivityId PushActivity(const void* origin,
                          Activity::Type type,
                          const ActivityData& data) {
    return PushActivity(GetProgramCounter(), origin, type, data);
  }







  void ChangeActivity(ActivityId id,
                      Activity::Type type,
                      const ActivityData& data);

  void PopActivity(ActivityId id);

  bool IsRecorded(ActivityId id);

  std::unique_ptr<ActivityUserData> GetUserData(
      ActivityId id,
      ActivityTrackerMemoryAllocator* allocator);


  bool HasUserData(ActivityId id);

  void ReleaseUserData(ActivityId id,
                       ActivityTrackerMemoryAllocator* allocator);

  void RecordExceptionActivity(const void* program_counter,
                               const void* origin,
                               Activity::Type type,
                               const ActivityData& data);


  bool IsValid() const;




  bool CreateSnapshot(Snapshot* output_snapshot) const;

  const void* GetBaseAddress();


  uint32_t GetDataVersionForTesting();

  void SetOwningProcessIdForTesting(int64_t pid, int64_t stamp);



  static bool GetOwningProcessId(const void* memory,
                                 int64_t* out_id,
                                 int64_t* out_stamp);


  static size_t SizeForStackDepth(int stack_depth);

 private:
  friend class ActivityTrackerTest;

  bool CalledOnValidThread();

  std::unique_ptr<ActivityUserData> CreateUserDataForActivity(
      Activity* activity,
      ActivityTrackerMemoryAllocator* allocator);

  Header* const header_;        // Pointer to the Header structure.
  Activity* const stack_;       // The stack of activities.

#if DCHECK_IS_ON()



  const PlatformThreadRef thread_id_;  // The thread this instance is bound to.
#endif
  const uint32_t stack_slots_;  // The total number of stack slots.

  bool valid_ = false;          // Tracks whether the data is valid or not.

  DISALLOW_COPY_AND_ASSIGN(ThreadActivityTracker);
};

// the thread trackers is taken from a PersistentMemoryAllocator which allows
// for the data to be analyzed by a parallel process or even post-mortem.
class BASE_EXPORT GlobalActivityTracker {
 public:






  enum : uint32_t {
    kTypeIdActivityTracker = 0x5D7381AF + 4,   // SHA1(ActivityTracker) v4
    kTypeIdUserDataRecord = 0x615EDDD7 + 3,    // SHA1(UserDataRecord) v3
    kTypeIdGlobalLogMessage = 0x4CF434F9 + 1,  // SHA1(GlobalLogMessage) v1
    kTypeIdProcessDataRecord = kTypeIdUserDataRecord + 0x100,

    kTypeIdActivityTrackerFree = ~kTypeIdActivityTracker,
    kTypeIdUserDataRecordFree = ~kTypeIdUserDataRecord,
    kTypeIdProcessDataRecordFree = ~kTypeIdProcessDataRecord,
  };



  enum ProcessPhase : int {

    PROCESS_PHASE_UNKNOWN = 0,
    PROCESS_LAUNCHED = 1,
    PROCESS_LAUNCH_FAILED = 2,
    PROCESS_EXITED_CLEANLY = 10,
    PROCESS_EXITED_WITH_CODE = 11,

    PROCESS_SHUTDOWN_STARTED = 100,
    PROCESS_MAIN_LOOP_STARTED = 101,
  };




  using ProcessExitCallback =
      RepeatingCallback<void(int64_t process_id,
                             int64_t exit_stamp,
                             int exit_code,
                             ProcessPhase exit_phase,
                             std::string&& command_line,
                             ActivityUserData::Snapshot&& process_data)>;


  struct BASE_EXPORT ModuleInfo {
    ModuleInfo();
    ModuleInfo(ModuleInfo&& rhs);
    ModuleInfo(const ModuleInfo& rhs);
    ~ModuleInfo();

    ModuleInfo& operator=(ModuleInfo&& rhs);
    ModuleInfo& operator=(const ModuleInfo& rhs);

    bool is_loaded = false;  // Was the last operation a load or unload?
    uintptr_t address = 0;   // Address of the last load operation.
    int64_t load_time = 0;   // Time of last change; set automatically.


    size_t size = 0;         // The size of the loaded module.
    uint32_t timestamp = 0;  // Opaque "timestamp" for the module.
    uint32_t age = 0;        // Opaque "age" for the module.
    uint8_t identifier[16];  // Opaque identifier (GUID, etc.) for the module.
    std::string file;        // The full path to the file. (UTF-8)
    std::string debug_file;  // The full path to the debug file.
  };



  class BASE_EXPORT ScopedThreadActivity
      : public ThreadActivityTracker::ScopedActivity {
   public:
    ScopedThreadActivity(const void* program_counter,
                         const void* origin,
                         Activity::Type type,
                         const ActivityData& data,
                         bool lock_allowed);
    ~ScopedThreadActivity();

    ActivityUserData& user_data();

   private:





    static ThreadActivityTracker* GetOrCreateTracker(bool lock_allowed) {
      GlobalActivityTracker* global_tracker = Get();
      if (!global_tracker)
        return nullptr;

      if (lock_allowed)
        return global_tracker->GetOrCreateTrackerForCurrentThread();
      else
        return global_tracker->GetTrackerForCurrentThread();
    }

    std::unique_ptr<ActivityUserData> user_data_;

    DISALLOW_COPY_AND_ASSIGN(ScopedThreadActivity);
  };

  ~GlobalActivityTracker();





  static void CreateWithAllocator(
      std::unique_ptr<PersistentMemoryAllocator> allocator,
      int stack_depth,
      int64_t process_id);

#if !defined(OS_NACL)




  static bool CreateWithFile(const FilePath& file_path,
                             size_t size,
                             uint64_t id,
                             StringPiece name,
                             int stack_depth);
#endif  // !defined(OS_NACL)



  static bool CreateWithLocalMemory(size_t size,
                                    uint64_t id,
                                    StringPiece name,
                                    int stack_depth,
                                    int64_t process_id);


  static bool CreateWithSharedMemory(base::WritableSharedMemoryMapping mapping,
                                     uint64_t id,
                                     StringPiece name,
                                     int stack_depth);

  static GlobalActivityTracker* Get() {
    return reinterpret_cast<GlobalActivityTracker*>(
        subtle::Acquire_Load(&g_tracker_));
  }

  static void SetForTesting(std::unique_ptr<GlobalActivityTracker> tracker);




  static std::unique_ptr<GlobalActivityTracker> ReleaseForTesting();

  static bool IsEnabled() { return Get() != nullptr; }



  PersistentMemoryAllocator* allocator() { return allocator_.get(); }




  ThreadActivityTracker* GetTrackerForCurrentThread() {

    if (base::ThreadLocalStorage::HasBeenDestroyed())
      return nullptr;

    return this_thread_tracker_.Get();
  }



  ThreadActivityTracker* GetOrCreateTrackerForCurrentThread() {
    ThreadActivityTracker* tracker = GetTrackerForCurrentThread();
    if (tracker)
      return tracker;
    return CreateTrackerForCurrentThread();
  }

  ThreadActivityTracker* CreateTrackerForCurrentThread();

  void ReleaseTrackerForCurrentThreadForTesting();

  void SetBackgroundTaskRunner(
      const scoped_refptr<SequencedTaskRunner>& runner);

  void SetProcessExitCallback(ProcessExitCallback callback);




  void RecordProcessLaunch(ProcessId process_id,
                           const FilePath::StringType& cmd);
  void RecordProcessLaunch(ProcessId process_id,
                           const FilePath::StringType& exe,
                           const FilePath::StringType& args);
  void RecordProcessExit(ProcessId process_id, int exit_code);
  static void RecordProcessLaunchIfEnabled(ProcessId process_id,
                                           const FilePath::StringType& cmd) {
    GlobalActivityTracker* tracker = Get();
    if (tracker)
      tracker->RecordProcessLaunch(process_id, cmd);
  }
  static void RecordProcessLaunchIfEnabled(ProcessId process_id,
                                           const FilePath::StringType& exe,
                                           const FilePath::StringType& args) {
    GlobalActivityTracker* tracker = Get();
    if (tracker)
      tracker->RecordProcessLaunch(process_id, exe, args);
  }
  static void RecordProcessExitIfEnabled(ProcessId process_id, int exit_code) {
    GlobalActivityTracker* tracker = Get();
    if (tracker)
      tracker->RecordProcessExit(process_id, exit_code);
  }


  void SetProcessPhase(ProcessPhase phase);
  static void SetProcessPhaseIfEnabled(ProcessPhase phase) {
    GlobalActivityTracker* tracker = Get();
    if (tracker)
      tracker->SetProcessPhase(phase);
  }


  void RecordLogMessage(StringPiece message);
  static void RecordLogMessageIfEnabled(StringPiece message) {
    GlobalActivityTracker* tracker = Get();
    if (tracker)
      tracker->RecordLogMessage(message);
  }


  void RecordModuleInfo(const ModuleInfo& info);
  static void RecordModuleInfoIfEnabled(const ModuleInfo& info) {
    GlobalActivityTracker* tracker = Get();
    if (tracker)
      tracker->RecordModuleInfo(info);
  }

  ALWAYS_INLINE
  void RecordException(const void* origin, uint32_t code) {
    return RecordExceptionImpl(GetProgramCounter(), origin, code);
  }
  void RecordException(const void* pc, const void* origin, uint32_t code);

  void MarkDeleted();


  int64_t process_id() { return process_id_; }


  ActivityUserData& process_data() { return process_data_; }

 private:
  friend class GlobalActivityAnalyzer;
  friend class ScopedThreadActivity;
  friend class ActivityTrackerTest;

  enum : int {


    kMaxThreadCount = 100,
    kCachedThreadMemories = 10,
    kCachedUserDataMemories = 10,
  };



  class ThreadSafeUserData : public ActivityUserData {
   public:
    ThreadSafeUserData(void* memory, size_t size, int64_t pid = 0);
    ~ThreadSafeUserData() override;

   private:
    void* Set(StringPiece name,
              ValueType type,
              const void* memory,
              size_t size) override;

    Lock data_lock_;

    DISALLOW_COPY_AND_ASSIGN(ThreadSafeUserData);
  };




  struct BASE_EXPORT ModuleInfoRecord {

    static constexpr uint32_t kPersistentTypeId = 0x05DB5F41 + 1;

    static constexpr size_t kExpectedInstanceSize =
        OwningProcess::kExpectedInstanceSize + 56;



    ModuleInfoRecord();
    ~ModuleInfoRecord();

    OwningProcess owner;            // The process that created this record.
    uint64_t address;               // The base address of the module.
    uint64_t load_time;             // Time of last load/unload.
    uint64_t size;                  // The size of the module in bytes.
    uint32_t timestamp;             // Opaque timestamp of the module.
    uint32_t age;                   // Opaque "age" associated with the module.
    uint8_t identifier[16];         // Opaque identifier for the module.
    std::atomic<uint32_t> changes;  // Number load/unload actions.
    uint16_t pickle_size;           // The size of the following pickle.
    uint8_t loaded;                 // Flag if module is loaded or not.
    char pickle[1];                 // Other strings; may allocate larger.

    bool DecodeTo(GlobalActivityTracker::ModuleInfo* info,
                  size_t record_size) const;
    static ModuleInfoRecord* CreateFrom(
        const GlobalActivityTracker::ModuleInfo& info,
        PersistentMemoryAllocator* allocator);


    bool UpdateFrom(const GlobalActivityTracker::ModuleInfo& info);

   private:
    DISALLOW_COPY_AND_ASSIGN(ModuleInfoRecord);
  };


  class ManagedActivityTracker : public ThreadActivityTracker {
   public:
    ManagedActivityTracker(PersistentMemoryAllocator::Reference mem_reference,
                           void* base,
                           size_t size);
    ~ManagedActivityTracker() override;


    const PersistentMemoryAllocator::Reference mem_reference_;

    void* const mem_base_;

   private:
    DISALLOW_COPY_AND_ASSIGN(ManagedActivityTracker);
  };





  GlobalActivityTracker(std::unique_ptr<PersistentMemoryAllocator> allocator,
                        int stack_depth,
                        int64_t process_id);


  void ReturnTrackerMemory(ManagedActivityTracker* tracker);

  void RecordExceptionImpl(const void* pc, const void* origin, uint32_t code);



  static void OnTLSDestroy(void* value);

  void CleanupAfterProcess(int64_t process_id,
                           int64_t exit_stamp,
                           int exit_code,
                           std::string&& command_line);


  std::unique_ptr<PersistentMemoryAllocator> allocator_;


  const size_t stack_memory_size_;


  const int64_t process_id_;

  ThreadLocalOwnedPointer<ThreadActivityTracker> this_thread_tracker_;

  std::atomic<int> thread_tracker_count_;

  ActivityTrackerMemoryAllocator thread_tracker_allocator_;
  Lock thread_tracker_allocator_lock_;

  ActivityTrackerMemoryAllocator user_data_allocator_;
  Lock user_data_allocator_lock_;

  ThreadSafeUserData process_data_;

  std::map<const std::string, ModuleInfoRecord*> modules_;
  Lock modules_lock_;

  static subtle::AtomicWord g_tracker_;

  Lock global_tracker_lock_;

  std::map<int64_t, std::string> known_processes_;

  scoped_refptr<SequencedTaskRunner> background_task_runner_;




  ProcessExitCallback process_exit_callback_;

  DISALLOW_COPY_AND_ASSIGN(GlobalActivityTracker);
};

class BASE_EXPORT ScopedActivity
    : public GlobalActivityTracker::ScopedThreadActivity {
 public:















  ALWAYS_INLINE
  ScopedActivity(uint8_t action, uint32_t id, int32_t info)
      : ScopedActivity(GetProgramCounter(), action, id, info) {}
  ScopedActivity(Location from_here, uint8_t action, uint32_t id, int32_t info)
      : ScopedActivity(from_here.program_counter(), action, id, info) {}
  ScopedActivity() : ScopedActivity(0, 0, 0) {}





  void ChangeAction(uint8_t action);
  void ChangeInfo(int32_t info);
  void ChangeActionAndInfo(uint8_t action, int32_t info);

 private:

  ScopedActivity(const void* program_counter,
                 uint8_t action,
                 uint32_t id,
                 int32_t info);


  uint32_t id_;

  DISALLOW_COPY_AND_ASSIGN(ScopedActivity);
};


class BASE_EXPORT ScopedTaskRunActivity
    : public GlobalActivityTracker::ScopedThreadActivity {
 public:
  ALWAYS_INLINE
  explicit ScopedTaskRunActivity(const PendingTask& task)
      : ScopedTaskRunActivity(GetProgramCounter(), task) {}

 private:
  ScopedTaskRunActivity(const void* program_counter, const PendingTask& task);
  DISALLOW_COPY_AND_ASSIGN(ScopedTaskRunActivity);
};

class BASE_EXPORT ScopedLockAcquireActivity
    : public GlobalActivityTracker::ScopedThreadActivity {
 public:
  ALWAYS_INLINE
  explicit ScopedLockAcquireActivity(const base::internal::LockImpl* lock)
      : ScopedLockAcquireActivity(GetProgramCounter(), lock) {}

 private:
  ScopedLockAcquireActivity(const void* program_counter,
                            const base::internal::LockImpl* lock);
  DISALLOW_COPY_AND_ASSIGN(ScopedLockAcquireActivity);
};

class BASE_EXPORT ScopedEventWaitActivity
    : public GlobalActivityTracker::ScopedThreadActivity {
 public:
  ALWAYS_INLINE
  explicit ScopedEventWaitActivity(const WaitableEvent* event)
      : ScopedEventWaitActivity(GetProgramCounter(), event) {}

 private:
  ScopedEventWaitActivity(const void* program_counter,
                          const WaitableEvent* event);
  DISALLOW_COPY_AND_ASSIGN(ScopedEventWaitActivity);
};

class BASE_EXPORT ScopedThreadJoinActivity
    : public GlobalActivityTracker::ScopedThreadActivity {
 public:
  ALWAYS_INLINE
  explicit ScopedThreadJoinActivity(const PlatformThreadHandle* thread)
      : ScopedThreadJoinActivity(GetProgramCounter(), thread) {}

 private:
  ScopedThreadJoinActivity(const void* program_counter,
                           const PlatformThreadHandle* thread);
  DISALLOW_COPY_AND_ASSIGN(ScopedThreadJoinActivity);
};

#if !defined(OS_NACL) && !defined(OS_IOS)
class BASE_EXPORT ScopedProcessWaitActivity
    : public GlobalActivityTracker::ScopedThreadActivity {
 public:
  ALWAYS_INLINE
  explicit ScopedProcessWaitActivity(const Process* process)
      : ScopedProcessWaitActivity(GetProgramCounter(), process) {}

 private:
  ScopedProcessWaitActivity(const void* program_counter,
                            const Process* process);
  DISALLOW_COPY_AND_ASSIGN(ScopedProcessWaitActivity);
};
#endif

}  // namespace debug
}  // namespace base

#endif  // BASE_DEBUG_ACTIVITY_TRACKER_H_
