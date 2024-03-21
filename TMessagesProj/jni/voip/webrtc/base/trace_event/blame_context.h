// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_BLAME_CONTEXT_H_
#define BASE_TRACE_EVENT_BLAME_CONTEXT_H_

#include <inttypes.h>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/threading/thread_checker.h"
#include "base/trace_event/trace_log.h"

namespace base {
namespace trace_event {
class TracedValue;
}

namespace trace_event {

// different costs (e.g., CPU, network, or memory usage). An example of a blame
// context is an <iframe> element on a web page. Different subsystems can
// "enter" and "leave" blame contexts to indicate that they are doing work which
// should be accounted against this blame context.
//
// A blame context can optionally have a parent context, forming a blame context
// tree. When work is attributed to a particular blame context, it is considered
// to count against all of that context's children too. This is useful when work
// cannot be exactly attributed into a more specific context. For example,
// Javascript garbage collection generally needs to inspect all objects on a
// page instead looking at each <iframe> individually. In this case the work
// should be attributed to a blame context which is the parent of all <iframe>
// blame contexts.
class BASE_EXPORT BlameContext
    : public trace_event::TraceLog::AsyncEnabledStateObserver {
 public:



























  BlameContext(const char* category,
               const char* name,
               const char* type,
               const char* scope,
               int64_t id,
               const BlameContext* parent_context);
  ~BlameContext() override;


  void Initialize();




  void Enter();






  void Leave();



  void TakeSnapshot();

  const char* category() const { return category_; }
  const char* name() const { return name_; }
  const char* type() const { return type_; }
  const char* scope() const { return scope_; }
  int64_t id() const { return id_; }

  void OnTraceLogEnabled() override;
  void OnTraceLogDisabled() override;

 protected:




  virtual void AsValueInto(trace_event::TracedValue* state);

 private:
  bool WasInitialized() const;

  const char* category_;
  const char* name_;
  const char* type_;
  const char* scope_;
  const int64_t id_;

  const char* parent_scope_;
  const int64_t parent_id_;

  const unsigned char* category_group_enabled_;

  ThreadChecker thread_checker_;
  WeakPtrFactory<BlameContext> weak_factory_{this};

  DISALLOW_COPY_AND_ASSIGN(BlameContext);
};

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_BLAME_CONTEXT_H_
