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

#ifndef ABSL_STRINGS_INTERNAL_CORD_REP_RING_H_
#define ABSL_STRINGS_INTERNAL_CORD_REP_RING_H_

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <limits>
#include <memory>

#include "absl/container/internal/layout.h"
#include "absl/strings/internal/cord_internal.h"
#include "absl/strings/internal/cord_rep_flat.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace cord_internal {

// requiring a CordRepRing instance with a reference adopted by the method.
//
// The methods return the modified ring buffer, which may be equal to the input
// if the input was not shared, and having large enough capacity to accommodate
// any newly added node(s). Otherwise, a copy of the input rep with the new
// node(s) added is returned.
//
// Any modification on non shared ring buffers with enough capacity will then
// require minimum atomic operations. Caller should where possible provide
// reasonable `extra` hints for both anticipated extra `flat` byte space, as
// well as anticipated extra nodes required for complex operations.
//
// Example of code creating a ring buffer, adding some data to it,
// and discarding the buffer when done:
//
//   void FunWithRings() {
//     // Create ring with 3 flats
//     CordRep* flat = CreateFlat("Hello");
//     CordRepRing* ring = CordRepRing::Create(flat, 2);
//     ring = CordRepRing::Append(ring, CreateFlat(" "));
//     ring = CordRepRing::Append(ring, CreateFlat("world"));
//     DoSomethingWithRing(ring);
//     CordRep::Unref(ring);
//   }
//
// Example of code Copying an existing ring buffer and modifying it:
//
//   void MoreFunWithRings(CordRepRing* src) {
//     CordRepRing* ring = CordRep::Ref(src)->ring();
//     ring = CordRepRing::Append(ring, CreateFlat("Hello"));
//     ring = CordRepRing::Append(ring, CreateFlat(" "));
//     ring = CordRepRing::Append(ring, CreateFlat("world"));
//     DoSomethingWithRing(ring);
//     CordRep::Unref(ring);
//   }
//
class CordRepRing : public CordRep {
 public:





  using pos_type = size_t;


  using index_type = uint32_t;

  using offset_type = uint32_t;



  struct Position {
    index_type index;
    size_t offset;
  };

  static constexpr size_t kMaxCapacity = (std::numeric_limits<uint32_t>::max)();

  CordRepRing() = delete;
  CordRepRing(const CordRepRing&) = delete;
  CordRepRing& operator=(const CordRepRing&) = delete;



  bool IsValid(std::ostream& output) const;

  static constexpr size_t AllocSize(size_t capacity);

  static constexpr size_t Distance(pos_type pos, pos_type end_pos);


  static CordRepRing* Create(CordRep* child, size_t extra = 0);

  index_type head() const { return head_; }
  index_type tail() const { return tail_; }
  index_type capacity() const { return capacity_; }

  index_type entries() const { return entries(head_, tail_); }

  pos_type begin_pos() const { return begin_pos_; }


  index_type entries(index_type head, index_type tail) const {
    assert(head < capacity_ && tail < capacity_);
    return tail - head + ((tail > head) ? 0 : capacity_);
  }

  pos_type const& entry_end_pos(index_type index) const {
    assert(IsValidIndex(index));
    return Layout::Partial().Pointer<0>(data_)[index];
  }

  CordRep* const& entry_child(index_type index) const {
    assert(IsValidIndex(index));
    return Layout::Partial(capacity()).Pointer<1>(data_)[index];
  }

  offset_type const& entry_data_offset(index_type index) const {
    assert(IsValidIndex(index));
    return Layout::Partial(capacity(), capacity()).Pointer<2>(data_)[index];
  }








  static CordRepRing* Append(CordRepRing* rep, CordRep* child);






  static CordRepRing* Append(CordRepRing* rep, string_view data,
                             size_t extra = 0);








  static CordRepRing* Prepend(CordRepRing* rep, CordRep* child);






  static CordRepRing* Prepend(CordRepRing* rep, string_view data,
                              size_t extra = 0);






  Span<char> GetAppendBuffer(size_t size);



  Span<char> GetPrependBuffer(size_t size);





  static CordRepRing* SubRing(CordRepRing* r, size_t offset, size_t len,
                              size_t extra = 0);





  static CordRepRing* RemoveSuffix(CordRepRing* r, size_t len,
                                   size_t extra = 0);





  static CordRepRing* RemovePrefix(CordRepRing* r, size_t len,
                                   size_t extra = 0);

  char GetCharacter(size_t offset) const;



  bool IsFlat(absl::string_view* fragment) const;





  bool IsFlat(size_t offset, size_t len, absl::string_view* fragment) const;

  void SetCapacityForTesting(size_t capacity);


  static const char* GetLeafData(const CordRep* rep);


  static const char* GetRepData(const CordRep* rep);


  inline index_type advance(index_type index) const;


  inline index_type advance(index_type index, index_type n) const;


  inline index_type retreat(index_type index) const;


  inline index_type retreat(index_type index, index_type n) const;

  pos_type const& entry_begin_pos(index_type index) const {
    return (index == head_) ? begin_pos_ : entry_end_pos(retreat(index));
  }

  size_t entry_start_offset(index_type index) const {
    return Distance(begin_pos_, entry_begin_pos(index));
  }

  size_t entry_end_offset(index_type index) const {
    return Distance(begin_pos_, entry_end_pos(index));
  }

  size_t entry_length(index_type index) const {
    return Distance(entry_begin_pos(index), entry_end_pos(index));
  }

  absl::string_view entry_data(index_type index) const;










  inline Position Find(size_t offset) const;

  inline Position Find(index_type head, size_t offset) const;


















  inline Position FindTail(size_t offset) const;

  inline Position FindTail(index_type head, size_t offset) const;

  template <typename F>
  void ForEach(index_type head, index_type tail, F&& f) const {
    index_type n1 = (tail > head) ? tail : capacity_;
    for (index_type i = head; i < n1; ++i) f(i);
    if (tail <= head) {
      for (index_type i = 0; i < tail; ++i) f(i);
    }
  }

  template <typename F>
  void ForEach(F&& f) const {
    ForEach(head_, tail_, std::forward<F>(f));
  }


  friend std::ostream& operator<<(std::ostream& s, const CordRepRing& rep);

 private:
  enum class AddMode { kAppend, kPrepend };

  using Layout = container_internal::Layout<pos_type, CordRep*, offset_type>;

  class Filler;
  class Transaction;
  class CreateTransaction;

  static constexpr size_t kLayoutAlignment = Layout::Partial().Alignment();

  explicit CordRepRing(index_type capacity) : capacity_(capacity) {}

  bool IsValidIndex(index_type index) const;




  static CordRepRing* Validate(CordRepRing* rep, const char* file = nullptr,
                               int line = 0);




  static CordRepRing* New(size_t capacity, size_t extra);

  static void Delete(CordRepRing* rep);




  static void Destroy(CordRepRing* rep);

  pos_type* entry_end_pos() {
    return Layout::Partial().Pointer<0>(data_);
  }

  CordRep** entry_child() {
    return Layout::Partial(capacity()).Pointer<1>(data_);
  }

  offset_type* entry_data_offset() {
    return Layout::Partial(capacity(), capacity()).Pointer<2>(data_);
  }

  Position FindSlow(index_type head, size_t offset) const;
  Position FindTailSlow(index_type head, size_t offset) const;


  template <bool wrap>
  index_type FindBinary(index_type head, index_type tail, size_t offset) const;


  template <bool ref>
  void Fill(const CordRepRing* src, index_type head, index_type tail);


  static CordRepRing* Copy(CordRepRing* rep, index_type head, index_type tail,
                           size_t extra = 0);











  static CordRepRing* Mutable(CordRepRing* rep, size_t extra);



  static CordRepRing* AppendSlow(CordRepRing* rep, CordRep* child);

  static CordRepRing* AppendLeaf(CordRepRing* rep, CordRep* child,
                                 size_t offset, size_t length);

  static CordRepRing* PrependLeaf(CordRepRing* rep, CordRep* child,
                                  size_t offset, size_t length);



  static CordRepRing* PrependSlow(CordRepRing* rep, CordRep* child);



  static CordRepRing* CreateSlow(CordRep* child, size_t extra);



  static CordRepRing* CreateFromLeaf(CordRep* child, size_t offset,
                                     size_t length, size_t extra);


  template <AddMode mode>
  static CordRepRing* AddRing(CordRepRing* rep, CordRepRing* ring,
                              size_t offset, size_t len);

  void AddDataOffset(index_type index, size_t n);

  void SubLength(index_type index, size_t n);

  index_type head_;
  index_type tail_;
  index_type capacity_;
  pos_type begin_pos_;

  alignas(kLayoutAlignment) char data_[kLayoutAlignment];

  friend struct CordRep;
};

constexpr size_t CordRepRing::AllocSize(size_t capacity) {
  return sizeof(CordRepRing) - sizeof(data_) +
         Layout(capacity, capacity, capacity).AllocSize();
}

inline constexpr size_t CordRepRing::Distance(pos_type pos, pos_type end_pos) {
  return (end_pos - pos);
}

inline const char* CordRepRing::GetLeafData(const CordRep* rep) {
  return rep->tag != EXTERNAL ? rep->flat()->Data() : rep->external()->base;
}

inline const char* CordRepRing::GetRepData(const CordRep* rep) {
  if (rep->tag >= FLAT) return rep->flat()->Data();
  if (rep->tag == EXTERNAL) return rep->external()->base;
  return GetLeafData(rep->substring()->child) + rep->substring()->start;
}

inline CordRepRing::index_type CordRepRing::advance(index_type index) const {
  assert(index < capacity_);
  return ++index == capacity_ ? 0 : index;
}

inline CordRepRing::index_type CordRepRing::advance(index_type index,
                                                    index_type n) const {
  assert(index < capacity_ && n <= capacity_);
  return (index += n) >= capacity_ ? index - capacity_ : index;
}

inline CordRepRing::index_type CordRepRing::retreat(index_type index) const {
  assert(index < capacity_);
  return (index > 0 ? index : capacity_) - 1;
}

inline CordRepRing::index_type CordRepRing::retreat(index_type index,
                                                    index_type n) const {
  assert(index < capacity_ && n <= capacity_);
  return index >= n ? index - n : capacity_ - n + index;
}

inline absl::string_view CordRepRing::entry_data(index_type index) const {
  size_t data_offset = entry_data_offset(index);
  return {GetRepData(entry_child(index)) + data_offset, entry_length(index)};
}

inline bool CordRepRing::IsValidIndex(index_type index) const {
  if (index >= capacity_) return false;
  return (tail_ > head_) ? (index >= head_ && index < tail_)
                         : (index >= head_ || index < tail_);
}

#ifndef EXTRA_CORD_RING_VALIDATION
inline CordRepRing* CordRepRing::Validate(CordRepRing* rep,
                                          const char* /*file*/, int /*line*/) {
  return rep;
}
#endif

inline CordRepRing::Position CordRepRing::Find(size_t offset) const {
  assert(offset < length);
  return (offset == 0) ? Position{head_, 0} : FindSlow(head_, offset);
}

inline CordRepRing::Position CordRepRing::Find(index_type head,
                                               size_t offset) const {
  assert(offset < length);
  assert(IsValidIndex(head) && offset >= entry_start_offset(head));
  return (offset == 0) ? Position{head_, 0} : FindSlow(head, offset);
}

inline CordRepRing::Position CordRepRing::FindTail(size_t offset) const {
  assert(offset > 0 && offset <= length);
  return (offset == length) ? Position{tail_, 0} : FindTailSlow(head_, offset);
}

inline CordRepRing::Position CordRepRing::FindTail(index_type head,
                                                   size_t offset) const {
  assert(offset > 0 && offset <= length);
  assert(IsValidIndex(head) && offset >= entry_start_offset(head) + 1);
  return (offset == length) ? Position{tail_, 0} : FindTailSlow(head, offset);
}

inline CordRepRing* CordRep::ring() {
  assert(IsRing());
  return static_cast<CordRepRing*>(this);
}

inline const CordRepRing* CordRep::ring() const {
  assert(IsRing());
  return static_cast<const CordRepRing*>(this);
}

inline bool CordRepRing::IsFlat(absl::string_view* fragment) const {
  if (entries() == 1) {
    if (fragment) *fragment = entry_data(head());
    return true;
  }
  return false;
}

inline bool CordRepRing::IsFlat(size_t offset, size_t len,
                                absl::string_view* fragment) const {
  const Position pos = Find(offset);
  const absl::string_view data = entry_data(pos.index);
  if (data.length() >= len && data.length() - len >= pos.offset) {
    if (fragment) *fragment = data.substr(pos.offset, len);
    return true;
  }
  return false;
}

std::ostream& operator<<(std::ostream& s, const CordRepRing& rep);

}  // namespace cord_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STRINGS_INTERNAL_CORD_REP_RING_H_
