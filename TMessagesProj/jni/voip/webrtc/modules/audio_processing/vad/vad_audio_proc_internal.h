/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_PROCESSING_VAD_VAD_AUDIO_PROC_INTERNAL_H_
#define MODULES_AUDIO_PROCESSING_VAD_VAD_AUDIO_PROC_INTERNAL_H_

#include <stddef.h>

namespace webrtc {

static const double kCorrWeight[] = {
    1.000000, 0.985000, 0.970225, 0.955672, 0.941337, 0.927217,
    0.913308, 0.899609, 0.886115, 0.872823, 0.859730, 0.846834,
    0.834132, 0.821620, 0.809296, 0.797156, 0.785199};

static const double kLpcAnalWin[] = {
    0.00000000, 0.01314436, 0.02628645, 0.03942400, 0.05255473, 0.06567639,
    0.07878670, 0.09188339, 0.10496421, 0.11802689, 0.13106918, 0.14408883,
    0.15708358, 0.17005118, 0.18298941, 0.19589602, 0.20876878, 0.22160547,
    0.23440387, 0.24716177, 0.25987696, 0.27254725, 0.28517045, 0.29774438,
    0.31026687, 0.32273574, 0.33514885, 0.34750406, 0.35979922, 0.37203222,
    0.38420093, 0.39630327, 0.40833713, 0.42030043, 0.43219112, 0.44400713,
    0.45574642, 0.46740697, 0.47898676, 0.49048379, 0.50189608, 0.51322164,
    0.52445853, 0.53560481, 0.54665854, 0.55761782, 0.56848075, 0.57924546,
    0.58991008, 0.60047278, 0.61093173, 0.62128512, 0.63153117, 0.64166810,
    0.65169416, 0.66160761, 0.67140676, 0.68108990, 0.69065536, 0.70010148,
    0.70942664, 0.71862923, 0.72770765, 0.73666033, 0.74548573, 0.75418233,
    0.76274862, 0.77118312, 0.77948437, 0.78765094, 0.79568142, 0.80357442,
    0.81132858, 0.81894256, 0.82641504, 0.83374472, 0.84093036, 0.84797069,
    0.85486451, 0.86161063, 0.86820787, 0.87465511, 0.88095122, 0.88709512,
    0.89308574, 0.89892206, 0.90460306, 0.91012776, 0.91549520, 0.92070447,
    0.92575465, 0.93064488, 0.93537432, 0.93994213, 0.94434755, 0.94858979,
    0.95266814, 0.95658189, 0.96033035, 0.96391289, 0.96732888, 0.97057773,
    0.97365889, 0.97657181, 0.97931600, 0.98189099, 0.98429632, 0.98653158,
    0.98859639, 0.99049038, 0.99221324, 0.99376466, 0.99514438, 0.99635215,
    0.99738778, 0.99825107, 0.99894188, 0.99946010, 0.99980562, 0.99997840,
    0.99997840, 0.99980562, 0.99946010, 0.99894188, 0.99825107, 0.99738778,
    0.99635215, 0.99514438, 0.99376466, 0.99221324, 0.99049038, 0.98859639,
    0.98653158, 0.98429632, 0.98189099, 0.97931600, 0.97657181, 0.97365889,
    0.97057773, 0.96732888, 0.96391289, 0.96033035, 0.95658189, 0.95266814,
    0.94858979, 0.94434755, 0.93994213, 0.93537432, 0.93064488, 0.92575465,
    0.92070447, 0.91549520, 0.91012776, 0.90460306, 0.89892206, 0.89308574,
    0.88709512, 0.88095122, 0.87465511, 0.86820787, 0.86161063, 0.85486451,
    0.84797069, 0.84093036, 0.83374472, 0.82641504, 0.81894256, 0.81132858,
    0.80357442, 0.79568142, 0.78765094, 0.77948437, 0.77118312, 0.76274862,
    0.75418233, 0.74548573, 0.73666033, 0.72770765, 0.71862923, 0.70942664,
    0.70010148, 0.69065536, 0.68108990, 0.67140676, 0.66160761, 0.65169416,
    0.64166810, 0.63153117, 0.62128512, 0.61093173, 0.60047278, 0.58991008,
    0.57924546, 0.56848075, 0.55761782, 0.54665854, 0.53560481, 0.52445853,
    0.51322164, 0.50189608, 0.49048379, 0.47898676, 0.46740697, 0.45574642,
    0.44400713, 0.43219112, 0.42030043, 0.40833713, 0.39630327, 0.38420093,
    0.37203222, 0.35979922, 0.34750406, 0.33514885, 0.32273574, 0.31026687,
    0.29774438, 0.28517045, 0.27254725, 0.25987696, 0.24716177, 0.23440387,
    0.22160547, 0.20876878, 0.19589602, 0.18298941, 0.17005118, 0.15708358,
    0.14408883, 0.13106918, 0.11802689, 0.10496421, 0.09188339, 0.07878670,
    0.06567639, 0.05255473, 0.03942400, 0.02628645, 0.01314436, 0.00000000};

static const size_t kFilterOrder = 2;
static const float kCoeffNumerator[kFilterOrder + 1] = {0.974827f, -1.949650f,
                                                        0.974827f};
static const float kCoeffDenominator[kFilterOrder + 1] = {1.0f, -1.971999f,
                                                          0.972457f};

static_assert(kFilterOrder + 1 ==
                  sizeof(kCoeffNumerator) / sizeof(kCoeffNumerator[0]),
              "numerator coefficients incorrect size");
static_assert(kFilterOrder + 1 ==
                  sizeof(kCoeffDenominator) / sizeof(kCoeffDenominator[0]),
              "denominator coefficients incorrect size");

}  // namespace webrtc

#endif  // MODULES_AUDIO_PROCESSING_VAD_VAD_AUDIO_PROCESSING_H_
