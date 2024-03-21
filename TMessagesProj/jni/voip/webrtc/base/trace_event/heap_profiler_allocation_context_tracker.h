// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_HEAP_PROFILER_ALLOCATION_CONTEXT_TRACKER_H_
#define BASE_TRACE_EVENT_HEAP_PROFILER_ALLOCATION_CONTEXT_TRACKER_H_

#include <vector>

#include "base/atomicops.h"
#include "base/base_export.h"
#include "base/macros.h"
#include "base/trace_event/heap_profiler_allocation_context.h"

namespace base {
namespace trace_event {

// keep track of a pseudo stack of trace events. Chrome has been instrumented
// with lots of `TRACE_EVENT` macros. These trace events push their name to a
// thread-local stack when they go into scope, and pop when they go out of
// scope, if all of the following conditions have been met:
//
//  * A trace is being recorded.
//  * The category of the event is enabled in the trace config.
//  * Heap profiling is enabled (with the `--enable-heap-profiling` flag).
//
// This means that allocations that occur before tracing is started will not
// have backtrace information in their context.
//
// AllocationContextTracker also keeps track of some thread state not related to
// trace events. See |AllocationContext|.
//
// A thread-local instance of the context tracker is initialized lazily when it
// is first accessed. This might be because a trace event pushed or popped, or
// because `GetContextSnapshot()` was called when an allocation occurred
class BASE_EXPORT AllocationContextTracker {
 public:
  enum class CaptureMode : int32_t {
    DISABLED,      // Don't capture anything
    PSEUDO_STACK,  // Backtrace has trace events
    MIXED_STACK,   // Backtrace has trace events + from

    NATIVE_STACK,  // Backtrace has full native backtraces from stack unwinding
  };

  struct BASE_EXPORT PseudoStackFrame {
    const char* trace_event_category;
    const char* trace_event_name;

    bool operator==(const PseudoStackFrame& other) const {
      return trace_event_category == other.trace_event_category &&
             trace_event_name == other.trace_event_name;
    }
  };


  static void SetCaptureMode(CaptureMode mode);

  inline static CaptureMode capture_mode() {



    if (subtle::NoBarrier_Load(&capture_mode_) ==
            static_cast<int32_t>(CaptureMode::DISABLED))
      return CaptureMode::DISABLED;




    return static_cast<CaptureMode>(subtle::Acquire_Load(&capture_mode_));
  }



  static AllocationContextTracker* GetInstanceForCurrentThread();


  static void SetCurrentThreadName(const char* name);



  void begin_ignore_scope() { ignore_scope_depth_++; }
  void end_ignore_scope() {
    if (ignore_scope_depth_)
      ignore_scope_depth_--;
  }



  void PushPseudoStackFrame(PseudoStackFrame stack_frame);
  void PopPseudoStackFrame(PseudoStackFrame stack_frame);

  void PushNativeStackFrame(const void* pc);
  void PopNativeStackFrame(const void* pc);


  void PushCurrentTaskContext(const char* context);
  void PopCurrentTaskContext(const char* context);

  const char* TaskContext() const {
    return task_contexts_.empty() ? nullptr : task_contexts_.back();
  }


  bool GetContextSnapshot(AllocationContext* snapshot);

  ~AllocationContextTracker();

 private:
  AllocationContextTracker();

  static subtle::Atomic32 capture_mode_;

  std::vector<StackFrame> tracked_stack_;

  const char* thread_name_;


  std::vector<const char*> task_contexts_;

  uint32_t ignore_scope_depth_;

  DISALLOW_COPY_AND_ASSIGN(AllocationContextTracker);
};

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_HEAP_PROFILER_ALLOCATION_CONTEXT_TRACKER_H_
