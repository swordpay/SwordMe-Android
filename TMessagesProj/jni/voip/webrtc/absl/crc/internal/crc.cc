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

// Treats the input as a polynomial with coefficients in Z(2),
// and finds the remainder when divided by an irreducible polynomial
// of the appropriate length.
// It handles all CRC sizes from 8 to 128 bits.
// It's somewhat complicated by having separate implementations optimized for
// CRC's <=32 bits, <= 64 bits, and <= 128 bits.
// The input string is prefixed with a "1" bit, and has "degree" "0" bits
// appended to it before the remainder is found.   This ensures that
// short strings are scrambled somewhat and that strings consisting
// of all nulls have a non-zero CRC.
//
// Uses the "interleaved word-by-word" method from
// "Everything we know about CRC but afraid to forget" by Andrew Kadatch
// and Bob Jenkins,
// http://crcutil.googlecode.com/files/crc-doc.1.0.pdf
//
// The idea is to compute kStride CRCs simultaneously, allowing the
// processor to more effectively use multiple execution units. Each of
// the CRCs is calculated on one word of data followed by kStride - 1
// words of zeroes; the CRC starting points are staggered by one word.
// Assuming a stride of 4 with data words "ABCDABCDABCD", the first
// CRC is over A000A000A, the second over 0B000B000B, and so on.
// The CRC of the whole data is then calculated by properly aligning the
// CRCs by appending zeroes until the data lengths agree then XORing
// the CRCs.

#include "absl/crc/internal/crc.h"

#include <cstdint>

#include "absl/base/internal/endian.h"
#include "absl/base/internal/prefetch.h"
#include "absl/base/internal/raw_logging.h"
#include "absl/crc/internal/crc_internal.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace crc_internal {

namespace {

#if defined(__i386__) || defined(__x86_64__)
constexpr bool kNeedAlignedLoads = false;
#else
constexpr bool kNeedAlignedLoads = true;
#endif

// pre-computing the zero extensions for all possible components of such an
// expression (numbers in a form a*ZEROES_BASE**b), we can calculate the
// resulting extension by multiplying the extensions for individual components
// using log_{ZEROES_BASE}(num_zeroes) polynomial multiplications. The tables of
// zero extensions contain (ZEROES_BASE - 1) * (log_{ZEROES_BASE}(64)) entries.
constexpr int ZEROES_BASE_LG = 4;                   // log_2(ZEROES_BASE)
constexpr int ZEROES_BASE = (1 << ZEROES_BASE_LG);  // must be a power of 2

constexpr uint32_t kCrc32cPoly = 0x82f63b78;

uint32_t ReverseBits(uint32_t bits) {
  bits = (bits & 0xaaaaaaaau) >> 1 | (bits & 0x55555555u) << 1;
  bits = (bits & 0xccccccccu) >> 2 | (bits & 0x33333333u) << 2;
  bits = (bits & 0xf0f0f0f0u) >> 4 | (bits & 0x0f0f0f0fu) << 4;
  return absl::gbswap_32(bits);
}

void PolyMultiply(uint32_t* val, uint32_t m, uint32_t poly) {
  uint32_t l = *val;
  uint32_t result = 0;
  auto onebit = uint32_t{0x80000000u};
  for (uint32_t one = onebit; one != 0; one >>= 1) {
    if ((l & one) != 0) {
      result ^= m;
    }
    if (m & 1) {
      m = (m >> 1) ^ poly;
    } else {
      m >>= 1;
    }
  }
  *val = result;
}
}  // namespace

void CRCImpl::FillWordTable(uint32_t poly, uint32_t last, int word_size,
                            Uint32By256* t) {
  for (int j = 0; j != word_size; j++) {  // for each byte of extension....
    t[j][0] = 0;                          // a zero has no effect
    for (int i = 128; i != 0; i >>= 1) {  // fill in entries for powers of 2
      if (j == 0 && i == 128) {
        t[j][i] = last;  // top bit in last byte is given
      } else {


        uint32_t pred;
        if (i == 128) {
          pred = t[j - 1][1];
        } else {
          pred = t[j][i << 1];
        }


        if (pred & 1) {
          t[j][i] = (pred >> 1) ^ poly;
        } else {
          t[j][i] = pred >> 1;
        }
      }
    }



    for (int i = 2; i != 256; i <<= 1) {
      for (int k = i + 1; k != (i << 1); k++) {
        t[j][k] = t[j][i] ^ t[j][k - i];
      }
    }
  }
}

int CRCImpl::FillZeroesTable(uint32_t poly, Uint32By256* t) {
  uint32_t inc = 1;
  inc <<= 31;

  inc >>= 1;

  for (int i = 0; i < 3; ++i) {
    PolyMultiply(&inc, inc, poly);
  }

  int j = 0;
  for (uint64_t inc_len = 1; inc_len != 0; inc_len <<= ZEROES_BASE_LG) {

    uint32_t v = inc;
    for (int a = 1; a != ZEROES_BASE; a++) {
      t[0][j] = v;
      PolyMultiply(&v, inc, poly);
      j++;
    }
    inc = v;
  }
  ABSL_RAW_CHECK(j <= 256, "");
  return j;
}

CRCImpl* CRCImpl::NewInternal() {

  CRCImpl* result = TryNewCRC32AcceleratedX86ARMCombined();

  if (result == nullptr) {
    result = new CRC32();
  }

  result->InitTables();

  return result;
}

void CRCImpl::Empty(uint32_t* crc) const { *crc = kCrc32cPoly; }


void CRC32::InitTables() {

  Uint32By256* t = new Uint32By256[4];
  FillWordTable(kCrc32cPoly, kCrc32cPoly, 1, t);
  for (int i = 0; i != 256; i++) {
    this->table0_[i] = t[0][i];
  }






  uint32_t last = kCrc32cPoly;
  const size_t size = 12;
  for (size_t i = 0; i < size; ++i) {
    last = (last >> 8) ^ this->table0_[last & 0xff];
  }
  FillWordTable(kCrc32cPoly, last, 4, t);
  for (size_t b = 0; b < 4; ++b) {
    for (int i = 0; i < 256; ++i) {
      this->table_[b][i] = t[b][i];
    }
  }

  int j = FillZeroesTable(kCrc32cPoly, t);
  ABSL_RAW_CHECK(j <= static_cast<int>(ABSL_ARRAYSIZE(this->zeroes_)), "");
  for (int i = 0; i < j; i++) {
    this->zeroes_[i] = t[0][i];
  }

  delete[] t;































  const uint32_t kCrc32cUnextendPoly =
      ReverseBits(static_cast<uint32_t>((kCrc32cPoly << 1) ^ 1));
  FillWordTable(kCrc32cUnextendPoly, kCrc32cUnextendPoly, 1, &reverse_table0_);

  j = FillZeroesTable(kCrc32cUnextendPoly, &reverse_zeroes_);
  ABSL_RAW_CHECK(j <= static_cast<int>(ABSL_ARRAYSIZE(this->reverse_zeroes_)),
                 "");
}

void CRC32::Extend(uint32_t* crc, const void* bytes, size_t length) const {
  const uint8_t* p = static_cast<const uint8_t*>(bytes);
  const uint8_t* e = p + length;
  uint32_t l = *crc;

  auto step_one_byte = [this, &p, &l] () {
    int c = (l & 0xff) ^ *p++;
    l = this->table0_[c] ^ (l >> 8);
  };

  if (kNeedAlignedLoads) {


    const uint8_t* x = RoundUp<4>(p);
    if (x <= e) {

      while (p != x) {
        step_one_byte();
      }
    }
  }

  const size_t kSwathSize = 16;
  if (static_cast<size_t>(e - p) >= kSwathSize) {

    uint32_t buf0 = absl::little_endian::Load32(p) ^ l;
    uint32_t buf1 = absl::little_endian::Load32(p + 4);
    uint32_t buf2 = absl::little_endian::Load32(p + 8);
    uint32_t buf3 = absl::little_endian::Load32(p + 12);
    p += kSwathSize;



    const auto step_swath = [this](uint32_t crc_in, const std::uint8_t* ptr) {
      return absl::little_endian::Load32(ptr) ^
             this->table_[3][crc_in & 0xff] ^
             this->table_[2][(crc_in >> 8) & 0xff] ^
             this->table_[1][(crc_in >> 16) & 0xff] ^
             this->table_[0][crc_in >> 24];
    };

    const auto step_stride = [&]() {
      buf0 = step_swath(buf0, p);
      buf1 = step_swath(buf1, p + 4);
      buf2 = step_swath(buf2, p + 8);
      buf3 = step_swath(buf3, p + 12);
      p += 16;
    };

    while ((e - p) > kPrefetchHorizon) {
      base_internal::PrefetchNta(
          reinterpret_cast<const void*>(p + kPrefetchHorizon));

      step_stride();
      step_stride();
      step_stride();
      step_stride();
    }
    while (static_cast<size_t>(e - p) >= kSwathSize) {
      step_stride();
    }


    while (static_cast<size_t>(e - p) >= 4) {
      buf0 = step_swath(buf0, p);
      uint32_t tmp = buf0;
      buf0 = buf1;
      buf1 = buf2;
      buf2 = buf3;
      buf3 = tmp;
      p += 4;
    }


    auto combine_one_word = [this](uint32_t crc_in, uint32_t w) {
      w ^= crc_in;
      for (size_t i = 0; i < 4; ++i) {
        w = (w >> 8) ^ this->table0_[w & 0xff];
      }
      return w;
    };

    l = combine_one_word(0, buf0);
    l = combine_one_word(l, buf1);
    l = combine_one_word(l, buf2);
    l = combine_one_word(l, buf3);
  }

  while (p != e) {
    step_one_byte();
  }

  *crc = l;
}

void CRC32::ExtendByZeroesImpl(uint32_t* crc, size_t length,
                               const uint32_t zeroes_table[256],
                               const uint32_t poly_table[256]) const {
  if (length != 0) {
    uint32_t l = *crc;





    for (int i = 0; length != 0;
         i += ZEROES_BASE - 1, length >>= ZEROES_BASE_LG) {
      int c = length & (ZEROES_BASE - 1);  // pick next ZEROES_BASE_LG bits
      if (c != 0) {                        // if they are not zero,



        uint64_t m = zeroes_table[c + i - 1];
        m <<= 1;
        uint64_t m2 = m << 1;
        uint64_t mtab[4] = {0, m, m2, m2 ^ m};

        uint64_t result = 0;
        for (int x = 0; x < 32; x += 8) {

          result ^= mtab[l & 3] ^ (mtab[(l >> 2) & 3] << 2) ^
                    (mtab[(l >> 4) & 3] << 4) ^ (mtab[(l >> 6) & 3] << 6);
          l >>= 8;

          result = (result >> 8) ^ poly_table[result & 0xff];
        }
        l = static_cast<uint32_t>(result);
      }
    }
    *crc = l;
  }
}

void CRC32::ExtendByZeroes(uint32_t* crc, size_t length) const {
  return CRC32::ExtendByZeroesImpl(crc, length, zeroes_, table0_);
}

void CRC32::UnextendByZeroes(uint32_t* crc, size_t length) const {


  *crc = ReverseBits(*crc);
  ExtendByZeroesImpl(crc, length, reverse_zeroes_, reverse_table0_);
  *crc = ReverseBits(*crc);
}

void CRC32::Scramble(uint32_t* crc) const {


  constexpr int scramble_rotate = (32 / 2) + 1;
  *crc = RotateRight<uint32_t>(static_cast<unsigned int>(*crc + kScrambleLo),
                               32, scramble_rotate) &
         MaskOfLength<uint32_t>(32);
}

void CRC32::Unscramble(uint32_t* crc) const {
  constexpr int scramble_rotate = (32 / 2) + 1;
  uint64_t rotated = RotateRight<uint32_t>(static_cast<unsigned int>(*crc), 32,
                                           32 - scramble_rotate);
  *crc = (rotated - kScrambleLo) & MaskOfLength<uint32_t>(32);
}

CRC::~CRC() {}
CRC::CRC() {}

CRC* CRC::Crc32c() {
  static CRC* singleton = CRCImpl::NewInternal();
  return singleton;
}

void CRC::Concat(uint32_t* px, uint32_t y, size_t ylen) {


















  uint32_t z;
  uint32_t t;
  Empty(&z);
  t = *px ^ z;
  ExtendByZeroes(&t, ylen);
  *px = t ^ y;
}

}  // namespace crc_internal
ABSL_NAMESPACE_END
}  // namespace absl
