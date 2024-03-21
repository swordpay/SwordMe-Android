/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#ifndef COMMON_AUDIO_VAD_VAD_SP_H_
#define COMMON_AUDIO_VAD_VAD_SP_H_

#include "common_audio/vad/vad_core.h"

//
// Inputs:
//      - signal_in     : Input signal.
//      - in_length     : Length of input signal in samples.
//
// Input & Output:
//      - filter_state  : Current filter states of the two all-pass filters. The
//                        `filter_state` is updated after all samples have been
//                        processed.
//
// Output:
//      - signal_out    : Downsampled signal (of length `in_length` / 2).
void WebRtcVad_Downsampling(const int16_t* signal_in,
                            int16_t* signal_out,
                            int32_t* filter_state,
                            size_t in_length);

// median of the five smallest feature values in a 100 frames long window.
// As long as `handle->frame_counter` is zero, that is, we haven't received any
// "valid" data, FindMinimum() outputs the default value of 1600.
//
// Inputs:
//      - feature_value : New feature value to update with.
//      - channel       : Channel number.
//
// Input & Output:
//      - handle        : State information of the VAD.
//
// Returns:
//                      : Smoothed minimum value for a moving window.
int16_t WebRtcVad_FindMinimum(VadInstT* handle,
                              int16_t feature_value,
                              int channel);

#endif  // COMMON_AUDIO_VAD_VAD_SP_H_
