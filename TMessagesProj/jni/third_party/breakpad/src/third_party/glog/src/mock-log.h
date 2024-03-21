// Copyright (c) 2007, Google Inc.
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
// Author: Zhanyong Wan
//
// Defines the ScopedMockLog class (using Google C++ Mocking
// Framework), which is convenient for testing code that uses LOG().

#ifndef GLOG_SRC_MOCK_LOG_H_
#define GLOG_SRC_MOCK_LOG_H_

#include "utilities.h"

#include <string>

#include <gmock/gmock.h>

#include "glog/logging.h"

_START_GOOGLE_NAMESPACE_
namespace glog_testing {

// lifespan.  Using this together with Google C++ Mocking Framework,
// it's very easy to test how a piece of code calls LOG().  The
// typical usage:
//
//   TEST(FooTest, LogsCorrectly) {
//     ScopedMockLog log;
//
//     // We expect the WARNING "Something bad!" exactly twice.
//     EXPECT_CALL(log, Log(WARNING, _, "Something bad!"))
//         .Times(2);
//
//     // We allow foo.cc to call LOG(INFO) any number of times.
//     EXPECT_CALL(log, Log(INFO, HasSubstr("/foo.cc"), _))
//         .Times(AnyNumber());
//
//     Foo();  // Exercises the code under test.
//   }
class ScopedMockLog : public GOOGLE_NAMESPACE::LogSink {
 public:


  ScopedMockLog() { AddLogSink(this); }

  virtual ~ScopedMockLog() { RemoveLogSink(this); }













  MOCK_METHOD3(Log, void(GOOGLE_NAMESPACE::LogSeverity severity,
                         const std::string& file_path,
                         const std::string& message));

 private:





















  virtual void send(GOOGLE_NAMESPACE::LogSeverity severity,
                    const char* full_filename,
                    const char* base_filename, int line, const tm* tm_time,
                    const char* message, size_t message_len) {


    message_info_.severity = severity;
    message_info_.file_path = full_filename;
    message_info_.message = std::string(message, message_len);
  }







  virtual void WaitTillSent() {



    MessageInfo message_info = message_info_;
    Log(message_info.severity, message_info.file_path, message_info.message);
  }


  struct MessageInfo {
    GOOGLE_NAMESPACE::LogSeverity severity;
    std::string file_path;
    std::string message;
  };
  MessageInfo message_info_;
};

}  // namespace glog_testing
_END_GOOGLE_NAMESPACE_

#endif  // GLOG_SRC_MOCK_LOG_H_
