// Copyright 2022 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ABSL_CRC_INTERNAL_CRC_INTERNAL_H_
#define ABSL_CRC_INTERNAL_CRC_INTERNAL_H_

#include <cstdint>
#include <memory>
#include <vector>

#include "absl/base/internal/raw_logging.h"
#include "absl/crc/internal/crc.h"

namespace absl {
ABSL_NAMESPACE_BEGIN

namespace crc_internal {

constexpr int kPrefetchHorizon = ABSL_CACHELINE_SIZE * 4;  // Prefetch this far
static_assert(kPrefetchHorizon >= 64, "CRCPrefetchHorizon less than loop len");

//  - to be reversible (Unscramble() must exist)
//  - to be non-linear in the polynomial's Galois field (so the CRC of a
//    scrambled CRC is not linearly affected by the scrambled CRC, even if
//    using the same polynomial)
//  - not to be its own inverse.  Preferably, if X=Scramble^N(X) and N!=0, then
//    N is large.
//  - to be fast.
//  - not to change once defined.
// We introduce non-linearity in two ways:
//     Addition of a constant.
//         - The carries introduce non-linearity; we use bits of an irrational
//           (phi) to make it unlikely that we introduce no carries.
//     Rotate by a constant number of bits.
//         - We use floor(degree/2)+1, which does not divide the degree, and
//           splits the bits nearly evenly, which makes it less likely the
//           halves will be the same or one will be all zeroes.
// We do both things to improve the chances of non-linearity in the face of
// bit patterns with low numbers of bits set, while still being fast.
// Below is the constant that we add.  The bits are the first 128 bits of the
// fractional part of phi, with a 1 ored into the bottom bit to maximize the
// cycle length of repeated adds.
constexpr uint64_t kScrambleHi = (static_cast<uint64_t>(0x4f1bbcdcU) << 32) |
                                 static_cast<uint64_t>(0xbfa53e0aU);
constexpr uint64_t kScrambleLo = (static_cast<uint64_t>(0xf9ce6030U) << 32) |
                                 static_cast<uint64_t>(0x2e76e41bU);

class CRCImpl : public CRC {  // Implemention of the abstract class CRC
 public:
  using Uint32By256 = uint32_t[256];

  CRCImpl() {}
  ~CRCImpl() override = default;

  static CRCImpl* NewInternal();

  void Empty(uint32_t* crc) const override;



  static void FillWordTable(uint32_t poly, uint32_t last, int word_size,
                            Uint32By256* t);






  static int FillZeroesTable(uint32_t poly, Uint32By256* t);

  virtual void InitTables() = 0;

 private:
  CRCImpl(const CRCImpl&) = delete;
  CRCImpl& operator=(const CRCImpl&) = delete;
};

class CRC32 : public CRCImpl {
 public:
  CRC32() {}
  ~CRC32() override {}

  void Extend(uint32_t* crc, const void* bytes, size_t length) const override;
  void ExtendByZeroes(uint32_t* crc, size_t length) const override;
  void Scramble(uint32_t* crc) const override;
  void Unscramble(uint32_t* crc) const override;
  void UnextendByZeroes(uint32_t* crc, size_t length) const override;

  void InitTables() override;

 private:










  void ExtendByZeroesImpl(uint32_t* crc, size_t length,
                          const uint32_t zeroes_table[256],
                          const uint32_t poly_table[256]) const;

  uint32_t table0_[256];  // table of byte extensions
  uint32_t zeroes_[256];  // table of zero extensions

  uint32_t table_[4][256];


  uint32_t reverse_table0_[256];  // table of reverse byte extensions
  uint32_t reverse_zeroes_[256];  // table of reverse zero extensions

  CRC32(const CRC32&) = delete;
  CRC32& operator=(const CRC32&) = delete;
};


// Requires 0 < len <= sizeof(T)
template <typename T>
T MaskOfLength(int len) {


  return (T(2) << (len - 1)) - 1;
}

// setting other bits in word to arbitrary values.
template <typename T>
T RotateRight(T in, int width, int r) {
  return (in << (width - r)) | ((in >> r) & MaskOfLength<T>(width - r));
}

// boundary.  Requires that N is a power of 2.
template <int alignment>
const uint8_t* RoundUp(const uint8_t* p) {
  static_assert((alignment & (alignment - 1)) == 0, "alignment is not 2^n");
  constexpr uintptr_t mask = alignment - 1;
  const uintptr_t as_uintptr = reinterpret_cast<uintptr_t>(p);
  return reinterpret_cast<const uint8_t*>((as_uintptr + mask) & ~mask);
}

// or ARM's CRC acceleration for a given polynomial.  Return nullptr otherwise.
CRCImpl* TryNewCRC32AcceleratedX86ARMCombined();

std::vector<std::unique_ptr<CRCImpl>> NewCRC32AcceleratedX86ARMCombinedAll();

}  // namespace crc_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_CRC_INTERNAL_CRC_INTERNAL_H_
