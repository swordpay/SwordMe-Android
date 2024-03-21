/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// The return value is  0 - OK and -1 - Error, unless otherwise stated.

#ifndef MODULES_AUDIO_PROCESSING_UTILITY_DELAY_ESTIMATOR_H_
#define MODULES_AUDIO_PROCESSING_UTILITY_DELAY_ESTIMATOR_H_

#include <stdint.h>

namespace webrtc {

static const int32_t kMaxBitCountsQ9 = (32 << 9);  // 32 matching bits in Q9.

typedef struct {

  int* far_bit_counts;

  uint32_t* binary_far_history;
  int history_size;
} BinaryDelayEstimatorFarend;

typedef struct {

  int32_t* mean_bit_counts;


  int32_t* bit_counts;

  uint32_t* binary_near_history;
  int near_history_size;
  int history_size;

  int32_t minimum_probability;
  int last_delay_probability;

  int last_delay;

  int robust_validation_enabled;
  int allowed_offset;
  int last_candidate_delay;
  int compare_delay;
  int candidate_hits;
  float* histogram;
  float last_delay_histogram;

  int lookahead;

  BinaryDelayEstimatorFarend* farend;
} BinaryDelayEstimator;

// WebRtc_CreateBinaryDelayEstimatorFarend(...).
// Input:
//    - self              : Pointer to the binary delay estimation far-end
//                          instance which is the return value of
//                          WebRtc_CreateBinaryDelayEstimatorFarend().
//
void WebRtc_FreeBinaryDelayEstimatorFarend(BinaryDelayEstimatorFarend* self);

// estimation. The memory needs to be initialized separately through
// WebRtc_InitBinaryDelayEstimatorFarend(...).
//
// Inputs:
//      - history_size    : Size of the far-end binary spectrum history.
//
// Return value:
//      - BinaryDelayEstimatorFarend*
//                        : Created `handle`. If the memory can't be allocated
//                          or if any of the input parameters are invalid NULL
//                          is returned.
//
BinaryDelayEstimatorFarend* WebRtc_CreateBinaryDelayEstimatorFarend(
    int history_size);

//
// Inputs:
//      - self            : Pointer to the binary estimation far-end instance
//                          which is the return value of
//                          WebRtc_CreateBinaryDelayEstimatorFarend().
//      - history_size    : Size of the far-end binary spectrum history.
//
// Return value:
//      - history_size    : The history size allocated.
int WebRtc_AllocateFarendBufferMemory(BinaryDelayEstimatorFarend* self,
                                      int history_size);

// WebRtc_CreateBinaryDelayEstimatorFarend(...).
//
// Input:
//    - self              : Pointer to the delay estimation far-end instance.
//
// Output:
//    - self              : Initialized far-end instance.
//
void WebRtc_InitBinaryDelayEstimatorFarend(BinaryDelayEstimatorFarend* self);

// WebRtc_CreateBinaryDelayEstimatorFarend(...).
//
// Input:
//    - delay_shift   : The amount of blocks to shift history buffers.
//
void WebRtc_SoftResetBinaryDelayEstimatorFarend(
    BinaryDelayEstimatorFarend* self,
    int delay_shift);

// spectrum is used as reference when calculating the delay using
// WebRtc_ProcessBinarySpectrum().
//
// Inputs:
//    - self                  : Pointer to the delay estimation far-end
//                              instance.
//    - binary_far_spectrum   : Far-end binary spectrum.
//
// Output:
//    - self                  : Updated far-end instance.
//
void WebRtc_AddBinaryFarSpectrum(BinaryDelayEstimatorFarend* self,
                                 uint32_t binary_far_spectrum);

//
// Note that BinaryDelayEstimator utilizes BinaryDelayEstimatorFarend, but does
// not take ownership of it, hence the BinaryDelayEstimator has to be torn down
// before the far-end.
//
// Input:
//    - self              : Pointer to the binary delay estimation instance
//                          which is the return value of
//                          WebRtc_CreateBinaryDelayEstimator().
//
void WebRtc_FreeBinaryDelayEstimator(BinaryDelayEstimator* self);

// to be initialized separately through WebRtc_InitBinaryDelayEstimator(...).
//
// See WebRtc_CreateDelayEstimator(..) in delay_estimator_wrapper.c for detailed
// description.
BinaryDelayEstimator* WebRtc_CreateBinaryDelayEstimator(
    BinaryDelayEstimatorFarend* farend,
    int max_lookahead);

// updated at the same time if needed.
//
// Input:
//      - self            : Pointer to the binary estimation instance which is
//                          the return value of
//                          WebRtc_CreateBinaryDelayEstimator().
//      - history_size    : Size of the history buffers.
//
// Return value:
//      - history_size    : The history size allocated.
int WebRtc_AllocateHistoryBufferMemory(BinaryDelayEstimator* self,
                                       int history_size);

// WebRtc_CreateBinaryDelayEstimator(...).
//
// Input:
//    - self              : Pointer to the delay estimation instance.
//
// Output:
//    - self              : Initialized instance.
//
void WebRtc_InitBinaryDelayEstimator(BinaryDelayEstimator* self);

// WebRtc_CreateBinaryDelayEstimator(...).
//
// Input:
//    - delay_shift   : The amount of blocks to shift history buffers.
//
// Return value:
//    - actual_shifts : The actual number of shifts performed.
//
int WebRtc_SoftResetBinaryDelayEstimator(BinaryDelayEstimator* self,
                                         int delay_shift);

// end spectra. It is assumed the binary far-end spectrum has been added using
// WebRtc_AddBinaryFarSpectrum() prior to this call. The value will be offset by
// the lookahead (i.e. the lookahead should be subtracted from the returned
// value).
//
// Inputs:
//    - self                  : Pointer to the delay estimation instance.
//    - binary_near_spectrum  : Near-end binary spectrum of the current block.
//
// Output:
//    - self                  : Updated instance.
//
// Return value:
//    - delay                 :  >= 0 - Calculated delay value.
//                              -2    - Insufficient data for estimation.
//
int WebRtc_ProcessBinarySpectrum(BinaryDelayEstimator* self,
                                 uint32_t binary_near_spectrum);

// WebRtc_ProcessBinarySpectrum(...).
//
// Input:
//    - self                  : Pointer to the delay estimation instance.
//
// Return value:
//    - delay                 :  >= 0 - Last calculated delay value
//                              -2    - Insufficient data for estimation.
//
int WebRtc_binary_last_delay(BinaryDelayEstimator* self);

// function WebRtc_ProcessBinarySpectrum(...). The estimation quality is a value
// in the interval [0, 1].  The higher the value, the better the quality.
//
// Return value:
//    - delay_quality         :  >= 0 - Estimation quality of last calculated
//                                      delay value.
float WebRtc_binary_last_delay_quality(BinaryDelayEstimator* self);

// function is used internally in the Binary Delay Estimator as well as the
// Fixed point wrapper.
//
// Inputs:
//    - new_value             : The new value the mean should be updated with.
//    - factor                : The step size, in number of right shifts.
//
// Input/Output:
//    - mean_value            : Pointer to the mean value.
//
void WebRtc_MeanEstimatorFix(int32_t new_value,
                             int factor,
                             int32_t* mean_value);

}  // namespace webrtc

#endif  // MODULES_AUDIO_PROCESSING_UTILITY_DELAY_ESTIMATOR_H_
