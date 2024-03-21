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
//
// -----------------------------------------------------------------------------
// File: cord.h
// -----------------------------------------------------------------------------
//
// This file defines the `absl::Cord` data structure and operations on that data
// structure. A Cord is a string-like sequence of characters optimized for
// specific use cases. Unlike a `std::string`, which stores an array of
// contiguous characters, Cord data is stored in a structure consisting of
// separate, reference-counted "chunks."
//
// Because a Cord consists of these chunks, data can be added to or removed from
// a Cord during its lifetime. Chunks may also be shared between Cords. Unlike a
// `std::string`, a Cord can therefore accommodate data that changes over its
// lifetime, though it's not quite "mutable"; it can change only in the
// attachment, detachment, or rearrangement of chunks of its constituent data.
//
// A Cord provides some benefit over `std::string` under the following (albeit
// narrow) circumstances:
//
//   * Cord data is designed to grow and shrink over a Cord's lifetime. Cord
//     provides efficient insertions and deletions at the start and end of the
//     character sequences, avoiding copies in those cases. Static data should
//     generally be stored as strings.
//   * External memory consisting of string-like data can be directly added to
//     a Cord without requiring copies or allocations.
//   * Cord data may be shared and copied cheaply. Cord provides a copy-on-write
//     implementation and cheap sub-Cord operations. Copying a Cord is an O(1)
//     operation.
//
// As a consequence to the above, Cord data is generally large. Small data
// should generally use strings, as construction of a Cord requires some
// overhead. Small Cords (<= 15 bytes) are represented inline, but most small
// Cords are expected to grow over their lifetimes.
//
// Note that because a Cord is made up of separate chunked data, random access
// to character data within a Cord is slower than within a `std::string`.
//
// Thread Safety
//
// Cord has the same thread-safety properties as many other types like
// std::string, std::vector<>, int, etc -- it is thread-compatible. In
// particular, if threads do not call non-const methods, then it is safe to call
// const methods without synchronization. Copying a Cord produces a new instance
// that can be used concurrently with the original in arbitrary ways.

#ifndef ABSL_STRINGS_CORD_H_
#define ABSL_STRINGS_CORD_H_

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <string>
#include <type_traits>

#include "absl/base/attributes.h"
#include "absl/base/config.h"
#include "absl/base/internal/endian.h"
#include "absl/base/internal/per_thread_tls.h"
#include "absl/base/macros.h"
#include "absl/base/port.h"
#include "absl/container/inlined_vector.h"
#include "absl/functional/function_ref.h"
#include "absl/meta/type_traits.h"
#include "absl/strings/cord_analysis.h"
#include "absl/strings/cord_buffer.h"
#include "absl/strings/internal/cord_data_edge.h"
#include "absl/strings/internal/cord_internal.h"
#include "absl/strings/internal/cord_rep_btree.h"
#include "absl/strings/internal/cord_rep_btree_reader.h"
#include "absl/strings/internal/cord_rep_crc.h"
#include "absl/strings/internal/cord_rep_ring.h"
#include "absl/strings/internal/cordz_functions.h"
#include "absl/strings/internal/cordz_info.h"
#include "absl/strings/internal/cordz_statistics.h"
#include "absl/strings/internal/cordz_update_scope.h"
#include "absl/strings/internal/cordz_update_tracker.h"
#include "absl/strings/internal/resize_uninitialized.h"
#include "absl/strings/internal/string_constant.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
class Cord;
class CordTestPeer;
template <typename Releaser>
Cord MakeCordFromExternal(absl::string_view, Releaser&&);
void CopyCordToString(const Cord& src, std::string* dst);

enum class CordMemoryAccounting {



  kTotal,




  kFairShare,
};

//
// A Cord is a sequence of characters, designed to be more efficient than a
// `std::string` in certain circumstances: namely, large string data that needs
// to change over its lifetime or shared, especially when such data is shared
// across API boundaries.
//
// A Cord stores its character data in a structure that allows efficient prepend
// and append operations. This makes a Cord useful for large string data sent
// over in a wire format that may need to be prepended or appended at some point
// during the data exchange (e.g. HTTP, protocol buffers). For example, a
// Cord is useful for storing an HTTP request, and prepending an HTTP header to
// such a request.
//
// Cords should not be used for storing general string data, however. They
// require overhead to construct and are slower than strings for random access.
//
// The Cord API provides the following common API operations:
//
// * Create or assign Cords out of existing string data, memory, or other Cords
// * Append and prepend data to an existing Cord
// * Create new Sub-Cords from existing Cord data
// * Swap Cord data and compare Cord equality
// * Write out Cord data by constructing a `std::string`
//
// Additionally, the API provides iterator utilities to iterate through Cord
// data via chunks or character bytes.
//
class Cord {
 private:
  template <typename T>
  using EnableIfString =
      absl::enable_if_t<std::is_same<T, std::string>::value, int>;

 public:


  constexpr Cord() noexcept;


  Cord(const Cord& src);
  Cord(Cord&& src) noexcept;
  Cord& operator=(const Cord& x);
  Cord& operator=(Cord&& x) noexcept;



  explicit Cord(absl::string_view src);
  Cord& operator=(absl::string_view src);



  template <typename T, EnableIfString<T> = 0>
  explicit Cord(T&& src);
  template <typename T, EnableIfString<T> = 0>
  Cord& operator=(T&& src);



  ~Cord() {
    if (contents_.is_tree()) DestroyCordSlow();
  }







































  template <typename Releaser>
  friend Cord MakeCordFromExternal(absl::string_view data, Releaser&& releaser);




  ABSL_ATTRIBUTE_REINITIALIZES void Clear();




  void Append(const Cord& src);
  void Append(Cord&& src);
  void Append(absl::string_view src);
  template <typename T, EnableIfString<T> = 0>
  void Append(T&& src);



  void Append(CordBuffer buffer);
































  CordBuffer GetAppendBuffer(size_t capacity, size_t min_capacity = 16);










  CordBuffer GetCustomAppendBuffer(size_t block_size, size_t capacity,
                                   size_t min_capacity = 16);




  void Prepend(const Cord& src);
  void Prepend(absl::string_view src);
  template <typename T, EnableIfString<T> = 0>
  void Prepend(T&& src);



  void Prepend(CordBuffer buffer);



  void RemovePrefix(size_t n);
  void RemoveSuffix(size_t n);





  Cord Subcord(size_t pos, size_t new_size) const;



  void swap(Cord& other) noexcept;



  friend void swap(Cord& x, Cord& y) noexcept { x.swap(y); }



  size_t size() const;



  bool empty() const;




  size_t EstimatedMemoryUsage(CordMemoryAccounting accounting_method =
                                  CordMemoryAccounting::kTotal) const;









  int Compare(absl::string_view rhs) const;
  int Compare(const Cord& rhs) const;



  bool StartsWith(const Cord& rhs) const;
  bool StartsWith(absl::string_view rhs) const;



  bool EndsWith(absl::string_view rhs) const;
  bool EndsWith(const Cord& rhs) const;




  explicit operator std::string() const;









  friend void CopyCordToString(const Cord& src, std::string* dst);

  class CharIterator;




























  class ChunkIterator {
   public:
    using iterator_category = std::input_iterator_tag;
    using value_type = absl::string_view;
    using difference_type = ptrdiff_t;
    using pointer = const value_type*;
    using reference = value_type;

    ChunkIterator() = default;

    ChunkIterator& operator++();
    ChunkIterator operator++(int);
    bool operator==(const ChunkIterator& other) const;
    bool operator!=(const ChunkIterator& other) const;
    reference operator*() const;
    pointer operator->() const;

    friend class Cord;
    friend class CharIterator;

   private:
    using CordRep = absl::cord_internal::CordRep;
    using CordRepBtree = absl::cord_internal::CordRepBtree;
    using CordRepBtreeReader = absl::cord_internal::CordRepBtreeReader;

    explicit ChunkIterator(cord_internal::CordRep* tree);

    explicit ChunkIterator(const Cord* cord);

    void InitTree(cord_internal::CordRep* tree);


    void RemoveChunkPrefix(size_t n);
    Cord AdvanceAndReadBytes(size_t n);
    void AdvanceBytes(size_t n);

    ChunkIterator& AdvanceBtree();
    void AdvanceBytesBtree(size_t n);


    absl::string_view current_chunk_;



    absl::cord_internal::CordRep* current_leaf_ = nullptr;

    size_t bytes_remaining_ = 0;

    CordRepBtreeReader btree_reader_;
  };














  ChunkIterator chunk_begin() const;







  ChunkIterator chunk_end() const;












  class ChunkRange {
   public:




    using value_type = absl::string_view;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = ChunkIterator;
    using const_iterator = ChunkIterator;

    explicit ChunkRange(const Cord* cord) : cord_(cord) {}

    ChunkIterator begin() const;
    ChunkIterator end() const;

   private:
    const Cord* cord_;
  };



















  ChunkRange Chunks() const;
























  class CharIterator {
   public:
    using iterator_category = std::input_iterator_tag;
    using value_type = char;
    using difference_type = ptrdiff_t;
    using pointer = const char*;
    using reference = const char&;

    CharIterator() = default;

    CharIterator& operator++();
    CharIterator operator++(int);
    bool operator==(const CharIterator& other) const;
    bool operator!=(const CharIterator& other) const;
    reference operator*() const;
    pointer operator->() const;

    friend Cord;

   private:
    explicit CharIterator(const Cord* cord) : chunk_iterator_(cord) {}

    ChunkIterator chunk_iterator_;
  };






  static Cord AdvanceAndRead(CharIterator* it, size_t n_bytes);





  static void Advance(CharIterator* it, size_t n_bytes);





  static absl::string_view ChunkRemaining(const CharIterator& it);







  CharIterator char_begin() const;







  CharIterator char_end() const;










  class CharRange {
   public:




    using value_type = char;
    using reference = value_type&;
    using const_reference = const value_type&;
    using iterator = CharIterator;
    using const_iterator = CharIterator;

    explicit CharRange(const Cord* cord) : cord_(cord) {}

    CharIterator begin() const;
    CharIterator end() const;

   private:
    const Cord* cord_;
  };



















  CharRange Chars() const;










  char operator[](size_t i) const;




  absl::optional<absl::string_view> TryFlat() const;





  absl::string_view Flatten();

  friend void AbslFormatFlush(absl::Cord* cord, absl::string_view part) {
    cord->Append(part);
  }

















  void SetExpectedChecksum(uint32_t crc);


  absl::optional<uint32_t> ExpectedChecksum() const;

  template <typename H>
  friend H AbslHashValue(H hash_state, const absl::Cord& c) {
    absl::optional<absl::string_view> maybe_flat = c.TryFlat();
    if (maybe_flat.has_value()) {
      return H::combine(std::move(hash_state), *maybe_flat);
    }
    return c.HashFragmented(std::move(hash_state));
  }





  template <typename T>

  constexpr Cord(strings_internal::StringConstant<T>);

 private:
  using CordRep = absl::cord_internal::CordRep;
  using CordRepFlat = absl::cord_internal::CordRepFlat;
  using CordzInfo = cord_internal::CordzInfo;
  using CordzUpdateScope = cord_internal::CordzUpdateScope;
  using CordzUpdateTracker = cord_internal::CordzUpdateTracker;
  using InlineData = cord_internal::InlineData;
  using MethodIdentifier = CordzUpdateTracker::MethodIdentifier;


  explicit Cord(absl::string_view src, MethodIdentifier method);

  friend class CordTestPeer;
  friend bool operator==(const Cord& lhs, const Cord& rhs);
  friend bool operator==(const Cord& lhs, absl::string_view rhs);

  friend const CordzInfo* GetCordzInfoForTesting(const Cord& cord);


  void ForEachChunk(absl::FunctionRef<void(absl::string_view)>) const;


  absl::string_view FlattenSlowPath();





  class InlineRep {
   public:
    static constexpr unsigned char kMaxInline = cord_internal::kMaxInline;
    static_assert(kMaxInline >= sizeof(absl::cord_internal::CordRep*), "");

    constexpr InlineRep() : data_() {}
    explicit InlineRep(InlineData::DefaultInitType init) : data_(init) {}
    InlineRep(const InlineRep& src);
    InlineRep(InlineRep&& src);
    InlineRep& operator=(const InlineRep& src);
    InlineRep& operator=(InlineRep&& src) noexcept;

    explicit constexpr InlineRep(cord_internal::InlineData data);

    void Swap(InlineRep* rhs);
    bool empty() const;
    size_t size() const;
    const char* data() const;  // Returns nullptr if holding pointer
    void set_data(const char* data, size_t n);  // Discards pointer, if any
    char* set_data(size_t n);                   // Write data to the result

    absl::cord_internal::CordRep* tree() const;
    absl::cord_internal::CordRep* as_tree() const;
    const char* as_chars() const;

    absl::cord_internal::CordRep* clear();

    void reduce_size(size_t n);    // REQUIRES: holding data
    void remove_prefix(size_t n);  // REQUIRES: holding data
    void AppendArray(absl::string_view src, MethodIdentifier method);
    absl::string_view FindFlatStartPiece() const;


    CordRepFlat* MakeFlatWithExtraCapacity(size_t extra);





    void SetTree(CordRep* rep, const CordzUpdateScope& scope);


    void SetTreeOrEmpty(CordRep* rep, const CordzUpdateScope& scope);




    void EmplaceTree(CordRep* rep, MethodIdentifier method);


    void EmplaceTree(CordRep* rep, const InlineData& parent,
                     MethodIdentifier method);



    void CommitTree(const CordRep* old_rep, CordRep* rep,
                    const CordzUpdateScope& scope, MethodIdentifier method);

    void AppendTreeToInlined(CordRep* tree, MethodIdentifier method);
    void AppendTreeToTree(CordRep* tree, MethodIdentifier method);
    void AppendTree(CordRep* tree, MethodIdentifier method);
    void PrependTreeToInlined(CordRep* tree, MethodIdentifier method);
    void PrependTreeToTree(CordRep* tree, MethodIdentifier method);
    void PrependTree(CordRep* tree, MethodIdentifier method);

    bool IsSame(const InlineRep& other) const {
      return memcmp(&data_, &other.data_, sizeof(data_)) == 0;
    }
    void CopyTo(std::string* dst) const {



      absl::strings_internal::STLStringResizeUninitialized(dst, kMaxInline);
      memcpy(&(*dst)[0], data_.as_chars(), kMaxInline);


      dst->erase(inline_size());
    }

    void CopyToArray(char* dst) const;

    bool is_tree() const { return data_.is_tree(); }

    bool is_profiled() const { return data_.is_tree() && data_.is_profiled(); }

    size_t remaining_inline_capacity() const {
      return data_.is_tree() ? 0 : kMaxInline - data_.inline_size();
    }

    absl::cord_internal::CordzInfo* cordz_info() const {
      return data_.cordz_info();
    }

    void set_cordz_info(cord_internal::CordzInfo* cordz_info) {
      assert(cordz_info != nullptr);
      data_.set_cordz_info(cordz_info);
    }

    void clear_cordz_info() { data_.clear_cordz_info(); }

   private:
    friend class Cord;

    void AssignSlow(const InlineRep& src);

    void UnrefTree();

    void ResetToEmpty() { data_ = {}; }

    void set_inline_size(size_t size) { data_.set_inline_size(size); }
    size_t inline_size() const { return data_.inline_size(); }





    void MaybeRemoveEmptyCrcNode();

    cord_internal::InlineData data_;
  };
  InlineRep contents_;

  static bool GetFlatAux(absl::cord_internal::CordRep* rep,
                         absl::string_view* fragment);

  static void ForEachChunkAux(
      absl::cord_internal::CordRep* rep,
      absl::FunctionRef<void(absl::string_view)> callback);

  void DestroyCordSlow();

  void CopyToArraySlowPath(char* dst) const;
  int CompareSlowPath(absl::string_view rhs, size_t compared_size,
                      size_t size_to_compare) const;
  int CompareSlowPath(const Cord& rhs, size_t compared_size,
                      size_t size_to_compare) const;
  bool EqualsImpl(absl::string_view rhs, size_t size_to_compare) const;
  bool EqualsImpl(const Cord& rhs, size_t size_to_compare) const;
  int CompareImpl(const Cord& rhs) const;

  template <typename ResultType, typename RHS>
  friend ResultType GenericCompare(const Cord& lhs, const RHS& rhs,
                                   size_t size_to_compare);
  static absl::string_view GetFirstChunk(const Cord& c);
  static absl::string_view GetFirstChunk(absl::string_view sv);


  absl::cord_internal::CordRep* TakeRep() const&;
  absl::cord_internal::CordRep* TakeRep() &&;

  template <typename C>
  void AppendImpl(C&& src);




  void AppendPrecise(absl::string_view src, MethodIdentifier method);
  void PrependPrecise(absl::string_view src, MethodIdentifier method);

  CordBuffer GetAppendBufferSlowPath(size_t block_size, size_t capacity,
                                     size_t min_capacity);


  void PrependArray(absl::string_view src, MethodIdentifier method);


  Cord& AssignLargeString(std::string&& src);

  template <typename H>
  H HashFragmented(H hash_state) const {
    typename H::AbslInternalPiecewiseCombiner combiner;
    ForEachChunk([&combiner, &hash_state](absl::string_view chunk) {
      hash_state = combiner.add_buffer(std::move(hash_state), chunk.data(),
                                       chunk.size());
    });
    return H::combine(combiner.finalize(std::move(hash_state)), size());
  }
};

ABSL_NAMESPACE_END
}  // namespace absl

namespace absl {
ABSL_NAMESPACE_BEGIN

extern std::ostream& operator<<(std::ostream& out, const Cord& cord);

// Internal details follow.  Clients should ignore.

namespace cord_internal {

// safe for overlapping regions. If nullify_tail is true, the destination is
// padded with '\0' up to 15 bytes.
template <bool nullify_tail = false>
inline void SmallMemmove(char* dst, const char* src, size_t n) {
  if (n >= 8) {
    assert(n <= 15);
    uint64_t buf1;
    uint64_t buf2;
    memcpy(&buf1, src, 8);
    memcpy(&buf2, src + n - 8, 8);
    if (nullify_tail) {
      memset(dst + 7, 0, 8);
    }
    memcpy(dst, &buf1, 8);
    memcpy(dst + n - 8, &buf2, 8);
  } else if (n >= 4) {
    uint32_t buf1;
    uint32_t buf2;
    memcpy(&buf1, src, 4);
    memcpy(&buf2, src + n - 4, 4);
    if (nullify_tail) {
      memset(dst + 4, 0, 4);
      memset(dst + 7, 0, 8);
    }
    memcpy(dst, &buf1, 4);
    memcpy(dst + n - 4, &buf2, 4);
  } else {
    if (n != 0) {
      dst[0] = src[0];
      dst[n / 2] = src[n / 2];
      dst[n - 1] = src[n - 1];
    }
    if (nullify_tail) {
      memset(dst + 7, 0, 8);
      memset(dst + n, 0, 8);
    }
  }
}

// Requires `data` to be non-empty.
void InitializeCordRepExternal(absl::string_view data, CordRepExternal* rep);

// to it. Requires `data` to be non-empty.
template <typename Releaser>
// NOLINTNEXTLINE - suppress clang-tidy raw pointer return.
CordRep* NewExternalRep(absl::string_view data, Releaser&& releaser) {
  assert(!data.empty());
  using ReleaserType = absl::decay_t<Releaser>;
  CordRepExternal* rep = new CordRepExternalImpl<ReleaserType>(
      std::forward<Releaser>(releaser), 0);
  InitializeCordRepExternal(data, rep);
  return rep;
}

// pointer because there are no `alignof()` or `sizeof()` a function reference.
// NOLINTNEXTLINE - suppress clang-tidy raw pointer return.
inline CordRep* NewExternalRep(absl::string_view data,
                               void (&releaser)(absl::string_view)) {
  return NewExternalRep(data, &releaser);
}

}  // namespace cord_internal

template <typename Releaser>
Cord MakeCordFromExternal(absl::string_view data, Releaser&& releaser) {
  Cord cord;
  if (ABSL_PREDICT_TRUE(!data.empty())) {
    cord.contents_.EmplaceTree(::absl::cord_internal::NewExternalRep(
                                   data, std::forward<Releaser>(releaser)),
                               Cord::MethodIdentifier::kMakeCordFromExternal);
  } else {
    using ReleaserType = absl::decay_t<Releaser>;
    cord_internal::InvokeReleaser(
        cord_internal::Rank0{}, ReleaserType(std::forward<Releaser>(releaser)),
        data);
  }
  return cord;
}

constexpr Cord::InlineRep::InlineRep(cord_internal::InlineData data)
    : data_(data) {}

inline Cord::InlineRep::InlineRep(const Cord::InlineRep& src)
    : data_(InlineData::kDefaultInit) {
  if (CordRep* tree = src.tree()) {
    EmplaceTree(CordRep::Ref(tree), src.data_,
                CordzUpdateTracker::kConstructorCord);
  } else {
    data_ = src.data_;
  }
}

inline Cord::InlineRep::InlineRep(Cord::InlineRep&& src) : data_(src.data_) {
  src.ResetToEmpty();
}

inline Cord::InlineRep& Cord::InlineRep::operator=(const Cord::InlineRep& src) {
  if (this == &src) {
    return *this;
  }
  if (!is_tree() && !src.is_tree()) {
    data_ = src.data_;
    return *this;
  }
  AssignSlow(src);
  return *this;
}

inline Cord::InlineRep& Cord::InlineRep::operator=(
    Cord::InlineRep&& src) noexcept {
  if (is_tree()) {
    UnrefTree();
  }
  data_ = src.data_;
  src.ResetToEmpty();
  return *this;
}

inline void Cord::InlineRep::Swap(Cord::InlineRep* rhs) {
  if (rhs == this) {
    return;
  }
  std::swap(data_, rhs->data_);
}

inline const char* Cord::InlineRep::data() const {
  return is_tree() ? nullptr : data_.as_chars();
}

inline const char* Cord::InlineRep::as_chars() const {
  assert(!data_.is_tree());
  return data_.as_chars();
}

inline absl::cord_internal::CordRep* Cord::InlineRep::as_tree() const {
  assert(data_.is_tree());
  return data_.as_tree();
}

inline absl::cord_internal::CordRep* Cord::InlineRep::tree() const {
  if (is_tree()) {
    return as_tree();
  } else {
    return nullptr;
  }
}

inline bool Cord::InlineRep::empty() const { return data_.is_empty(); }

inline size_t Cord::InlineRep::size() const {
  return is_tree() ? as_tree()->length : inline_size();
}

inline cord_internal::CordRepFlat* Cord::InlineRep::MakeFlatWithExtraCapacity(
    size_t extra) {
  static_assert(cord_internal::kMinFlatLength >= sizeof(data_), "");
  size_t len = data_.inline_size();
  auto* result = CordRepFlat::New(len + extra);
  result->length = len;
  memcpy(result->Data(), data_.as_chars(), InlineRep::kMaxInline);
  return result;
}

inline void Cord::InlineRep::EmplaceTree(CordRep* rep,
                                         MethodIdentifier method) {
  assert(rep);
  data_.make_tree(rep);
  CordzInfo::MaybeTrackCord(data_, method);
}

inline void Cord::InlineRep::EmplaceTree(CordRep* rep, const InlineData& parent,
                                         MethodIdentifier method) {
  data_.make_tree(rep);
  CordzInfo::MaybeTrackCord(data_, parent, method);
}

inline void Cord::InlineRep::SetTree(CordRep* rep,
                                     const CordzUpdateScope& scope) {
  assert(rep);
  assert(data_.is_tree());
  data_.set_tree(rep);
  scope.SetCordRep(rep);
}

inline void Cord::InlineRep::SetTreeOrEmpty(CordRep* rep,
                                            const CordzUpdateScope& scope) {
  assert(data_.is_tree());
  if (rep) {
    data_.set_tree(rep);
  } else {
    data_ = {};
  }
  scope.SetCordRep(rep);
}

inline void Cord::InlineRep::CommitTree(const CordRep* old_rep, CordRep* rep,
                                        const CordzUpdateScope& scope,
                                        MethodIdentifier method) {
  if (old_rep) {
    SetTree(rep, scope);
  } else {
    EmplaceTree(rep, method);
  }
}

inline absl::cord_internal::CordRep* Cord::InlineRep::clear() {
  if (is_tree()) {
    CordzInfo::MaybeUntrackCord(cordz_info());
  }
  absl::cord_internal::CordRep* result = tree();
  ResetToEmpty();
  return result;
}

inline void Cord::InlineRep::CopyToArray(char* dst) const {
  assert(!is_tree());
  size_t n = inline_size();
  assert(n != 0);
  cord_internal::SmallMemmove(dst, data_.as_chars(), n);
}

inline void Cord::InlineRep::MaybeRemoveEmptyCrcNode() {
  CordRep* rep = tree();
  if (rep == nullptr || ABSL_PREDICT_TRUE(rep->length > 0)) {
    return;
  }
  assert(rep->IsCrc());
  assert(rep->crc()->child == nullptr);
  CordzInfo::MaybeUntrackCord(cordz_info());
  CordRep::Unref(rep);
  ResetToEmpty();
}

constexpr inline Cord::Cord() noexcept {}

inline Cord::Cord(absl::string_view src)
    : Cord(src, CordzUpdateTracker::kConstructorString) {}

template <typename T>
constexpr Cord::Cord(strings_internal::StringConstant<T>)
    : contents_(strings_internal::StringConstant<T>::value.size() <=
                        cord_internal::kMaxInline
                    ? cord_internal::InlineData(
                          strings_internal::StringConstant<T>::value)
                    : cord_internal::InlineData(
                          &cord_internal::ConstInitExternalStorage<
                              strings_internal::StringConstant<T>>::value)) {}

inline Cord& Cord::operator=(const Cord& x) {
  contents_ = x.contents_;
  return *this;
}

template <typename T, Cord::EnableIfString<T>>
Cord& Cord::operator=(T&& src) {
  if (src.size() <= cord_internal::kMaxBytesToCopy) {
    return operator=(absl::string_view(src));
  } else {
    return AssignLargeString(std::forward<T>(src));
  }
}

inline Cord::Cord(const Cord& src) : contents_(src.contents_) {}

inline Cord::Cord(Cord&& src) noexcept : contents_(std::move(src.contents_)) {}

inline void Cord::swap(Cord& other) noexcept {
  contents_.Swap(&other.contents_);
}

inline Cord& Cord::operator=(Cord&& x) noexcept {
  contents_ = std::move(x.contents_);
  return *this;
}

extern template Cord::Cord(std::string&& src);

inline size_t Cord::size() const {

  return contents_.size();
}

inline bool Cord::empty() const { return size() == 0; }

inline size_t Cord::EstimatedMemoryUsage(
    CordMemoryAccounting accounting_method) const {
  size_t result = sizeof(Cord);
  if (const absl::cord_internal::CordRep* rep = contents_.tree()) {
    if (accounting_method == CordMemoryAccounting::kFairShare) {
      result += cord_internal::GetEstimatedFairShareMemoryUsage(rep);
    } else {
      result += cord_internal::GetEstimatedMemoryUsage(rep);
    }
  }
  return result;
}

inline absl::optional<absl::string_view> Cord::TryFlat() const {
  absl::cord_internal::CordRep* rep = contents_.tree();
  if (rep == nullptr) {
    return absl::string_view(contents_.data(), contents_.size());
  }
  absl::string_view fragment;
  if (GetFlatAux(rep, &fragment)) {
    return fragment;
  }
  return absl::nullopt;
}

inline absl::string_view Cord::Flatten() {
  absl::cord_internal::CordRep* rep = contents_.tree();
  if (rep == nullptr) {
    return absl::string_view(contents_.data(), contents_.size());
  } else {
    absl::string_view already_flat_contents;
    if (GetFlatAux(rep, &already_flat_contents)) {
      return already_flat_contents;
    }
  }
  return FlattenSlowPath();
}

inline void Cord::Append(absl::string_view src) {
  contents_.AppendArray(src, CordzUpdateTracker::kAppendString);
}

inline void Cord::Prepend(absl::string_view src) {
  PrependArray(src, CordzUpdateTracker::kPrependString);
}

inline void Cord::Append(CordBuffer buffer) {
  if (ABSL_PREDICT_FALSE(buffer.length() == 0)) return;
  absl::string_view short_value;
  if (CordRep* rep = buffer.ConsumeValue(short_value)) {
    contents_.AppendTree(rep, CordzUpdateTracker::kAppendCordBuffer);
  } else {
    AppendPrecise(short_value, CordzUpdateTracker::kAppendCordBuffer);
  }
}

inline void Cord::Prepend(CordBuffer buffer) {
  if (ABSL_PREDICT_FALSE(buffer.length() == 0)) return;
  absl::string_view short_value;
  if (CordRep* rep = buffer.ConsumeValue(short_value)) {
    contents_.PrependTree(rep, CordzUpdateTracker::kPrependCordBuffer);
  } else {
    PrependPrecise(short_value, CordzUpdateTracker::kPrependCordBuffer);
  }
}

inline CordBuffer Cord::GetAppendBuffer(size_t capacity, size_t min_capacity) {
  if (empty()) return CordBuffer::CreateWithDefaultLimit(capacity);
  return GetAppendBufferSlowPath(0, capacity, min_capacity);
}

inline CordBuffer Cord::GetCustomAppendBuffer(size_t block_size,
                                              size_t capacity,
                                              size_t min_capacity) {
  if (empty()) {
    return block_size ? CordBuffer::CreateWithCustomLimit(block_size, capacity)
                      : CordBuffer::CreateWithDefaultLimit(capacity);
  }
  return GetAppendBufferSlowPath(block_size, capacity, min_capacity);
}

extern template void Cord::Append(std::string&& src);
extern template void Cord::Prepend(std::string&& src);

inline int Cord::Compare(const Cord& rhs) const {
  if (!contents_.is_tree() && !rhs.contents_.is_tree()) {
    return contents_.data_.Compare(rhs.contents_.data_);
  }

  return CompareImpl(rhs);
}

inline bool Cord::StartsWith(const Cord& rhs) const {
  if (contents_.IsSame(rhs.contents_)) return true;
  size_t rhs_size = rhs.size();
  if (size() < rhs_size) return false;
  return EqualsImpl(rhs, rhs_size);
}

inline bool Cord::StartsWith(absl::string_view rhs) const {
  size_t rhs_size = rhs.size();
  if (size() < rhs_size) return false;
  return EqualsImpl(rhs, rhs_size);
}

inline void Cord::ChunkIterator::InitTree(cord_internal::CordRep* tree) {
  tree = cord_internal::SkipCrcNode(tree);
  if (tree->tag == cord_internal::BTREE) {
    current_chunk_ = btree_reader_.Init(tree->btree());
  } else {
    current_leaf_ = tree;
    current_chunk_ = cord_internal::EdgeData(tree);
  }
}

inline Cord::ChunkIterator::ChunkIterator(cord_internal::CordRep* tree) {
  bytes_remaining_ = tree->length;
  InitTree(tree);
}

inline Cord::ChunkIterator::ChunkIterator(const Cord* cord) {
  if (CordRep* tree = cord->contents_.tree()) {
    bytes_remaining_ = tree->length;
    if (ABSL_PREDICT_TRUE(bytes_remaining_ != 0)) {
      InitTree(tree);
    } else {
      current_chunk_ = {};
    }
  } else {
    bytes_remaining_ = cord->contents_.inline_size();
    current_chunk_ = {cord->contents_.data(), bytes_remaining_};
  }
}

inline Cord::ChunkIterator& Cord::ChunkIterator::AdvanceBtree() {
  current_chunk_ = btree_reader_.Next();
  return *this;
}

inline void Cord::ChunkIterator::AdvanceBytesBtree(size_t n) {
  assert(n >= current_chunk_.size());
  bytes_remaining_ -= n;
  if (bytes_remaining_) {
    if (n == current_chunk_.size()) {
      current_chunk_ = btree_reader_.Next();
    } else {
      size_t offset = btree_reader_.length() - bytes_remaining_;
      current_chunk_ = btree_reader_.Seek(offset);
    }
  } else {
    current_chunk_ = {};
  }
}

inline Cord::ChunkIterator& Cord::ChunkIterator::operator++() {
  ABSL_HARDENING_ASSERT(bytes_remaining_ > 0 &&
                        "Attempted to iterate past `end()`");
  assert(bytes_remaining_ >= current_chunk_.size());
  bytes_remaining_ -= current_chunk_.size();
  if (bytes_remaining_ > 0) {
    if (btree_reader_) {
      return AdvanceBtree();
    } else {
      assert(!current_chunk_.empty());  // Called on invalid iterator.
    }
    current_chunk_ = {};
  }
  return *this;
}

inline Cord::ChunkIterator Cord::ChunkIterator::operator++(int) {
  ChunkIterator tmp(*this);
  operator++();
  return tmp;
}

inline bool Cord::ChunkIterator::operator==(const ChunkIterator& other) const {
  return bytes_remaining_ == other.bytes_remaining_;
}

inline bool Cord::ChunkIterator::operator!=(const ChunkIterator& other) const {
  return !(*this == other);
}

inline Cord::ChunkIterator::reference Cord::ChunkIterator::operator*() const {
  ABSL_HARDENING_ASSERT(bytes_remaining_ != 0);
  return current_chunk_;
}

inline Cord::ChunkIterator::pointer Cord::ChunkIterator::operator->() const {
  ABSL_HARDENING_ASSERT(bytes_remaining_ != 0);
  return &current_chunk_;
}

inline void Cord::ChunkIterator::RemoveChunkPrefix(size_t n) {
  assert(n < current_chunk_.size());
  current_chunk_.remove_prefix(n);
  bytes_remaining_ -= n;
}

inline void Cord::ChunkIterator::AdvanceBytes(size_t n) {
  assert(bytes_remaining_ >= n);
  if (ABSL_PREDICT_TRUE(n < current_chunk_.size())) {
    RemoveChunkPrefix(n);
  } else if (n != 0) {
    if (btree_reader_) {
      AdvanceBytesBtree(n);
    } else {
      bytes_remaining_ = 0;
    }
  }
}

inline Cord::ChunkIterator Cord::chunk_begin() const {
  return ChunkIterator(this);
}

inline Cord::ChunkIterator Cord::chunk_end() const { return ChunkIterator(); }

inline Cord::ChunkIterator Cord::ChunkRange::begin() const {
  return cord_->chunk_begin();
}

inline Cord::ChunkIterator Cord::ChunkRange::end() const {
  return cord_->chunk_end();
}

inline Cord::ChunkRange Cord::Chunks() const { return ChunkRange(this); }

inline Cord::CharIterator& Cord::CharIterator::operator++() {
  if (ABSL_PREDICT_TRUE(chunk_iterator_->size() > 1)) {
    chunk_iterator_.RemoveChunkPrefix(1);
  } else {
    ++chunk_iterator_;
  }
  return *this;
}

inline Cord::CharIterator Cord::CharIterator::operator++(int) {
  CharIterator tmp(*this);
  operator++();
  return tmp;
}

inline bool Cord::CharIterator::operator==(const CharIterator& other) const {
  return chunk_iterator_ == other.chunk_iterator_;
}

inline bool Cord::CharIterator::operator!=(const CharIterator& other) const {
  return !(*this == other);
}

inline Cord::CharIterator::reference Cord::CharIterator::operator*() const {
  return *chunk_iterator_->data();
}

inline Cord::CharIterator::pointer Cord::CharIterator::operator->() const {
  return chunk_iterator_->data();
}

inline Cord Cord::AdvanceAndRead(CharIterator* it, size_t n_bytes) {
  assert(it != nullptr);
  return it->chunk_iterator_.AdvanceAndReadBytes(n_bytes);
}

inline void Cord::Advance(CharIterator* it, size_t n_bytes) {
  assert(it != nullptr);
  it->chunk_iterator_.AdvanceBytes(n_bytes);
}

inline absl::string_view Cord::ChunkRemaining(const CharIterator& it) {
  return *it.chunk_iterator_;
}

inline Cord::CharIterator Cord::char_begin() const {
  return CharIterator(this);
}

inline Cord::CharIterator Cord::char_end() const { return CharIterator(); }

inline Cord::CharIterator Cord::CharRange::begin() const {
  return cord_->char_begin();
}

inline Cord::CharIterator Cord::CharRange::end() const {
  return cord_->char_end();
}

inline Cord::CharRange Cord::Chars() const { return CharRange(this); }

inline void Cord::ForEachChunk(
    absl::FunctionRef<void(absl::string_view)> callback) const {
  absl::cord_internal::CordRep* rep = contents_.tree();
  if (rep == nullptr) {
    callback(absl::string_view(contents_.data(), contents_.size()));
  } else {
    ForEachChunkAux(rep, callback);
  }
}

inline bool operator==(const Cord& lhs, const Cord& rhs) {
  if (lhs.contents_.IsSame(rhs.contents_)) return true;
  size_t rhs_size = rhs.size();
  if (lhs.size() != rhs_size) return false;
  return lhs.EqualsImpl(rhs, rhs_size);
}

inline bool operator!=(const Cord& x, const Cord& y) { return !(x == y); }
inline bool operator<(const Cord& x, const Cord& y) { return x.Compare(y) < 0; }
inline bool operator>(const Cord& x, const Cord& y) { return x.Compare(y) > 0; }
inline bool operator<=(const Cord& x, const Cord& y) {
  return x.Compare(y) <= 0;
}
inline bool operator>=(const Cord& x, const Cord& y) {
  return x.Compare(y) >= 0;
}

//
// Due to implicit conversions, these also enable comparisons of Cord with
// with std::string, ::string, and const char*.
inline bool operator==(const Cord& lhs, absl::string_view rhs) {
  size_t lhs_size = lhs.size();
  size_t rhs_size = rhs.size();
  if (lhs_size != rhs_size) return false;
  return lhs.EqualsImpl(rhs, rhs_size);
}

inline bool operator==(absl::string_view x, const Cord& y) { return y == x; }
inline bool operator!=(const Cord& x, absl::string_view y) { return !(x == y); }
inline bool operator!=(absl::string_view x, const Cord& y) { return !(x == y); }
inline bool operator<(const Cord& x, absl::string_view y) {
  return x.Compare(y) < 0;
}
inline bool operator<(absl::string_view x, const Cord& y) {
  return y.Compare(x) > 0;
}
inline bool operator>(const Cord& x, absl::string_view y) { return y < x; }
inline bool operator>(absl::string_view x, const Cord& y) { return y < x; }
inline bool operator<=(const Cord& x, absl::string_view y) { return !(y < x); }
inline bool operator<=(absl::string_view x, const Cord& y) { return !(y < x); }
inline bool operator>=(const Cord& x, absl::string_view y) { return !(x < y); }
inline bool operator>=(absl::string_view x, const Cord& y) { return !(x < y); }

namespace strings_internal {
class CordTestAccess {
 public:
  static size_t FlatOverhead();
  static size_t MaxFlatLength();
  static size_t SizeofCordRepExternal();
  static size_t SizeofCordRepSubstring();
  static size_t FlatTagToLength(uint8_t tag);
  static uint8_t LengthToTag(size_t s);
};
}  // namespace strings_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STRINGS_CORD_H_
