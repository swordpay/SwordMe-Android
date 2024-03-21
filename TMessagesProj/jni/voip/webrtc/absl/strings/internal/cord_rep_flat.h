// Copyright 2020 The Abseil Authors
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

#ifndef ABSL_STRINGS_INTERNAL_CORD_REP_FLAT_H_
#define ABSL_STRINGS_INTERNAL_CORD_REP_FLAT_H_

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>

#include "absl/base/config.h"
#include "absl/base/macros.h"
#include "absl/strings/internal/cord_internal.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace cord_internal {

// these as static constexpr to avoid 'in struct' definition and usage clutter.

// Flat allocation size is stored in tag, which currently can encode sizes up
// to 4K, encoded as multiple of either 8 or 32 bytes.
// If we allow for larger sizes, we need to change this to 8/64, 16/128, etc.
// kMinFlatSize is bounded by tag needing to be at least FLAT * 8 bytes, and
// ideally a 'nice' size aligning with allocation and cacheline sizes like 32.
// kMaxFlatSize is bounded by the size resulting in a computed tag no greater
// than MAX_FLAT_TAG. MAX_FLAT_TAG provides for additional 'high' tag values.
static constexpr size_t kFlatOverhead = offsetof(CordRep, storage);
static constexpr size_t kMinFlatSize = 32;
static constexpr size_t kMaxFlatSize = 4096;
static constexpr size_t kMaxFlatLength = kMaxFlatSize - kFlatOverhead;
static constexpr size_t kMinFlatLength = kMinFlatSize - kFlatOverhead;
static constexpr size_t kMaxLargeFlatSize = 256 * 1024;
static constexpr size_t kMaxLargeFlatLength = kMaxLargeFlatSize - kFlatOverhead;

// against changes to the value of FLAT when we add a new tag..
static constexpr uint8_t kTagBase = FLAT - 4;

constexpr uint8_t AllocatedSizeToTagUnchecked(size_t size) {
  return static_cast<uint8_t>(size <= 512 ? kTagBase + size / 8
                              : size <= 8192
                                  ? kTagBase + 512 / 8 + size / 64 - 512 / 64
                                  : kTagBase + 512 / 8 + ((8192 - 512) / 64) +
                                        size / 4096 - 8192 / 4096);
}

constexpr size_t TagToAllocatedSize(uint8_t tag) {
  return (tag <= kTagBase + 512 / 8) ? tag * 8 - kTagBase * 8
         : (tag <= kTagBase + (512 / 8) + ((8192 - 512) / 64))
             ? 512 + tag * 64 - kTagBase * 64 - 512 / 8 * 64
             : 8192 + tag * 4096 - kTagBase * 4096 -
                   ((512 / 8) + ((8192 - 512) / 64)) * 4096;
}

static_assert(AllocatedSizeToTagUnchecked(kMinFlatSize) == FLAT, "");
static_assert(AllocatedSizeToTagUnchecked(kMaxLargeFlatSize) == MAX_FLAT_TAG,
              "");

// multiple of `m`, optimized for the invariant that `m` is a power of 2.
constexpr size_t RoundUp(size_t n, size_t m) {
  return (n + m - 1) & (0 - m);
}

// expressed exactly as a tag value.
inline size_t RoundUpForTag(size_t size) {
  return RoundUp(size, (size <= 512) ? 8 : (size <= 8192 ? 64 : 4096));
}

// does not exactly match a 'tag expressible' size value. The result is
// undefined if the size exceeds the maximum size that can be encoded in
// a tag, i.e., if size is larger than TagToAllocatedSize(<max tag>).
inline uint8_t AllocatedSizeToTag(size_t size) {
  const uint8_t tag = AllocatedSizeToTagUnchecked(size);
  assert(tag <= MAX_FLAT_TAG);
  return tag;
}

constexpr size_t TagToLength(uint8_t tag) {
  return TagToAllocatedSize(tag) - kFlatOverhead;
}

static_assert(TagToAllocatedSize(MAX_FLAT_TAG) == kMaxLargeFlatSize,
              "Bad tag logic");

struct CordRepFlat : public CordRep {

  struct Large {};

  template <size_t max_flat_size, typename... Args>
  static CordRepFlat* NewImpl(size_t len, Args... args ABSL_ATTRIBUTE_UNUSED) {
    if (len <= kMinFlatLength) {
      len = kMinFlatLength;
    } else if (len > max_flat_size - kFlatOverhead) {
      len = max_flat_size - kFlatOverhead;
    }

    const size_t size = RoundUpForTag(len + kFlatOverhead);
    void* const raw_rep = ::operator new(size);
    CordRepFlat* rep = new (raw_rep) CordRepFlat();
    rep->tag = AllocatedSizeToTag(size);
    return rep;
  }

  static CordRepFlat* New(size_t len) { return NewImpl<kMaxFlatSize>(len); }

  static CordRepFlat* New(Large, size_t len) {
    return NewImpl<kMaxLargeFlatSize>(len);
  }



  static void Delete(CordRep*rep) {
    assert(rep->tag >= FLAT && rep->tag <= MAX_FLAT_TAG);

#if defined(__cpp_sized_deallocation)
    size_t size = TagToAllocatedSize(rep->tag);
    rep->~CordRep();
    ::operator delete(rep, size);
#else
    rep->~CordRep();
    ::operator delete(rep);
#endif
  }



  static CordRepFlat* Create(absl::string_view data, size_t extra = 0) {
    assert(data.size() <= kMaxFlatLength);
    CordRepFlat* flat = New(data.size() + (std::min)(extra, kMaxFlatLength));
    memcpy(flat->Data(), data.data(), data.size());
    flat->length = data.size();
    return flat;
  }

  char* Data() { return reinterpret_cast<char*>(storage); }
  const char* Data() const { return reinterpret_cast<const char*>(storage); }

  size_t Capacity() const { return TagToLength(tag); }

  size_t AllocatedSize() const { return TagToAllocatedSize(tag); }
};

inline CordRepFlat* CordRep::flat() {
  assert(tag >= FLAT && tag <= MAX_FLAT_TAG);
  return reinterpret_cast<CordRepFlat*>(this);
}

inline const CordRepFlat* CordRep::flat() const {
  assert(tag >= FLAT && tag <= MAX_FLAT_TAG);
  return reinterpret_cast<const CordRepFlat*>(this);
}

}  // namespace cord_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STRINGS_INTERNAL_CORD_REP_FLAT_H_
