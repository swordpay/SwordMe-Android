// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_TRACING_AGENT_H_
#define BASE_TRACE_EVENT_TRACING_AGENT_H_

#include "base/base_export.h"
#include "base/callback.h"
#include "base/memory/ref_counted_memory.h"
#include "base/values.h"

namespace base {

class TimeTicks;

namespace trace_event {

class TraceConfig;

// tracing method that produces its own trace log should implement this
// interface. All tracing agents must only be controlled by TracingController.
// Some existing examples include TracingControllerImpl for Chrome trace events,
// DebugDaemonClient for CrOs system trace, and EtwTracingAgent for Windows
// system.
class BASE_EXPORT TracingAgent {
 public:
  using StartAgentTracingCallback =
      base::OnceCallback<void(const std::string& agent_name, bool success)>;


  using StopAgentTracingCallback = base::OnceCallback<void(
      const std::string& agent_name,
      const std::string& events_label,
      const scoped_refptr<base::RefCountedString>& events_str_ptr)>;
  using RecordClockSyncMarkerCallback =
      base::OnceCallback<void(const std::string& sync_id,
                              const TimeTicks& issue_ts,
                              const TimeTicks& issue_end_ts)>;

  virtual ~TracingAgent();


  virtual std::string GetTracingAgentName() = 0;







  virtual std::string GetTraceEventLabel() = 0;

  virtual void StartAgentTracing(const TraceConfig& trace_config,
                                 StartAgentTracingCallback callback) = 0;


  virtual void StopAgentTracing(StopAgentTracingCallback callback) = 0;

  virtual bool SupportsExplicitClockSync();





















  virtual void RecordClockSyncMarker(const std::string& sync_id,
                                     RecordClockSyncMarkerCallback callback);
};

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_TRACING_AGENT_H_
