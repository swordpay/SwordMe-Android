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

#ifndef ABSL_STRINGS_INTERNAL_CORD_REP_BTREE_READER_H_
#define ABSL_STRINGS_INTERNAL_CORD_REP_BTREE_READER_H_

#include <cassert>

#include "absl/base/config.h"
#include "absl/strings/internal/cord_data_edge.h"
#include "absl/strings/internal/cord_internal.h"
#include "absl/strings/internal/cord_rep_btree.h"
#include "absl/strings/internal/cord_rep_btree_navigator.h"
#include "absl/strings/internal/cord_rep_flat.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace cord_internal {

// References to the underlying data are returned as absl::string_view values.
// The most typical use case is a forward only iteration over tree data.
// The class also provides `Skip()`, `Seek()` and `Read()` methods similar to
// CordRepBtreeNavigator that allow more advanced navigation.
//
// Example: iterate over all data inside a cord btree:
//
//   CordRepBtreeReader reader;
//   for (string_view sv = reader.Init(tree); !sv.Empty(); sv = sv.Next()) {
//     DoSomethingWithDataIn(sv);
//   }
//
// All navigation methods always return the next 'chunk' of data. The class
// assumes that all data is directly 'consumed' by the caller. For example:
// invoking `Skip()` will skip the desired number of bytes, and directly
// read and return the next chunk of data directly after the skipped bytes.
//
// Example: iterate over all data inside a btree skipping the first 100 bytes:
//
//   CordRepBtreeReader reader;
//   absl::string_view sv = reader.Init(tree);
//   if (sv.length() > 100) {
//     sv.RemovePrefix(100);
//   } else {
//     sv = reader.Skip(100 - sv.length());
//   }
//   while (!sv.empty()) {
//     DoSomethingWithDataIn(sv);
//     absl::string_view sv = reader.Next();
//   }
//
// It is important to notice that `remaining` is based on the end position of
// the last data edge returned to the caller, not the cumulative data returned
// to the caller which can be less in cases of skipping or seeking over data.
//
// For example, consider a cord btree with five data edges: "abc", "def", "ghi",
// "jkl" and "mno":
//
//   absl::string_view sv;
//   CordRepBtreeReader reader;
//
//   sv = reader.Init(tree); // sv = "abc", remaining = 12
//   sv = reader.Skip(4);    // sv = "hi",  remaining = 6
//   sv = reader.Skip(2);    // sv = "l",   remaining = 3
//   sv = reader.Next();     // sv = "mno", remaining = 0
//   sv = reader.Seek(1);    // sv = "bc", remaining = 12
//
class CordRepBtreeReader {
 public:
  using ReadResult = CordRepBtreeNavigator::ReadResult;
  using Position = CordRepBtreeNavigator::Position;

  explicit operator bool() const { return navigator_.btree() != nullptr; }

  CordRepBtree* btree() const { return navigator_.btree(); }


  CordRep* node() const { return navigator_.Current(); }


  size_t length() const;







  size_t remaining() const { return remaining_; }

  void Reset() { navigator_.Reset(); }


  absl::string_view Init(CordRepBtree* tree);




  absl::string_view Next();


  absl::string_view Skip(size_t skip);






















  absl::string_view Read(size_t n, size_t chunk_size, CordRep*& tree);







  absl::string_view Seek(size_t offset);

 private:
  size_t remaining_ = 0;
  CordRepBtreeNavigator navigator_;
};

inline size_t CordRepBtreeReader::length() const {
  assert(btree() != nullptr);
  return btree()->length;
}

inline absl::string_view CordRepBtreeReader::Init(CordRepBtree* tree) {
  assert(tree != nullptr);
  const CordRep* edge = navigator_.InitFirst(tree);
  remaining_ = tree->length - edge->length;
  return EdgeData(edge);
}

inline absl::string_view CordRepBtreeReader::Next() {
  if (remaining_ == 0) return {};
  const CordRep* edge = navigator_.Next();
  assert(edge != nullptr);
  remaining_ -= edge->length;
  return EdgeData(edge);
}

inline absl::string_view CordRepBtreeReader::Skip(size_t skip) {


  const size_t edge_length = navigator_.Current()->length;
  CordRepBtreeNavigator::Position pos = navigator_.Skip(skip + edge_length);
  if (ABSL_PREDICT_FALSE(pos.edge == nullptr)) {
    remaining_ = 0;
    return {};
  }


  remaining_ -= skip - pos.offset + pos.edge->length;
  return EdgeData(pos.edge).substr(pos.offset);
}

inline absl::string_view CordRepBtreeReader::Seek(size_t offset) {
  const CordRepBtreeNavigator::Position pos = navigator_.Seek(offset);
  if (ABSL_PREDICT_FALSE(pos.edge == nullptr)) {
    remaining_ = 0;
    return {};
  }
  absl::string_view chunk = EdgeData(pos.edge).substr(pos.offset);
  remaining_ = length() - offset - chunk.length();
  return chunk;
}

}  // namespace cord_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STRINGS_INTERNAL_CORD_REP_BTREE_READER_H_
