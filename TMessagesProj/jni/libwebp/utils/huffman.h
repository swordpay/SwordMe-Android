// Copyright 2012 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Utilities for building and looking up Huffman trees.
//
// Author: Urvang Joshi (urvang@google.com)

#ifndef WEBP_UTILS_HUFFMAN_H_
#define WEBP_UTILS_HUFFMAN_H_

#include <assert.h>
#include "../webp/format_constants.h"
#include "../webp/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int symbol_;
  int children_;  // delta offset to both children (contiguous) or 0 if leaf.
} HuffmanTreeNode;

#define HUFF_LUT_BITS 7
#define HUFF_LUT (1U << HUFF_LUT_BITS)
typedef struct HuffmanTree HuffmanTree;
struct HuffmanTree {

  uint8_t lut_bits_[HUFF_LUT];
  int16_t lut_symbol_[HUFF_LUT];
  int16_t lut_jump_[HUFF_LUT];

  HuffmanTreeNode* root_;   // all the nodes, starting at root.
  int max_nodes_;           // max number of nodes
  int num_nodes_;           // number of currently occupied nodes
};

typedef struct HTreeGroup HTreeGroup;
struct HTreeGroup {
  HuffmanTree htrees_[HUFFMAN_CODES_PER_META_CODE];
};

static WEBP_INLINE int HuffmanTreeNodeIsNotLeaf(
    const HuffmanTreeNode* const node) {
  return node->children_;
}

static WEBP_INLINE const HuffmanTreeNode* HuffmanTreeNextNode(
    const HuffmanTreeNode* node, int right_child) {
  return node + node->children_ + right_child;
}

// Note: It does NOT free 'tree' itself.
void VP8LHuffmanTreeFree(HuffmanTree* const tree);

HTreeGroup* VP8LHtreeGroupsNew(int num_htree_groups);

void VP8LHtreeGroupsFree(HTreeGroup* htree_groups, int num_htree_groups);

// The 'huff_codes' and 'code_lengths' are pre-allocated temporary memory
// buffers, used for creating the huffman tree.
// Returns false in case of error (invalid tree or memory error).
int VP8LHuffmanTreeBuildImplicit(HuffmanTree* const tree,
                                 const int* const code_lengths,
                                 int* const huff_codes,
                                 int code_lengths_size);

// and symbols. Verifies that all symbols added are smaller than max_symbol.
// Returns false in case of an invalid symbol, invalid tree or memory error.
int VP8LHuffmanTreeBuildExplicit(HuffmanTree* const tree,
                                 const int* const code_lengths,
                                 const int* const codes,
                                 const int* const symbols, int max_symbol,
                                 int num_symbols);

// 'huff_codes' should be pre-allocated.
// Returns false in case of error (memory allocation, invalid codes).
int VP8LHuffmanCodeLengthsToCodes(const int* const code_lengths,
                                  int code_lengths_size, int* const huff_codes);

#ifdef __cplusplus
}    // extern "C"
#endif

#endif  // WEBP_UTILS_HUFFMAN_H_
