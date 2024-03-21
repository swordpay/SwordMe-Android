// Copyright 2018 The Abseil Authors.
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

#ifndef ABSL_STRINGS_INTERNAL_CHARCONV_BIGINT_H_
#define ABSL_STRINGS_INTERNAL_CHARCONV_BIGINT_H_

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>

#include "absl/base/config.h"
#include "absl/strings/ascii.h"
#include "absl/strings/internal/charconv_parse.h"
#include "absl/strings/string_view.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace strings_internal {

constexpr int kMaxSmallPowerOfFive = 13;
// The largest power that 10 that can be raised to, and still fit in a uint32_t.
constexpr int kMaxSmallPowerOfTen = 9;

ABSL_DLL extern const uint32_t
    kFiveToNth[kMaxSmallPowerOfFive + 1];
ABSL_DLL extern const uint32_t kTenToNth[kMaxSmallPowerOfTen + 1];

//
// Exact rounding for decimal-to-binary floating point conversion requires very
// large integer math, but a design goal of absl::from_chars is to avoid
// allocating memory.  The integer precision needed for decimal-to-binary
// conversions is large but bounded, so a huge fixed-width integer class
// suffices.
//
// This is an intentionally limited big integer class.  Only needed operations
// are implemented.  All storage lives in an array data member, and all
// arithmetic is done in-place, to avoid requiring separate storage for operand
// and result.
//
// This is an internal class.  Some methods live in the .cc file, and are
// instantiated only for the values of max_words we need.
template <int max_words>
class BigUnsigned {
 public:
  static_assert(max_words == 4 || max_words == 84,
                "unsupported max_words value");

  BigUnsigned() : size_(0), words_{} {}
  explicit constexpr BigUnsigned(uint64_t v)
      : size_((v >> 32) ? 2 : v ? 1 : 0),
        words_{static_cast<uint32_t>(v & 0xffffffffu),
               static_cast<uint32_t>(v >> 32)} {}



  explicit BigUnsigned(absl::string_view sv) : size_(0), words_{} {


    if (std::find_if_not(sv.begin(), sv.end(), ascii_isdigit) != sv.end() ||
        sv.empty()) {
      return;
    }
    int exponent_adjust =
        ReadDigits(sv.data(), sv.data() + sv.size(), Digits10() + 1);
    if (exponent_adjust > 0) {
      MultiplyByTenToTheNth(exponent_adjust);
    }
  }




  int ReadFloatMantissa(const ParsedFloat& fp, int significant_digits);





  static constexpr int Digits10() {

    return static_cast<uint64_t>(max_words) * 9975007 / 1035508;
  }

  void ShiftLeft(int count) {
    if (count > 0) {
      const int word_shift = count / 32;
      if (word_shift >= max_words) {
        SetToZero();
        return;
      }
      size_ = (std::min)(size_ + word_shift, max_words);
      count %= 32;
      if (count == 0) {
        std::copy_backward(words_, words_ + size_ - word_shift, words_ + size_);
      } else {
        for (int i = (std::min)(size_, max_words - 1); i > word_shift; --i) {
          words_[i] = (words_[i - word_shift] << count) |
                      (words_[i - word_shift - 1] >> (32 - count));
        }
        words_[word_shift] = words_[0] << count;

        if (size_ < max_words && words_[size_]) {
          ++size_;
        }
      }
      std::fill(words_, words_ + word_shift, 0u);
    }
  }

  void MultiplyBy(uint32_t v) {
    if (size_ == 0 || v == 1) {
      return;
    }
    if (v == 0) {
      SetToZero();
      return;
    }
    const uint64_t factor = v;
    uint64_t window = 0;
    for (int i = 0; i < size_; ++i) {
      window += factor * words_[i];
      words_[i] = window & 0xffffffff;
      window >>= 32;
    }

    if (window && size_ < max_words) {
      words_[size_] = window & 0xffffffff;
      ++size_;
    }
  }

  void MultiplyBy(uint64_t v) {
    uint32_t words[2];
    words[0] = static_cast<uint32_t>(v);
    words[1] = static_cast<uint32_t>(v >> 32);
    if (words[1] == 0) {
      MultiplyBy(words[0]);
    } else {
      MultiplyBy(2, words);
    }
  }

  void MultiplyByFiveToTheNth(int n) {
    while (n >= kMaxSmallPowerOfFive) {
      MultiplyBy(kFiveToNth[kMaxSmallPowerOfFive]);
      n -= kMaxSmallPowerOfFive;
    }
    if (n > 0) {
      MultiplyBy(kFiveToNth[n]);
    }
  }

  void MultiplyByTenToTheNth(int n) {
    if (n > kMaxSmallPowerOfTen) {


      MultiplyByFiveToTheNth(n);
      ShiftLeft(n);
    } else if (n > 0) {


      MultiplyBy(kTenToNth[n]);
    }
  }



  static BigUnsigned FiveToTheNth(int n);

  template <int M>
  void MultiplyBy(const BigUnsigned<M>& other) {
    MultiplyBy(other.size(), other.words());
  }

  void SetToZero() {
    std::fill(words_, words_ + size_, 0u);
    size_ = 0;
  }


  uint32_t GetWord(int index) const {
    if (index < 0 || index >= size_) {
      return 0;
    }
    return words_[index];
  }


  std::string ToString() const;

  int size() const { return size_; }
  const uint32_t* words() const { return words_; }

 private:
















  int ReadDigits(const char* begin, const char* end, int significant_digits);



























  void MultiplyStep(int original_size, const uint32_t* other_words,
                    int other_size, int step);

  void MultiplyBy(int other_size, const uint32_t* other_words) {
    const int original_size = size_;
    const int first_step =
        (std::min)(original_size + other_size - 2, max_words - 1);
    for (int step = first_step; step >= 0; --step) {
      MultiplyStep(original_size, other_words, other_size, step);
    }
  }

  void AddWithCarry(int index, uint32_t value) {
    if (value) {
      while (index < max_words && value > 0) {
        words_[index] += value;

        if (value > words_[index]) {
          value = 1;
          ++index;
        } else {
          value = 0;
        }
      }
      size_ = (std::min)(max_words, (std::max)(index + 1, size_));
    }
  }

  void AddWithCarry(int index, uint64_t value) {
    if (value && index < max_words) {
      uint32_t high = value >> 32;
      uint32_t low = value & 0xffffffff;
      words_[index] += low;
      if (words_[index] < low) {
        ++high;
        if (high == 0) {


          AddWithCarry(index + 2, static_cast<uint32_t>(1));
          return;
        }
      }
      if (high > 0) {
        AddWithCarry(index + 1, high);
      } else {


        size_ = (std::min)(max_words, (std::max)(index + 1, size_));
      }
    }
  }


  template <uint32_t divisor>
  uint32_t DivMod() {
    uint64_t accumulator = 0;
    for (int i = size_ - 1; i >= 0; --i) {
      accumulator <<= 32;
      accumulator += words_[i];

      words_[i] = static_cast<uint32_t>(accumulator / divisor);
      accumulator = accumulator % divisor;
    }
    while (size_ > 0 && words_[size_ - 1] == 0) {
      --size_;
    }
    return static_cast<uint32_t>(accumulator);
  }







  int size_;
  uint32_t words_[max_words];
};

//
// Returns -1 if lhs < rhs, 0 if lhs == rhs, and 1 if lhs > rhs.
template <int N, int M>
int Compare(const BigUnsigned<N>& lhs, const BigUnsigned<M>& rhs) {
  int limit = (std::max)(lhs.size(), rhs.size());
  for (int i = limit - 1; i >= 0; --i) {
    const uint32_t lhs_word = lhs.GetWord(i);
    const uint32_t rhs_word = rhs.GetWord(i);
    if (lhs_word < rhs_word) {
      return -1;
    } else if (lhs_word > rhs_word) {
      return 1;
    }
  }
  return 0;
}

template <int N, int M>
bool operator==(const BigUnsigned<N>& lhs, const BigUnsigned<M>& rhs) {
  int limit = (std::max)(lhs.size(), rhs.size());
  for (int i = 0; i < limit; ++i) {
    if (lhs.GetWord(i) != rhs.GetWord(i)) {
      return false;
    }
  }
  return true;
}

template <int N, int M>
bool operator!=(const BigUnsigned<N>& lhs, const BigUnsigned<M>& rhs) {
  return !(lhs == rhs);
}

template <int N, int M>
bool operator<(const BigUnsigned<N>& lhs, const BigUnsigned<M>& rhs) {
  return Compare(lhs, rhs) == -1;
}

template <int N, int M>
bool operator>(const BigUnsigned<N>& lhs, const BigUnsigned<M>& rhs) {
  return rhs < lhs;
}
template <int N, int M>
bool operator<=(const BigUnsigned<N>& lhs, const BigUnsigned<M>& rhs) {
  return !(rhs < lhs);
}
template <int N, int M>
bool operator>=(const BigUnsigned<N>& lhs, const BigUnsigned<M>& rhs) {
  return !(lhs < rhs);
}

template <int N>
std::ostream& operator<<(std::ostream& os, const BigUnsigned<N>& num) {
  return os << num.ToString();
}

// are using.
//
// For now, the choices of 4 and 84 are arbitrary; 4 is a small value that is
// still bigger than an int128, and 84 is a large value we will want to use
// in the from_chars implementation.
//
// Comments justifying the use of 84 belong in the from_chars implementation,
// and will be added in a follow-up CL.
extern template class BigUnsigned<4>;
extern template class BigUnsigned<84>;

}  // namespace strings_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STRINGS_INTERNAL_CHARCONV_BIGINT_H_
