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

#ifndef ABSL_STRINGS_INTERNAL_CORD_REP_BTREE_NAVIGATOR_H_
#define ABSL_STRINGS_INTERNAL_CORD_REP_BTREE_NAVIGATOR_H_

#include <cassert>
#include <iostream>

#include "absl/strings/internal/cord_internal.h"
#include "absl/strings/internal/cord_rep_btree.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace cord_internal {

// navigate all the (leaf) data edges in a CordRepBtree instance.
//
// A CordRepBtreeNavigator instance is by default empty. Callers initialize a
// navigator instance by calling one of `InitFirst()`, `InitLast()` or
// `InitOffset()`, which establishes a current position. Callers can then
// navigate using the `Next`, `Previous`, `Skip` and `Seek` methods.
//
// The navigator instance does not take or adopt a reference on the provided
// `tree` on any of the initialization calls. Callers are responsible for
// guaranteeing the lifecycle of the provided tree. A navigator instance can
// be reset to the empty state by calling `Reset`.
//
// A navigator only keeps positional state on the 'current data edge', it does
// explicitly not keep any 'offset' state. The class does accept and return
// offsets in the `Read()`, `Skip()` and 'Seek()` methods as these would
// otherwise put a big burden on callers. Callers are expected to maintain
// (returned) offset info if they require such granular state.
class CordRepBtreeNavigator {
 public:



  struct Position {
    CordRep* edge;
    size_t offset;
  };





  struct ReadResult {
    CordRep* tree;
    size_t n;
  };

  explicit operator bool() const;

  CordRepBtree* btree() const;


  CordRep* Current() const;

  CordRep* InitFirst(CordRepBtree* tree);

  CordRep* InitLast(CordRepBtree* tree);





  Position InitOffset(CordRepBtree* tree, size_t offset);



  CordRep* Next();



  CordRep* Previous();




  Position Seek(size_t offset);








  ReadResult Read(size_t edge_offset, size_t n);




  Position Skip(size_t n);

  void Reset();

 private:



  CordRep* NextUp();




  CordRep* PreviousUp();

  template <CordRepBtree::EdgeType edge_type>
  CordRep* Init(CordRepBtree* tree);

  int height_ = -1;



  uint8_t index_[CordRepBtree::kMaxDepth];
  CordRepBtree* node_[CordRepBtree::kMaxDepth];
};

inline CordRepBtreeNavigator::operator bool() const { return height_ >= 0; }

inline CordRepBtree* CordRepBtreeNavigator::btree() const {
  return height_ >= 0 ? node_[height_] : nullptr;
}

inline CordRep* CordRepBtreeNavigator::Current() const {
  assert(height_ >= 0);
  return node_[0]->Edge(index_[0]);
}

inline void CordRepBtreeNavigator::Reset() { height_ = -1; }

inline CordRep* CordRepBtreeNavigator::InitFirst(CordRepBtree* tree) {
  return Init<CordRepBtree::kFront>(tree);
}

inline CordRep* CordRepBtreeNavigator::InitLast(CordRepBtree* tree) {
  return Init<CordRepBtree::kBack>(tree);
}

template <CordRepBtree::EdgeType edge_type>
inline CordRep* CordRepBtreeNavigator::Init(CordRepBtree* tree) {
  assert(tree != nullptr);
  assert(tree->size() > 0);
  assert(tree->height() <= CordRepBtree::kMaxHeight);
  int height = height_ = tree->height();
  size_t index = tree->index(edge_type);
  node_[height] = tree;
  index_[height] = static_cast<uint8_t>(index);
  while (--height >= 0) {
    tree = tree->Edge(index)->btree();
    node_[height] = tree;
    index = tree->index(edge_type);
    index_[height] = static_cast<uint8_t>(index);
  }
  return node_[0]->Edge(index);
}

inline CordRepBtreeNavigator::Position CordRepBtreeNavigator::Seek(
    size_t offset) {
  assert(btree() != nullptr);
  int height = height_;
  CordRepBtree* edge = node_[height];
  if (ABSL_PREDICT_FALSE(offset >= edge->length)) return {nullptr, 0};
  CordRepBtree::Position index = edge->IndexOf(offset);
  index_[height] = static_cast<uint8_t>(index.index);
  while (--height >= 0) {
    edge = edge->Edge(index.index)->btree();
    node_[height] = edge;
    index = edge->IndexOf(index.n);
    index_[height] = static_cast<uint8_t>(index.index);
  }
  return {edge->Edge(index.index), index.n};
}

inline CordRepBtreeNavigator::Position CordRepBtreeNavigator::InitOffset(
    CordRepBtree* tree, size_t offset) {
  assert(tree != nullptr);
  assert(tree->height() <= CordRepBtree::kMaxHeight);
  if (ABSL_PREDICT_FALSE(offset >= tree->length)) return {nullptr, 0};
  height_ = tree->height();
  node_[height_] = tree;
  return Seek(offset);
}

inline CordRep* CordRepBtreeNavigator::Next() {
  CordRepBtree* edge = node_[0];
  return index_[0] == edge->back() ? NextUp() : edge->Edge(++index_[0]);
}

inline CordRep* CordRepBtreeNavigator::Previous() {
  CordRepBtree* edge = node_[0];
  return index_[0] == edge->begin() ? PreviousUp() : edge->Edge(--index_[0]);
}

inline CordRep* CordRepBtreeNavigator::NextUp() {
  assert(index_[0] == node_[0]->back());
  CordRepBtree* edge;
  size_t index;
  int height = 0;
  do {
    if (++height > height_) return nullptr;
    edge = node_[height];
    index = index_[height] + 1;
  } while (index == edge->end());
  index_[height] = static_cast<uint8_t>(index);
  do {
    node_[--height] = edge = edge->Edge(index)->btree();
    index_[height] = static_cast<uint8_t>(index = edge->begin());
  } while (height > 0);
  return edge->Edge(index);
}

inline CordRep* CordRepBtreeNavigator::PreviousUp() {
  assert(index_[0] == node_[0]->begin());
  CordRepBtree* edge;
  size_t index;
  int height = 0;
  do {
    if (++height > height_) return nullptr;
    edge = node_[height];
    index = index_[height];
  } while (index == edge->begin());
  index_[height] = static_cast<uint8_t>(--index);
  do {
    node_[--height] = edge = edge->Edge(index)->btree();
    index_[height] = static_cast<uint8_t>(index = edge->back());
  } while (height > 0);
  return edge->Edge(index);
}

}  // namespace cord_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STRINGS_INTERNAL_CORD_REP_BTREE_NAVIGATOR_H_
