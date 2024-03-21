/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_PROCESSING_VAD_VAD_CIRCULAR_BUFFER_H_
#define MODULES_AUDIO_PROCESSING_VAD_VAD_CIRCULAR_BUFFER_H_

#include <memory>

namespace webrtc {

// K samples of the input, and keeps track of the mean of the last samples.
//
// It is used in class "PitchBasedActivity" to keep track of posterior
// probabilities in the past few seconds. The posterior probabilities are used
// to recursively update prior probabilities.
class VadCircularBuffer {
 public:
  static VadCircularBuffer* Create(int buffer_size);
  ~VadCircularBuffer();

  bool is_full() const { return is_full_; }

  double Oldest() const;

  void Insert(double value);

  void Reset();


  double Mean();



  int RemoveTransient(int width_threshold, double val_threshold);

 private:
  explicit VadCircularBuffer(int buffer_size);



  int Get(int index, double* value) const;

  int Set(int index, double value);

  int BufferLevel();


  int ConvertToLinearIndex(int* index) const;

  std::unique_ptr<double[]> buffer_;
  bool is_full_;
  int index_;
  int buffer_size_;
  double sum_;
};

}  // namespace webrtc
#endif  // MODULES_AUDIO_PROCESSING_VAD_VAD_CIRCULAR_BUFFER_H_
