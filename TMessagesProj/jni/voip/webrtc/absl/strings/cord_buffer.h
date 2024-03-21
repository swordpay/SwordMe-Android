// Copyright 2021 The Abseil Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// -----------------------------------------------------------------------------
// File: cord_buffer.h
// -----------------------------------------------------------------------------
//
// This file defines an `absl::CordBuffer` data structure to hold data for
// eventual inclusion within an existing `Cord` data structure. Cord buffers are
// useful for building large Cords that may require custom allocation of its
// associated memory.
//
#ifndef ABSL_STRINGS_CORD_BUFFER_H_
#define ABSL_STRINGS_CORD_BUFFER_H_

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>

#include "absl/base/config.h"
#include "absl/base/macros.h"
#include "absl/numeric/bits.h"
#include "absl/strings/internal/cord_internal.h"
#include "absl/strings/internal/cord_rep_flat.h"
#include "absl/types/span.h"

namespace absl {
ABSL_NAMESPACE_BEGIN

class Cord;
class CordBufferTestPeer;

//
// CordBuffer manages memory buffers for purposes such as zero-copy APIs as well
// as applications building cords with large data requiring granular control
// over the allocation and size of cord data. For example, a function creating
// a cord of random data could use a CordBuffer as follows:
//
//   absl::Cord CreateRandomCord(size_t length) {
//     absl::Cord cord;
//     while (length > 0) {
//       CordBuffer buffer = CordBuffer::CreateWithDefaultLimit(length);
//       absl::Span<char> data = buffer.available_up_to(length);
//       FillRandomValues(data.data(), data.size());
//       buffer.IncreaseLengthBy(data.size());
//       cord.Append(std::move(buffer));
//       length -= data.size();
//     }
//     return cord;
//   }
//
// CordBuffer instances are by default limited to a capacity of `kDefaultLimit`
// bytes. `kDefaultLimit` is currently just under 4KiB, but this default may
// change in the future and/or for specific architectures. The default limit is
// aimed to provide a good trade-off between performance and memory overhead.
// Smaller buffers typically incur more compute cost while larger buffers are
// more CPU efficient but create significant memory overhead because of such
// allocations being less granular. Using larger buffers may also increase the
// risk of memory fragmentation.
//
// Applications create a buffer using one of the `CreateWithDefaultLimit()` or
// `CreateWithCustomLimit()` methods. The returned instance will have a non-zero
// capacity and a zero length. Applications use the `data()` method to set the
// contents of the managed memory, and once done filling the buffer, use the
// `IncreaseLengthBy()` or 'SetLength()' method to specify the length of the
// initialized data before adding the buffer to a Cord.
//
// The `CreateWithCustomLimit()` method is intended for applications needing
// larger buffers than the default memory limit, allowing the allocation of up
// to a capacity of `kCustomLimit` bytes minus some minimum internal overhead.
// The usage of `CreateWithCustomLimit()` should be limited to only those use
// cases where the distribution of the input is relatively well known, and/or
// where the trade-off between the efficiency gains outweigh the risk of memory
// fragmentation. See the documentation for `CreateWithCustomLimit()` for more
// information on using larger custom limits.
//
// The capacity of a `CordBuffer` returned by one of the `Create` methods may
// be larger than the requested capacity due to rounding, alignment and
// granularity of the memory allocator. Applications should use the `capacity`
// method to obtain the effective capacity of the returned instance as
// demonstrated in the provided example above.
//
// CordBuffer is a move-only class. All references into the managed memory are
// invalidated when an instance is moved into either another CordBuffer instance
// or a Cord. Writing to a location obtained by a previous call to `data()`
// after an instance was moved will lead to undefined behavior.
//
// A `moved from` CordBuffer instance will have a valid, but empty state.
// CordBuffer is thread compatible.
class CordBuffer {
 public:




  static constexpr size_t kDefaultLimit = cord_internal::kMaxFlatLength;





  static constexpr size_t kCustomLimit = 64U << 10;


  CordBuffer() = default;


  ~CordBuffer();

  CordBuffer(CordBuffer&& rhs) noexcept;
  CordBuffer& operator=(CordBuffer&&) noexcept;
  CordBuffer(const CordBuffer&) = delete;
  CordBuffer& operator=(const CordBuffer&) = delete;













  static constexpr size_t MaximumPayload();



  static constexpr size_t MaximumPayload(size_t block_size);






  static CordBuffer CreateWithDefaultLimit(size_t capacity);


































































  static CordBuffer CreateWithCustomLimit(size_t block_size, size_t capacity);




  absl::Span<char> available();




  absl::Span<char> available_up_to(size_t size);






  char* data();
  const char* data() const;





  size_t length() const;




  size_t capacity() const;












  void IncreaseLengthBy(size_t n);









  void SetLength(size_t length);

 private:

  static_assert(kCustomLimit <= cord_internal::kMaxLargeFlatSize, "");








  static constexpr size_t kMaxPageSlop = 128;

  static constexpr size_t kOverhead = cord_internal::kFlatOverhead;

  using CordRepFlat = cord_internal::CordRepFlat;



  struct Rep {

    static constexpr size_t kInlineCapacity = sizeof(intptr_t) * 2 - 1;

    Rep() : short_rep{} {}

    explicit Rep(cord_internal::CordRepFlat* rep) : long_rep{rep} {
      assert(rep != nullptr);
    }

    bool is_short() const {
      constexpr size_t offset = offsetof(Short, raw_size);
      return (reinterpret_cast<const char*>(this)[offset] & 1) != 0;
    }

    absl::Span<char> short_available() {
      const size_t length = short_length();
      return absl::Span<char>(short_rep.data + length,
                              kInlineCapacity - length);
    }

    absl::Span<char> long_available() {
      assert(!is_short());
      const size_t length = long_rep.rep->length;
      return absl::Span<char>(long_rep.rep->Data() + length,
                              long_rep.rep->Capacity() - length);
    }

    size_t short_length() const {
      assert(is_short());
      return static_cast<size_t>(short_rep.raw_size >> 1);
    }


    void set_short_length(size_t length) {
      short_rep.raw_size = static_cast<char>((length << 1) + 1);
    }

    void add_short_length(size_t n) {
      assert(is_short());
      short_rep.raw_size += static_cast<char>(n << 1);
    }

    char* data() {
      assert(is_short());
      return short_rep.data;
    }
    const char* data() const {
      assert(is_short());
      return short_rep.data;
    }

    cord_internal::CordRepFlat* rep() const {
      assert(!is_short());
      return long_rep.rep;
    }




#if defined(ABSL_IS_BIG_ENDIAN)
    struct Long {
      explicit Long(cord_internal::CordRepFlat* rep_arg) : rep(rep_arg) {}
      void* padding;
      cord_internal::CordRepFlat* rep;
    };
    struct Short {
      char data[sizeof(Long) - 1];
      char raw_size = 1;
    };
#else
    struct Long {
      explicit Long(cord_internal::CordRepFlat* rep_arg) : rep(rep_arg) {}
      cord_internal::CordRepFlat* rep;
      void* padding;
    };
    struct Short {
      char raw_size = 1;
      char data[sizeof(Long) - 1];
    };
#endif

    union {
      Long long_rep;
      Short short_rep;
    };
  };

  static bool IsPow2(size_t size) { return absl::has_single_bit(size); }
  static size_t Log2Floor(size_t size) {
    return static_cast<size_t>(absl::bit_width(size) - 1);
  }
  static size_t Log2Ceil(size_t size) {
    return static_cast<size_t>(absl::bit_width(size - 1));
  }



  template <typename... AllocationHints>
  static CordBuffer CreateWithCustomLimitImpl(size_t block_size,
                                              size_t capacity,
                                              AllocationHints... hints);







  cord_internal::CordRep* ConsumeValue(absl::string_view& short_value) {
    cord_internal::CordRep* rep = nullptr;
    if (rep_.is_short()) {
      short_value = absl::string_view(rep_.data(), rep_.short_length());
    } else {
      rep = rep_.rep();
    }
    rep_.set_short_length(0);
    return rep;
  }

  explicit CordBuffer(cord_internal::CordRepFlat* rep) : rep_(rep) {
    assert(rep != nullptr);
  }

  Rep rep_;

  friend class Cord;
  friend class CordBufferTestPeer;
};

inline constexpr size_t CordBuffer::MaximumPayload() {
  return cord_internal::kMaxFlatLength;
}

inline constexpr size_t CordBuffer::MaximumPayload(size_t block_size) {

  return (kCustomLimit < block_size ? kCustomLimit : block_size) -
         cord_internal::kFlatOverhead;
}

inline CordBuffer CordBuffer::CreateWithDefaultLimit(size_t capacity) {
  if (capacity > Rep::kInlineCapacity) {
    auto* rep = cord_internal::CordRepFlat::New(capacity);
    rep->length = 0;
    return CordBuffer(rep);
  }
  return CordBuffer();
}

template <typename... AllocationHints>
inline CordBuffer CordBuffer::CreateWithCustomLimitImpl(
    size_t block_size, size_t capacity, AllocationHints... hints) {
  assert(IsPow2(block_size));
  capacity = (std::min)(capacity, kCustomLimit);
  block_size = (std::min)(block_size, kCustomLimit);
  if (capacity + kOverhead >= block_size) {
    capacity = block_size;
  } else if (capacity <= kDefaultLimit) {
    capacity = capacity + kOverhead;
  } else if (!IsPow2(capacity)) {


    const size_t rounded_up = size_t{1} << Log2Ceil(capacity);
    const size_t slop = rounded_up - capacity;
    if (slop >= kOverhead && slop <= kMaxPageSlop + kOverhead) {
      capacity = rounded_up;
    } else {



      const size_t rounded_down = size_t{1} << Log2Floor(capacity);
      capacity = rounded_down;
    }
  }
  const size_t length = capacity - kOverhead;
  auto* rep = CordRepFlat::New(CordRepFlat::Large(), length, hints...);
  rep->length = 0;
  return CordBuffer(rep);
}

inline CordBuffer CordBuffer::CreateWithCustomLimit(size_t block_size,
                                                    size_t capacity) {
  return CreateWithCustomLimitImpl(block_size, capacity);
}

inline CordBuffer::~CordBuffer() {
  if (!rep_.is_short()) {
    cord_internal::CordRepFlat::Delete(rep_.rep());
  }
}

inline CordBuffer::CordBuffer(CordBuffer&& rhs) noexcept : rep_(rhs.rep_) {
  rhs.rep_.set_short_length(0);
}

inline CordBuffer& CordBuffer::operator=(CordBuffer&& rhs) noexcept {
  if (!rep_.is_short()) cord_internal::CordRepFlat::Delete(rep_.rep());
  rep_ = rhs.rep_;
  rhs.rep_.set_short_length(0);
  return *this;
}

inline absl::Span<char> CordBuffer::available() {
  return rep_.is_short() ? rep_.short_available() : rep_.long_available();
}

inline absl::Span<char> CordBuffer::available_up_to(size_t size) {
  return available().subspan(0, size);
}

inline char* CordBuffer::data() {
  return rep_.is_short() ? rep_.data() : rep_.rep()->Data();
}

inline const char* CordBuffer::data() const {
  return rep_.is_short() ? rep_.data() : rep_.rep()->Data();
}

inline size_t CordBuffer::capacity() const {
  return rep_.is_short() ? Rep::kInlineCapacity : rep_.rep()->Capacity();
}

inline size_t CordBuffer::length() const {
  return rep_.is_short() ? rep_.short_length() : rep_.rep()->length;
}

inline void CordBuffer::SetLength(size_t length) {
  ABSL_HARDENING_ASSERT(length <= capacity());
  if (rep_.is_short()) {
    rep_.set_short_length(length);
  } else {
    rep_.rep()->length = length;
  }
}

inline void CordBuffer::IncreaseLengthBy(size_t n) {
  ABSL_HARDENING_ASSERT(n <= capacity() && length() + n <= capacity());
  if (rep_.is_short()) {
    rep_.add_short_length(n);
  } else {
    rep_.rep()->length += n;
  }
}

ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STRINGS_CORD_BUFFER_H_
