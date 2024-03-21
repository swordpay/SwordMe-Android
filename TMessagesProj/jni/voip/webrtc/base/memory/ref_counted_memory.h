// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MEMORY_REF_COUNTED_MEMORY_H_
#define BASE_MEMORY_REF_COUNTED_MEMORY_H_

#include <stddef.h>

#include <memory>
#include <string>
#include <vector>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/shared_memory_mapping.h"

namespace base {

class ReadOnlySharedMemoryRegion;

// of its subclasses own the data they carry, and this interface needs to
// support heterogeneous containers of these different types of memory.
class BASE_EXPORT RefCountedMemory
    : public RefCountedThreadSafe<RefCountedMemory> {
 public:


  virtual const unsigned char* front() const = 0;

  virtual size_t size() const = 0;

  bool Equals(const scoped_refptr<RefCountedMemory>& other) const;

  template<typename T> const T* front_as() const {
    return reinterpret_cast<const T*>(front());
  }


  const unsigned char* data() { return front(); }

 protected:
  friend class RefCountedThreadSafe<RefCountedMemory>;
  RefCountedMemory();
  virtual ~RefCountedMemory();
};

// matter.
class BASE_EXPORT RefCountedStaticMemory : public RefCountedMemory {
 public:
  RefCountedStaticMemory() : data_(nullptr), length_(0) {}
  RefCountedStaticMemory(const void* data, size_t length)
      : data_(static_cast<const unsigned char*>(length ? data : nullptr)),
        length_(length) {}

  const unsigned char* front() const override;
  size_t size() const override;

 private:
  ~RefCountedStaticMemory() override;

  const unsigned char* data_;
  size_t length_;

  DISALLOW_COPY_AND_ASSIGN(RefCountedStaticMemory);
};

// vector.
class BASE_EXPORT RefCountedBytes : public RefCountedMemory {
 public:
  RefCountedBytes();

  explicit RefCountedBytes(const std::vector<unsigned char>& initializer);

  RefCountedBytes(const unsigned char* p, size_t size);


  explicit RefCountedBytes(size_t size);



  static scoped_refptr<RefCountedBytes> TakeVector(
      std::vector<unsigned char>* to_destroy);

  const unsigned char* front() const override;
  size_t size() const override;

  const std::vector<unsigned char>& data() const { return data_; }
  std::vector<unsigned char>& data() { return data_; }


  unsigned char* front() { return data_.data(); }
  template <typename T>
  T* front_as() {
    return reinterpret_cast<T*>(front());
  }

 private:
  ~RefCountedBytes() override;

  std::vector<unsigned char> data_;

  DISALLOW_COPY_AND_ASSIGN(RefCountedBytes);
};

// string. Use this if your data naturally arrives in that format.
class BASE_EXPORT RefCountedString : public RefCountedMemory {
 public:
  RefCountedString();



  static scoped_refptr<RefCountedString> TakeString(std::string* to_destroy);

  const unsigned char* front() const override;
  size_t size() const override;

  const std::string& data() const { return data_; }
  std::string& data() { return data_; }

 private:
  ~RefCountedString() override;

  std::string data_;

  DISALLOW_COPY_AND_ASSIGN(RefCountedString);
};

// ReadOnlySharedMemoryMapping.
class BASE_EXPORT RefCountedSharedMemoryMapping : public RefCountedMemory {
 public:


  explicit RefCountedSharedMemoryMapping(ReadOnlySharedMemoryMapping mapping);


  static scoped_refptr<RefCountedSharedMemoryMapping> CreateFromWholeRegion(
      const ReadOnlySharedMemoryRegion& region);

  const unsigned char* front() const override;
  size_t size() const override;

 private:
  ~RefCountedSharedMemoryMapping() override;

  const ReadOnlySharedMemoryMapping mapping_;
  const size_t size_;

  DISALLOW_COPY_AND_ASSIGN(RefCountedSharedMemoryMapping);
};

}  // namespace base

#endif  // BASE_MEMORY_REF_COUNTED_MEMORY_H_
