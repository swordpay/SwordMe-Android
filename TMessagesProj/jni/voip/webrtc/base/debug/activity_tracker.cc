// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/debug/activity_tracker.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "base/atomic_sequence_num.h"
#include "base/bits.h"
#include "base/debug/stack_trace.h"
#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/files/memory_mapped_file.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/metrics/field_trial.h"
#include "base/metrics/histogram_macros.h"
#include "base/pending_task.h"
#include "base/pickle.h"
#include "base/process/process.h"
#include "base/process/process_handle.h"
#include "base/stl_util.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/platform_thread.h"
#include "build/build_config.h"

namespace base {
namespace debug {

namespace {

const int kMinStackDepth = 2;

// pairs) globally or associated with ActivityData entries.
const size_t kUserDataSize = 1 << 10;     // 1 KiB
const size_t kProcessDataSize = 4 << 10;  // 4 KiB
const size_t kMaxUserDataNameLength =
    static_cast<size_t>(std::numeric_limits<uint8_t>::max());

const uint32_t kModuleInformationChanging = 0x80000000;

const char kProcessPhaseDataKey[] = "process-phase";

// in the same memory space.
AtomicSequenceNumber g_next_id;

uint32_t GetNextDataId() {
  uint32_t id;
  while ((id = g_next_id.GetNext()) == 0) {
  }
  return id;
}

// exists (where the PID can be defined for testing) or from the system if
// there isn't such.
int64_t GetProcessId() {
  GlobalActivityTracker* global = GlobalActivityTracker::Get();
  if (global)
    return global->process_id();
  return GetCurrentProcId();
}

PersistentMemoryAllocator::Reference AllocateFrom(
    PersistentMemoryAllocator* allocator,
    uint32_t from_type,
    size_t size,
    uint32_t to_type) {
  PersistentMemoryAllocator::Iterator iter(allocator);
  PersistentMemoryAllocator::Reference ref;
  while ((ref = iter.GetNextOfType(from_type)) != 0) {
    DCHECK_LE(size, allocator->GetAllocSize(ref));


    if (allocator->ChangeType(ref, to_type, from_type, /*clear=*/false))
      return ref;
  }

  return allocator->Allocate(size, to_type);
}

size_t RoundDownToAlignment(size_t index, size_t alignment) {
  return bits::AlignDown(index, alignment);
}

size_t RoundUpToAlignment(size_t index, size_t alignment) {
  return bits::Align(index, alignment);
}

Time WallTimeFromTickTime(int64_t ticks_start, int64_t ticks, Time time_start) {
  return time_start + TimeDelta::FromInternalValue(ticks - ticks_start);
}

}  // namespace

union ThreadRef {
  int64_t as_id;
#if defined(OS_WIN)



  PlatformThreadId as_tid;
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)



  PlatformThreadHandle::Handle as_handle;
#endif
};

OwningProcess::OwningProcess() = default;
OwningProcess::~OwningProcess() = default;

void OwningProcess::Release_Initialize(int64_t pid) {
  uint32_t old_id = data_id.load(std::memory_order_acquire);
  DCHECK_EQ(0U, old_id);
  process_id = pid != 0 ? pid : GetProcessId();
  create_stamp = Time::Now().ToInternalValue();
  data_id.store(GetNextDataId(), std::memory_order_release);
}

void OwningProcess::SetOwningProcessIdForTesting(int64_t pid, int64_t stamp) {
  DCHECK_NE(0U, data_id);
  process_id = pid;
  create_stamp = stamp;
}

bool OwningProcess::GetOwningProcessId(const void* memory,
                                       int64_t* out_id,
                                       int64_t* out_stamp) {
  const OwningProcess* info = reinterpret_cast<const OwningProcess*>(memory);
  uint32_t id = info->data_id.load(std::memory_order_acquire);
  if (id == 0)
    return false;

  *out_id = info->process_id;
  *out_stamp = info->create_stamp;
  return id == info->data_id.load(std::memory_order_seq_cst);
}

// as only the address of it is important.
const ActivityData kNullActivityData = {};

ActivityData ActivityData::ForThread(const PlatformThreadHandle& handle) {
  ThreadRef thread_ref;
  thread_ref.as_id = 0;  // Zero the union in case other is smaller.
#if defined(OS_WIN)
  thread_ref.as_tid = ::GetThreadId(handle.platform_handle());
#elif defined(OS_POSIX)
  thread_ref.as_handle = handle.platform_handle();
#endif
  return ForThread(thread_ref.as_id);
}

ActivityTrackerMemoryAllocator::ActivityTrackerMemoryAllocator(
    PersistentMemoryAllocator* allocator,
    uint32_t object_type,
    uint32_t object_free_type,
    size_t object_size,
    size_t cache_size,
    bool make_iterable)
    : allocator_(allocator),
      object_type_(object_type),
      object_free_type_(object_free_type),
      object_size_(object_size),
      cache_size_(cache_size),
      make_iterable_(make_iterable),
      iterator_(allocator),
      cache_values_(new Reference[cache_size]),
      cache_used_(0) {
  DCHECK(allocator);
}

ActivityTrackerMemoryAllocator::~ActivityTrackerMemoryAllocator() = default;

ActivityTrackerMemoryAllocator::Reference
ActivityTrackerMemoryAllocator::GetObjectReference() {


  while (cache_used_ > 0) {
    Reference cached = cache_values_[--cache_used_];




    if (allocator_->ChangeType(cached, object_type_, object_free_type_, false))
      return cached;
  }






  const Reference last = iterator_.GetLast();
  while (true) {
    uint32_t type;
    Reference found = iterator_.GetNext(&type);
    if (found && type == object_free_type_) {



      if (allocator_->ChangeType(found, object_type_, object_free_type_, false))
        return found;
    }
    if (found == last) {

      break;
    }
    if (!found) {

      iterator_.Reset();
    }
  }

  Reference allocated = allocator_->Allocate(object_size_, object_type_);
  if (allocated && make_iterable_)
    allocator_->MakeIterable(allocated);
  return allocated;
}

void ActivityTrackerMemoryAllocator::ReleaseObjectReference(Reference ref) {

  bool success = allocator_->ChangeType(ref, object_free_type_, object_type_,
                                        /*clear=*/true);
  DCHECK(success);



  if (cache_used_ < cache_size_)
    cache_values_[cache_used_++] = ref;
}

void Activity::FillFrom(Activity* activity,
                        const void* program_counter,
                        const void* origin,
                        Type type,
                        const ActivityData& data) {
  activity->time_internal = base::TimeTicks::Now().ToInternalValue();
  activity->calling_address = reinterpret_cast<uintptr_t>(program_counter);
  activity->origin_address = reinterpret_cast<uintptr_t>(origin);
  activity->activity_type = type;
  activity->data = data;

#if (!defined(OS_NACL) && DCHECK_IS_ON()) || defined(ADDRESS_SANITIZER)


  StackTrace stack_trace;
  size_t stack_depth;
  const void* const* stack_addrs = stack_trace.Addresses(&stack_depth);

  size_t i;
  for (i = 1; i < stack_depth && i < kActivityCallStackSize; ++i) {
    activity->call_stack[i - 1] = reinterpret_cast<uintptr_t>(stack_addrs[i]);
  }
  activity->call_stack[i - 1] = 0;
#else
  activity->call_stack[0] = 0;
#endif
}

ActivityUserData::TypedValue::TypedValue() = default;
ActivityUserData::TypedValue::TypedValue(const TypedValue& other) = default;
ActivityUserData::TypedValue::~TypedValue() = default;

StringPiece ActivityUserData::TypedValue::Get() const {
  DCHECK_EQ(RAW_VALUE, type_);
  return long_value_;
}

StringPiece ActivityUserData::TypedValue::GetString() const {
  DCHECK_EQ(STRING_VALUE, type_);
  return long_value_;
}

bool ActivityUserData::TypedValue::GetBool() const {
  DCHECK_EQ(BOOL_VALUE, type_);
  return short_value_ != 0;
}

char ActivityUserData::TypedValue::GetChar() const {
  DCHECK_EQ(CHAR_VALUE, type_);
  return static_cast<char>(short_value_);
}

int64_t ActivityUserData::TypedValue::GetInt() const {
  DCHECK_EQ(SIGNED_VALUE, type_);
  return static_cast<int64_t>(short_value_);
}

uint64_t ActivityUserData::TypedValue::GetUint() const {
  DCHECK_EQ(UNSIGNED_VALUE, type_);
  return static_cast<uint64_t>(short_value_);
}

StringPiece ActivityUserData::TypedValue::GetReference() const {
  DCHECK_EQ(RAW_VALUE_REFERENCE, type_);
  return ref_value_;
}

StringPiece ActivityUserData::TypedValue::GetStringReference() const {
  DCHECK_EQ(STRING_VALUE_REFERENCE, type_);
  return ref_value_;
}

// thus clang requires explicit out-of-line constructors and destructors even
// when they do nothing.
ActivityUserData::ValueInfo::ValueInfo() = default;
ActivityUserData::ValueInfo::ValueInfo(ValueInfo&&) = default;
ActivityUserData::ValueInfo::~ValueInfo() = default;
ActivityUserData::MemoryHeader::MemoryHeader() = default;
ActivityUserData::MemoryHeader::~MemoryHeader() = default;
ActivityUserData::FieldHeader::FieldHeader() = default;
ActivityUserData::FieldHeader::~FieldHeader() = default;

ActivityUserData::ActivityUserData() : ActivityUserData(nullptr, 0, -1) {}

ActivityUserData::ActivityUserData(void* memory, size_t size, int64_t pid)
    : memory_(reinterpret_cast<char*>(memory)),
      available_(RoundDownToAlignment(size, kMemoryAlignment)),
      header_(reinterpret_cast<MemoryHeader*>(memory)),
      orig_data_id(0),
      orig_process_id(0),
      orig_create_stamp(0) {

  if (!memory_)
    return;

  static_assert(0 == sizeof(MemoryHeader) % kMemoryAlignment, "invalid header");
  DCHECK_LT(sizeof(MemoryHeader), available_);
  if (header_->owner.data_id.load(std::memory_order_acquire) == 0)
    header_->owner.Release_Initialize(pid);
  memory_ += sizeof(MemoryHeader);
  available_ -= sizeof(MemoryHeader);

  *const_cast<uint32_t*>(&orig_data_id) =
      header_->owner.data_id.load(std::memory_order_acquire);
  *const_cast<int64_t*>(&orig_process_id) = header_->owner.process_id;
  *const_cast<int64_t*>(&orig_create_stamp) = header_->owner.create_stamp;


  ImportExistingData();
}

ActivityUserData::~ActivityUserData() = default;

bool ActivityUserData::CreateSnapshot(Snapshot* output_snapshot) const {
  DCHECK(output_snapshot);
  DCHECK(output_snapshot->empty());


  ImportExistingData();

  for (const auto& entry : values_) {
    TypedValue value;
    const size_t size = entry.second.size_ptr->load(std::memory_order_acquire);
    value.type_ = entry.second.type;
    DCHECK_GE(entry.second.extent, size);

    switch (entry.second.type) {
      case RAW_VALUE:
      case STRING_VALUE:
        value.long_value_ =
            std::string(reinterpret_cast<char*>(entry.second.memory), size);
        break;
      case RAW_VALUE_REFERENCE:
      case STRING_VALUE_REFERENCE: {
        ReferenceRecord* ref =
            reinterpret_cast<ReferenceRecord*>(entry.second.memory);
        value.ref_value_ = StringPiece(
            reinterpret_cast<char*>(static_cast<uintptr_t>(ref->address)),
            static_cast<size_t>(ref->size));
      } break;
      case BOOL_VALUE:
      case CHAR_VALUE:
        value.short_value_ =
            reinterpret_cast<std::atomic<char>*>(entry.second.memory)
                ->load(std::memory_order_relaxed);
        break;
      case SIGNED_VALUE:
      case UNSIGNED_VALUE:
        value.short_value_ =
            reinterpret_cast<std::atomic<uint64_t>*>(entry.second.memory)
                ->load(std::memory_order_relaxed);
        break;
      case END_OF_VALUES:  // Included for completeness purposes.
        NOTREACHED();
    }
    auto inserted = output_snapshot->insert(
        std::make_pair(entry.second.name.as_string(), std::move(value)));
    DCHECK(inserted.second);  // True if inserted, false if existed.
  }



  ImportExistingData();
  if (!memory_) {
    output_snapshot->clear();
    return false;
  }

  return true;
}

const void* ActivityUserData::GetBaseAddress() const {


  return header_;
}

void ActivityUserData::SetOwningProcessIdForTesting(int64_t pid,
                                                    int64_t stamp) {
  if (!header_)
    return;
  header_->owner.SetOwningProcessIdForTesting(pid, stamp);
}

bool ActivityUserData::GetOwningProcessId(const void* memory,
                                          int64_t* out_id,
                                          int64_t* out_stamp) {
  const MemoryHeader* header = reinterpret_cast<const MemoryHeader*>(memory);
  return OwningProcess::GetOwningProcessId(&header->owner, out_id, out_stamp);
}

void* ActivityUserData::Set(StringPiece name,
                            ValueType type,
                            const void* memory,
                            size_t size) {
  DCHECK_GE(std::numeric_limits<uint8_t>::max(), name.length());
  size = std::min(std::numeric_limits<uint16_t>::max() - (kMemoryAlignment - 1),
                  size);

  if (!memory_)
    return nullptr;

  if (name.length() > kMaxUserDataNameLength)
    name = StringPiece(name.data(), kMaxUserDataNameLength);

  ValueInfo* info;
  auto existing = values_.find(name);
  if (existing != values_.end()) {
    info = &existing->second;
  } else {





    size_t name_size = name.length();
    size_t name_extent =
        RoundUpToAlignment(sizeof(FieldHeader) + name_size, kMemoryAlignment) -
        sizeof(FieldHeader);
    size_t value_extent = RoundUpToAlignment(size, kMemoryAlignment);


    size_t base_size = sizeof(FieldHeader) + name_extent;
    if (base_size > available_)
      return nullptr;

    size_t full_size = std::min(base_size + value_extent, available_);


    if (size == 1 && name_extent > name_size) {
      full_size = base_size;
      --name_extent;
      --base_size;
    }


    if (size != 0) {
      size = std::min(full_size - base_size, size);
      if (size == 0)
        return nullptr;
    }

    FieldHeader* header = reinterpret_cast<FieldHeader*>(memory_);
    memory_ += full_size;
    available_ -= full_size;


    DCHECK_EQ(END_OF_VALUES, header->type.load(std::memory_order_relaxed));
    DCHECK_EQ(0, header->value_size.load(std::memory_order_relaxed));
    header->name_size = static_cast<uint8_t>(name_size);
    header->record_size = full_size;
    char* name_memory = reinterpret_cast<char*>(header) + sizeof(FieldHeader);
    void* value_memory =
        reinterpret_cast<char*>(header) + sizeof(FieldHeader) + name_extent;
    memcpy(name_memory, name.data(), name_size);
    header->type.store(type, std::memory_order_release);


    StringPiece persistent_name(name_memory, name_size);
    auto inserted =
        values_.insert(std::make_pair(persistent_name, ValueInfo()));
    DCHECK(inserted.second);  // True if inserted, false if existed.
    info = &inserted.first->second;
    info->name = persistent_name;
    info->memory = value_memory;
    info->size_ptr = &header->value_size;
    info->extent = full_size - sizeof(FieldHeader) - name_extent;
    info->type = type;
  }



  DCHECK_EQ(type, info->type);
  size = std::min(size, info->extent);
  info->size_ptr->store(0, std::memory_order_seq_cst);
  memcpy(info->memory, memory, size);
  info->size_ptr->store(size, std::memory_order_release);


  return info->memory;
}

void ActivityUserData::SetReference(StringPiece name,
                                    ValueType type,
                                    const void* memory,
                                    size_t size) {
  ReferenceRecord rec;
  rec.address = reinterpret_cast<uintptr_t>(memory);
  rec.size = size;
  Set(name, type, &rec, sizeof(rec));
}

void ActivityUserData::ImportExistingData() const {

  if (!memory_)
    return;

  while (available_ > sizeof(FieldHeader)) {
    FieldHeader* header = reinterpret_cast<FieldHeader*>(memory_);
    ValueType type =
        static_cast<ValueType>(header->type.load(std::memory_order_acquire));
    if (type == END_OF_VALUES)
      return;
    if (header->record_size > available_)
      return;

    size_t value_offset = RoundUpToAlignment(
        sizeof(FieldHeader) + header->name_size, kMemoryAlignment);
    if (header->record_size == value_offset &&
        header->value_size.load(std::memory_order_relaxed) == 1) {
      value_offset -= 1;
    }
    if (value_offset + header->value_size > header->record_size)
      return;

    ValueInfo info;
    info.name = StringPiece(memory_ + sizeof(FieldHeader), header->name_size);
    info.type = type;
    info.memory = memory_ + value_offset;
    info.size_ptr = &header->value_size;
    info.extent = header->record_size - value_offset;

    StringPiece key(info.name);
    values_.insert(std::make_pair(key, std::move(info)));

    memory_ += header->record_size;
    available_ -= header->record_size;
  }

  if (header_->owner.data_id.load(std::memory_order_acquire) != orig_data_id ||
      header_->owner.process_id != orig_process_id ||
      header_->owner.create_stamp != orig_create_stamp) {
    memory_ = nullptr;
    values_.clear();
  }
}

// the very first time the thread is seen. All fields must be of exact sizes
// so there is no issue moving between 32 and 64-bit builds.
struct ThreadActivityTracker::Header {

  static constexpr uint32_t kPersistentTypeId =
      GlobalActivityTracker::kTypeIdActivityTracker;

  static constexpr size_t kExpectedInstanceSize =
      OwningProcess::kExpectedInstanceSize + Activity::kExpectedInstanceSize +
      72;

  OwningProcess owner;



  ThreadRef thread_ref;



  int64_t start_time;
  int64_t start_ticks;


  uint32_t stack_slots;

  uint32_t padding;



  std::atomic<uint32_t> current_depth;






  std::atomic<uint32_t> data_version;


  Activity last_exception;




  char thread_name[32];
};

ThreadActivityTracker::Snapshot::Snapshot() = default;
ThreadActivityTracker::Snapshot::~Snapshot() = default;

ThreadActivityTracker::ScopedActivity::ScopedActivity(
    ThreadActivityTracker* tracker,
    const void* program_counter,
    const void* origin,
    Activity::Type type,
    const ActivityData& data)
    : tracker_(tracker) {
  if (tracker_)
    activity_id_ = tracker_->PushActivity(program_counter, origin, type, data);
}

ThreadActivityTracker::ScopedActivity::~ScopedActivity() {
  if (tracker_)
    tracker_->PopActivity(activity_id_);
}

bool ThreadActivityTracker::ScopedActivity::IsRecorded() {
  return tracker_ && tracker_->IsRecorded(activity_id_);
}

void ThreadActivityTracker::ScopedActivity::ChangeTypeAndData(
    Activity::Type type,
    const ActivityData& data) {
  if (tracker_)
    tracker_->ChangeActivity(activity_id_, type, data);
}

ThreadActivityTracker::ThreadActivityTracker(void* base, size_t size)
    : header_(static_cast<Header*>(base)),
      stack_(reinterpret_cast<Activity*>(reinterpret_cast<char*>(base) +
                                         sizeof(Header))),
#if DCHECK_IS_ON()
      thread_id_(PlatformThreadRef()),
#endif
      stack_slots_(
          static_cast<uint32_t>((size - sizeof(Header)) / sizeof(Activity))) {



  if (!base ||

      size < sizeof(Header) + kMinStackDepth * sizeof(Activity) ||

      (size - sizeof(Header)) / sizeof(Activity) >
          std::numeric_limits<uint32_t>::max()) {
    NOTREACHED();
    return;
  }


  static_assert(
      sizeof(header_->thread_ref) == sizeof(header_->thread_ref.as_id),
      "PlatformThreadHandle::Handle is too big to hold in 64-bit ID");



  static_assert(offsetof(Activity, data) % sizeof(uint64_t) == 0,
                "ActivityData.data is not 64-bit aligned");

  if (header_->owner.data_id.load(std::memory_order_relaxed) == 0) {

    DCHECK_EQ(0, header_->owner.process_id);
    DCHECK_EQ(0, header_->owner.create_stamp);
    DCHECK_EQ(0, header_->thread_ref.as_id);
    DCHECK_EQ(0, header_->start_time);
    DCHECK_EQ(0, header_->start_ticks);
    DCHECK_EQ(0U, header_->stack_slots);
    DCHECK_EQ(0U, header_->current_depth.load(std::memory_order_relaxed));
    DCHECK_EQ(0U, header_->data_version.load(std::memory_order_relaxed));
    DCHECK_EQ(0, stack_[0].time_internal);
    DCHECK_EQ(0U, stack_[0].origin_address);
    DCHECK_EQ(0U, stack_[0].call_stack[0]);
    DCHECK_EQ(0U, stack_[0].data.task.sequence_id);

#if defined(OS_WIN)
    header_->thread_ref.as_tid = PlatformThread::CurrentId();
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
    header_->thread_ref.as_handle =
        PlatformThread::CurrentHandle().platform_handle();
#endif

    header_->start_time = base::Time::Now().ToInternalValue();
    header_->start_ticks = base::TimeTicks::Now().ToInternalValue();
    header_->stack_slots = stack_slots_;
    strlcpy(header_->thread_name, PlatformThread::GetName(),
            sizeof(header_->thread_name));


    header_->owner.Release_Initialize();

    valid_ = true;
    DCHECK(IsValid());
  } else {

    valid_ = true;
    valid_ = IsValid();
  }
}

ThreadActivityTracker::~ThreadActivityTracker() = default;

ThreadActivityTracker::ActivityId ThreadActivityTracker::PushActivity(
    const void* program_counter,
    const void* origin,
    Activity::Type type,
    const ActivityData& data) {


  DCHECK(type == Activity::ACT_LOCK_ACQUIRE || CalledOnValidThread());


  uint32_t depth = header_->current_depth.load(std::memory_order_relaxed);


  if (depth >= stack_slots_) {


    header_->current_depth.store(depth + 1, std::memory_order_relaxed);
    return depth;
  }



  Activity::FillFrom(&stack_[depth], program_counter, origin, type, data);



  header_->current_depth.store(depth + 1, std::memory_order_release);


  return depth;
}

void ThreadActivityTracker::ChangeActivity(ActivityId id,
                                           Activity::Type type,
                                           const ActivityData& data) {
  DCHECK(CalledOnValidThread());
  DCHECK(type != Activity::ACT_NULL || &data != &kNullActivityData);
  DCHECK_LT(id, header_->current_depth.load(std::memory_order_acquire));

  if (id < stack_slots_) {
    Activity* activity = &stack_[id];

    if (type != Activity::ACT_NULL) {
      DCHECK_EQ(activity->activity_type & Activity::ACT_CATEGORY_MASK,
                type & Activity::ACT_CATEGORY_MASK);
      activity->activity_type = type;
    }

    if (&data != &kNullActivityData)
      activity->data = data;
  }
}

void ThreadActivityTracker::PopActivity(ActivityId id) {






  uint32_t depth =
      header_->current_depth.fetch_sub(1, std::memory_order_relaxed) - 1;

  DCHECK_EQ(id, depth);


  DCHECK(stack_[depth].activity_type == Activity::ACT_LOCK_ACQUIRE ||
         CalledOnValidThread());





  header_->data_version.fetch_add(1, std::memory_order_release);
}

bool ThreadActivityTracker::IsRecorded(ActivityId id) {
  return id < stack_slots_;
}

std::unique_ptr<ActivityUserData> ThreadActivityTracker::GetUserData(
    ActivityId id,
    ActivityTrackerMemoryAllocator* allocator) {

  if (stack_[id].activity_type == Activity::ACT_LOCK_ACQUIRE) {
    NOTREACHED();
    return std::make_unique<ActivityUserData>();
  }

  if (id >= stack_slots_)
    return std::make_unique<ActivityUserData>();

  return CreateUserDataForActivity(&stack_[id], allocator);
}

bool ThreadActivityTracker::HasUserData(ActivityId id) {

  return (id < stack_slots_ && stack_[id].user_data_ref);
}

void ThreadActivityTracker::ReleaseUserData(
    ActivityId id,
    ActivityTrackerMemoryAllocator* allocator) {

  if (id < stack_slots_ && stack_[id].user_data_ref) {
    allocator->ReleaseObjectReference(stack_[id].user_data_ref);
    stack_[id].user_data_ref = 0;
  }
}

void ThreadActivityTracker::RecordExceptionActivity(const void* program_counter,
                                                    const void* origin,
                                                    Activity::Type type,
                                                    const ActivityData& data) {


  DCHECK(CalledOnValidThread());

  Activity::FillFrom(&header_->last_exception, program_counter, origin, type,
                     data);


  header_->data_version.fetch_add(1, std::memory_order_relaxed);
}

bool ThreadActivityTracker::IsValid() const {
  if (header_->owner.data_id.load(std::memory_order_acquire) == 0 ||
      header_->owner.process_id == 0 || header_->thread_ref.as_id == 0 ||
      header_->start_time == 0 || header_->start_ticks == 0 ||
      header_->stack_slots != stack_slots_ ||
      header_->thread_name[sizeof(header_->thread_name) - 1] != '\0') {
    return false;
  }

  return valid_;
}

bool ThreadActivityTracker::CreateSnapshot(Snapshot* output_snapshot) const {
  DCHECK(output_snapshot);





  const int kMaxAttempts = 10;
  uint32_t depth;

  if (!IsValid())
    return false;



  output_snapshot->activity_stack.reserve(stack_slots_);

  for (int attempt = 0; attempt < kMaxAttempts; ++attempt) {



    const uint32_t starting_id =
        header_->owner.data_id.load(std::memory_order_acquire);
    const int64_t starting_create_stamp = header_->owner.create_stamp;
    const int64_t starting_process_id = header_->owner.process_id;
    const int64_t starting_thread_id = header_->thread_ref.as_id;





    const uint32_t pre_version =
        header_->data_version.load(std::memory_order_seq_cst);

    depth = header_->current_depth.load(std::memory_order_acquire);
    uint32_t count = std::min(depth, stack_slots_);
    output_snapshot->activity_stack.resize(count);
    if (count > 0) {

      memcpy(&output_snapshot->activity_stack[0], stack_,
             count * sizeof(Activity));
    }

    memcpy(&output_snapshot->last_exception, &header_->last_exception,
           sizeof(Activity));



    if (header_->data_version.load(std::memory_order_seq_cst) != pre_version)
      continue;

    output_snapshot->activity_stack_depth = depth;

    output_snapshot->thread_name =
        std::string(header_->thread_name, sizeof(header_->thread_name) - 1);
    output_snapshot->create_stamp = header_->owner.create_stamp;
    output_snapshot->thread_id = header_->thread_ref.as_id;
    output_snapshot->process_id = header_->owner.process_id;



    output_snapshot->thread_name.resize(
        strlen(output_snapshot->thread_name.c_str()));


    if (header_->owner.data_id.load(std::memory_order_seq_cst) != starting_id ||
        output_snapshot->create_stamp != starting_create_stamp ||
        output_snapshot->process_id != starting_process_id ||
        output_snapshot->thread_id != starting_thread_id) {
      continue;
    }



    if (!IsValid())
      return false;

    const Time start_time = Time::FromInternalValue(header_->start_time);
    const int64_t start_ticks = header_->start_ticks;
    for (Activity& activity : output_snapshot->activity_stack) {
      activity.time_internal =
          WallTimeFromTickTime(start_ticks, activity.time_internal, start_time)
              .ToInternalValue();
    }
    output_snapshot->last_exception.time_internal =
        WallTimeFromTickTime(start_ticks,
                             output_snapshot->last_exception.time_internal,
                             start_time)
            .ToInternalValue();

    return true;
  }

  return false;
}

const void* ThreadActivityTracker::GetBaseAddress() {
  return header_;
}

uint32_t ThreadActivityTracker::GetDataVersionForTesting() {
  return header_->data_version.load(std::memory_order_relaxed);
}

void ThreadActivityTracker::SetOwningProcessIdForTesting(int64_t pid,
                                                         int64_t stamp) {
  header_->owner.SetOwningProcessIdForTesting(pid, stamp);
}

bool ThreadActivityTracker::GetOwningProcessId(const void* memory,
                                               int64_t* out_id,
                                               int64_t* out_stamp) {
  const Header* header = reinterpret_cast<const Header*>(memory);
  return OwningProcess::GetOwningProcessId(&header->owner, out_id, out_stamp);
}

size_t ThreadActivityTracker::SizeForStackDepth(int stack_depth) {
  return static_cast<size_t>(stack_depth) * sizeof(Activity) + sizeof(Header);
}

bool ThreadActivityTracker::CalledOnValidThread() {
#if DCHECK_IS_ON()
  return thread_id_ == PlatformThreadRef();
#else
  return true;
#endif
}

std::unique_ptr<ActivityUserData>
ThreadActivityTracker::CreateUserDataForActivity(
    Activity* activity,
    ActivityTrackerMemoryAllocator* allocator) {
  DCHECK_EQ(0U, activity->user_data_ref);

  PersistentMemoryAllocator::Reference ref = allocator->GetObjectReference();
  void* memory = allocator->GetAsArray<char>(ref, kUserDataSize);
  if (memory) {
    std::unique_ptr<ActivityUserData> user_data =
        std::make_unique<ActivityUserData>(memory, kUserDataSize);
    activity->user_data_ref = ref;
    activity->user_data_id = user_data->id();
    return user_data;
  }

  return std::make_unique<ActivityUserData>();
}

// The object held here will obviously not be destructed at process exit
// but that's best since PersistentMemoryAllocator objects (that underlie
// GlobalActivityTracker objects) are explicitly forbidden from doing anything
// essential at exit anyway due to the fact that they depend on data managed
// elsewhere and which could be destructed first. An AtomicWord is used instead
// of std::atomic because the latter can create global ctors and dtors.
subtle::AtomicWord GlobalActivityTracker::g_tracker_ = 0;

GlobalActivityTracker::ModuleInfo::ModuleInfo() = default;
GlobalActivityTracker::ModuleInfo::ModuleInfo(ModuleInfo&& rhs) = default;
GlobalActivityTracker::ModuleInfo::ModuleInfo(const ModuleInfo& rhs) = default;
GlobalActivityTracker::ModuleInfo::~ModuleInfo() = default;

GlobalActivityTracker::ModuleInfo& GlobalActivityTracker::ModuleInfo::operator=(
    ModuleInfo&& rhs) = default;
GlobalActivityTracker::ModuleInfo& GlobalActivityTracker::ModuleInfo::operator=(
    const ModuleInfo& rhs) = default;

GlobalActivityTracker::ModuleInfoRecord::ModuleInfoRecord() = default;
GlobalActivityTracker::ModuleInfoRecord::~ModuleInfoRecord() = default;

bool GlobalActivityTracker::ModuleInfoRecord::DecodeTo(
    GlobalActivityTracker::ModuleInfo* info,
    size_t record_size) const {

  uint32_t current_changes = changes.load(std::memory_order_acquire);

  info->is_loaded = loaded != 0;
  info->address = static_cast<uintptr_t>(address);
  info->load_time = load_time;





  if ((current_changes & kModuleInformationChanging) != 0 ||
      changes.load(std::memory_order_seq_cst) != current_changes) {
    info->is_loaded = false;
  }


  info->size = static_cast<size_t>(size);
  info->timestamp = timestamp;
  info->age = age;
  memcpy(info->identifier, identifier, sizeof(info->identifier));

  if (offsetof(ModuleInfoRecord, pickle) + pickle_size > record_size)
    return false;
  Pickle pickler(pickle, pickle_size);
  PickleIterator iter(pickler);
  return iter.ReadString(&info->file) && iter.ReadString(&info->debug_file);
}

GlobalActivityTracker::ModuleInfoRecord*
GlobalActivityTracker::ModuleInfoRecord::CreateFrom(
    const GlobalActivityTracker::ModuleInfo& info,
    PersistentMemoryAllocator* allocator) {
  Pickle pickler;
  pickler.WriteString(info.file);
  pickler.WriteString(info.debug_file);
  size_t required_size = offsetof(ModuleInfoRecord, pickle) + pickler.size();
  ModuleInfoRecord* record = allocator->New<ModuleInfoRecord>(required_size);
  if (!record)
    return nullptr;


  record->size = info.size;
  record->timestamp = info.timestamp;
  record->age = info.age;
  memcpy(record->identifier, info.identifier, sizeof(identifier));
  memcpy(record->pickle, pickler.data(), pickler.size());
  record->pickle_size = pickler.size();
  record->changes.store(0, std::memory_order_relaxed);

  record->owner.Release_Initialize();

  bool success = record->UpdateFrom(info);
  DCHECK(success);
  return record;
}

bool GlobalActivityTracker::ModuleInfoRecord::UpdateFrom(
    const GlobalActivityTracker::ModuleInfo& info) {


  uint32_t old_changes = changes.load(std::memory_order_relaxed);
  uint32_t new_changes = old_changes | kModuleInformationChanging;
  if ((old_changes & kModuleInformationChanging) != 0 ||
      !changes.compare_exchange_strong(old_changes, new_changes,
                                       std::memory_order_acquire,
                                       std::memory_order_acquire)) {
    NOTREACHED() << "Multiple sources are updating module information.";
    return false;
  }

  loaded = info.is_loaded ? 1 : 0;
  address = info.address;
  load_time = Time::Now().ToInternalValue();

  bool success = changes.compare_exchange_strong(new_changes, old_changes + 1,
                                                 std::memory_order_release,
                                                 std::memory_order_relaxed);
  DCHECK(success);
  return true;
}

GlobalActivityTracker::ScopedThreadActivity::ScopedThreadActivity(
    const void* program_counter,
    const void* origin,
    Activity::Type type,
    const ActivityData& data,
    bool lock_allowed)
    : ThreadActivityTracker::ScopedActivity(GetOrCreateTracker(lock_allowed),
                                            program_counter,
                                            origin,
                                            type,
                                            data) {}

GlobalActivityTracker::ScopedThreadActivity::~ScopedThreadActivity() {
  if (tracker_ && tracker_->HasUserData(activity_id_)) {
    GlobalActivityTracker* global = GlobalActivityTracker::Get();
    AutoLock lock(global->user_data_allocator_lock_);
    tracker_->ReleaseUserData(activity_id_, &global->user_data_allocator_);
  }
}

ActivityUserData& GlobalActivityTracker::ScopedThreadActivity::user_data() {
  if (!user_data_) {
    if (tracker_) {
      GlobalActivityTracker* global = GlobalActivityTracker::Get();
      AutoLock lock(global->user_data_allocator_lock_);
      user_data_ =
          tracker_->GetUserData(activity_id_, &global->user_data_allocator_);
    } else {
      user_data_ = std::make_unique<ActivityUserData>();
    }
  }
  return *user_data_;
}

GlobalActivityTracker::ThreadSafeUserData::ThreadSafeUserData(void* memory,
                                                              size_t size,
                                                              int64_t pid)
    : ActivityUserData(memory, size, pid) {}

GlobalActivityTracker::ThreadSafeUserData::~ThreadSafeUserData() = default;

void* GlobalActivityTracker::ThreadSafeUserData::Set(StringPiece name,
                                                     ValueType type,
                                                     const void* memory,
                                                     size_t size) {
  AutoLock lock(data_lock_);
  return ActivityUserData::Set(name, type, memory, size);
}

GlobalActivityTracker::ManagedActivityTracker::ManagedActivityTracker(
    PersistentMemoryAllocator::Reference mem_reference,
    void* base,
    size_t size)
    : ThreadActivityTracker(base, size),
      mem_reference_(mem_reference),
      mem_base_(base) {}

GlobalActivityTracker::ManagedActivityTracker::~ManagedActivityTracker() {



  DCHECK(g_tracker_);
  GlobalActivityTracker::Get()->ReturnTrackerMemory(this);
}

void GlobalActivityTracker::CreateWithAllocator(
    std::unique_ptr<PersistentMemoryAllocator> allocator,
    int stack_depth,
    int64_t process_id) {

  GlobalActivityTracker* global_tracker =
      new GlobalActivityTracker(std::move(allocator), stack_depth, process_id);

  global_tracker->CreateTrackerForCurrentThread();
}

#if !defined(OS_NACL)
// static
bool GlobalActivityTracker::CreateWithFile(const FilePath& file_path,
                                           size_t size,
                                           uint64_t id,
                                           StringPiece name,
                                           int stack_depth) {
  DCHECK(!file_path.empty());
  DCHECK_GE(static_cast<uint64_t>(std::numeric_limits<int64_t>::max()), size);

  std::unique_ptr<MemoryMappedFile> mapped_file(new MemoryMappedFile());
  bool success = mapped_file->Initialize(
      File(file_path, File::FLAG_CREATE_ALWAYS | File::FLAG_READ |
                          File::FLAG_WRITE | File::FLAG_SHARE_DELETE),
      {0, size}, MemoryMappedFile::READ_WRITE_EXTEND);
  if (!success)
    return false;
  if (!FilePersistentMemoryAllocator::IsFileAcceptable(*mapped_file, false))
    return false;
  CreateWithAllocator(std::make_unique<FilePersistentMemoryAllocator>(
                          std::move(mapped_file), size, id, name, false),
                      stack_depth, 0);
  return true;
}
#endif  // !defined(OS_NACL)

bool GlobalActivityTracker::CreateWithLocalMemory(size_t size,
                                                  uint64_t id,
                                                  StringPiece name,
                                                  int stack_depth,
                                                  int64_t process_id) {
  CreateWithAllocator(
      std::make_unique<LocalPersistentMemoryAllocator>(size, id, name),
      stack_depth, process_id);
  return true;
}

bool GlobalActivityTracker::CreateWithSharedMemory(
    base::WritableSharedMemoryMapping mapping,
    uint64_t id,
    StringPiece name,
    int stack_depth) {
  if (!mapping.IsValid() ||
      !WritableSharedPersistentMemoryAllocator::IsSharedMemoryAcceptable(
          mapping)) {
    return false;
  }
  CreateWithAllocator(std::make_unique<WritableSharedPersistentMemoryAllocator>(
                          std::move(mapping), id, name),
                      stack_depth, 0);
  return true;
}

void GlobalActivityTracker::SetForTesting(
    std::unique_ptr<GlobalActivityTracker> tracker) {
  CHECK(!subtle::NoBarrier_Load(&g_tracker_));
  subtle::Release_Store(&g_tracker_,
                        reinterpret_cast<uintptr_t>(tracker.release()));
}

std::unique_ptr<GlobalActivityTracker>
GlobalActivityTracker::ReleaseForTesting() {
  GlobalActivityTracker* tracker = Get();
  if (!tracker)
    return nullptr;


  tracker->ReleaseTrackerForCurrentThreadForTesting();
  DCHECK_EQ(0, tracker->thread_tracker_count_.load(std::memory_order_relaxed));

  subtle::Release_Store(&g_tracker_, 0);
  return WrapUnique(tracker);
}

ThreadActivityTracker* GlobalActivityTracker::CreateTrackerForCurrentThread() {

  if (base::ThreadLocalStorage::HasBeenDestroyed())
    return nullptr;

  DCHECK(!this_thread_tracker_.Get());

  PersistentMemoryAllocator::Reference mem_reference;

  {
    base::AutoLock autolock(thread_tracker_allocator_lock_);
    mem_reference = thread_tracker_allocator_.GetObjectReference();
  }

  if (!mem_reference) {



    NOTREACHED();

    return nullptr;
  }



  DCHECK(mem_reference);
  void* mem_base;
  mem_base =
      allocator_->GetAsObject<ThreadActivityTracker::Header>(mem_reference);

  DCHECK(mem_base);
  DCHECK_LE(stack_memory_size_, allocator_->GetAllocSize(mem_reference));


  auto tracker = std::make_unique<ManagedActivityTracker>(
      mem_reference, mem_base, stack_memory_size_);
  DCHECK(tracker->IsValid());
  auto* tracker_raw = tracker.get();
  this_thread_tracker_.Set(std::move(tracker));
  thread_tracker_count_.fetch_add(1, std::memory_order_relaxed);
  return tracker_raw;
}

void GlobalActivityTracker::ReleaseTrackerForCurrentThreadForTesting() {
  if (this_thread_tracker_.Get())
    this_thread_tracker_.Set(nullptr);
}

void GlobalActivityTracker::SetBackgroundTaskRunner(
    const scoped_refptr<SequencedTaskRunner>& runner) {
  AutoLock lock(global_tracker_lock_);
  background_task_runner_ = std::move(runner);
}

void GlobalActivityTracker::SetProcessExitCallback(
    ProcessExitCallback callback) {
  AutoLock lock(global_tracker_lock_);
  process_exit_callback_ = callback;
}

void GlobalActivityTracker::RecordProcessLaunch(
    ProcessId process_id,
    const FilePath::StringType& cmd) {
  const int64_t pid = process_id;
  DCHECK_NE(GetProcessId(), pid);
  DCHECK_NE(0, pid);

  base::AutoLock lock(global_tracker_lock_);
  if (base::Contains(known_processes_, pid)) {

    NOTREACHED() << "Process #" << process_id
                 << " was previously recorded as \"launched\""
                 << " with no corresponding exit.\n"
                 << known_processes_[pid];
    known_processes_.erase(pid);
  }

#if defined(OS_WIN)
  known_processes_.insert(std::make_pair(pid, WideToUTF8(cmd)));
#else
  known_processes_.insert(std::make_pair(pid, cmd));
#endif
}

void GlobalActivityTracker::RecordProcessLaunch(
    ProcessId process_id,
    const FilePath::StringType& exe,
    const FilePath::StringType& args) {
  if (exe.find(FILE_PATH_LITERAL(" "))) {
    RecordProcessLaunch(process_id,
                        FilePath::StringType(FILE_PATH_LITERAL("\"")) + exe +
                            FILE_PATH_LITERAL("\" ") + args);
  } else {
    RecordProcessLaunch(process_id, exe + FILE_PATH_LITERAL(' ') + args);
  }
}

void GlobalActivityTracker::RecordProcessExit(ProcessId process_id,
                                              int exit_code) {
  const int64_t pid = process_id;
  DCHECK_NE(GetProcessId(), pid);
  DCHECK_NE(0, pid);

  scoped_refptr<SequencedTaskRunner> task_runner;
  std::string command_line;
  {
    base::AutoLock lock(global_tracker_lock_);
    task_runner = background_task_runner_;
    auto found = known_processes_.find(pid);
    if (found != known_processes_.end()) {
      command_line = std::move(found->second);
      known_processes_.erase(found);
    } else {
      DLOG(ERROR) << "Recording exit of unknown process #" << process_id;
    }
  }


  int64_t now_stamp = Time::Now().ToInternalValue();


  if (task_runner && !task_runner->RunsTasksInCurrentSequence()) {
    task_runner->PostTask(
        FROM_HERE,
        BindOnce(&GlobalActivityTracker::CleanupAfterProcess, Unretained(this),
                 pid, now_stamp, exit_code, std::move(command_line)));
    return;
  }

  CleanupAfterProcess(pid, now_stamp, exit_code, std::move(command_line));
}

void GlobalActivityTracker::SetProcessPhase(ProcessPhase phase) {
  process_data().SetInt(kProcessPhaseDataKey, phase);
}

void GlobalActivityTracker::CleanupAfterProcess(int64_t process_id,
                                                int64_t exit_stamp,
                                                int exit_code,
                                                std::string&& command_line) {





  PersistentMemoryAllocator::Iterator iter(allocator_.get());
  PersistentMemoryAllocator::Reference ref;

  ProcessExitCallback process_exit_callback;
  {
    AutoLock lock(global_tracker_lock_);
    process_exit_callback = process_exit_callback_;
  }
  if (process_exit_callback) {


    ActivityUserData::Snapshot process_data_snapshot;
    while ((ref = iter.GetNextOfType(kTypeIdProcessDataRecord)) != 0) {
      const void* memory = allocator_->GetAsArray<char>(
          ref, kTypeIdProcessDataRecord, PersistentMemoryAllocator::kSizeAny);
      if (!memory)
        continue;
      int64_t found_id;
      int64_t create_stamp;
      if (ActivityUserData::GetOwningProcessId(memory, &found_id,
                                               &create_stamp)) {
        if (found_id == process_id && create_stamp < exit_stamp) {
          const ActivityUserData process_data(const_cast<void*>(memory),
                                              allocator_->GetAllocSize(ref));
          process_data.CreateSnapshot(&process_data_snapshot);
          break;  // No need to look for any others.
        }
      }
    }
    iter.Reset();  // So it starts anew when used below.


    ProcessPhase exit_phase = PROCESS_PHASE_UNKNOWN;
    auto phase = process_data_snapshot.find(kProcessPhaseDataKey);
    if (phase != process_data_snapshot.end())
      exit_phase = static_cast<ProcessPhase>(phase->second.GetInt());

    process_exit_callback.Run(process_id, exit_stamp, exit_code, exit_phase,
                              std::move(command_line),
                              std::move(process_data_snapshot));
  }

  uint32_t type;
  while ((ref = iter.GetNext(&type)) != 0) {
    switch (type) {
      case kTypeIdActivityTracker:
      case kTypeIdUserDataRecord:
      case kTypeIdProcessDataRecord:
      case ModuleInfoRecord::kPersistentTypeId: {
        const void* memory = allocator_->GetAsArray<char>(
            ref, type, PersistentMemoryAllocator::kSizeAny);
        if (!memory)
          continue;
        int64_t found_id;
        int64_t create_stamp;



        if (OwningProcess::GetOwningProcessId(memory, &found_id,
                                              &create_stamp)) {





          if (found_id == process_id && create_stamp < exit_stamp)
            allocator_->ChangeType(ref, ~type, type, /*clear=*/true);
        }
      } break;
    }
  }
}

void GlobalActivityTracker::RecordLogMessage(StringPiece message) {


  PersistentMemoryAllocator::Reference ref =
      allocator_->Allocate(message.size() + 1, kTypeIdGlobalLogMessage);
  char* memory = allocator_->GetAsArray<char>(ref, kTypeIdGlobalLogMessage,
                                              message.size() + 1);
  if (memory) {
    memcpy(memory, message.data(), message.size());
    allocator_->MakeIterable(ref);
  }
}

void GlobalActivityTracker::RecordModuleInfo(const ModuleInfo& info) {
  AutoLock lock(modules_lock_);
  auto found = modules_.find(info.file);
  if (found != modules_.end()) {
    ModuleInfoRecord* record = found->second;
    DCHECK(record);




    record->UpdateFrom(info);
    return;
  }

  ModuleInfoRecord* record =
      ModuleInfoRecord::CreateFrom(info, allocator_.get());
  if (!record)
    return;
  allocator_->MakeIterable(record);
  modules_.emplace(info.file, record);
}

void GlobalActivityTracker::RecordException(const void* pc,
                                            const void* origin,
                                            uint32_t code) {
  RecordExceptionImpl(pc, origin, code);
}

void GlobalActivityTracker::MarkDeleted() {
  allocator_->SetMemoryState(PersistentMemoryAllocator::MEMORY_DELETED);
}

GlobalActivityTracker::GlobalActivityTracker(
    std::unique_ptr<PersistentMemoryAllocator> allocator,
    int stack_depth,
    int64_t process_id)
    : allocator_(std::move(allocator)),
      stack_memory_size_(ThreadActivityTracker::SizeForStackDepth(stack_depth)),
      process_id_(process_id == 0 ? GetCurrentProcId() : process_id),
      thread_tracker_count_(0),
      thread_tracker_allocator_(allocator_.get(),
                                kTypeIdActivityTracker,
                                kTypeIdActivityTrackerFree,
                                stack_memory_size_,
                                kCachedThreadMemories,
                                /*make_iterable=*/true),
      user_data_allocator_(allocator_.get(),
                           kTypeIdUserDataRecord,
                           kTypeIdUserDataRecordFree,
                           kUserDataSize,
                           kCachedUserDataMemories,
                           /*make_iterable=*/true),
      process_data_(allocator_->GetAsArray<char>(
                        AllocateFrom(allocator_.get(),
                                     kTypeIdProcessDataRecordFree,
                                     kProcessDataSize,
                                     kTypeIdProcessDataRecord),
                        kTypeIdProcessDataRecord,
                        kProcessDataSize),
                    kProcessDataSize,
                    process_id_) {
  DCHECK_NE(0, process_id_);

  DCHECK(!g_tracker_);
  subtle::Release_Store(&g_tracker_, reinterpret_cast<uintptr_t>(this));

  allocator_->MakeIterable(allocator_->GetAsReference(
      process_data_.GetBaseAddress(), kTypeIdProcessDataRecord));

  SetProcessPhase(PROCESS_LAUNCHED);
}

GlobalActivityTracker::~GlobalActivityTracker() {
  DCHECK(Get() == nullptr || Get() == this);
  DCHECK_EQ(0, thread_tracker_count_.load(std::memory_order_relaxed));
  subtle::Release_Store(&g_tracker_, 0);
}

void GlobalActivityTracker::ReturnTrackerMemory(
    ManagedActivityTracker* tracker) {
  PersistentMemoryAllocator::Reference mem_reference = tracker->mem_reference_;
  void* mem_base = tracker->mem_base_;
  DCHECK(mem_reference);
  DCHECK(mem_base);

  DCHECK_LE(1, thread_tracker_count_.load(std::memory_order_relaxed));
  thread_tracker_count_.fetch_sub(1, std::memory_order_relaxed);

  base::AutoLock autolock(thread_tracker_allocator_lock_);
  thread_tracker_allocator_.ReleaseObjectReference(mem_reference);
}

void GlobalActivityTracker::RecordExceptionImpl(const void* pc,
                                                const void* origin,
                                                uint32_t code) {






  ThreadActivityTracker* tracker = GetTrackerForCurrentThread();
  if (!tracker)
    return;

  tracker->RecordExceptionActivity(pc, origin, Activity::ACT_EXCEPTION,
                                   ActivityData::ForException(code));
}

ScopedActivity::ScopedActivity(const void* program_counter,
                               uint8_t action,
                               uint32_t id,
                               int32_t info)
    : GlobalActivityTracker::ScopedThreadActivity(
          program_counter,
          nullptr,
          static_cast<Activity::Type>(Activity::ACT_GENERIC | action),
          ActivityData::ForGeneric(id, info),
          /*lock_allowed=*/true),
      id_(id) {

  DCHECK_EQ(0, action & Activity::ACT_CATEGORY_MASK);
}

void ScopedActivity::ChangeAction(uint8_t action) {
  DCHECK_EQ(0, action & Activity::ACT_CATEGORY_MASK);
  ChangeTypeAndData(static_cast<Activity::Type>(Activity::ACT_GENERIC | action),
                    kNullActivityData);
}

void ScopedActivity::ChangeInfo(int32_t info) {
  ChangeTypeAndData(Activity::ACT_NULL, ActivityData::ForGeneric(id_, info));
}

void ScopedActivity::ChangeActionAndInfo(uint8_t action, int32_t info) {
  DCHECK_EQ(0, action & Activity::ACT_CATEGORY_MASK);
  ChangeTypeAndData(static_cast<Activity::Type>(Activity::ACT_GENERIC | action),
                    ActivityData::ForGeneric(id_, info));
}

ScopedTaskRunActivity::ScopedTaskRunActivity(
    const void* program_counter,
    const base::PendingTask& task)
    : GlobalActivityTracker::ScopedThreadActivity(
          program_counter,
          task.posted_from.program_counter(),
          Activity::ACT_TASK_RUN,
          ActivityData::ForTask(task.sequence_num),
          /*lock_allowed=*/true) {}

ScopedLockAcquireActivity::ScopedLockAcquireActivity(
    const void* program_counter,
    const base::internal::LockImpl* lock)
    : GlobalActivityTracker::ScopedThreadActivity(
          program_counter,
          nullptr,
          Activity::ACT_LOCK_ACQUIRE,
          ActivityData::ForLock(lock),
          /*lock_allowed=*/false) {}

ScopedEventWaitActivity::ScopedEventWaitActivity(
    const void* program_counter,
    const base::WaitableEvent* event)
    : GlobalActivityTracker::ScopedThreadActivity(
          program_counter,
          nullptr,
          Activity::ACT_EVENT_WAIT,
          ActivityData::ForEvent(event),
          /*lock_allowed=*/true) {}

ScopedThreadJoinActivity::ScopedThreadJoinActivity(
    const void* program_counter,
    const base::PlatformThreadHandle* thread)
    : GlobalActivityTracker::ScopedThreadActivity(
          program_counter,
          nullptr,
          Activity::ACT_THREAD_JOIN,
          ActivityData::ForThread(*thread),
          /*lock_allowed=*/true) {}

#if !defined(OS_NACL) && !defined(OS_IOS)
ScopedProcessWaitActivity::ScopedProcessWaitActivity(
    const void* program_counter,
    const base::Process* process)
    : GlobalActivityTracker::ScopedThreadActivity(
          program_counter,
          nullptr,
          Activity::ACT_PROCESS_WAIT,
          ActivityData::ForProcess(process->Pid()),
          /*lock_allowed=*/true) {}
#endif

}  // namespace debug
}  // namespace base
