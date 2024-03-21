// Copyright 2008, Google Inc.
// All rights reserved.
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
//
// GOOGLETEST_CM0001 DO NOT DELETE

#ifndef GTEST_INCLUDE_GTEST_GTEST_TEST_PART_H_
#define GTEST_INCLUDE_GTEST_GTEST_TEST_PART_H_

#include <iosfwd>
#include <vector>
#include "gtest/internal/gtest-internal.h"
#include "gtest/internal/gtest-string.h"

GTEST_DISABLE_MSC_WARNINGS_PUSH_(4251 \
/* class A needs to have dll-interface to be used by clients of class B */)

namespace testing {

// assertion or an explicit FAIL(), ADD_FAILURE(), or SUCCESS()).
//
// Don't inherit from TestPartResult as its destructor is not virtual.
class GTEST_API_ TestPartResult {
 public:


  enum Type {
    kSuccess,          // Succeeded.
    kNonFatalFailure,  // Failed but the test can continue.
    kFatalFailure,     // Failed and the test should be terminated.
    kSkip              // Skipped.
  };



  TestPartResult(Type a_type, const char* a_file_name, int a_line_number,
                 const char* a_message)
      : type_(a_type),
        file_name_(a_file_name == nullptr ? "" : a_file_name),
        line_number_(a_line_number),
        summary_(ExtractSummary(a_message)),
        message_(a_message) {}

  Type type() const { return type_; }


  const char* file_name() const {
    return file_name_.empty() ? nullptr : file_name_.c_str();
  }


  int line_number() const { return line_number_; }

  const char* summary() const { return summary_.c_str(); }

  const char* message() const { return message_.c_str(); }

  bool skipped() const { return type_ == kSkip; }

  bool passed() const { return type_ == kSuccess; }

  bool nonfatally_failed() const { return type_ == kNonFatalFailure; }

  bool fatally_failed() const { return type_ == kFatalFailure; }

  bool failed() const { return fatally_failed() || nonfatally_failed(); }

 private:
  Type type_;


  static std::string ExtractSummary(const char* message);


  std::string file_name_;


  int line_number_;
  std::string summary_;  // The test failure summary.
  std::string message_;  // The test failure message.
};

std::ostream& operator<<(std::ostream& os, const TestPartResult& result);

//
// Don't inherit from TestPartResultArray as its destructor is not
// virtual.
class GTEST_API_ TestPartResultArray {
 public:
  TestPartResultArray() {}

  void Append(const TestPartResult& result);

  const TestPartResult& GetTestPartResult(int index) const;

  int size() const;

 private:
  std::vector<TestPartResult> array_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(TestPartResultArray);
};

class GTEST_API_ TestPartResultReporterInterface {
 public:
  virtual ~TestPartResultReporterInterface() {}

  virtual void ReportTestPartResult(const TestPartResult& result) = 0;
};

namespace internal {

// statement generates new fatal failures. To do so it registers itself as the
// current test part result reporter. Besides checking if fatal failures were
// reported, it only delegates the reporting to the former result reporter.
// The original result reporter is restored in the destructor.
// INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
class GTEST_API_ HasNewFatalFailureHelper
    : public TestPartResultReporterInterface {
 public:
  HasNewFatalFailureHelper();
  ~HasNewFatalFailureHelper() override;
  void ReportTestPartResult(const TestPartResult& result) override;
  bool has_new_fatal_failure() const { return has_new_fatal_failure_; }
 private:
  bool has_new_fatal_failure_;
  TestPartResultReporterInterface* original_reporter_;

  GTEST_DISALLOW_COPY_AND_ASSIGN_(HasNewFatalFailureHelper);
};

}  // namespace internal

}  // namespace testing

GTEST_DISABLE_MSC_WARNINGS_POP_()  //  4251

#endif  // GTEST_INCLUDE_GTEST_GTEST_TEST_PART_H_
