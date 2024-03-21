/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_PROCESSING_AGC2_RNN_VAD_TEST_UTILS_H_
#define MODULES_AUDIO_PROCESSING_AGC2_RNN_VAD_TEST_UTILS_H_

#include <array>
#include <fstream>
#include <memory>
#include <string>

#include "absl/strings/string_view.h"
#include "api/array_view.h"
#include "modules/audio_processing/agc2/rnn_vad/common.h"
#include "rtc_base/checks.h"
#include "rtc_base/numerics/safe_compare.h"

namespace webrtc {
namespace rnn_vad {

constexpr float kFloatMin = std::numeric_limits<float>::min();

// that the values in the pair do not match.
void ExpectEqualFloatArray(rtc::ArrayView<const float> expected,
                           rtc::ArrayView<const float> computed);

// that their absolute error is above a given threshold.
void ExpectNearAbsolute(rtc::ArrayView<const float> expected,
                        rtc::ArrayView<const float> computed,
                        float tolerance);

class FileReader {
 public:
  virtual ~FileReader() = default;

  virtual int size() const = 0;





  virtual bool ReadChunk(rtc::ArrayView<float> dst) = 0;




  virtual bool ReadValue(float& dst) = 0;

  virtual void SeekForward(int hop) = 0;

  virtual void SeekBeginning() = 0;
};

// `chunk_size`.
struct ChunksFileReader {
  const int chunk_size;
  const int num_chunks;
  std::unique_ptr<FileReader> reader;
};

std::unique_ptr<FileReader> CreatePcmSamplesReader();

ChunksFileReader CreatePitchBuffer24kHzReader();

ChunksFileReader CreateLpResidualAndPitchInfoReader();

std::unique_ptr<FileReader> CreateGruInputReader();

std::unique_ptr<FileReader> CreateVadProbsReader();

// analysis steps.
class PitchTestData {
 public:
  PitchTestData();
  ~PitchTestData();
  rtc::ArrayView<const float, kBufSize24kHz> PitchBuffer24kHzView() const {
    return pitch_buffer_24k_;
  }
  rtc::ArrayView<const float, kRefineNumLags24kHz> SquareEnergies24kHzView()
      const {
    return square_energies_24k_;
  }
  rtc::ArrayView<const float, kNumLags12kHz> AutoCorrelation12kHzView() const {
    return auto_correlation_12k_;
  }

 private:
  std::array<float, kBufSize24kHz> pitch_buffer_24k_;
  std::array<float, kRefineNumLags24kHz> square_energies_24k_;
  std::array<float, kNumLags12kHz> auto_correlation_12k_;
};

class FileWriter {
 public:
  explicit FileWriter(absl::string_view file_path)
      : os_(std::string(file_path), std::ios::binary) {}
  FileWriter(const FileWriter&) = delete;
  FileWriter& operator=(const FileWriter&) = delete;
  ~FileWriter() = default;
  void WriteChunk(rtc::ArrayView<const float> value) {
    const std::streamsize bytes_to_write = value.size() * sizeof(float);
    os_.write(reinterpret_cast<const char*>(value.data()), bytes_to_write);
  }

 private:
  std::ofstream os_;
};

}  // namespace rnn_vad
}  // namespace webrtc

#endif  // MODULES_AUDIO_PROCESSING_AGC2_RNN_VAD_TEST_UTILS_H_
