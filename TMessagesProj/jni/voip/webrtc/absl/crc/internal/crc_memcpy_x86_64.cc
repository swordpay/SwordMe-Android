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

// XMM registers do not support the CRC instruction (yet).  While copying,
// compute the running CRC of the data being copied.
//
// It is assumed that any CPU running this code has SSE4.2 instructions
// available (for CRC32C).  This file will do nothing if that is not true.
//
// The CRC instruction has a 3-byte latency, and we are stressing the ALU ports
// here (unlike a traditional memcopy, which has almost no ALU use), so we will
// need to copy in such a way that the CRC unit is used efficiently. We have two
// regimes in this code:
//  1. For operations of size < kCrcSmallSize, do the CRC then the memcpy
//  2. For operations of size > kCrcSmallSize:
//      a) compute an initial CRC + copy on a small amount of data to align the
//         destination pointer on a 16-byte boundary.
//      b) Split the data into 3 main regions and a tail (smaller than 48 bytes)
//      c) Do the copy and CRC of the 3 main regions, interleaving (start with
//         full cache line copies for each region, then move to single 16 byte
//         pieces per region).
//      d) Combine the CRCs with CRC32C::Concat.
//      e) Copy the tail and extend the CRC with the CRC of the tail.
// This method is not ideal for op sizes between ~1k and ~8k because CRC::Concat
// takes a significant amount of time.  A medium-sized approach could be added
// using 3 CRCs over fixed-size blocks where the zero-extensions required for
// CRC32C::Concat can be precomputed.

#include <cstddef>
#include <cstdint>

#include "absl/crc/crc32c.h"
#include "absl/strings/string_view.h"

#ifdef __SSE4_2__

#include <emmintrin.h>
#include <x86intrin.h>

#include <type_traits>

#include "absl/base/dynamic_annotations.h"
#include "absl/base/internal/prefetch.h"
#include "absl/base/optimization.h"
#include "absl/crc/internal/cpu_detect.h"
#include "absl/crc/internal/crc_memcpy.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace crc_internal {

namespace {

inline crc32c_t ShortCrcCopy(char* dst, const char* src, std::size_t length,
                             crc32c_t crc) {


  uint32_t crc_uint32 = static_cast<uint32_t>(crc);
  for (std::size_t i = 0; i < length; i++) {
    uint8_t data = *reinterpret_cast<const uint8_t*>(src);
    crc_uint32 = _mm_crc32_u8(crc_uint32, data);
    *reinterpret_cast<uint8_t*>(dst) = data;
    ++src;
    ++dst;
  }
  return ToCrc32c(crc_uint32);
}

constexpr int kIntLoadsPerVec = sizeof(__m128i) / sizeof(uint64_t);

template <int vec_regions, int int_regions>
inline void LargeTailCopy(crc32c_t* crcs, char** dst, const char** src,
                          size_t region_size, size_t copy_rounds) {
  __m128i data[vec_regions];
  uint64_t int_data[kIntLoadsPerVec * int_regions];

  while (copy_rounds > 0) {
#pragma unroll_completely
    for (int i = 0; i < vec_regions; i++) {
      int region = i;

      auto* vsrc =
          reinterpret_cast<const __m128i_u*>(*src + region_size * region);
      auto* vdst = reinterpret_cast<__m128i*>(*dst + region_size * region);

      data[i] = _mm_loadu_si128(vsrc);

      _mm_store_si128(vdst, data[i]);

      crcs[region] = ToCrc32c(_mm_crc32_u64(static_cast<uint32_t>(crcs[region]),
                                            _mm_extract_epi64(data[i], 0)));
      crcs[region] = ToCrc32c(_mm_crc32_u64(static_cast<uint32_t>(crcs[region]),
                                            _mm_extract_epi64(data[i], 1)));
    }

#pragma unroll_completely
    for (int i = 0; i < int_regions; i++) {
      int region = vec_regions + i;

      auto* usrc =
          reinterpret_cast<const uint64_t*>(*src + region_size * region);
      auto* udst = reinterpret_cast<uint64_t*>(*dst + region_size * region);

#pragma unroll_completely
      for (int j = 0; j < kIntLoadsPerVec; j++) {
        int data_index = i * kIntLoadsPerVec + j;

        int_data[data_index] = *(usrc + j);
        crcs[region] = ToCrc32c(_mm_crc32_u64(
            static_cast<uint32_t>(crcs[region]), int_data[data_index]));

        *(udst + j) = int_data[data_index];
      }
    }

    *src += sizeof(__m128i);
    *dst += sizeof(__m128i);
    --copy_rounds;
  }
}

}  // namespace

template <int vec_regions, int int_regions>
class AcceleratedCrcMemcpyEngine : public CrcMemcpyEngine {
 public:
  AcceleratedCrcMemcpyEngine() = default;
  AcceleratedCrcMemcpyEngine(const AcceleratedCrcMemcpyEngine&) = delete;
  AcceleratedCrcMemcpyEngine operator=(const AcceleratedCrcMemcpyEngine&) =
      delete;

  crc32c_t Compute(void* __restrict dst, const void* __restrict src,
                   std::size_t length, crc32c_t initial_crc) const override;
};

template <int vec_regions, int int_regions>
crc32c_t AcceleratedCrcMemcpyEngine<vec_regions, int_regions>::Compute(
    void* __restrict dst, const void* __restrict src, std::size_t length,
    crc32c_t initial_crc) const {
  constexpr std::size_t kRegions = vec_regions + int_regions;
  constexpr crc32c_t kCrcDataXor = crc32c_t{0xffffffff};
  constexpr std::size_t kBlockSize = sizeof(__m128i);
  constexpr std::size_t kCopyRoundSize = kRegions * kBlockSize;

  constexpr std::size_t kBlocksPerCacheLine = ABSL_CACHELINE_SIZE / kBlockSize;

  char* dst_bytes = static_cast<char*>(dst);
  const char* src_bytes = static_cast<const char*>(src);


  static_assert(ABSL_CACHELINE_SIZE % kBlockSize == 0,
                "Cache lines are not divided evenly into blocks, may have "
                "unintended behavior!");



  constexpr size_t kCrcSmallSize = 256;


  constexpr std::size_t kPrefetchAhead = 2 * ABSL_CACHELINE_SIZE;

  if (length < kCrcSmallSize) {
    crc32c_t crc =
        ExtendCrc32c(initial_crc, absl::string_view(src_bytes, length));
    memcpy(dst, src, length);
    return crc;
  }



  initial_crc = initial_crc ^ kCrcDataXor;




  std::size_t bytes_from_last_aligned =
      reinterpret_cast<uintptr_t>(dst) & (kBlockSize - 1);
  if (bytes_from_last_aligned != 0) {
    std::size_t bytes_for_alignment = kBlockSize - bytes_from_last_aligned;

    initial_crc =
        ShortCrcCopy(dst_bytes, src_bytes, bytes_for_alignment, initial_crc);
    src_bytes += bytes_for_alignment;
    dst_bytes += bytes_for_alignment;
    length -= bytes_for_alignment;
  }






  crc32c_t crcs[kRegions];
  crcs[0] = initial_crc;
  for (int i = 1; i < kRegions; i++) {
    crcs[i] = kCrcDataXor;
  }


  int64_t copy_rounds = length / kCopyRoundSize;

  const std::size_t region_size = copy_rounds * kBlockSize;
  const std::size_t tail_size = length - (kRegions * region_size);

  __m128i vec_data[vec_regions];
  uint64_t int_data[int_regions * kIntLoadsPerVec];

  while (copy_rounds > kBlocksPerCacheLine) {

#pragma unroll_completely
    for (int i = 0; i < kRegions; i++) {
      absl::base_internal::PrefetchT0(src_bytes + kPrefetchAhead +
                                      region_size * i);
      absl::base_internal::PrefetchT0(dst_bytes + kPrefetchAhead +
                                      region_size * i);
    }

#pragma unroll_completely
    for (int i = 0; i < kBlocksPerCacheLine; i++) {

#pragma unroll_completely
      for (int j = 0; j < vec_regions; j++) {



        int region = (j + i) % kRegions;

        auto* src = reinterpret_cast<const __m128i_u*>(src_bytes +
                                                       region_size * region);
        auto* dst =
            reinterpret_cast<__m128i*>(dst_bytes + region_size * region);

        vec_data[j] = _mm_loadu_si128(src + i);
        crcs[region] =
            ToCrc32c(_mm_crc32_u64(static_cast<uint32_t>(crcs[region]),
                                   _mm_extract_epi64(vec_data[j], 0)));
        crcs[region] =
            ToCrc32c(_mm_crc32_u64(static_cast<uint32_t>(crcs[region]),
                                   _mm_extract_epi64(vec_data[j], 1)));

        _mm_store_si128(dst + i, vec_data[j]);
      }

#pragma unroll_completely
      for (int j = 0; j < int_regions; j++) {



        int region = (j + vec_regions + i) % kRegions;

        auto* usrc =
            reinterpret_cast<const uint64_t*>(src_bytes + region_size * region);
        auto* udst =
            reinterpret_cast<uint64_t*>(dst_bytes + region_size * region);

#pragma unroll_completely
        for (int k = 0; k < kIntLoadsPerVec; k++) {
          int data_index = j * kIntLoadsPerVec + k;

          int_data[data_index] = *(usrc + i * kIntLoadsPerVec + k);
          crcs[region] = ToCrc32c(_mm_crc32_u64(
              static_cast<uint32_t>(crcs[region]), int_data[data_index]));

          *(udst + i * kIntLoadsPerVec + k) = int_data[data_index];
        }
      }
    }

    src_bytes += kBlockSize * kBlocksPerCacheLine;
    dst_bytes += kBlockSize * kBlocksPerCacheLine;
    copy_rounds -= kBlocksPerCacheLine;
  }

  LargeTailCopy<vec_regions, int_regions>(crcs, &dst_bytes, &src_bytes,
                                          region_size, copy_rounds);

  src_bytes += region_size * (kRegions - 1);
  dst_bytes += region_size * (kRegions - 1);


  for (int i = 0; i < kRegions - 1; i++) {
    crcs[i] = crcs[i] ^ kCrcDataXor;
  }

  crc32c_t full_crc = crcs[0];
  for (int i = 1; i < kRegions - 1; i++) {
    full_crc = ConcatCrc32c(full_crc, crcs[i], region_size);
  }

  std::size_t tail_blocks = tail_size / kBlockSize;
  LargeTailCopy<0, 1>(&crcs[kRegions - 1], &dst_bytes, &src_bytes, 0,
                      tail_blocks);

  crcs[kRegions - 1] =
      ShortCrcCopy(dst_bytes, src_bytes, tail_size - tail_blocks * kBlockSize,
                   crcs[kRegions - 1]);

  crcs[kRegions - 1] = crcs[kRegions - 1] ^ kCrcDataXor;
  return ConcatCrc32c(full_crc, crcs[kRegions - 1], region_size + tail_size);
}

CrcMemcpy::ArchSpecificEngines CrcMemcpy::GetArchSpecificEngines() {
#ifdef UNDEFINED_BEHAVIOR_SANITIZER


  CpuType cpu_type = GetCpuType();
  switch (cpu_type) {
    case CpuType::kUnknown:
    case CpuType::kAmdRome:
    case CpuType::kAmdNaples:
    case CpuType::kIntelCascadelakeXeon:
    case CpuType::kIntelSkylakeXeon:
    case CpuType::kIntelSkylake:
    case CpuType::kIntelBroadwell:
    case CpuType::kIntelHaswell:
    case CpuType::kIntelIvybridge:
      return {
          .temporal = new FallbackCrcMemcpyEngine(),
          .non_temporal = new CrcNonTemporalMemcpyAVXEngine(),
      };

    case CpuType::kIntelSandybridge:
      return {
          .temporal = new FallbackCrcMemcpyEngine(),
          .non_temporal = new CrcNonTemporalMemcpyEngine(),
      };
    default:
      return {.temporal = new FallbackCrcMemcpyEngine(),
              .non_temporal = new FallbackCrcMemcpyEngine()};
  }
#else

  CpuType cpu_type = GetCpuType();
  switch (cpu_type) {









    case CpuType::kAmdRome:
    case CpuType::kAmdNaples:
      return {
          .temporal = new AcceleratedCrcMemcpyEngine<1, 2>(),
          .non_temporal = new CrcNonTemporalMemcpyAVXEngine(),
      };


    case CpuType::kIntelCascadelakeXeon:
    case CpuType::kIntelSkylakeXeon:
    case CpuType::kIntelSkylake:
    case CpuType::kIntelBroadwell:
    case CpuType::kIntelHaswell:
    case CpuType::kIntelIvybridge:
      return {
          .temporal = new AcceleratedCrcMemcpyEngine<3, 0>(),
          .non_temporal = new CrcNonTemporalMemcpyAVXEngine(),
      };

    case CpuType::kIntelSandybridge:
      return {
          .temporal = new AcceleratedCrcMemcpyEngine<3, 0>(),
          .non_temporal = new CrcNonTemporalMemcpyEngine(),
      };
    default:
      return {.temporal = new FallbackCrcMemcpyEngine(),
              .non_temporal = new FallbackCrcMemcpyEngine()};
  }
#endif  // UNDEFINED_BEHAVIOR_SANITIZER
}

std::unique_ptr<CrcMemcpyEngine> CrcMemcpy::GetTestEngine(int vector,
                                                          int integer) {
  if (vector == 3 && integer == 0) {
    return std::make_unique<AcceleratedCrcMemcpyEngine<3, 0>>();
  } else if (vector == 1 && integer == 2) {
    return std::make_unique<AcceleratedCrcMemcpyEngine<1, 2>>();
  }
  return nullptr;
}

}  // namespace crc_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // __SSE4_2__
