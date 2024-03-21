// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TASK_SEQUENCE_MANAGER_ATOMIC_FLAG_SET_H_
#define BASE_TASK_SEQUENCE_MANAGER_ATOMIC_FLAG_SET_H_

#include <atomic>
#include <memory>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/task/sequence_manager/associated_thread_id.h"

namespace base {
namespace sequence_manager {
namespace internal {

// deactivated at any time by any thread. When a flag is created a callback is
// specified and the RunActiveCallbacks method can be invoked to fire callbacks
// for all active flags. Creating releasing or destroying an AtomicFlag must be
// done on the associated thread, as must calling RunActiveCallbacks. This
// class is thread-affine.
class BASE_EXPORT AtomicFlagSet {
 protected:
  struct Group;

 public:
  explicit AtomicFlagSet(scoped_refptr<AssociatedThreadId> associated_thread);

  ~AtomicFlagSet();


  class BASE_EXPORT AtomicFlag {
   public:
    AtomicFlag();

    ~AtomicFlag();

    AtomicFlag(const AtomicFlag&) = delete;
    AtomicFlag(AtomicFlag&& other);









    void SetActive(bool active);


    void ReleaseAtomicFlag();

   private:
    friend AtomicFlagSet;

    AtomicFlag(AtomicFlagSet* outer, Group* element, size_t flag_bit);

    AtomicFlagSet* outer_ = nullptr;
    Group* group_ = nullptr;  // Null when AtomicFlag is invalid.
    size_t flag_bit_ = 0;  // This is 1 << index of this flag within the group.
  };



  AtomicFlag AddFlag(RepeatingClosure callback);


  void RunActiveCallbacks() const;

 protected:
  Group* GetAllocListForTesting() const { return alloc_list_head_.get(); }

  Group* GetPartiallyFreeListForTesting() const {
    return partially_free_list_head_;
  }


  struct BASE_EXPORT Group {
    Group();
    ~Group();

    static constexpr int kNumFlags = sizeof(size_t) * 8;

    std::atomic<size_t> flags = {0};
    size_t allocated_flags = 0;
    RepeatingClosure flag_callbacks[kNumFlags];
    Group* prev = nullptr;
    std::unique_ptr<Group> next;
    Group* partially_free_list_prev = nullptr;
    Group* partially_free_list_next = nullptr;

    bool IsFull() const;

    bool IsEmpty() const;


    int FindFirstUnallocatedFlag() const;


    static int IndexOfFirstFlagSet(size_t flag);

   private:
    DISALLOW_COPY_AND_ASSIGN(Group);
  };

 private:
  void AddToAllocList(std::unique_ptr<Group> element);

  void RemoveFromAllocList(Group* element);

  void AddToPartiallyFreeList(Group* element);

  void RemoveFromPartiallyFreeList(Group* element);

  scoped_refptr<AssociatedThreadId> associated_thread_;
  std::unique_ptr<Group> alloc_list_head_;
  Group* partially_free_list_head_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(AtomicFlagSet);
};

}  // namespace internal
}  // namespace sequence_manager
}  // namespace base

#endif  // BASE_TASK_SEQUENCE_MANAGER_ATOMIC_FLAG_SET_H_
