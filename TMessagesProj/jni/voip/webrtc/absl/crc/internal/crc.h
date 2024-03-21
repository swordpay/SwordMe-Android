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

#ifndef ABSL_CRC_INTERNAL_CRC_H_
#define ABSL_CRC_INTERNAL_CRC_H_

#include <cstdint>

#include "absl/base/config.h"

// Treats the input as a polynomial with coefficients in Z(2),
// and finds the remainder when divided by an primitive polynomial
// of the appropriate length.

// but with the highest order bit not stored.
// The highest degree coefficient is stored in the lowest numbered bit
// in the lowest addressed byte.   Thus, in what follows, the highest degree
// coefficient that is stored is in the low order bit of "lo" or "*lo".


namespace absl {
ABSL_NAMESPACE_BEGIN
namespace crc_internal {

class CRC {
 public:
  virtual ~CRC();

  virtual void Empty(uint32_t* crc) const = 0;



  virtual void Extend(uint32_t* crc, const void* bytes,
                      size_t length) const = 0;


  virtual void ExtendByZeroes(uint32_t* crc, size_t length) const = 0;



  virtual void UnextendByZeroes(uint32_t* crc, size_t length) const = 0;



  virtual void Concat(uint32_t* px, uint32_t y, size_t ylen);






  virtual void Scramble(uint32_t* crc) const = 0;
  virtual void Unscramble(uint32_t* crc) const = 0;


  static CRC* Crc32c();

 protected:
  CRC();  // Clients may not call constructor; use Crc32c() instead.

 private:
  CRC(const CRC&) = delete;
  CRC& operator=(const CRC&) = delete;
};

}  // namespace crc_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_CRC_INTERNAL_CRC_H_
