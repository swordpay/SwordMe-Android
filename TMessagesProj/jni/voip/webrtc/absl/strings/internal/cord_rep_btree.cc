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

#include "absl/strings/internal/cord_rep_btree.h"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <string>

#include "absl/base/attributes.h"
#include "absl/base/config.h"
#include "absl/base/internal/raw_logging.h"
#include "absl/strings/internal/cord_data_edge.h"
#include "absl/strings/internal/cord_internal.h"
#include "absl/strings/internal/cord_rep_consume.h"
#include "absl/strings/internal/cord_rep_flat.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace cord_internal {

#ifdef ABSL_INTERNAL_NEED_REDUNDANT_CONSTEXPR_DECL
constexpr size_t CordRepBtree::kMaxCapacity;
#endif

namespace {

using NodeStack = CordRepBtree * [CordRepBtree::kMaxDepth];
using EdgeType = CordRepBtree::EdgeType;
using OpResult = CordRepBtree::OpResult;
using CopyResult = CordRepBtree::CopyResult;

constexpr auto kFront = CordRepBtree::kFront;
constexpr auto kBack = CordRepBtree::kBack;

inline bool exhaustive_validation() {
  return cord_btree_exhaustive_validation.load(std::memory_order_relaxed);
}

// Prints the entire tree structure or 'rep'. External callers should
// not specify 'depth' and leave it to its default (0) value.
// Rep may be a CordRepBtree tree, or a SUBSTRING / EXTERNAL / FLAT node.
void DumpAll(const CordRep* rep,
             bool include_contents,
             std::ostream& stream,
             size_t depth = 0) {

  assert(depth <= CordRepBtree::kMaxDepth + 2);
  std::string sharing = const_cast<CordRep*>(rep)->refcount.IsOne()
                            ? std::string("Private")
                            : absl::StrCat("Shared(", rep->refcount.Get(), ")");
  std::string sptr = absl::StrCat("0x", absl::Hex(rep));


  auto maybe_dump_data = [&stream, include_contents](const CordRep* r) {
    if (include_contents) {


      constexpr size_t kMaxDataLength = 60;
      stream << ", data = \""
             << EdgeData(r).substr(0, kMaxDataLength)
             << (r->length > kMaxDataLength ? "\"..." : "\"");
    }
    stream << '\n';
  };


  stream << std::string(depth * 2, ' ') << sharing << " (" << sptr << ") ";

  if (rep->IsBtree()) {
    const CordRepBtree* node = rep->btree();
    std::string label =
        node->height() ? absl::StrCat("Node(", node->height(), ")") : "Leaf";
    stream << label << ", len = " << node->length
           << ", begin = " << node->begin() << ", end = " << node->end()
           << "\n";
    for (CordRep* edge : node->Edges()) {
      DumpAll(edge, include_contents, stream, depth + 1);
    }
  } else if (rep->tag == SUBSTRING) {
    const CordRepSubstring* substring = rep->substring();
    stream << "Substring, len = " << rep->length
           << ", start = " << substring->start;
    maybe_dump_data(rep);
    DumpAll(substring->child, include_contents, stream, depth + 1);
  } else if (rep->tag >= FLAT) {
    stream << "Flat, len = " << rep->length
           << ", cap = " << rep->flat()->Capacity();
    maybe_dump_data(rep);
  } else if (rep->tag == EXTERNAL) {
    stream << "Extn, len = " << rep->length;
    maybe_dump_data(rep);
  }
}

// small data out of large reps, and general efficiency of 'always copy small
// data'. Consider making this a cord rep internal library function.
CordRepSubstring* CreateSubstring(CordRep* rep, size_t offset, size_t n) {
  assert(n != 0);
  assert(offset + n <= rep->length);
  assert(offset != 0 || n != rep->length);

  if (rep->tag == SUBSTRING) {
    CordRepSubstring* substring = rep->substring();
    offset += substring->start;
    rep = CordRep::Ref(substring->child);
    CordRep::Unref(substring);
  }
  assert(rep->IsExternal() || rep->IsFlat());
  CordRepSubstring* substring = new CordRepSubstring();
  substring->length = n;
  substring->tag = SUBSTRING;
  substring->start = offset;
  substring->child = rep;
  return substring;
}

inline CordRep* MakeSubstring(CordRep* rep, size_t offset, size_t n) {
  if (n == rep->length) return rep;
  if (n == 0) return CordRep::Unref(rep), nullptr;
  return CreateSubstring(rep, offset, n);
}

inline CordRep* MakeSubstring(CordRep* rep, size_t offset) {
  if (offset == 0) return rep;
  return CreateSubstring(rep, offset, rep->length - offset);
}

// This method directly returns `edge` if `length` equals `edge->length`.
// If `is_mutable` is set to true, this function may return `edge` with
// `edge->length` set to the new length depending on the type and size of
// `edge`. Otherwise, this function returns a new CordRepSubstring value.
// Requires `length > 0 && length <= edge->length`.
CordRep* ResizeEdge(CordRep* edge, size_t length, bool is_mutable) {
  assert(length > 0);
  assert(length <= edge->length);
  assert(IsDataEdge(edge));
  if (length >= edge->length) return edge;

  if (is_mutable && (edge->tag >= FLAT || edge->tag == SUBSTRING)) {
    edge->length = length;
    return edge;
  }

  return CreateSubstring(edge, 0, length);
}

template <EdgeType edge_type>
inline absl::string_view Consume(absl::string_view s, size_t n) {
  return edge_type == kBack ? s.substr(n) : s.substr(0, s.size() - n);
}

template <EdgeType edge_type>
inline absl::string_view Consume(char* dst, absl::string_view s, size_t n) {
  if (edge_type == kBack) {
    memcpy(dst, s.data(), n);
    return s.substr(n);
  } else {
    const size_t offset = s.size() - n;
    memcpy(dst, s.data() + offset, n);
    return s.substr(0, offset);
  }
}

// decrement introduces traffic between cpus (even if the result of that
// traffic does nothing), making this faster than a single call to
// refcount.Decrement() checking the zero refcount condition.
template <typename R, typename Fn>
inline void FastUnref(R* r, Fn&& fn) {
  if (r->refcount.IsOne()) {
    fn(r);
  } else if (!r->refcount.DecrementExpectHighRefcount()) {
    fn(r);
  }
}


void DeleteSubstring(CordRepSubstring* substring) {
  CordRep* rep = substring->child;
  if (!rep->refcount.Decrement()) {
    if (rep->tag >= FLAT) {
      CordRepFlat::Delete(rep->flat());
    } else {
      assert(rep->tag == EXTERNAL);
      CordRepExternal::Delete(rep->external());
    }
  }
  delete substring;
}

void DeleteLeafEdge(CordRep* rep) {
  assert(IsDataEdge(rep));
  if (rep->tag >= FLAT) {
    CordRepFlat::Delete(rep->flat());
  } else if (rep->tag == EXTERNAL) {
    CordRepExternal::Delete(rep->external());
  } else {
    DeleteSubstring(rep->substring());
  }
}

// (leg) down to the leaf level of a btree, and 'unwind' / 'Finalize' methods to
// propagate node changes up the stack.
template <EdgeType edge_type>
struct StackOperations {


  inline bool owned(int depth) const { return depth < share_depth; }

  inline CordRepBtree* node(int depth) const { return stack[depth]; }


  inline CordRepBtree* BuildStack(CordRepBtree* tree, int depth) {
    assert(depth <= tree->height());
    int current_depth = 0;
    while (current_depth < depth && tree->refcount.IsOne()) {
      stack[current_depth++] = tree;
      tree = tree->Edge(edge_type)->btree();
    }
    share_depth = current_depth + (tree->refcount.IsOne() ? 1 : 0);
    while (current_depth < depth) {
      stack[current_depth++] = tree;
      tree = tree->Edge(edge_type)->btree();
    }
    return tree;
  }



  inline void BuildOwnedStack(CordRepBtree* tree, int height) {
    assert(height <= CordRepBtree::kMaxHeight);
    int depth = 0;
    while (depth < height) {
      assert(tree->refcount.IsOne());
      stack[depth++] = tree;
      tree = tree->Edge(edge_type)->btree();
    }
    assert(tree->refcount.IsOne());
    share_depth = depth + 1;
  }


  static inline CordRepBtree* Finalize(CordRepBtree* tree, OpResult result) {
    switch (result.action) {
      case CordRepBtree::kPopped:
        tree = edge_type == kBack ? CordRepBtree::New(tree, result.tree)
                                  : CordRepBtree::New(result.tree, tree);
        if (ABSL_PREDICT_FALSE(tree->height() > CordRepBtree::kMaxHeight)) {
          tree = CordRepBtree::Rebuild(tree);
          ABSL_RAW_CHECK(tree->height() <= CordRepBtree::kMaxHeight,
                         "Max height exceeded");
        }
        return tree;
      case CordRepBtree::kCopied:
        CordRep::Unref(tree);
        ABSL_FALLTHROUGH_INTENDED;
      case CordRepBtree::kSelf:
        return result.tree;
    }
    ABSL_INTERNAL_UNREACHABLE;
    return result.tree;
  }






  template <bool propagate = false>
  inline CordRepBtree* Unwind(CordRepBtree* tree, int depth, size_t length,
                              OpResult result) {

















    if (depth != 0) {
      do {
        CordRepBtree* node = stack[--depth];
        const bool owned = depth < share_depth;
        switch (result.action) {
          case CordRepBtree::kPopped:
            assert(!propagate);
            result = node->AddEdge<edge_type>(owned, result.tree, length);
            break;
          case CordRepBtree::kCopied:
            result = node->SetEdge<edge_type>(owned, result.tree, length);
            if (propagate) stack[depth] = result.tree;
            break;
          case CordRepBtree::kSelf:
            node->length += length;
            while (depth > 0) {
              node = stack[--depth];
              node->length += length;
            }
            return node;
        }
      } while (depth > 0);
    }
    return Finalize(tree, result);
  }

  inline CordRepBtree* Propagate(CordRepBtree* tree, int depth, size_t length,
                                 OpResult result) {
    return Unwind</*propagate=*/true>(tree, depth, length, result);
  }






  int share_depth;

  NodeStack stack;
};

}  // namespace

void CordRepBtree::Dump(const CordRep* rep, absl::string_view label,
                        bool include_contents, std::ostream& stream) {
  stream << "===================================\n";
  if (!label.empty()) {
    stream << label << '\n';
    stream << "-----------------------------------\n";
  }
  if (rep) {
    DumpAll(rep, include_contents, stream);
  } else {
    stream << "NULL\n";
  }
}

void CordRepBtree::Dump(const CordRep* rep, absl::string_view label,
                        std::ostream& stream) {
  Dump(rep, label, false, stream);
}

void CordRepBtree::Dump(const CordRep* rep, std::ostream& stream) {
  Dump(rep, absl::string_view(), false, stream);
}

template <size_t size>
static void DestroyTree(CordRepBtree* tree) {
  for (CordRep* node : tree->Edges()) {
    if (node->refcount.Decrement()) continue;
    for (CordRep* edge : node->btree()->Edges()) {
      if (edge->refcount.Decrement()) continue;
      if (size == 1) {
        DeleteLeafEdge(edge);
      } else {
        CordRepBtree::Destroy(edge->btree());
      }
    }
    CordRepBtree::Delete(node->btree());
  }
  CordRepBtree::Delete(tree);
}

void CordRepBtree::Destroy(CordRepBtree* tree) {
  switch (tree->height()) {
    case 0:
      for (CordRep* edge : tree->Edges()) {
        if (!edge->refcount.Decrement()) {
          DeleteLeafEdge(edge);
        }
      }
      return CordRepBtree::Delete(tree);
    case 1:
      return DestroyTree<1>(tree);
    default:
      return DestroyTree<2>(tree);
  }
}

bool CordRepBtree::IsValid(const CordRepBtree* tree, bool shallow) {
#define NODE_CHECK_VALID(x)                                           \
  if (!(x)) {                                                         \
    ABSL_RAW_LOG(ERROR, "CordRepBtree::CheckValid() FAILED: %s", #x); \
    return false;                                                     \
  }
#define NODE_CHECK_EQ(x, y)                                                    \
  if ((x) != (y)) {                                                            \
    ABSL_RAW_LOG(ERROR,                                                        \
                 "CordRepBtree::CheckValid() FAILED: %s != %s (%s vs %s)", #x, \
                 #y, absl::StrCat(x).c_str(), absl::StrCat(y).c_str());        \
    return false;                                                              \
  }

  NODE_CHECK_VALID(tree != nullptr);
  NODE_CHECK_VALID(tree->IsBtree());
  NODE_CHECK_VALID(tree->height() <= kMaxHeight);
  NODE_CHECK_VALID(tree->begin() < tree->capacity());
  NODE_CHECK_VALID(tree->end() <= tree->capacity());
  NODE_CHECK_VALID(tree->begin() <= tree->end());
  size_t child_length = 0;
  for (CordRep* edge : tree->Edges()) {
    NODE_CHECK_VALID(edge != nullptr);
    if (tree->height() > 0) {
      NODE_CHECK_VALID(edge->IsBtree());
      NODE_CHECK_VALID(edge->btree()->height() == tree->height() - 1);
    } else {
      NODE_CHECK_VALID(IsDataEdge(edge));
    }
    child_length += edge->length;
  }
  NODE_CHECK_EQ(child_length, tree->length);
  if ((!shallow || exhaustive_validation()) && tree->height() > 0) {
    for (CordRep* edge : tree->Edges()) {
      if (!IsValid(edge->btree(), shallow)) return false;
    }
  }
  return true;

#undef NODE_CHECK_VALID
#undef NODE_CHECK_EQ
}

#ifndef NDEBUG

CordRepBtree* CordRepBtree::AssertValid(CordRepBtree* tree, bool shallow) {
  if (!IsValid(tree, shallow)) {
    Dump(tree, "CordRepBtree validation failed:", false, std::cout);
    ABSL_RAW_LOG(FATAL, "CordRepBtree::CheckValid() FAILED");
  }
  return tree;
}

const CordRepBtree* CordRepBtree::AssertValid(const CordRepBtree* tree,
                                              bool shallow) {
  if (!IsValid(tree, shallow)) {
    Dump(tree, "CordRepBtree validation failed:", false, std::cout);
    ABSL_RAW_LOG(FATAL, "CordRepBtree::CheckValid() FAILED");
  }
  return tree;
}

#endif  // NDEBUG

template <EdgeType edge_type>
inline OpResult CordRepBtree::AddEdge(bool owned, CordRep* edge, size_t delta) {
  if (size() >= kMaxCapacity) return {New(edge), kPopped};
  OpResult result = ToOpResult(owned);
  result.tree->Add<edge_type>(edge);
  result.tree->length += delta;
  return result;
}

template <EdgeType edge_type>
OpResult CordRepBtree::SetEdge(bool owned, CordRep* edge, size_t delta) {
  OpResult result;
  const size_t idx = index(edge_type);
  if (owned) {
    result = {this, kSelf};
    CordRep::Unref(edges_[idx]);
  } else {




    result = {CopyRaw(), kCopied};
    constexpr int shift = edge_type == kFront ? 1 : 0;
    for (CordRep* r : Edges(begin() + shift, back() + shift)) {
      CordRep::Ref(r);
    }
  }
  result.tree->edges_[idx] = edge;
  result.tree->length += delta;
  return result;
}

template <EdgeType edge_type>
CordRepBtree* CordRepBtree::AddCordRep(CordRepBtree* tree, CordRep* rep) {
  const int depth = tree->height();
  const size_t length = rep->length;
  StackOperations<edge_type> ops;
  CordRepBtree* leaf = ops.BuildStack(tree, depth);
  const OpResult result =
      leaf->AddEdge<edge_type>(ops.owned(depth), rep, length);
  return ops.Unwind(tree, depth, length, result);
}

template <>
CordRepBtree* CordRepBtree::NewLeaf<kBack>(absl::string_view data,
                                           size_t extra) {
  CordRepBtree* leaf = CordRepBtree::New(0);
  size_t length = 0;
  size_t end = 0;
  const size_t cap = leaf->capacity();
  while (!data.empty() && end != cap) {
    auto* flat = CordRepFlat::New(data.length() + extra);
    flat->length = (std::min)(data.length(), flat->Capacity());
    length += flat->length;
    leaf->edges_[end++] = flat;
    data = Consume<kBack>(flat->Data(), data, flat->length);
  }
  leaf->length = length;
  leaf->set_end(end);
  return leaf;
}

template <>
CordRepBtree* CordRepBtree::NewLeaf<kFront>(absl::string_view data,
                                            size_t extra) {
  CordRepBtree* leaf = CordRepBtree::New(0);
  size_t length = 0;
  size_t begin = leaf->capacity();
  leaf->set_end(leaf->capacity());
  while (!data.empty() && begin != 0) {
    auto* flat = CordRepFlat::New(data.length() + extra);
    flat->length = (std::min)(data.length(), flat->Capacity());
    length += flat->length;
    leaf->edges_[--begin] = flat;
    data = Consume<kFront>(flat->Data(), data, flat->length);
  }
  leaf->length = length;
  leaf->set_begin(begin);
  return leaf;
}

template <>
absl::string_view CordRepBtree::AddData<kBack>(absl::string_view data,
                                               size_t extra) {
  assert(!data.empty());
  assert(size() < capacity());
  AlignBegin();
  const size_t cap = capacity();
  do {
    CordRepFlat* flat = CordRepFlat::New(data.length() + extra);
    const size_t n = (std::min)(data.length(), flat->Capacity());
    flat->length = n;
    edges_[fetch_add_end(1)] = flat;
    data = Consume<kBack>(flat->Data(), data, n);
  } while (!data.empty() && end() != cap);
  return data;
}

template <>
absl::string_view CordRepBtree::AddData<kFront>(absl::string_view data,
                                                size_t extra) {
  assert(!data.empty());
  assert(size() < capacity());
  AlignEnd();
  do {
    CordRepFlat* flat = CordRepFlat::New(data.length() + extra);
    const size_t n = (std::min)(data.length(), flat->Capacity());
    flat->length = n;
    edges_[sub_fetch_begin(1)] = flat;
    data = Consume<kFront>(flat->Data(), data, n);
  } while (!data.empty() && begin() != 0);
  return data;
}

template <EdgeType edge_type>
CordRepBtree* CordRepBtree::AddData(CordRepBtree* tree, absl::string_view data,
                                    size_t extra) {
  if (ABSL_PREDICT_FALSE(data.empty())) return tree;

  const size_t original_data_size = data.size();
  int depth = tree->height();
  StackOperations<edge_type> ops;
  CordRepBtree* leaf = ops.BuildStack(tree, depth);


  if (leaf->size() < leaf->capacity()) {
    OpResult result = leaf->ToOpResult(ops.owned(depth));
    data = result.tree->AddData<edge_type>(data, extra);
    if (data.empty()) {
      result.tree->length += original_data_size;
      return ops.Unwind(tree, depth, original_data_size, result);
    }




    size_t delta = original_data_size - data.size();
    assert(delta > 0);
    result.tree->length += delta;
    tree = ops.Propagate(tree, depth, delta, result);
    ops.share_depth = depth + 1;
  }








  for (;;) {
    OpResult result = {CordRepBtree::NewLeaf<edge_type>(data, extra), kPopped};
    if (result.tree->length == data.size()) {
      return ops.Unwind(tree, depth, result.tree->length, result);
    }
    data = Consume<edge_type>(data, result.tree->length);
    tree = ops.Unwind(tree, depth, result.tree->length, result);
    depth = tree->height();
    ops.BuildOwnedStack(tree, depth);
  }
}

template <EdgeType edge_type>
CordRepBtree* CordRepBtree::Merge(CordRepBtree* dst, CordRepBtree* src) {
  assert(dst->height() >= src->height());

  const size_t length = src->length;

  const int depth = dst->height() - src->height();
  StackOperations<edge_type> ops;
  CordRepBtree* merge_node = ops.BuildStack(dst, depth);





  OpResult result;
  if (merge_node->size() + src->size() <= kMaxCapacity) {
    result = merge_node->ToOpResult(ops.owned(depth));
    result.tree->Add<edge_type>(src->Edges());
    result.tree->length += src->length;
    if (src->refcount.IsOne()) {
      Delete(src);
    } else {
      for (CordRep* edge : src->Edges()) CordRep::Ref(edge);
      CordRepBtree::Unref(src);
    }
  } else {
    result = {src, kPopped};
  }


  if (depth) {
    return ops.Unwind(dst, depth, length, result);
  }
  return ops.Finalize(dst, result);
}

CopyResult CordRepBtree::CopySuffix(size_t offset) {
  assert(offset < this->length);





  int height = this->height();
  CordRepBtree* node = this;
  size_t len = node->length - offset;
  CordRep* back = node->Edge(kBack);
  while (back->length >= len) {
    offset = back->length - len;
    if (--height < 0) {
      return {MakeSubstring(CordRep::Ref(back), offset), height};
    }
    node = back->btree();
    back = node->Edge(kBack);
  }
  if (offset == 0) return {CordRep::Ref(node), height};



  Position pos = node->IndexBeyond(offset);
  CordRepBtree* sub = node->CopyToEndFrom(pos.index, len);
  const CopyResult result = {sub, height};





  while (pos.n != 0) {
    assert(pos.index >= 1);
    const size_t begin = pos.index - 1;
    sub->set_begin(begin);
    CordRep* const edge = node->Edge(begin);

    len = pos.n;
    offset = edge->length - len;

    if (--height < 0) {
      sub->edges_[begin] = MakeSubstring(CordRep::Ref(edge), offset, len);
      return result;
    }

    node = edge->btree();
    pos = node->IndexBeyond(offset);

    CordRepBtree* nsub = node->CopyToEndFrom(pos.index, len);
    sub->edges_[begin] = nsub;
    sub = nsub;
  }
  sub->set_begin(pos.index);
  return result;
}

CopyResult CordRepBtree::CopyPrefix(size_t n, bool allow_folding) {
  assert(n > 0);
  assert(n <= this->length);





  int height = this->height();
  CordRepBtree* node = this;
  CordRep* front = node->Edge(kFront);
  if (allow_folding) {
    while (front->length >= n) {
      if (--height < 0) return {MakeSubstring(CordRep::Ref(front), 0, n), -1};
      node = front->btree();
      front = node->Edge(kFront);
    }
  }
  if (node->length == n) return {CordRep::Ref(node), height};

  Position pos = node->IndexOf(n);


  CordRepBtree* sub = node->CopyBeginTo(pos.index, n);
  const CopyResult result = {sub, height};



  while (pos.n != 0) {
    size_t end = pos.index;
    n = pos.n;

    CordRep* edge = node->Edge(pos.index);
    if (--height < 0) {
      sub->edges_[end++] = MakeSubstring(CordRep::Ref(edge), 0, n);
      sub->set_end(end);
      AssertValid(result.edge->btree());
      return result;
    }

    node = edge->btree();
    pos = node->IndexOf(n);
    CordRepBtree* nsub = node->CopyBeginTo(pos.index, n);
    sub->edges_[end++] = nsub;
    sub->set_end(end);
    sub = nsub;
  }
  sub->set_end(pos.index);
  AssertValid(result.edge->btree());
  return result;
}

CordRep* CordRepBtree::ExtractFront(CordRepBtree* tree) {
  CordRep* front = tree->Edge(tree->begin());
  if (tree->refcount.IsOne()) {
    Unref(tree->Edges(tree->begin() + 1, tree->end()));
    CordRepBtree::Delete(tree);
  } else {
    CordRep::Ref(front);
    CordRep::Unref(tree);
  }
  return front;
}

CordRepBtree* CordRepBtree::ConsumeBeginTo(CordRepBtree* tree, size_t end,
                                           size_t new_length) {
  assert(end <= tree->end());
  if (tree->refcount.IsOne()) {
    Unref(tree->Edges(end, tree->end()));
    tree->set_end(end);
    tree->length = new_length;
  } else {
    CordRepBtree* old = tree;
    tree = tree->CopyBeginTo(end, new_length);
    CordRep::Unref(old);
  }
  return tree;
}

CordRep* CordRepBtree::RemoveSuffix(CordRepBtree* tree, size_t n) {

  assert(tree != nullptr);
  assert(n <= tree->length);
  const size_t len = tree->length;
  if (ABSL_PREDICT_FALSE(n == 0)) {
    return tree;
  }
  if (ABSL_PREDICT_FALSE(n >= len)) {
    CordRepBtree::Unref(tree);
    return nullptr;
  }

  size_t length = len - n;
  int height = tree->height();
  bool is_mutable = tree->refcount.IsOne();

  Position pos = tree->IndexOfLength(length);
  while (pos.index == tree->begin()) {
    CordRep* edge = ExtractFront(tree);
    is_mutable &= edge->refcount.IsOne();
    if (height-- == 0) return ResizeEdge(edge, length, is_mutable);
    tree = edge->btree();
    pos = tree->IndexOfLength(length);
  }





  CordRepBtree* top = tree = ConsumeBeginTo(tree, pos.index + 1, length);
  CordRep* edge = tree->Edge(pos.index);
  length = pos.n;
  while (length != edge->length) {

    assert(tree->refcount.IsOne());
    const bool edge_is_mutable = edge->refcount.IsOne();

    if (height-- == 0) {
      tree->edges_[pos.index] = ResizeEdge(edge, length, edge_is_mutable);
      return AssertValid(top);
    }

    if (!edge_is_mutable) {


      tree->edges_[pos.index] = edge->btree()->CopyPrefix(length, false).edge;
      CordRep::Unref(edge);
      return AssertValid(top);
    }

    tree = edge->btree();
    pos = tree->IndexOfLength(length);
    tree = ConsumeBeginTo(edge->btree(), pos.index + 1, length);
    edge = tree->Edge(pos.index);
    length = pos.n;
  }

  return AssertValid(top);
}

CordRep* CordRepBtree::SubTree(size_t offset, size_t n) {
  assert(n <= this->length);
  assert(offset <= this->length - n);
  if (ABSL_PREDICT_FALSE(n == 0)) return nullptr;

  CordRepBtree* node = this;
  int height = node->height();
  Position front = node->IndexOf(offset);
  CordRep* left = node->edges_[front.index];
  while (front.n + n <= left->length) {
    if (--height < 0) return MakeSubstring(CordRep::Ref(left), front.n, n);
    node = left->btree();
    front = node->IndexOf(front.n);
    left = node->edges_[front.index];
  }

  const Position back = node->IndexBefore(front, n);
  CordRep* const right = node->edges_[back.index];
  assert(back.index > front.index);

  CopyResult prefix;
  CopyResult suffix;
  if (height > 0) {

    prefix = left->btree()->CopySuffix(front.n);
    suffix = right->btree()->CopyPrefix(back.n);




    if (front.index + 1 == back.index) {
      height = (std::max)(prefix.height, suffix.height) + 1;
    }

    for (int h = prefix.height + 1; h < height; ++h) {
      prefix.edge = CordRepBtree::New(prefix.edge);
    }
    for (int h = suffix.height + 1; h < height; ++h) {
      suffix.edge = CordRepBtree::New(suffix.edge);
    }
  } else {

    prefix = CopyResult{MakeSubstring(CordRep::Ref(left), front.n), -1};
    suffix = CopyResult{MakeSubstring(CordRep::Ref(right), 0, back.n), -1};
  }

  CordRepBtree* sub = CordRepBtree::New(height);
  size_t end = 0;
  sub->edges_[end++] = prefix.edge;
  for (CordRep* r : node->Edges(front.index + 1, back.index)) {
    sub->edges_[end++] = CordRep::Ref(r);
  }
  sub->edges_[end++] = suffix.edge;
  sub->set_end(end);
  sub->length = n;
  return AssertValid(sub);
}

CordRepBtree* CordRepBtree::MergeTrees(CordRepBtree* left,
                                       CordRepBtree* right) {
  return left->height() >= right->height() ? Merge<kBack>(left, right)
                                           : Merge<kFront>(right, left);
}

bool CordRepBtree::IsFlat(absl::string_view* fragment) const {
  if (height() == 0 && size() == 1) {
    if (fragment) *fragment = Data(begin());
    return true;
  }
  return false;
}

bool CordRepBtree::IsFlat(size_t offset, const size_t n,
                          absl::string_view* fragment) const {
  assert(n <= this->length);
  assert(offset <= this->length - n);
  if (ABSL_PREDICT_FALSE(n == 0)) return false;
  int height = this->height();
  const CordRepBtree* node = this;
  for (;;) {
    const Position front = node->IndexOf(offset);
    const CordRep* edge = node->Edge(front.index);
    if (edge->length < front.n + n) return false;
    if (--height < 0) {
      if (fragment) *fragment = EdgeData(edge).substr(front.n, n);
      return true;
    }
    offset = front.n;
    node = node->Edge(front.index)->btree();
  }
}

char CordRepBtree::GetCharacter(size_t offset) const {
  assert(offset < length);
  const CordRepBtree* node = this;
  int height = node->height();
  for (;;) {
    Position front = node->IndexOf(offset);
    if (--height < 0) return node->Data(front.index)[front.n];
    offset = front.n;
    node = node->Edge(front.index)->btree();
  }
}

Span<char> CordRepBtree::GetAppendBufferSlow(size_t size) {

  assert(height() >= 4);
  assert(refcount.IsOne());


  const int depth = height();
  CordRepBtree* node = this;
  CordRepBtree* stack[kMaxDepth];
  for (int i = 0; i < depth; ++i) {
    node = node->Edge(kBack)->btree();
    if (!node->refcount.IsOne()) return {};
    stack[i] = node;
  }

  CordRep* const edge = node->Edge(kBack);
  if (!edge->refcount.IsOne() || edge->tag < FLAT) return {};

  const size_t avail = edge->flat()->Capacity() - edge->length;
  if (avail == 0) return {};

  size_t delta = (std::min)(size, avail);
  Span<char> span = {edge->flat()->Data() + edge->length, delta};
  edge->length += delta;
  this->length += delta;
  for (int i = 0; i < depth; ++i) {
    stack[i]->length += delta;
  }
  return span;
}

CordRepBtree* CordRepBtree::CreateSlow(CordRep* rep) {
  if (rep->IsBtree()) return rep->btree();

  CordRepBtree* node = nullptr;
  auto consume = [&node](CordRep* r, size_t offset, size_t length) {
    r = MakeSubstring(r, offset, length);
    if (node == nullptr) {
      node = New(r);
    } else {
      node = CordRepBtree::AddCordRep<kBack>(node, r);
    }
  };
  Consume(rep, consume);
  return node;
}

CordRepBtree* CordRepBtree::AppendSlow(CordRepBtree* tree, CordRep* rep) {
  if (ABSL_PREDICT_TRUE(rep->IsBtree())) {
    return MergeTrees(tree, rep->btree());
  }
  auto consume = [&tree](CordRep* r, size_t offset, size_t length) {
    r = MakeSubstring(r, offset, length);
    tree = CordRepBtree::AddCordRep<kBack>(tree, r);
  };
  Consume(rep, consume);
  return tree;
}

CordRepBtree* CordRepBtree::PrependSlow(CordRepBtree* tree, CordRep* rep) {
  if (ABSL_PREDICT_TRUE(rep->IsBtree())) {
    return MergeTrees(rep->btree(), tree);
  }
  auto consume = [&tree](CordRep* r, size_t offset, size_t length) {
    r = MakeSubstring(r, offset, length);
    tree = CordRepBtree::AddCordRep<kFront>(tree, r);
  };
  ReverseConsume(rep, consume);
  return tree;
}

CordRepBtree* CordRepBtree::Append(CordRepBtree* tree, absl::string_view data,
                                   size_t extra) {
  return CordRepBtree::AddData<kBack>(tree, data, extra);
}

CordRepBtree* CordRepBtree::Prepend(CordRepBtree* tree, absl::string_view data,
                                    size_t extra) {
  return CordRepBtree::AddData<kFront>(tree, data, extra);
}

template CordRepBtree* CordRepBtree::AddCordRep<kFront>(CordRepBtree* tree,
                                                        CordRep* rep);
template CordRepBtree* CordRepBtree::AddCordRep<kBack>(CordRepBtree* tree,
                                                       CordRep* rep);
template CordRepBtree* CordRepBtree::AddData<kFront>(CordRepBtree* tree,
                                                     absl::string_view data,
                                                     size_t extra);
template CordRepBtree* CordRepBtree::AddData<kBack>(CordRepBtree* tree,
                                                    absl::string_view data,
                                                    size_t extra);

void CordRepBtree::Rebuild(CordRepBtree** stack, CordRepBtree* tree,
                           bool consume) {
  bool owned = consume && tree->refcount.IsOne();
  if (tree->height() == 0) {
    for (CordRep* edge : tree->Edges()) {
      if (!owned) edge = CordRep::Ref(edge);
      size_t height = 0;
      size_t length = edge->length;
      CordRepBtree* node = stack[0];
      OpResult result = node->AddEdge<kBack>(true, edge, length);
      while (result.action == CordRepBtree::kPopped) {
        stack[height] = result.tree;
        if (stack[++height] == nullptr) {
          result.action = CordRepBtree::kSelf;
          stack[height] = CordRepBtree::New(node, result.tree);
        } else {
          node = stack[height];
          result = node->AddEdge<kBack>(true, result.tree, length);
        }
      }
      while (stack[++height] != nullptr) {
        stack[height]->length += length;
      }
    }
  } else {
    for (CordRep* rep : tree->Edges()) {
      Rebuild(stack, rep->btree(), owned);
    }
  }
  if (consume) {
    if (owned) {
      CordRepBtree::Delete(tree);
    } else {
      CordRepBtree::Unref(tree);
    }
  }
}

CordRepBtree* CordRepBtree::Rebuild(CordRepBtree* tree) {

  CordRepBtree* node = CordRepBtree::New();
  CordRepBtree* stack[CordRepBtree::kMaxDepth + 1] = {node};

  Rebuild(stack, tree, /* consume reference */ true);

  for (CordRepBtree* parent : stack) {
    if (parent == nullptr) return node;
    node = parent;
  }

  assert(false);
  return nullptr;
}

CordRepBtree::ExtractResult CordRepBtree::ExtractAppendBuffer(
    CordRepBtree* tree, size_t extra_capacity) {
  int depth = 0;
  NodeStack stack;

  ExtractResult result;
  result.tree = tree;
  result.extracted = nullptr;

  while (tree->height() > 0) {
    if (!tree->refcount.IsOne()) return result;
    stack[depth++] = tree;
    tree = tree->Edge(kBack)->btree();
  }
  if (!tree->refcount.IsOne()) return result;

  CordRep* rep = tree->Edge(kBack);
  if (!(rep->IsFlat() && rep->refcount.IsOne())) return result;

  CordRepFlat* flat = rep->flat();
  const size_t length = flat->length;
  const size_t avail = flat->Capacity() - flat->length;
  if (extra_capacity > avail) return result;

  result.extracted = flat;

  while (tree->size() == 1) {
    CordRepBtree::Delete(tree);
    if (--depth < 0) {

      result.tree = nullptr;
      return result;
    }
    rep = tree;
    tree = stack[depth];
  }

  tree->set_end(tree->end() - 1);
  tree->length -= length;

  while (depth > 0) {
    tree = stack[--depth];
    tree->length -= length;
  }



  while (tree->size() == 1) {
    int height = tree->height();
    rep = tree->Edge(kBack);
    Delete(tree);
    if (height == 0) {

      result.tree = rep;
      return result;
    }
    tree = rep->btree();
  }

  result.tree = tree;
  return result;
}

}  // namespace cord_internal
ABSL_NAMESPACE_END
}  // namespace absl
