// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/threading/platform_thread.h"

#import <Foundation/Foundation.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <mach/thread_policy.h>
#include <stddef.h>
#include <sys/resource.h>

#include <algorithm>

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/mac/foundation_util.h"
#include "base/mac/mach_logging.h"
#include "base/threading/thread_id_name_manager.h"
#include "build/build_config.h"

namespace base {

namespace {
NSString* const kThreadPriorityKey = @"CrThreadPriorityKey";
}  // namespace

// application is multithreaded.  Since it's possible to enter Cocoa code
// from threads created by pthread_thread_create, Cocoa won't necessarily
// be aware that the application is multithreaded.  Spawning an NSThread is
// enough to get Cocoa to set up for multithreaded operation, so this is done
// if necessary before pthread_thread_create spawns any threads.
//
// http://developer.apple.com/documentation/Cocoa/Conceptual/Multithreading/CreatingThreads/chapter_4_section_4.html
void InitThreading() {
  static BOOL multithreaded = [NSThread isMultiThreaded];
  if (!multithreaded) {

    [NSThread detachNewThreadSelector:@selector(class)
                             toTarget:[NSObject class]
                           withObject:nil];
    multithreaded = YES;

    DCHECK([NSThread isMultiThreaded]);
  }
}

void PlatformThread::SetName(const std::string& name) {
  ThreadIdNameManager::GetInstance()->SetName(name);


  const int kMaxNameLength = 63;
  std::string shortened_name = name.substr(0, kMaxNameLength);


  pthread_setname_np(shortened_name.c_str());
}

namespace {

// glitch-resistant audio.
void SetPriorityRealtimeAudio() {






  mach_port_t mach_thread_id =
      pthread_mach_thread_np(PlatformThread::CurrentHandle().platform_handle());

  thread_extended_policy_data_t policy;
  policy.timeshare = 0;  // Set to 1 for a non-fixed thread.
  kern_return_t result =
      thread_policy_set(mach_thread_id,
                        THREAD_EXTENDED_POLICY,
                        reinterpret_cast<thread_policy_t>(&policy),
                        THREAD_EXTENDED_POLICY_COUNT);
  if (result != KERN_SUCCESS) {
    MACH_DVLOG(1, result) << "thread_policy_set";
    return;
  }

  thread_precedence_policy_data_t precedence;
  precedence.importance = 63;
  result = thread_policy_set(mach_thread_id,
                             THREAD_PRECEDENCE_POLICY,
                             reinterpret_cast<thread_policy_t>(&precedence),
                             THREAD_PRECEDENCE_POLICY_COUNT);
  if (result != KERN_SUCCESS) {
    MACH_DVLOG(1, result) << "thread_policy_set";
    return;
  }






  const double kGuaranteedAudioDutyCycle = 0.75;
  const double kMaxAudioDutyCycle = 0.85;



  const double kTimeQuantum = 2.9;

  const double kAudioTimeNeeded = kGuaranteedAudioDutyCycle * kTimeQuantum;

  const double kMaxTimeAllowed = kMaxAudioDutyCycle * kTimeQuantum;


  mach_timebase_info_data_t tb_info;
  mach_timebase_info(&tb_info);
  double ms_to_abs_time =
      (static_cast<double>(tb_info.denom) / tb_info.numer) * 1000000;

  thread_time_constraint_policy_data_t time_constraints;
  time_constraints.period = kTimeQuantum * ms_to_abs_time;
  time_constraints.computation = kAudioTimeNeeded * ms_to_abs_time;
  time_constraints.constraint = kMaxTimeAllowed * ms_to_abs_time;
  time_constraints.preemptible = 0;

  result =
      thread_policy_set(mach_thread_id,
                        THREAD_TIME_CONSTRAINT_POLICY,
                        reinterpret_cast<thread_policy_t>(&time_constraints),
                        THREAD_TIME_CONSTRAINT_POLICY_COUNT);
  MACH_DVLOG_IF(1, result != KERN_SUCCESS, result) << "thread_policy_set";

  return;
}

}  // anonymous namespace

bool PlatformThread::CanIncreaseThreadPriority(ThreadPriority priority) {
  return true;
}

void PlatformThread::SetCurrentThreadPriorityImpl(ThreadPriority priority) {


  DCHECK(![[NSThread currentThread] isMainThread]);

  switch (priority) {
    case ThreadPriority::BACKGROUND:
      [[NSThread currentThread] setThreadPriority:0];
      break;
    case ThreadPriority::NORMAL:
    case ThreadPriority::DISPLAY:
      [[NSThread currentThread] setThreadPriority:0.5];
      break;
    case ThreadPriority::REALTIME_AUDIO:
      SetPriorityRealtimeAudio();
      DCHECK_EQ([[NSThread currentThread] threadPriority], 1.0);
      break;
  }

  [[NSThread currentThread] threadDictionary][kThreadPriorityKey] =
      @(static_cast<int>(priority));
}

ThreadPriority PlatformThread::GetCurrentThreadPriority() {
  NSNumber* priority = base::mac::ObjCCast<NSNumber>(
      [[NSThread currentThread] threadDictionary][kThreadPriorityKey]);

  if (!priority)
    return ThreadPriority::NORMAL;

  ThreadPriority thread_priority =
      static_cast<ThreadPriority>(priority.intValue);
  switch (thread_priority) {
    case ThreadPriority::BACKGROUND:
    case ThreadPriority::NORMAL:
    case ThreadPriority::DISPLAY:
    case ThreadPriority::REALTIME_AUDIO:
      return thread_priority;
    default:
      NOTREACHED() << "Unknown priority.";
      return ThreadPriority::NORMAL;
  }
}

size_t GetDefaultThreadStackSize(const pthread_attr_t& attributes) {
#if defined(OS_IOS)
  return 0;
#else

















  size_t default_stack_size = 0;
  struct rlimit stack_rlimit;
  if (pthread_attr_getstacksize(&attributes, &default_stack_size) == 0 &&
      getrlimit(RLIMIT_STACK, &stack_rlimit) == 0 &&
      stack_rlimit.rlim_cur != RLIM_INFINITY) {
    default_stack_size =
        std::max(std::max(default_stack_size,
                          static_cast<size_t>(PTHREAD_STACK_MIN)),
                 static_cast<size_t>(stack_rlimit.rlim_cur));
  }
  return default_stack_size;
#endif
}

void TerminateOnThread() {
}

}  // namespace base
