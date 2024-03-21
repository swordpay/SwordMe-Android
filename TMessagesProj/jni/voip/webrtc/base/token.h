// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TOKEN_H_
#define BASE_TOKEN_H_

#include <stdint.h>

#include <iosfwd>
#include <tuple>

#include "base/base_export.h"
#include "base/hash/hash.h"
#include "base/optional.h"

namespace base {

// from a cryptographically strong random source, or constexpr construction over
// fixed values (e.g. to store a pre-generated constant value). Tokens are
// similar in spirit and purpose to UUIDs, without many of the constraints and
// expectations (such as byte layout and string representation) clasically
// associated with UUIDs.
class BASE_EXPORT Token {
 public:

  constexpr Token() : high_(0), low_(0) {}

  constexpr Token(uint64_t high, uint64_t low) : high_(high), low_(low) {}


  static Token CreateRandom();

  uint64_t high() const { return high_; }
  uint64_t low() const { return low_; }

  bool is_zero() const { return high_ == 0 && low_ == 0; }

  bool operator==(const Token& other) const {
    return high_ == other.high_ && low_ == other.low_;
  }

  bool operator!=(const Token& other) const { return !(*this == other); }

  bool operator<(const Token& other) const {
    return std::tie(high_, low_) < std::tie(other.high_, other.low_);
  }

  std::string ToString() const;

 private:



  uint64_t high_;
  uint64_t low_;
};

struct TokenHash {
  size_t operator()(const base::Token& token) const {
    return base::HashInts64(token.high(), token.low());
  }
};

class Pickle;
class PickleIterator;

BASE_EXPORT void WriteTokenToPickle(Pickle* pickle, const Token& token);
BASE_EXPORT Optional<Token> ReadTokenFromPickle(
    PickleIterator* pickle_iterator);

}  // namespace base

#endif  // BASE_TOKEN_H_
