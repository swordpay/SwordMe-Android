// Copyright 2005, Google Inc.
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

// The Google C++ Testing and Mocking Framework (Google Test)
//
// This header file defines the Message class.
//
// IMPORTANT NOTE: Due to limitation of the C++ language, we have to
// leave some internal implementation details in this header file.
// They are clearly marked by comments like this:
//
//   // INTERNAL IMPLEMENTATION - DO NOT USE IN A USER PROGRAM.
//
// Such code is NOT meant to be used by a user directly, and is subject
// to CHANGE WITHOUT NOTICE.  Therefore DO NOT DEPEND ON IT in a user
// program!


#ifndef GTEST_INCLUDE_GTEST_GTEST_MESSAGE_H_
#define GTEST_INCLUDE_GTEST_GTEST_MESSAGE_H_

#include <limits>
#include <memory>

#include "gtest/internal/gtest-port.h"

GTEST_DISABLE_MSC_WARNINGS_PUSH_(4251 \
/* class A needs to have dll-interface to be used by clients of class B */)

// See Message& operator<<(...) below for why.
void operator<<(const testing::internal::Secret&, int);

namespace testing {

//
// Typical usage:
//
//   1. You stream a bunch of values to a Message object.
//      It will remember the text in a stringstream.
//   2. Then you stream the Message object to an ostream.
//      This causes the text in the Message to be streamed
//      to the ostream.
//
// For example;
//
//   testing::Message foo;
//   foo << 1 << " != " << 2;
//   std::cout << foo;
//
// will print "1 != 2".
//
// Message is not intended to be inherited from.  In particular, its
// destructor is not virtual.
//
// Note that stringstream behaves differently in gcc and in MSVC.  You
// can stream a NULL char pointer to it in the former, but not in the
// latter (it causes an access violation if you do).  The Message
// class hides this difference by treating a NULL char pointer as
// "(null)".
class GTEST_API_ Message {
 private:


  typedef std::ostream& (*BasicNarrowIoManip)(std::ostream&);

 public:

  Message();

  Message(const Message& msg) : ss_(new ::std::stringstream) {  // NOLINT
    *ss_ << msg.GetString();
  }

  explicit Message(const char* str) : ss_(new ::std::stringstream) {
    *ss_ << str;
  }

  template <typename T>
  inline Message& operator <<(const T& val) {














    using ::operator <<;
    *ss_ << val;
    return *this;
  }













  template <typename T>
  inline Message& operator <<(T* const& pointer) {  // NOLINT
    if (pointer == nullptr) {
      *ss_ << "(null)";
    } else {
      *ss_ << pointer;
    }
    return *this;
  }






  Message& operator <<(BasicNarrowIoManip val) {
    *ss_ << val;
    return *this;
  }

  Message& operator <<(bool b) {
    return *this << (b ? "true" : "false");
  }


  Message& operator <<(const wchar_t* wide_c_str);
  Message& operator <<(wchar_t* wide_c_str);

#if GTEST_HAS_STD_WSTRING


  Message& operator <<(const ::std::wstring& wstr);
#endif  // GTEST_HAS_STD_WSTRING




  std::string GetString() const;

 private:

  const std::unique_ptr< ::std::stringstream> ss_;


  void operator=(const Message&);
};

inline std::ostream& operator <<(std::ostream& os, const Message& sb) {
  return os << sb.GetString();
}

namespace internal {

// converted to "(null)".  When the input value is a ::string,
// ::std::string, ::wstring, or ::std::wstring object, each NUL
// character in it is replaced with "\\0".
template <typename T>
std::string StreamableToString(const T& streamable) {
  return (Message() << streamable).GetString();
}

}  // namespace internal
}  // namespace testing

GTEST_DISABLE_MSC_WARNINGS_POP_()  //  4251

#endif  // GTEST_INCLUDE_GTEST_GTEST_MESSAGE_H_
