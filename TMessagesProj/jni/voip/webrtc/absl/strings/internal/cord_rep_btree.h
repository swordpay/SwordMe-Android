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

#ifndef ABSL_STRINGS_INTERNAL_CORD_REP_BTREE_H_
#define ABSL_STRINGS_INTERNAL_CORD_REP_BTREE_H_

#include <cassert>
#include <cstdint>
#include <iosfwd>

#include "absl/base/config.h"
#include "absl/base/internal/raw_logging.h"
#include "absl/base/optimization.h"
#include "absl/strings/internal/cord_data_edge.h"
#include "absl/strings/internal/cord_internal.h"
#include "absl/strings/internal/cord_rep_flat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace cord_internal {

class CordRepBtreeNavigator;

// Data is stored at the leaf level only, non leaf nodes contain down pointers
// only. Allowed types of data edges are FLAT, EXTERNAL and SUBSTRINGs of FLAT
// or EXTERNAL nodes. The implementation allows for data to be added to either
// end of the tree only, it does not provide any 'insert' logic. This has the
// benefit that we can expect good fill ratios: all nodes except the outer
// 'legs' will have 100% fill ratios for trees built using Append/Prepend
// methods. Merged trees will typically have a fill ratio well above 50% as in a
// similar fashion, one side of the merged tree will typically have a 100% fill
// ratio, and the 'open' end will average 50%. All operations are O(log(n)) or
// better, and the tree never needs balancing.
//
// All methods accepting a CordRep* or CordRepBtree* adopt a reference on that
// input unless explicitly stated otherwise. All functions returning a CordRep*
// or CordRepBtree* instance transfer a reference back to the caller.
// Simplified, callers both 'donate' and 'consume' a reference count on each
// call, simplifying the API. An example of building a tree:
//
//   CordRepBtree* tree = CordRepBtree::Create(MakeFlat("Hello"));
//   tree = CordRepBtree::Append(tree, MakeFlat("world"));
//
// In the above example, all inputs are consumed, making each call affecting
// `tree` reference count neutral. The returned `tree` value can be different
// from the input if the input is shared with other threads, or if the tree
// grows in height, but callers typically never have to concern themselves with
// that and trust that all methods DTRT at all times.
class CordRepBtree : public CordRep {
 public:





  enum class EdgeType { kFront, kBack };

  static constexpr EdgeType kFront = EdgeType::kFront;
  static constexpr EdgeType kBack = EdgeType::kBack;





  static constexpr size_t kMaxCapacity = 6;















  static constexpr size_t kMaxDepth = 12;

  static constexpr int kMaxHeight = static_cast<int>(kMaxDepth - 1);





























  enum Action { kSelf, kCopied, kPopped };

  struct OpResult {
    CordRepBtree* tree;
    Action action;
  };




  struct CopyResult {
    CordRep* edge;
    int height;
  };



  struct Position {
    size_t index;
    size_t n;
  };






  static CordRepBtree* Create(CordRep* rep);


  static void Destroy(CordRepBtree* tree);

  static void Delete(CordRepBtree* tree) { delete tree; }

  using CordRep::Unref;

  static void Unref(absl::Span<CordRep* const> edges);









  static CordRepBtree* Append(CordRepBtree* tree, CordRep* rep);
  static CordRepBtree* Prepend(CordRepBtree* tree, CordRep* rep);











  static CordRepBtree* Append(CordRepBtree* tree, string_view data,
                              size_t extra = 0);
  static CordRepBtree* Prepend(CordRepBtree* tree, string_view data,
                               size_t extra = 0);





  CordRep* SubTree(size_t offset, size_t n);











  static CordRep* RemoveSuffix(CordRepBtree* tree, size_t n);

  char GetCharacter(size_t offset) const;



  bool IsFlat(absl::string_view* fragment) const;




  bool IsFlat(size_t offset, size_t n, absl::string_view* fragment) const;












  Span<char> GetAppendBuffer(size_t size);



























  static ExtractResult ExtractAppendBuffer(CordRepBtree* tree,
                                           size_t extra_capacity = 1);



  int height() const { return static_cast<int>(storage[0]); }

  size_t begin() const { return static_cast<size_t>(storage[1]); }
  size_t back() const { return static_cast<size_t>(storage[2]) - 1; }
  size_t end() const { return static_cast<size_t>(storage[2]); }
  size_t index(EdgeType edge) const {
    return edge == kFront ? begin() : back();
  }






  size_t size() const { return end() - begin(); }
  size_t capacity() const { return kMaxCapacity; }

  inline CordRep* Edge(size_t index) const;
  inline CordRep* Edge(EdgeType edge_type) const;
  inline absl::Span<CordRep* const> Edges() const;
  inline absl::Span<CordRep* const> Edges(size_t begin, size_t end) const;


  inline absl::string_view Data(size_t index) const;








  static bool IsValid(const CordRepBtree* tree, bool shallow = false);






  static CordRepBtree* AssertValid(CordRepBtree* tree, bool shallow = true);
  static const CordRepBtree* AssertValid(const CordRepBtree* tree,
                                         bool shallow = true);


  static void Dump(const CordRep* rep, std::ostream& stream);
  static void Dump(const CordRep* rep, absl::string_view label,
                   std::ostream& stream);
  static void Dump(const CordRep* rep, absl::string_view label,
                   bool include_contents, std::ostream& stream);








  template <EdgeType edge_type>
  inline OpResult AddEdge(bool owned, CordRep* edge, size_t delta);






  template <EdgeType edge_type>
  OpResult SetEdge(bool owned, CordRep* edge, size_t delta);

  static CordRepBtree* New(int height = 0);


  static CordRepBtree* New(CordRep* rep);


  static CordRepBtree* New(CordRepBtree* front, CordRepBtree* back);



  static CordRepBtree* Rebuild(CordRepBtree* tree);

 private:
  CordRepBtree() = default;
  ~CordRepBtree() = default;

  inline void InitInstance(int height, size_t begin = 0, size_t end = 0);

  void set_begin(size_t begin) { storage[1] = static_cast<uint8_t>(begin); }
  void set_end(size_t end) { storage[2] = static_cast<uint8_t>(end); }



  size_t sub_fetch_begin(size_t n) {
    storage[1] -= static_cast<uint8_t>(n);
    return storage[1];
  }


  size_t fetch_add_end(size_t n) {
    const uint8_t current = storage[2];
    storage[2] = static_cast<uint8_t>(current + n);
    return current;
  }



  Position IndexOf(size_t offset) const;










  Position IndexBefore(size_t offset) const;




  Position IndexOfLength(size_t n) const;



  Position IndexBefore(Position front, size_t offset) const;





  Position IndexBeyond(size_t offset) const;





  template <EdgeType edge_type>
  static CordRepBtree* NewLeaf(absl::string_view data, size_t extra);


  CordRepBtree* CopyRaw() const;

  CordRepBtree* Copy() const;



  CordRepBtree* CopyBeginTo(size_t end, size_t new_length) const;






  static CordRepBtree* ConsumeBeginTo(CordRepBtree* tree, size_t end,
                                      size_t new_length);



  CordRepBtree* CopyToEndFrom(size_t begin, size_t new_length) const;






  static CordRep* ExtractFront(CordRepBtree* tree);

  static CordRepBtree* MergeTrees(CordRepBtree* left, CordRepBtree* right);


  static CordRepBtree* CreateSlow(CordRep* rep);
  static CordRepBtree* AppendSlow(CordRepBtree*, CordRep* rep);
  static CordRepBtree* PrependSlow(CordRepBtree*, CordRep* rep);




  static void Rebuild(CordRepBtree** stack, CordRepBtree* tree, bool consume);


  inline void AlignBegin();


  inline void AlignEnd();



  template <EdgeType edge_type>
  inline void Add(CordRep* rep);



  template <EdgeType edge_type>
  inline void Add(absl::Span<CordRep* const>);








  template <EdgeType edge_type>
  absl::string_view AddData(absl::string_view data, size_t extra);


  template <EdgeType edge_type>
  inline void SetEdge(CordRep* edge);







  CopyResult CopyPrefix(size_t n, bool allow_folding = true);







  CopyResult CopySuffix(size_t offset);


  inline OpResult ToOpResult(bool owned);

  template <EdgeType edge_type>
  static CordRepBtree* AddCordRep(CordRepBtree* tree, CordRep* rep);


  template <EdgeType edge_type>
  static CordRepBtree* AddData(CordRepBtree* tree, absl::string_view data,
                               size_t extra = 0);



  template <EdgeType edge_type>
  static CordRepBtree* Merge(CordRepBtree* dst, CordRepBtree* src);



  Span<char> GetAppendBufferSlow(size_t size);







  CordRep* edges_[kMaxCapacity];

  friend class CordRepBtreeTestPeer;
  friend class CordRepBtreeNavigator;
};

inline CordRepBtree* CordRep::btree() {
  assert(IsBtree());
  return static_cast<CordRepBtree*>(this);
}

inline const CordRepBtree* CordRep::btree() const {
  assert(IsBtree());
  return static_cast<const CordRepBtree*>(this);
}

inline void CordRepBtree::InitInstance(int height, size_t begin, size_t end) {
  tag = BTREE;
  storage[0] = static_cast<uint8_t>(height);
  storage[1] = static_cast<uint8_t>(begin);
  storage[2] = static_cast<uint8_t>(end);
}

inline CordRep* CordRepBtree::Edge(size_t index) const {
  assert(index >= begin());
  assert(index < end());
  return edges_[index];
}

inline CordRep* CordRepBtree::Edge(EdgeType edge_type) const {
  return edges_[edge_type == kFront ? begin() : back()];
}

inline absl::Span<CordRep* const> CordRepBtree::Edges() const {
  return {edges_ + begin(), size()};
}

inline absl::Span<CordRep* const> CordRepBtree::Edges(size_t begin,
                                                      size_t end) const {
  assert(begin <= end);
  assert(begin >= this->begin());
  assert(end <= this->end());
  return {edges_ + begin, static_cast<size_t>(end - begin)};
}

inline absl::string_view CordRepBtree::Data(size_t index) const {
  assert(height() == 0);
  return EdgeData(Edge(index));
}

inline CordRepBtree* CordRepBtree::New(int height) {
  CordRepBtree* tree = new CordRepBtree;
  tree->length = 0;
  tree->InitInstance(height);
  return tree;
}

inline CordRepBtree* CordRepBtree::New(CordRep* rep) {
  CordRepBtree* tree = new CordRepBtree;
  int height = rep->IsBtree() ? rep->btree()->height() + 1 : 0;
  tree->length = rep->length;
  tree->InitInstance(height, /*begin=*/0, /*end=*/1);
  tree->edges_[0] = rep;
  return tree;
}

inline CordRepBtree* CordRepBtree::New(CordRepBtree* front,
                                       CordRepBtree* back) {
  assert(front->height() == back->height());
  CordRepBtree* tree = new CordRepBtree;
  tree->length = front->length + back->length;
  tree->InitInstance(front->height() + 1, /*begin=*/0, /*end=*/2);
  tree->edges_[0] = front;
  tree->edges_[1] = back;
  return tree;
}

inline void CordRepBtree::Unref(absl::Span<CordRep* const> edges) {
  for (CordRep* edge : edges) {
    if (ABSL_PREDICT_FALSE(!edge->refcount.Decrement())) {
      CordRep::Destroy(edge);
    }
  }
}

inline CordRepBtree* CordRepBtree::CopyRaw() const {
  auto* tree = static_cast<CordRepBtree*>(::operator new(sizeof(CordRepBtree)));
  memcpy(static_cast<void*>(tree), this, sizeof(CordRepBtree));
  new (&tree->refcount) RefcountAndFlags;
  return tree;
}

inline CordRepBtree* CordRepBtree::Copy() const {
  CordRepBtree* tree = CopyRaw();
  for (CordRep* rep : Edges()) CordRep::Ref(rep);
  return tree;
}

inline CordRepBtree* CordRepBtree::CopyToEndFrom(size_t begin,
                                                 size_t new_length) const {
  assert(begin >= this->begin());
  assert(begin <= this->end());
  CordRepBtree* tree = CopyRaw();
  tree->length = new_length;
  tree->set_begin(begin);
  for (CordRep* edge : tree->Edges()) CordRep::Ref(edge);
  return tree;
}

inline CordRepBtree* CordRepBtree::CopyBeginTo(size_t end,
                                               size_t new_length) const {
  assert(end <= capacity());
  assert(end >= this->begin());
  CordRepBtree* tree = CopyRaw();
  tree->length = new_length;
  tree->set_end(end);
  for (CordRep* edge : tree->Edges()) CordRep::Ref(edge);
  return tree;
}

inline void CordRepBtree::AlignBegin() {





  const size_t delta = begin();
  if (ABSL_PREDICT_FALSE(delta != 0)) {
    const size_t new_end = end() - delta;
    set_begin(0);
    set_end(new_end);





    ABSL_ASSUME(new_end <= kMaxCapacity);
#ifdef __clang__
#pragma unroll 1
#endif
    for (size_t i = 0; i < new_end; ++i) {
      edges_[i] = edges_[i + delta];
    }
  }
}

inline void CordRepBtree::AlignEnd() {

  const size_t delta = capacity() - end();
  if (delta != 0) {
    const size_t new_begin = begin() + delta;
    const size_t new_end = end() + delta;
    set_begin(new_begin);
    set_end(new_end);
    ABSL_ASSUME(new_end <= kMaxCapacity);
#ifdef __clang__
#pragma unroll 1
#endif
    for (size_t i = new_end - 1; i >= new_begin; --i) {
      edges_[i] = edges_[i - delta];
    }
  }
}

template <>
inline void CordRepBtree::Add<CordRepBtree::kBack>(CordRep* rep) {
  AlignBegin();
  edges_[fetch_add_end(1)] = rep;
}

template <>
inline void CordRepBtree::Add<CordRepBtree::kBack>(
    absl::Span<CordRep* const> edges) {
  AlignBegin();
  size_t new_end = end();
  for (CordRep* edge : edges) edges_[new_end++] = edge;
  set_end(new_end);
}

template <>
inline void CordRepBtree::Add<CordRepBtree::kFront>(CordRep* rep) {
  AlignEnd();
  edges_[sub_fetch_begin(1)] = rep;
}

template <>
inline void CordRepBtree::Add<CordRepBtree::kFront>(
    absl::Span<CordRep* const> edges) {
  AlignEnd();
  size_t new_begin = begin() - edges.size();
  set_begin(new_begin);
  for (CordRep* edge : edges) edges_[new_begin++] = edge;
}

template <CordRepBtree::EdgeType edge_type>
inline void CordRepBtree::SetEdge(CordRep* edge) {
  const int idx = edge_type == kFront ? begin() : back();
  CordRep::Unref(edges_[idx]);
  edges_[idx] = edge;
}

inline CordRepBtree::OpResult CordRepBtree::ToOpResult(bool owned) {
  return owned ? OpResult{this, kSelf} : OpResult{Copy(), kCopied};
}

inline CordRepBtree::Position CordRepBtree::IndexOf(size_t offset) const {
  assert(offset < length);
  size_t index = begin();
  while (offset >= edges_[index]->length) offset -= edges_[index++]->length;
  return {index, offset};
}

inline CordRepBtree::Position CordRepBtree::IndexBefore(size_t offset) const {
  assert(offset > 0);
  assert(offset <= length);
  size_t index = begin();
  while (offset > edges_[index]->length) offset -= edges_[index++]->length;
  return {index, offset};
}

inline CordRepBtree::Position CordRepBtree::IndexBefore(Position front,
                                                        size_t offset) const {
  size_t index = front.index;
  offset = offset + front.n;
  while (offset > edges_[index]->length) offset -= edges_[index++]->length;
  return {index, offset};
}

inline CordRepBtree::Position CordRepBtree::IndexOfLength(size_t n) const {
  assert(n <= length);
  size_t index = back();
  size_t strip = length - n;
  while (strip >= edges_[index]->length) strip -= edges_[index--]->length;
  return {index, edges_[index]->length - strip};
}

inline CordRepBtree::Position CordRepBtree::IndexBeyond(
    const size_t offset) const {




  size_t off = 0;
  size_t index = begin();
  while (offset > off) off += edges_[index++]->length;
  return {index, off - offset};
}

inline CordRepBtree* CordRepBtree::Create(CordRep* rep) {
  if (IsDataEdge(rep)) return New(rep);
  return CreateSlow(rep);
}

inline Span<char> CordRepBtree::GetAppendBuffer(size_t size) {
  assert(refcount.IsOne());
  CordRepBtree* tree = this;
  const int height = this->height();
  CordRepBtree* n1 = tree;
  CordRepBtree* n2 = tree;
  CordRepBtree* n3 = tree;
  switch (height) {
    case 3:
      tree = tree->Edge(kBack)->btree();
      if (!tree->refcount.IsOne()) return {};
      n2 = tree;
      ABSL_FALLTHROUGH_INTENDED;
    case 2:
      tree = tree->Edge(kBack)->btree();
      if (!tree->refcount.IsOne()) return {};
      n1 = tree;
      ABSL_FALLTHROUGH_INTENDED;
    case 1:
      tree = tree->Edge(kBack)->btree();
      if (!tree->refcount.IsOne()) return {};
      ABSL_FALLTHROUGH_INTENDED;
    case 0:
      CordRep* edge = tree->Edge(kBack);
      if (!edge->refcount.IsOne()) return {};
      if (edge->tag < FLAT) return {};
      size_t avail = edge->flat()->Capacity() - edge->length;
      if (avail == 0) return {};
      size_t delta = (std::min)(size, avail);
      Span<char> span = {edge->flat()->Data() + edge->length, delta};
      edge->length += delta;
      switch (height) {
        case 3:
          n3->length += delta;
          ABSL_FALLTHROUGH_INTENDED;
        case 2:
          n2->length += delta;
          ABSL_FALLTHROUGH_INTENDED;
        case 1:
          n1->length += delta;
          ABSL_FALLTHROUGH_INTENDED;
        case 0:
          tree->length += delta;
          return span;
      }
      break;
  }
  return GetAppendBufferSlow(size);
}

extern template CordRepBtree* CordRepBtree::AddCordRep<CordRepBtree::kBack>(
    CordRepBtree* tree, CordRep* rep);

extern template CordRepBtree* CordRepBtree::AddCordRep<CordRepBtree::kFront>(
    CordRepBtree* tree, CordRep* rep);

inline CordRepBtree* CordRepBtree::Append(CordRepBtree* tree, CordRep* rep) {
  if (ABSL_PREDICT_TRUE(IsDataEdge(rep))) {
    return CordRepBtree::AddCordRep<kBack>(tree, rep);
  }
  return AppendSlow(tree, rep);
}

inline CordRepBtree* CordRepBtree::Prepend(CordRepBtree* tree, CordRep* rep) {
  if (ABSL_PREDICT_TRUE(IsDataEdge(rep))) {
    return CordRepBtree::AddCordRep<kFront>(tree, rep);
  }
  return PrependSlow(tree, rep);
}

#ifdef NDEBUG

inline CordRepBtree* CordRepBtree::AssertValid(CordRepBtree* tree,
                                               bool /* shallow */) {
  return tree;
}

inline const CordRepBtree* CordRepBtree::AssertValid(const CordRepBtree* tree,
                                                     bool /* shallow */) {
  return tree;
}

#endif

}  // namespace cord_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STRINGS_INTERNAL_CORD_REP_BTREE_H_
