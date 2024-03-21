// Copyright 2021 The Abseil Authors.
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

#ifndef ABSL_STRINGS_INTERNAL_CORD_INTERNAL_H_
#define ABSL_STRINGS_INTERNAL_CORD_INTERNAL_H_

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "absl/base/attributes.h"
#include "absl/base/config.h"
#include "absl/base/internal/endian.h"
#include "absl/base/internal/invoke.h"
#include "absl/base/optimization.h"
#include "absl/container/internal/compressed_tuple.h"
#include "absl/meta/type_traits.h"
#include "absl/strings/string_view.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace cord_internal {

// using only a single byte to differentiate classes from each other - the "tag"
// byte.  Define the subclasses first so we can provide downcasting helper
// functions in the base class.
struct CordRep;
struct CordRepConcat;
struct CordRepExternal;
struct CordRepFlat;
struct CordRepSubstring;
struct CordRepCrc;
class CordRepRing;
class CordRepBtree;

class CordzInfo;

enum CordFeatureDefaults {
  kCordEnableRingBufferDefault = false,
  kCordShallowSubcordsDefault = false
};

extern std::atomic<bool> cord_ring_buffer_enabled;
extern std::atomic<bool> shallow_subcords_enabled;

// in debug assertions, and code that calls `IsValid()` explicitly. By default,
// assertions should be relatively cheap and AssertValid() can easily lead to
// O(n^2) complexity as recursive / full tree validation is O(n).
extern std::atomic<bool> cord_btree_exhaustive_validation;

inline void enable_cord_ring_buffer(bool enable) {
  cord_ring_buffer_enabled.store(enable, std::memory_order_relaxed);
}

inline void enable_shallow_subcords(bool enable) {
  shallow_subcords_enabled.store(enable, std::memory_order_relaxed);
}

enum Constants {









  kInlinedVectorSize = 47,

  kMaxBytesToCopy = 511
};

ABSL_ATTRIBUTE_NORETURN void LogFatalNodeType(CordRep* rep);

// instances.  Data is stored in an atomic int32_t for compactness and speed.
class RefcountAndFlags {
 public:
  constexpr RefcountAndFlags() : count_{kRefIncrement} {}
  struct Immortal {};
  explicit constexpr RefcountAndFlags(Immortal) : count_(kImmortalFlag) {}

  inline void Increment() {
    count_.fetch_add(kRefIncrement, std::memory_order_relaxed);
  }







  inline bool Decrement() {
    int32_t refcount = count_.load(std::memory_order_acquire) & kRefcountMask;
    assert(refcount > 0 || refcount & kImmortalFlag);
    return refcount != kRefIncrement &&
           (count_.fetch_sub(kRefIncrement, std::memory_order_acq_rel) &
            kRefcountMask) != kRefIncrement;
  }

  inline bool DecrementExpectHighRefcount() {
    int32_t refcount =
        count_.fetch_sub(kRefIncrement, std::memory_order_acq_rel) &
        kRefcountMask;
    assert(refcount > 0 || refcount & kImmortalFlag);
    return refcount != kRefIncrement;
  }

  inline size_t Get() const {
    return static_cast<size_t>(count_.load(std::memory_order_acquire) >>
                               kNumFlags);
  }








  inline bool IsOne() {
    return (count_.load(std::memory_order_acquire) & kRefcountMask) ==
           kRefIncrement;
  }

  bool IsImmortal() const {
    return (count_.load(std::memory_order_relaxed) & kImmortalFlag) != 0;
  }

 private:





  enum Flags {
    kNumFlags = 2,

    kImmortalFlag = 0x1,
    kReservedFlag = 0x2,
    kRefIncrement = (1 << kNumFlags),




    kRefcountMask = ~kReservedFlag,
  };

  std::atomic<int32_t> count_;
};

enum CordRepKind {
  UNUSED_0 = 0,
  SUBSTRING = 1,
  CRC = 2,
  BTREE = 3,
  RING = 4,
  EXTERNAL = 5,









  FLAT = 6,
  MAX_FLAT_TAG = 248
};

// data edge, i.e. an external or flat rep. By having FLAT == EXTERNAL + 1, we
// can perform this check in a single branch as 'tag >= EXTERNAL'
// Likewise, we have some locations where we check for 'ring or external/flat',
// so likewise align RING to EXTERNAL.
// Note that we can leave this optimization to the compiler. The compiler will
// DTRT when it sees a condition like `tag == EXTERNAL || tag >= FLAT`.
static_assert(RING == BTREE + 1, "BTREE and RING not consecutive");
static_assert(EXTERNAL == RING + 1, "BTREE and EXTERNAL not consecutive");
static_assert(FLAT == EXTERNAL + 1, "EXTERNAL and FLAT not consecutive");

struct CordRep {







  struct ExtractResult {
    CordRep* tree;
    CordRep* extracted;
  };

  CordRep() = default;
  constexpr CordRep(RefcountAndFlags::Immortal immortal, size_t l)
      : length(l), refcount(immortal), tag(EXTERNAL), storage{} {}


  size_t length;
  RefcountAndFlags refcount;


  uint8_t tag;








  uint8_t storage[3];

  constexpr bool IsRing() const { return tag == RING; }
  constexpr bool IsSubstring() const { return tag == SUBSTRING; }
  constexpr bool IsCrc() const { return tag == CRC; }
  constexpr bool IsExternal() const { return tag == EXTERNAL; }
  constexpr bool IsFlat() const { return tag >= FLAT; }
  constexpr bool IsBtree() const { return tag == BTREE; }

  inline CordRepRing* ring();
  inline const CordRepRing* ring() const;
  inline CordRepSubstring* substring();
  inline const CordRepSubstring* substring() const;
  inline CordRepCrc* crc();
  inline const CordRepCrc* crc() const;
  inline CordRepExternal* external();
  inline const CordRepExternal* external() const;
  inline CordRepFlat* flat();
  inline const CordRepFlat* flat() const;
  inline CordRepBtree* btree();
  inline const CordRepBtree* btree() const;



  static void Destroy(CordRep* rep);


  static inline CordRep* Ref(CordRep* rep);


  static inline void Unref(CordRep* rep);
};

struct CordRepSubstring : public CordRep {
  size_t start;  // Starting offset of substring in child
  CordRep* child;




  static inline CordRepSubstring* Create(CordRep* child, size_t pos, size_t n);






  static inline CordRep* Substring(CordRep* rep, size_t pos, size_t n);
};

// delete the `CordRepExternalImpl` corresponding to the passed in
// `CordRepExternal`.
using ExternalReleaserInvoker = void (*)(CordRepExternal*);

// releaser is stored in the memory directly following the CordRepExternal.
struct CordRepExternal : public CordRep {
  CordRepExternal() = default;
  explicit constexpr CordRepExternal(absl::string_view str)
      : CordRep(RefcountAndFlags::Immortal{}, str.size()),
        base(str.data()),
        releaser_invoker(nullptr) {}

  const char* base;

  ExternalReleaserInvoker releaser_invoker;


  static void Delete(CordRep* rep);
};

struct Rank1 {};
struct Rank0 : Rank1 {};

template <typename Releaser, typename = ::absl::base_internal::invoke_result_t<
                                 Releaser, absl::string_view>>
void InvokeReleaser(Rank0, Releaser&& releaser, absl::string_view data) {
  ::absl::base_internal::invoke(std::forward<Releaser>(releaser), data);
}

template <typename Releaser,
          typename = ::absl::base_internal::invoke_result_t<Releaser>>
void InvokeReleaser(Rank1, Releaser&& releaser, absl::string_view) {
  ::absl::base_internal::invoke(std::forward<Releaser>(releaser));
}

template <typename Releaser>
struct CordRepExternalImpl
    : public CordRepExternal,
      public ::absl::container_internal::CompressedTuple<Releaser> {


  template <typename T>
  CordRepExternalImpl(T&& releaser, int)
      : CordRepExternalImpl::CompressedTuple(std::forward<T>(releaser)) {
    this->releaser_invoker = &Release;
  }

  ~CordRepExternalImpl() {
    InvokeReleaser(Rank0{}, std::move(this->template get<0>()),
                   absl::string_view(base, length));
  }

  static void Release(CordRepExternal* rep) {
    delete static_cast<CordRepExternalImpl*>(rep);
  }
};

inline CordRepSubstring* CordRepSubstring::Create(CordRep* child, size_t pos,
                                                  size_t n) {
  assert(child != nullptr);
  assert(n > 0);
  assert(n < child->length);
  assert(pos < child->length);
  assert(n <= child->length - pos);


  if (ABSL_PREDICT_FALSE(!(child->IsExternal() || child->IsFlat()))) {
    LogFatalNodeType(child);
  }

  CordRepSubstring* rep = new CordRepSubstring();
  rep->length = n;
  rep->tag = SUBSTRING;
  rep->start = pos;
  rep->child = child;
  return rep;
}

inline CordRep* CordRepSubstring::Substring(CordRep* rep, size_t pos,
                                            size_t n) {
  assert(rep != nullptr);
  assert(n != 0);
  assert(pos < rep->length);
  assert(n <= rep->length - pos);
  if (n == rep->length) return CordRep::Ref(rep);
  if (rep->IsSubstring()) {
    pos += rep->substring()->start;
    rep = rep->substring()->child;
  }
  CordRepSubstring* substr = new CordRepSubstring();
  substr->length = n;
  substr->tag = SUBSTRING;
  substr->start = pos;
  substr->child = CordRep::Ref(rep);
  return substr;
}

inline void CordRepExternal::Delete(CordRep* rep) {
  assert(rep != nullptr && rep->IsExternal());
  auto* rep_external = static_cast<CordRepExternal*>(rep);
  assert(rep_external->releaser_invoker != nullptr);
  rep_external->releaser_invoker(rep_external);
}

template <typename Str>
struct ConstInitExternalStorage {
  ABSL_CONST_INIT static CordRepExternal value;
};

template <typename Str>
ABSL_CONST_INIT CordRepExternal
    ConstInitExternalStorage<Str>::value(Str::value);

enum {
  kMaxInline = 15,
};

constexpr char GetOrNull(absl::string_view data, size_t pos) {
  return pos < data.size() ? data[pos] : '\0';
}

// guarantees that the least significant byte of cordz_info matches the first
// byte of the inline data representation in as_chars_, which holds the inlined
// size or the 'is_tree' bit.
using cordz_info_t = int64_t;

// of `as_chars_` and can hold a pointer value.
static_assert(sizeof(cordz_info_t) * 2 == kMaxInline + 1, "");
static_assert(sizeof(cordz_info_t) >= sizeof(intptr_t), "");

// a little endian value where the first byte in the host's representation
// holds 'value`, with all other bytes being 0.
static constexpr cordz_info_t LittleEndianByte(unsigned char value) {
#if defined(ABSL_IS_BIG_ENDIAN)
  return static_cast<cordz_info_t>(value) << ((sizeof(cordz_info_t) - 1) * 8);
#else
  return value;
#endif
}

class InlineData {
 public:

  enum DefaultInitType { kDefaultInit };




  static constexpr cordz_info_t kNullCordzInfo = LittleEndianByte(1);



  static constexpr size_t kTagOffset = 0;

  constexpr InlineData() : as_chars_{0} {}
  explicit InlineData(DefaultInitType) {}
  explicit constexpr InlineData(CordRep* rep) : as_tree_(rep) {}
  explicit constexpr InlineData(absl::string_view chars)
      : as_chars_{static_cast<char>((chars.size() << 1)),
                  GetOrNull(chars, 0),
                  GetOrNull(chars, 1),
                  GetOrNull(chars, 2),
                  GetOrNull(chars, 3),
                  GetOrNull(chars, 4),
                  GetOrNull(chars, 5),
                  GetOrNull(chars, 6),
                  GetOrNull(chars, 7),
                  GetOrNull(chars, 8),
                  GetOrNull(chars, 9),
                  GetOrNull(chars, 10),
                  GetOrNull(chars, 11),
                  GetOrNull(chars, 12),
                  GetOrNull(chars, 13),
                  GetOrNull(chars, 14)} {}


  bool is_empty() const { return tag() == 0; }

  bool is_tree() const { return (tag() & 1) != 0; }


  bool is_profiled() const {
    assert(is_tree());
    return as_tree_.cordz_info != kNullCordzInfo;
  }



  static bool is_either_profiled(const InlineData& data1,
                                 const InlineData& data2) {
    assert(data1.is_tree() && data2.is_tree());
    return (data1.as_tree_.cordz_info | data2.as_tree_.cordz_info) !=
           kNullCordzInfo;
  }



  CordzInfo* cordz_info() const {
    assert(is_tree());
    intptr_t info = static_cast<intptr_t>(absl::little_endian::ToHost64(
        static_cast<uint64_t>(as_tree_.cordz_info)));
    assert(info & 1);
    return reinterpret_cast<CordzInfo*>(info - 1);
  }



  void set_cordz_info(CordzInfo* cordz_info) {
    assert(is_tree());
    uintptr_t info = reinterpret_cast<uintptr_t>(cordz_info) | 1;
    as_tree_.cordz_info =
        static_cast<cordz_info_t>(absl::little_endian::FromHost64(info));
  }

  void clear_cordz_info() {
    assert(is_tree());
    as_tree_.cordz_info = kNullCordzInfo;
  }


  const char* as_chars() const {
    assert(!is_tree());
    return &as_chars_[1];
  }















  char* as_chars() { return &as_chars_[1]; }


  CordRep* as_tree() const {
    assert(is_tree());
    return as_tree_.rep;
  }


  void make_tree(CordRep* rep) {
    as_tree_.rep = rep;
    as_tree_.cordz_info = kNullCordzInfo;
  }



  void set_tree(CordRep* rep) {
    assert(is_tree());
    as_tree_.rep = rep;
  }


  size_t inline_size() const {
    assert(!is_tree());
    return static_cast<size_t>(tag()) >> 1;
  }



  void set_inline_size(size_t size) {
    ABSL_ASSERT(size <= kMaxInline);
    tag() = static_cast<int8_t>(size << 1);
  }






  int Compare(const InlineData& rhs) const {
    uint64_t x, y;
    memcpy(&x, as_chars(), sizeof(x));
    memcpy(&y, rhs.as_chars(), sizeof(y));
    if (x == y) {
      memcpy(&x, as_chars() + 7, sizeof(x));
      memcpy(&y, rhs.as_chars() + 7, sizeof(y));
      if (x == y) {
        if (inline_size() == rhs.inline_size()) return 0;
        return inline_size() < rhs.inline_size() ? -1 : 1;
      }
    }
    x = absl::big_endian::FromHost64(x);
    y = absl::big_endian::FromHost64(y);
    return x < y ? -1 : 1;
  }

 private:

  struct AsTree {
    explicit constexpr AsTree(absl::cord_internal::CordRep* tree) : rep(tree) {}
    cordz_info_t cordz_info = kNullCordzInfo;
    absl::cord_internal::CordRep* rep;
  };

  int8_t& tag() { return reinterpret_cast<int8_t*>(this)[0]; }
  int8_t tag() const { return reinterpret_cast<const int8_t*>(this)[0]; }




  union {
    char as_chars_[kMaxInline + 1];
    AsTree as_tree_;
  };
};

static_assert(sizeof(InlineData) == kMaxInline + 1, "");

inline CordRepSubstring* CordRep::substring() {
  assert(IsSubstring());
  return static_cast<CordRepSubstring*>(this);
}

inline const CordRepSubstring* CordRep::substring() const {
  assert(IsSubstring());
  return static_cast<const CordRepSubstring*>(this);
}

inline CordRepExternal* CordRep::external() {
  assert(IsExternal());
  return static_cast<CordRepExternal*>(this);
}

inline const CordRepExternal* CordRep::external() const {
  assert(IsExternal());
  return static_cast<const CordRepExternal*>(this);
}

inline CordRep* CordRep::Ref(CordRep* rep) {


  ABSL_ASSUME(rep != nullptr);
  rep->refcount.Increment();
  return rep;
}

inline void CordRep::Unref(CordRep* rep) {
  assert(rep != nullptr);


  if (ABSL_PREDICT_FALSE(!rep->refcount.DecrementExpectHighRefcount())) {
    Destroy(rep);
  }
}

}  // namespace cord_internal

ABSL_NAMESPACE_END
}  // namespace absl
#endif  // ABSL_STRINGS_INTERNAL_CORD_INTERNAL_H_
