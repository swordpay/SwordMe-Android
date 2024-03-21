//
// Copyright 2020 The Abseil Authors.
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

#ifndef ABSL_FLAGS_INTERNAL_SEQUENCE_LOCK_H_
#define ABSL_FLAGS_INTERNAL_SEQUENCE_LOCK_H_

#include <stddef.h>
#include <stdint.h>

#include <atomic>
#include <cassert>
#include <cstring>

#include "absl/base/optimization.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace flags_internal {

inline constexpr size_t AlignUp(size_t x, size_t align) {
  return align * ((x + align - 1) / align);
}

// before and after each write, and readers access the counter before and after
// accessing the protected data. If the counter is verified to not change during
// the access, and the sequence counter value was even, then the reader knows
// that the read was race-free and valid. Otherwise, the reader must fall back
// to a Mutex-based code path.
//
// This particular SequenceLock starts in an "uninitialized" state in which
// TryRead() returns false. It must be enabled by calling MarkInitialized().
// This serves as a marker that the associated flag value has not yet been
// initialized and a slow path needs to be taken.
//
// The memory reads and writes protected by this lock must use the provided
// `TryRead()` and `Write()` functions. These functions behave similarly to
// `memcpy()`, with one oddity: the protected data must be an array of
// `std::atomic<uint64>`. This is to comply with the C++ standard, which
// considers data races on non-atomic objects to be undefined behavior. See "Can
// Seqlocks Get Along With Programming Language Memory Models?"[1] by Hans J.
// Boehm for more details.
//
// [1] https://www.hpl.hp.com/techreports/2012/HPL-2012-68.pdf
class SequenceLock {
 public:
  constexpr SequenceLock() : lock_(kUninitialized) {}

  void MarkInitialized() {
    assert(lock_.load(std::memory_order_relaxed) == kUninitialized);
    lock_.store(0, std::memory_order_release);
  }












  bool TryRead(void* dst, const std::atomic<uint64_t>* src, size_t size) const {


    int64_t seq_before = lock_.load(std::memory_order_acquire);
    if (ABSL_PREDICT_FALSE(seq_before & 1) == 1) return false;
    RelaxedCopyFromAtomic(dst, src, size);


    std::atomic_thread_fence(std::memory_order_acquire);
    int64_t seq_after = lock_.load(std::memory_order_relaxed);
    return ABSL_PREDICT_TRUE(seq_before == seq_after);
  }






  void Write(std::atomic<uint64_t>* dst, const void* src, size_t size) {




    int64_t orig_seq = lock_.load(std::memory_order_relaxed);
    assert((orig_seq & 1) == 0);  // Must be initially unlocked.
    lock_.store(orig_seq + 1, std::memory_order_relaxed);





    std::atomic_thread_fence(std::memory_order_release);
    RelaxedCopyToAtomic(dst, src, size);


    lock_.store(orig_seq + 2, std::memory_order_release);
  }





  int64_t ModificationCount() const {
    int64_t val = lock_.load(std::memory_order_relaxed);
    assert(val != kUninitialized && (val & 1) == 0);
    return val / 2;
  }



  void IncrementModificationCount() {
    int64_t val = lock_.load(std::memory_order_relaxed);
    assert(val != kUninitialized);
    lock_.store(val + 2, std::memory_order_relaxed);
  }

 private:


  static void RelaxedCopyFromAtomic(void* dst, const std::atomic<uint64_t>* src,
                                    size_t size) {
    char* dst_byte = static_cast<char*>(dst);
    while (size >= sizeof(uint64_t)) {
      uint64_t word = src->load(std::memory_order_relaxed);
      std::memcpy(dst_byte, &word, sizeof(word));
      dst_byte += sizeof(word);
      src++;
      size -= sizeof(word);
    }
    if (size > 0) {
      uint64_t word = src->load(std::memory_order_relaxed);
      std::memcpy(dst_byte, &word, size);
    }
  }


  static void RelaxedCopyToAtomic(std::atomic<uint64_t>* dst, const void* src,
                                  size_t size) {
    const char* src_byte = static_cast<const char*>(src);
    while (size >= sizeof(uint64_t)) {
      uint64_t word;
      std::memcpy(&word, src_byte, sizeof(word));
      dst->store(word, std::memory_order_relaxed);
      src_byte += sizeof(word);
      dst++;
      size -= sizeof(word);
    }
    if (size > 0) {
      uint64_t word = 0;
      std::memcpy(&word, src_byte, size);
      dst->store(word, std::memory_order_relaxed);
    }
  }

  static constexpr int64_t kUninitialized = -1;
  std::atomic<int64_t> lock_;
};

}  // namespace flags_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_FLAGS_INTERNAL_SEQUENCE_LOCK_H_
