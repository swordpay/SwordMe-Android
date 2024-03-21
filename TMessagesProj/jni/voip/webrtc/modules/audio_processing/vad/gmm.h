/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_PROCESSING_VAD_GMM_H_
#define MODULES_AUDIO_PROCESSING_VAD_GMM_H_

namespace webrtc {

// A GMM is formulated as
//  f(x) = w[0] * mixture[0] + w[1] * mixture[1] + ... +
//         w[num_mixtures - 1] * mixture[num_mixtures - 1];
// Where a 'mixture' is a Gaussian density.

struct GmmParameters {


  const double* weight;


  const double* mean;



  const double* covar_inverse;

  int dimension;

  int num_mixtures;
};

// `x`. If the dimensionality of the given GMM is larger that the maximum
// acceptable dimension by the following function -1 is returned.
double EvaluateGmm(const double* x, const GmmParameters& gmm_parameters);

}  // namespace webrtc
#endif  // MODULES_AUDIO_PROCESSING_VAD_GMM_H_
