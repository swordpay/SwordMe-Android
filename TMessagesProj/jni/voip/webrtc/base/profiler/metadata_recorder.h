// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PROFILER_METADATA_RECORDER_H_
#define BASE_PROFILER_METADATA_RECORDER_H_

#include <array>
#include <atomic>
#include <utility>

#include "base/optional.h"
#include "base/profiler/profile_builder.h"
#include "base/synchronization/lock.h"
#include "base/thread_annotations.h"

namespace base {

// to be associated with samples taken by the sampling profiler. Whatever
// metadata is present in this map when a sample is recorded is then associated
// with the sample.
//
// Methods on this class are safe to call unsynchronized from arbitrary threads.
//
// This class was designed to read metadata from a single sampling thread and
// write metadata from many Chrome threads within the same process. These other
// threads might be suspended by the sampling thread at any time in order to
// collect a sample.
//
// This class has a few notable constraints:
//
// A) If a lock that's required to read the metadata might be held while writing
//    the metadata, that lock must be acquirable *before* the thread is
//    suspended. Otherwise, the sampling thread might suspend the target thread
//    while it is holding the required lock, causing deadlock.
//
//      Ramifications:
//
//      - When retrieving items, lock acquisition (through
//        CreateMetadataProvider()) and actual item retrieval (through
//        MetadataProvider::GetItems()) are separate.
//
// B) We can't allocate data on the heap while reading the metadata items. This
//    is because, on many operating systems, there's a process-wide heap lock
//    that is held while allocating on the heap. If a thread is suspended while
//    holding this lock and the sampling thread then tries to allocate on the
//    heap to read the metadata, it will deadlock trying to acquire the heap
//    lock.
//
//      Ramifications:
//
//      - We hold and retrieve the metadata using a fixed-size array, which
//        allows readers to preallocate the data structure that we pass back
//        the metadata in.
//
// C) We shouldn't guard writes with a lock that also guards reads. It can take
//    ~30us from the time that the sampling thread requests that a thread be
//    suspended and the time that it actually happens. If all metadata writes
//    block their thread during that time, we're very likely to block all Chrome
//    threads for an additional 30us per sample.
//
//      Ramifications:
//
//      - We use two locks to guard the metadata: a read lock and a write
//        lock. Only the write lock is required to write into the metadata, and
//        only the read lock is required to read the metadata.
//
//      - Because we can't guard reads and writes with the same lock, we have to
//        face the possibility of writes occurring during a read. This is
//        especially problematic because there's no way to read both the key and
//        value for an item atomically without using mutexes, which violates
//        constraint A). If the sampling thread were to see the following
//        interleaving of reads and writes:
//
//          * Reader thread reads key for slot 0
//          * Writer thread removes item at slot 0
//          * Writer thread creates new item with different key in slot 0
//          * Reader thread reads value for slot 0
//
//        then the reader would see an invalid value for the given key. Because
//        of this possibility, we keep slots reserved for a specific key even
//        after that item has been removed. We reclaim these slots on a
//        best-effort basis during writes when the metadata recorder has become
//        sufficiently full and we can acquire the read lock.
//
//      - We use state stored in atomic data types to ensure that readers and
//        writers are synchronized about where data should be written to and
//        read from. We must use atomic data types to guarantee that there's no
//        instruction during a write after which the recorder is in an
//        inconsistent state that might yield garbage data for a reader.
//
// Here are a few of the many states the recorder can be in:
//
// - No thread is using the recorder.
//
// - A single writer is writing into the recorder without a simultaneous
//   read. The write will succeed.
//
// - A reader is reading from the recorder without a simultaneous write. The
//   read will succeed.
//
// - Multiple writers attempt to write into the recorder simultaneously. All
//   writers but one will block because only one can hold the write lock.
//
// - A writer is writing into the recorder, which hasn't reached the threshold
//   at which it will try to reclaim inactive slots. The writer won't try to
//   acquire the read lock to reclaim inactive slots. The reader will therefore
//   be able to immediately acquire the read lock, suspend the target thread,
//   and read the metadata.
//
// - A writer is writing into the recorder, the recorder has reached the
//   threshold at which it needs to reclaim inactive slots, and the writer
//   thread is now in the middle of reclaiming those slots when a reader
//   arrives. The reader will try to acquire the read lock before suspending the
//   thread but will block until the writer thread finishes reclamation and
//   releases the read lock. The reader will then be able to acquire the read
//   lock and suspend the target thread.
//
// - A reader is reading the recorder when a writer attempts to write. The write
//   will be successful. However, if the writer deems it necessary to reclaim
//   inactive slots, it will skip doing so because it won't be able to acquire
//   the read lock.
class BASE_EXPORT MetadataRecorder {
 public:
  MetadataRecorder();
  virtual ~MetadataRecorder();
  MetadataRecorder(const MetadataRecorder&) = delete;
  MetadataRecorder& operator=(const MetadataRecorder&) = delete;



  void Set(uint64_t name_hash, Optional<int64_t> key, int64_t value);


  void Remove(uint64_t name_hash, Optional<int64_t> key);






















  std::unique_ptr<ProfileBuilder::MetadataProvider> CreateMetadataProvider();

 private:





  class SCOPED_LOCKABLE ScopedGetItems
      : public ProfileBuilder::MetadataProvider {
   public:


    ScopedGetItems(MetadataRecorder* metadata_recorder)
        EXCLUSIVE_LOCK_FUNCTION(metadata_recorder->read_lock_);
    ~ScopedGetItems() override UNLOCK_FUNCTION(metadata_recorder_->read_lock_);
    ScopedGetItems(const ScopedGetItems&) = delete;
    ScopedGetItems& operator=(const ScopedGetItems&) = delete;







    size_t GetItems(ProfileBuilder::MetadataItemArray* const items) override
        EXCLUSIVE_LOCKS_REQUIRED(metadata_recorder_->read_lock_);

   private:
    const MetadataRecorder* const metadata_recorder_;
    base::ReleasableAutoLock auto_lock_;
  };

  struct ItemInternal {
    ItemInternal();
    ~ItemInternal();






    std::atomic<bool> is_active{false};






    uint64_t name_hash;
    Optional<int64_t> key;




    std::atomic<int64_t> value;
  };




  size_t TryReclaimInactiveSlots(size_t item_slots_used)
      EXCLUSIVE_LOCKS_REQUIRED(write_lock_) LOCKS_EXCLUDED(read_lock_);




  size_t ReclaimInactiveSlots(size_t item_slots_used)
      EXCLUSIVE_LOCKS_REQUIRED(write_lock_);



  size_t GetItems(ProfileBuilder::MetadataItemArray* const items) const;








  std::array<ItemInternal, ProfileBuilder::MAX_METADATA_COUNT> items_;






  std::atomic<size_t> item_slots_used_{0};

  size_t inactive_item_count_ GUARDED_BY(write_lock_) = 0;


  base::Lock write_lock_;







  base::Lock read_lock_;
};

}  // namespace base

#endif  // BASE_PROFILER_METADATA_RECORDER_H_
