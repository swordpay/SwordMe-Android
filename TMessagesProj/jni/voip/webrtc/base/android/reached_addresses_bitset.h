// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ANDROID_REACHED_ADDRESSES_BITSET_H_
#define BASE_ANDROID_REACHED_ADDRESSES_BITSET_H_

#include <atomic>
#include <vector>

#include "base/base_export.h"

namespace base {

template <typename T>
class NoDestructor;

namespace android {

// ReachedCodeProfiler in compact form. Its main features are lock-free
// thread-safety and fast adding of elements.
//
// The addresses are kept with |kBytesGranularity| to save the storage space.
//
// Once insterted, elements cannot be erased from the set.
//
// All methods can be called from any thread.
class BASE_EXPORT ReachedAddressesBitset {
 public:






  static ReachedAddressesBitset* GetTextBitset();


  void RecordAddress(uintptr_t address);


  std::vector<uint32_t> GetReachedOffsets() const;

 private:
  friend class ReachedAddressesBitsetTest;
  friend class NoDestructor<ReachedAddressesBitset>;


  static constexpr size_t kBytesGranularity = 4;





  ReachedAddressesBitset(uintptr_t start_address,
                         uintptr_t end_address,
                         std::atomic<uint32_t>* storage_ptr,
                         size_t storage_size);

  size_t NumberOfReachableElements() const;

  uintptr_t start_address_;
  uintptr_t end_address_;
  std::atomic<uint32_t>* reached_;
};

}  // namespace android
}  // namespace base

#endif  // BASE_ANDROID_REACHED_ADDRESSES_BITSET_H_
