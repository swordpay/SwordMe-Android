// Copyright 2012 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Author: Jyrki Alakuijala (jyrki@google.com)
//
// Models the histograms of literal and distance codes.

#ifndef WEBP_ENC_HISTOGRAM_H_
#define WEBP_ENC_HISTOGRAM_H_

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "./backward_references.h"
#include "../webp/format_constants.h"
#include "../webp/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {


  uint32_t* literal_;         // Pointer to the allocated buffer for literal.
  uint32_t red_[NUM_LITERAL_CODES];
  uint32_t blue_[NUM_LITERAL_CODES];
  uint32_t alpha_[NUM_LITERAL_CODES];

  uint32_t distance_[NUM_DISTANCE_CODES];
  int palette_code_bits_;
  double bit_cost_;      // cached value of VP8LHistogramEstimateBits(this)
  double literal_cost_;  // Cached values of dominant entropy costs:
  double red_cost_;      //   literal, red & blue.
  double blue_cost_;
} VP8LHistogram;

// big memory chunk. Can be destroyed by calling WebPSafeFree().
typedef struct {
  int size;         // number of slots currently in use
  int max_size;     // maximum capacity
  VP8LHistogram** histograms;
} VP8LHistogramSet;

//
// The input data is the PixOrCopy data, which models the literals, stop
// codes and backward references (both distances and lengths).  Also: if
// palette_code_bits is >= 0, initialize the histogram with this value.
void VP8LHistogramCreate(VP8LHistogram* const p,
                         const VP8LBackwardRefs* const refs,
                         int palette_code_bits);

int VP8LGetHistogramSize(int palette_code_bits);

void VP8LHistogramInit(VP8LHistogram* const p, int palette_code_bits);

void VP8LHistogramStoreRefs(const VP8LBackwardRefs* const refs,
                            VP8LHistogram* const histo);

void VP8LFreeHistogram(VP8LHistogram* const histo);

void VP8LFreeHistogramSet(VP8LHistogramSet* const histo);

// using 'cache_bits'. Return NULL in case of memory error.
VP8LHistogramSet* VP8LAllocateHistogramSet(int size, int cache_bits);

// Returns NULL in case of memory error.
// Special case of VP8LAllocateHistogramSet, with size equals 1.
VP8LHistogram* VP8LAllocateHistogram(int cache_bits);

void VP8LHistogramAddSinglePixOrCopy(VP8LHistogram* const histo,
                                     const PixOrCopy* const v);

// approximately maps to.
double VP8LHistogramEstimateBits(const VP8LHistogram* const p);

// represent the entropy code itself.
double VP8LHistogramEstimateBitsBulk(const VP8LHistogram* const p);

static WEBP_INLINE int VP8LHistogramNumCodes(int palette_code_bits) {
  return NUM_LITERAL_CODES + NUM_LENGTH_CODES +
      ((palette_code_bits > 0) ? (1 << palette_code_bits) : 0);
}

int VP8LGetHistoImageSymbols(int xsize, int ysize,
                             const VP8LBackwardRefs* const refs,
                             int quality, int histogram_bits, int cache_bits,
                             VP8LHistogramSet* const image_in,
                             uint16_t* const histogram_symbols);

#ifdef __cplusplus
}
#endif

#endif  // WEBP_ENC_HISTOGRAM_H_
