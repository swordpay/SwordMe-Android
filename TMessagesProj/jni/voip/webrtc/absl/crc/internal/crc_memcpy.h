// Copyright 2022 The Abseil Authors
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

#ifndef ABSL_CRC_INTERNAL_CRC_MEMCPY_H_
#define ABSL_CRC_INTERNAL_CRC_MEMCPY_H_

#include <cstddef>
#include <memory>

#include "absl/base/config.h"
#include "absl/crc/crc32c.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace crc_internal {

class CrcMemcpyEngine {
 public:
  virtual ~CrcMemcpyEngine() = default;

  virtual crc32c_t Compute(void* __restrict dst, const void* __restrict src,
                           std::size_t length, crc32c_t initial_crc) const = 0;

 protected:
  CrcMemcpyEngine() = default;
};

class CrcMemcpy {
 public:
  static crc32c_t CrcAndCopy(void* __restrict dst, const void* __restrict src,
                             std::size_t length,
                             crc32c_t initial_crc = ToCrc32c(0),
                             bool non_temporal = false) {
    static const ArchSpecificEngines engines = GetArchSpecificEngines();
    auto* engine = non_temporal ? engines.non_temporal : engines.temporal;
    return engine->Compute(dst, src, length, initial_crc);
  }

  static std::unique_ptr<CrcMemcpyEngine> GetTestEngine(int vector,
                                                        int integer);

 private:
  struct ArchSpecificEngines {
    CrcMemcpyEngine* temporal;
    CrcMemcpyEngine* non_temporal;
  };

  static ArchSpecificEngines GetArchSpecificEngines();
};

class FallbackCrcMemcpyEngine : public CrcMemcpyEngine {
 public:
  FallbackCrcMemcpyEngine() = default;
  FallbackCrcMemcpyEngine(const FallbackCrcMemcpyEngine&) = delete;
  FallbackCrcMemcpyEngine operator=(const FallbackCrcMemcpyEngine&) = delete;

  crc32c_t Compute(void* __restrict dst, const void* __restrict src,
                   std::size_t length, crc32c_t initial_crc) const override;
};

class CrcNonTemporalMemcpyEngine : public CrcMemcpyEngine {
 public:
  CrcNonTemporalMemcpyEngine() = default;
  CrcNonTemporalMemcpyEngine(const CrcNonTemporalMemcpyEngine&) = delete;
  CrcNonTemporalMemcpyEngine operator=(const CrcNonTemporalMemcpyEngine&) =
      delete;

  crc32c_t Compute(void* __restrict dst, const void* __restrict src,
                   std::size_t length, crc32c_t initial_crc) const override;
};

class CrcNonTemporalMemcpyAVXEngine : public CrcMemcpyEngine {
 public:
  CrcNonTemporalMemcpyAVXEngine() = default;
  CrcNonTemporalMemcpyAVXEngine(const CrcNonTemporalMemcpyAVXEngine&) = delete;
  CrcNonTemporalMemcpyAVXEngine operator=(
      const CrcNonTemporalMemcpyAVXEngine&) = delete;

  crc32c_t Compute(void* __restrict dst, const void* __restrict src,
                   std::size_t length, crc32c_t initial_crc) const override;
};

// accelerated version is available, use the accelerated version, otherwise use
// the generic fallback version.
inline crc32c_t Crc32CAndCopy(void* __restrict dst, const void* __restrict src,
                              std::size_t length,
                              crc32c_t initial_crc = ToCrc32c(0),
                              bool non_temporal = false) {
  return CrcMemcpy::CrcAndCopy(dst, src, length, initial_crc, non_temporal);
}

}  // namespace crc_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_CRC_INTERNAL_CRC_MEMCPY_H_
