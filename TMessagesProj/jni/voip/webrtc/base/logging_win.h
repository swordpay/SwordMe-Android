// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_LOGGING_WIN_H_
#define BASE_LOGGING_WIN_H_

#include <stddef.h>

#include <string>

#include "base/base_export.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/win/event_trace_provider.h"

namespace base {
template <typename Type>
struct StaticMemorySingletonTraits;
}  // namespace base

namespace logging {

EXTERN_C BASE_EXPORT const GUID kLogEventId;

enum LogEnableMask {


  ENABLE_STACK_TRACE_CAPTURE = 0x0001,



  ENABLE_LOG_MESSAGE_ONLY = 0x0002,
};

// ETW likes user message types to start at 10.
enum LogMessageTypes {

  LOG_MESSAGE = 10,


  LOG_MESSAGE_WITH_STACKTRACE = 11,





  LOG_MESSAGE_FULL = 12,
};

// with Event Tracing for Windows.
class BASE_EXPORT LogEventProvider : public base::win::EtwTraceProvider {
 public:
  static LogEventProvider* GetInstance();

  static bool LogMessage(logging::LogSeverity severity, const char* file,
      int line, size_t message_start, const std::string& str);

  static void Initialize(const GUID& provider_name);
  static void Uninitialize();

 protected:

  void OnEventsEnabled() override;
  void OnEventsDisabled() override;

 private:
  LogEventProvider();


  logging::LogSeverity old_log_level_;

  friend struct base::StaticMemorySingletonTraits<LogEventProvider>;
  DISALLOW_COPY_AND_ASSIGN(LogEventProvider);
};

}  // namespace logging

#endif  // BASE_LOGGING_WIN_H_
