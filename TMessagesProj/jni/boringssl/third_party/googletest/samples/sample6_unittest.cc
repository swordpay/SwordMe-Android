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

// implementations of the same interface (aka interface tests).

#include "prime_tables.h"

#include "gtest/gtest.h"
namespace {
// First, we define some factory functions for creating instances of
// the implementations.  You may be able to skip this step if all your
// implementations can be constructed the same way.

template <class T>
PrimeTable* CreatePrimeTable();

template <>
PrimeTable* CreatePrimeTable<OnTheFlyPrimeTable>() {
  return new OnTheFlyPrimeTable;
}

template <>
PrimeTable* CreatePrimeTable<PreCalculatedPrimeTable>() {
  return new PreCalculatedPrimeTable(10000);
}

template <class T>
class PrimeTableTest : public testing::Test {
 protected:


  PrimeTableTest() : table_(CreatePrimeTable<T>()) {}

  ~PrimeTableTest() override { delete table_; }







  PrimeTable* const table_;
};

#if GTEST_HAS_TYPED_TEST

using testing::Types;

// The first is called "typed tests".  You should use it if you
// already know *all* the types you are gonna exercise when you write
// the tests.

//
//   TYPED_TEST_SUITE(TestCaseName, TypeList);
//
// to declare it and specify the type parameters.  As with TEST_F,
// TestCaseName must match the test fixture name.

typedef Types<OnTheFlyPrimeTable, PreCalculatedPrimeTable> Implementations;

TYPED_TEST_SUITE(PrimeTableTest, Implementations);

// similar to TEST_F.
TYPED_TEST(PrimeTableTest, ReturnsFalseForNonPrimes) {






  EXPECT_FALSE(this->table_->IsPrime(-5));
  EXPECT_FALSE(this->table_->IsPrime(0));
  EXPECT_FALSE(this->table_->IsPrime(1));
  EXPECT_FALSE(this->table_->IsPrime(4));
  EXPECT_FALSE(this->table_->IsPrime(6));
  EXPECT_FALSE(this->table_->IsPrime(100));
}

TYPED_TEST(PrimeTableTest, ReturnsTrueForPrimes) {
  EXPECT_TRUE(this->table_->IsPrime(2));
  EXPECT_TRUE(this->table_->IsPrime(3));
  EXPECT_TRUE(this->table_->IsPrime(5));
  EXPECT_TRUE(this->table_->IsPrime(7));
  EXPECT_TRUE(this->table_->IsPrime(11));
  EXPECT_TRUE(this->table_->IsPrime(131));
}

TYPED_TEST(PrimeTableTest, CanGetNextPrime) {
  EXPECT_EQ(2, this->table_->GetNextPrime(0));
  EXPECT_EQ(3, this->table_->GetNextPrime(2));
  EXPECT_EQ(5, this->table_->GetNextPrime(3));
  EXPECT_EQ(7, this->table_->GetNextPrime(5));
  EXPECT_EQ(11, this->table_->GetNextPrime(7));
  EXPECT_EQ(131, this->table_->GetNextPrime(128));
}

// in the type list specified in TYPED_TEST_SUITE.  Sit back and be
// happy that you don't have to define them multiple times.

#endif  // GTEST_HAS_TYPED_TEST

#if GTEST_HAS_TYPED_TEST_P

using testing::Types;

// to test when you write the tests.  For example, if you are the
// author of an interface and expect other people to implement it, you
// might want to write a set of tests to make sure each implementation
// conforms to some basic requirements, but you don't know what
// implementations will be written in the future.
//
// How can you write the tests without committing to the type
// parameters?  That's what "type-parameterized tests" can do for you.
// It is a bit more involved than typed tests, but in return you get a
// test pattern that can be reused in many contexts, which is a big
// win.  Here's how you do it:

// the PrimeTableTest fixture defined earlier:

template <class T>
class PrimeTableTest2 : public PrimeTableTest<T> {
};

// fixture, and also the name of the test case (as usual).  The _P
// suffix is for "parameterized" or "pattern".
TYPED_TEST_SUITE_P(PrimeTableTest2);

// similar to what you do with TEST_F.
TYPED_TEST_P(PrimeTableTest2, ReturnsFalseForNonPrimes) {
  EXPECT_FALSE(this->table_->IsPrime(-5));
  EXPECT_FALSE(this->table_->IsPrime(0));
  EXPECT_FALSE(this->table_->IsPrime(1));
  EXPECT_FALSE(this->table_->IsPrime(4));
  EXPECT_FALSE(this->table_->IsPrime(6));
  EXPECT_FALSE(this->table_->IsPrime(100));
}

TYPED_TEST_P(PrimeTableTest2, ReturnsTrueForPrimes) {
  EXPECT_TRUE(this->table_->IsPrime(2));
  EXPECT_TRUE(this->table_->IsPrime(3));
  EXPECT_TRUE(this->table_->IsPrime(5));
  EXPECT_TRUE(this->table_->IsPrime(7));
  EXPECT_TRUE(this->table_->IsPrime(11));
  EXPECT_TRUE(this->table_->IsPrime(131));
}

TYPED_TEST_P(PrimeTableTest2, CanGetNextPrime) {
  EXPECT_EQ(2, this->table_->GetNextPrime(0));
  EXPECT_EQ(3, this->table_->GetNextPrime(2));
  EXPECT_EQ(5, this->table_->GetNextPrime(3));
  EXPECT_EQ(7, this->table_->GetNextPrime(5));
  EXPECT_EQ(11, this->table_->GetNextPrime(7));
  EXPECT_EQ(131, this->table_->GetNextPrime(128));
}

// enumerate the tests you defined:
REGISTER_TYPED_TEST_SUITE_P(
    PrimeTableTest2,  // The first argument is the test case name.

    ReturnsFalseForNonPrimes, ReturnsTrueForPrimes, CanGetNextPrime);

// any real test yet as you haven't said which types you want to run
// the tests with.

// it with a list of types.  Usually the test pattern will be defined
// in a .h file, and anyone can #include and instantiate it.  You can
// even instantiate it more than once in the same program.  To tell
// different instances apart, you give each of them a name, which will
// become part of the test case name and can be used in test filters.

// defined at the time we write the TYPED_TEST_P()s.
typedef Types<OnTheFlyPrimeTable, PreCalculatedPrimeTable>
    PrimeTableImplementations;
INSTANTIATE_TYPED_TEST_SUITE_P(OnTheFlyAndPreCalculated,    // Instance name
                               PrimeTableTest2,             // Test case name
                               PrimeTableImplementations);  // Type list

#endif  // GTEST_HAS_TYPED_TEST_P
}  // namespace
