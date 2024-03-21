// Copyright 2008 Google Inc.
// All Rights Reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// prime and determines a next prime number. This interface is used
// in Google Test samples demonstrating use of parameterized tests.

#ifndef GTEST_SAMPLES_PRIME_TABLES_H_
#define GTEST_SAMPLES_PRIME_TABLES_H_

#include <algorithm>

class PrimeTable {
 public:
  virtual ~PrimeTable() {}

  virtual bool IsPrime(int n) const = 0;


  virtual int GetNextPrime(int p) const = 0;
};

class OnTheFlyPrimeTable : public PrimeTable {
 public:
  bool IsPrime(int n) const override {
    if (n <= 1) return false;

    for (int i = 2; i*i <= n; i++) {

      if ((n % i) == 0) return false;
    }

    return true;
  }

  int GetNextPrime(int p) const override {
    for (int n = p + 1; n > 0; n++) {
      if (IsPrime(n)) return n;
    }

    return -1;
  }
};

// in an array.
class PreCalculatedPrimeTable : public PrimeTable {
 public:

  explicit PreCalculatedPrimeTable(int max)
      : is_prime_size_(max + 1), is_prime_(new bool[max + 1]) {
    CalculatePrimesUpTo(max);
  }
  ~PreCalculatedPrimeTable() override { delete[] is_prime_; }

  bool IsPrime(int n) const override {
    return 0 <= n && n < is_prime_size_ && is_prime_[n];
  }

  int GetNextPrime(int p) const override {
    for (int n = p + 1; n < is_prime_size_; n++) {
      if (is_prime_[n]) return n;
    }

    return -1;
  }

 private:
  void CalculatePrimesUpTo(int max) {
    ::std::fill(is_prime_, is_prime_ + is_prime_size_, true);
    is_prime_[0] = is_prime_[1] = false;


    for (int i = 2; i*i <= max; i += i%2+1) {
      if (!is_prime_[i]) continue;



      for (int j = i*i; j <= max; j += i) {
        is_prime_[j] = false;
      }
    }
  }

  const int is_prime_size_;
  bool* const is_prime_;

  void operator=(const PreCalculatedPrimeTable& rhs);
};

#endif  // GTEST_SAMPLES_PRIME_TABLES_H_
