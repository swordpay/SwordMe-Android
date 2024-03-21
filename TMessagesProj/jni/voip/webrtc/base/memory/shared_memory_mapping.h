// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MEMORY_SHARED_MEMORY_MAPPING_H_
#define BASE_MEMORY_SHARED_MEMORY_MAPPING_H_

#include <cstddef>
#include <type_traits>

#include "base/containers/buffer_iterator.h"
#include "base/containers/span.h"
#include "base/macros.h"
#include "base/unguessable_token.h"

namespace base {

namespace subtle {
class PlatformSharedMemoryRegion;
}  // namespace subtle

// shared memory region. Created shared memory mappings remain valid even if the
// creator region is transferred or destroyed.
//
// Each mapping has an UnguessableToken that identifies the shared memory region
// it was created from. This is used for memory metrics, to avoid overcounting
// shared memory.
class BASE_EXPORT SharedMemoryMapping {
 public:

  SharedMemoryMapping();

  SharedMemoryMapping(SharedMemoryMapping&& mapping) noexcept;
  SharedMemoryMapping& operator=(SharedMemoryMapping&& mapping) noexcept;

  virtual ~SharedMemoryMapping();


  bool IsValid() const { return memory_ != nullptr; }



  size_t size() const {
    DCHECK(IsValid());
    return size_;
  }



  size_t mapped_size() const {
    DCHECK(IsValid());
    return mapped_size_;
  }

  const UnguessableToken& guid() const {
    DCHECK(IsValid());
    return guid_;
  }

 protected:
  SharedMemoryMapping(void* address,
                      size_t size,
                      size_t mapped_size,
                      const UnguessableToken& guid);
  void* raw_memory_ptr() const { return memory_; }

 private:
  friend class SharedMemoryTracker;

  void Unmap();

  void* memory_ = nullptr;
  size_t size_ = 0;
  size_t mapped_size_ = 0;
  UnguessableToken guid_;

  DISALLOW_COPY_AND_ASSIGN(SharedMemoryMapping);
};

// current process' address space. This is created by ReadOnlySharedMemoryRegion
// instances.
class BASE_EXPORT ReadOnlySharedMemoryMapping : public SharedMemoryMapping {
 public:

  ReadOnlySharedMemoryMapping();

  ReadOnlySharedMemoryMapping(ReadOnlySharedMemoryMapping&&) noexcept;
  ReadOnlySharedMemoryMapping& operator=(
      ReadOnlySharedMemoryMapping&&) noexcept;


  const void* memory() const { return raw_memory_ptr(); }


  template <typename T>
  const T* GetMemoryAs() const {
    static_assert(std::is_trivially_copyable<T>::value,
                  "Copying non-trivially-copyable object across memory spaces "
                  "is dangerous");
    if (!IsValid())
      return nullptr;
    if (sizeof(T) > size())
      return nullptr;
    return static_cast<const T*>(raw_memory_ptr());
  }






  template <typename T>
  span<const T> GetMemoryAsSpan() const {
    static_assert(std::is_trivially_copyable<T>::value,
                  "Copying non-trivially-copyable object across memory spaces "
                  "is dangerous");
    if (!IsValid())
      return span<const T>();
    size_t count = size() / sizeof(T);
    return GetMemoryAsSpan<T>(count);
  }



  template <typename T>
  span<const T> GetMemoryAsSpan(size_t count) const {
    static_assert(std::is_trivially_copyable<T>::value,
                  "Copying non-trivially-copyable object across memory spaces "
                  "is dangerous");
    if (!IsValid())
      return span<const T>();
    if (size() / sizeof(T) < count)
      return span<const T>();
    return span<const T>(static_cast<const T*>(raw_memory_ptr()), count);
  }

  template <typename T>
  BufferIterator<const T> GetMemoryAsBufferIterator() const {
    return BufferIterator<const T>(GetMemoryAsSpan<T>());
  }

 private:
  friend class ReadOnlySharedMemoryRegion;
  ReadOnlySharedMemoryMapping(void* address,
                              size_t size,
                              size_t mapped_size,
                              const UnguessableToken& guid);

  DISALLOW_COPY_AND_ASSIGN(ReadOnlySharedMemoryMapping);
};

// current process' address space. This is created by *SharedMemoryRegion
// instances.
class BASE_EXPORT WritableSharedMemoryMapping : public SharedMemoryMapping {
 public:

  WritableSharedMemoryMapping();

  WritableSharedMemoryMapping(WritableSharedMemoryMapping&&) noexcept;
  WritableSharedMemoryMapping& operator=(
      WritableSharedMemoryMapping&&) noexcept;


  void* memory() const { return raw_memory_ptr(); }


  template <typename T>
  T* GetMemoryAs() const {
    static_assert(std::is_trivially_copyable<T>::value,
                  "Copying non-trivially-copyable object across memory spaces "
                  "is dangerous");
    if (!IsValid())
      return nullptr;
    if (sizeof(T) > size())
      return nullptr;
    return static_cast<T*>(raw_memory_ptr());
  }





  template <typename T>
  span<T> GetMemoryAsSpan() const {
    static_assert(std::is_trivially_copyable<T>::value,
                  "Copying non-trivially-copyable object across memory spaces "
                  "is dangerous");
    if (!IsValid())
      return span<T>();
    size_t count = size() / sizeof(T);
    return GetMemoryAsSpan<T>(count);
  }



  template <typename T>
  span<T> GetMemoryAsSpan(size_t count) const {
    static_assert(std::is_trivially_copyable<T>::value,
                  "Copying non-trivially-copyable object across memory spaces "
                  "is dangerous");
    if (!IsValid())
      return span<T>();
    if (size() / sizeof(T) < count)
      return span<T>();
    return span<T>(static_cast<T*>(raw_memory_ptr()), count);
  }

  template <typename T>
  BufferIterator<T> GetMemoryAsBufferIterator() {
    return BufferIterator<T>(GetMemoryAsSpan<T>());
  }

 private:
  friend WritableSharedMemoryMapping MapAtForTesting(
      subtle::PlatformSharedMemoryRegion* region,
      off_t offset,
      size_t size);
  friend class ReadOnlySharedMemoryRegion;
  friend class WritableSharedMemoryRegion;
  friend class UnsafeSharedMemoryRegion;
  WritableSharedMemoryMapping(void* address,
                              size_t size,
                              size_t mapped_size,
                              const UnguessableToken& guid);

  DISALLOW_COPY_AND_ASSIGN(WritableSharedMemoryMapping);
};

}  // namespace base

#endif  // BASE_MEMORY_SHARED_MEMORY_MAPPING_H_
