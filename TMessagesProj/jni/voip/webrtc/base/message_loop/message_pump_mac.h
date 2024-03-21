// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// used directly, it can be used as the driving force behind the similar
// Foundation NSRunLoop, and it can be used to implement higher-level event
// loops such as the NSApplication event loop.
//
// This file introduces a basic CFRunLoop-based implementation of the
// MessagePump interface called CFRunLoopBase.  CFRunLoopBase contains all
// of the machinery necessary to dispatch events to a delegate, but does not
// implement the specific run loop.  Concrete subclasses must provide their
// own DoRun and DoQuit implementations.
//
// A concrete subclass that just runs a CFRunLoop loop is provided in
// MessagePumpCFRunLoop.  For an NSRunLoop, the similar MessagePumpNSRunLoop
// is provided.
//
// For the application's event loop, an implementation based on AppKit's
// NSApplication event system is provided in MessagePumpNSApplication.
//
// Typically, MessagePumpNSApplication only makes sense on a Cocoa
// application's main thread.  If a CFRunLoop-based message pump is needed on
// any other thread, one of the other concrete subclasses is preferable.
// MessagePumpMac::Create is defined, which returns a new NSApplication-based
// or NSRunLoop-based MessagePump subclass depending on which thread it is
// called on.

#ifndef BASE_MESSAGE_LOOP_MESSAGE_PUMP_MAC_H_
#define BASE_MESSAGE_LOOP_MESSAGE_PUMP_MAC_H_

#include "base/message_loop/message_pump.h"


#include <CoreFoundation/CoreFoundation.h>

#include "base/macros.h"
#include "base/memory/weak_ptr.h"
#include "base/message_loop/timer_slack.h"
#include "base/optional.h"
#include "build/build_config.h"

#if defined(__OBJC__)
#if defined(OS_IOS)
#import <Foundation/Foundation.h>
#else
#import <AppKit/AppKit.h>

// MessagePumpMac.
@protocol CrAppProtocol
// Must return true if -[NSApplication sendEvent:] is currently on the stack.
// See the comment for |CreateAutoreleasePool()| in the cc file for why this is
// necessary.
- (BOOL)isHandlingSendEvent;
@end
#endif  // !defined(OS_IOS)
#endif  // defined(__OBJC__)

namespace base {

class RunLoop;
class TimeTicks;

// depends on the translation unit (TU) in which this header appears. In pure
// C++ TUs, it is defined as a forward C++ class declaration (that is never
// defined), because autorelease pools are an Objective-C concept. In Automatic
// Reference Counting (ARC) Objective-C TUs, it is similarly defined as a
// forward C++ class declaration, because clang will not allow the type
// "NSAutoreleasePool" in such TUs. Finally, in Manual Retain Release (MRR)
// Objective-C TUs, it is a type alias for NSAutoreleasePool. In all cases, a
// method that takes or returns an NSAutoreleasePool* can use
// AutoreleasePoolType* instead.
#if !defined(__OBJC__) || __has_feature(objc_arc)
class AutoreleasePoolType;
#else   // !defined(__OBJC__) || __has_feature(objc_arc)
typedef NSAutoreleasePool AutoreleasePoolType;
#endif  // !defined(__OBJC__) || __has_feature(objc_arc)

class BASE_EXPORT MessagePumpCFRunLoopBase : public MessagePump {
 public:

  void Run(Delegate* delegate) override;
  void Quit() override;
  void ScheduleWork() override;
  void ScheduleDelayedWork(const TimeTicks& delayed_work_time) override;
  void SetTimerSlack(TimerSlack timer_slack) override;

#if defined(OS_IOS)




  virtual void Attach(Delegate* delegate);
  virtual void Detach();
#endif  // OS_IOS

 protected:

  friend class MessagePumpScopedAutoreleasePool;
  friend class TestMessagePumpCFRunLoopBase;



  explicit MessagePumpCFRunLoopBase(int initial_mode_mask);
  ~MessagePumpCFRunLoopBase() override;




  virtual void DoRun(Delegate* delegate) = 0;




  virtual bool DoQuit() = 0;

  void OnDidQuit();

  CFRunLoopRef run_loop() const { return run_loop_; }
  int nesting_level() const { return nesting_level_; }
  int run_nesting_level() const { return run_nesting_level_; }
  bool keep_running() const { return keep_running_; }


  void SetDelegate(Delegate* delegate);




  virtual AutoreleasePoolType* CreateAutoreleasePool();

  void SetModeMask(int mode_mask);

  int GetModeMask() const;


  void SetTimerInvalidationAllowed(bool allowed);

 private:
  class ScopedModeEnabler;

  static constexpr int kNumModes = 4;


  void ScheduleDelayedWorkImpl(TimeDelta delta);









  static bool CanInvalidateCFRunLoopTimers();


  static void ChromeCFRunLoopTimerSetValid(CFRunLoopTimerRef timer, bool valid);


  void SetDelayedWorkTimerValid(bool valid);



  static void RunDelayedWorkTimer(CFRunLoopTimerRef timer, void* info);




  static void RunWorkSource(void* info);
  bool RunWork();




  static void RunIdleWorkSource(void* info);
  void RunIdleWork();






  static void RunNestingDeferredWorkSource(void* info);
  void RunNestingDeferredWork();





  void MaybeScheduleNestingDeferredWork();


  static void PreWaitObserver(CFRunLoopObserverRef observer,
                              CFRunLoopActivity activity, void* info);


  static void PreSourceObserver(CFRunLoopObserverRef observer,
                                CFRunLoopActivity activity, void* info);



  static void EnterExitObserver(CFRunLoopObserverRef observer,
                                CFRunLoopActivity activity, void* info);



  virtual void EnterExitRunLoop(CFRunLoopActivity activity);

  CFRunLoopRef run_loop_;

  std::unique_ptr<ScopedModeEnabler> enabled_modes_[kNumModes];


  CFRunLoopTimerRef delayed_work_timer_;
  CFRunLoopSourceRef work_source_;
  CFRunLoopSourceRef idle_work_source_;
  CFRunLoopSourceRef nesting_deferred_work_source_;
  CFRunLoopObserverRef pre_wait_observer_;
  CFRunLoopObserverRef pre_source_observer_;
  CFRunLoopObserverRef enter_exit_observer_;

  Delegate* delegate_;

  base::TimerSlack timer_slack_;



  int nesting_level_;


  int run_nesting_level_;


  int deepest_nesting_level_;


  bool keep_running_;





  bool delegateless_work_;
  bool delegateless_idle_work_;


  bool allow_timer_invalidation_;


  Optional<bool> pending_timer_validity_;

  DISALLOW_COPY_AND_ASSIGN(MessagePumpCFRunLoopBase);
};

class BASE_EXPORT MessagePumpCFRunLoop : public MessagePumpCFRunLoopBase {
 public:
  MessagePumpCFRunLoop();
  ~MessagePumpCFRunLoop() override;

  void DoRun(Delegate* delegate) override;
  bool DoQuit() override;

 private:
  void EnterExitRunLoop(CFRunLoopActivity activity) override;



  bool quit_pending_;

  DISALLOW_COPY_AND_ASSIGN(MessagePumpCFRunLoop);
};

class BASE_EXPORT MessagePumpNSRunLoop : public MessagePumpCFRunLoopBase {
 public:
  MessagePumpNSRunLoop();
  ~MessagePumpNSRunLoop() override;

  void DoRun(Delegate* delegate) override;
  bool DoQuit() override;

 private:



  CFRunLoopSourceRef quit_source_;

  DISALLOW_COPY_AND_ASSIGN(MessagePumpNSRunLoop);
};

#if defined(OS_IOS)
// This is a fake message pump.  It attaches sources to the main thread's
// CFRunLoop, so PostTask() will work, but it is unable to drive the loop
// directly, so calling Run() or Quit() are errors.
class MessagePumpUIApplication : public MessagePumpCFRunLoopBase {
 public:
  MessagePumpUIApplication();
  ~MessagePumpUIApplication() override;
  void DoRun(Delegate* delegate) override;
  bool DoQuit() override;




  void Attach(Delegate* delegate) override;
  void Detach() override;

 private:
  RunLoop* run_loop_;

  DISALLOW_COPY_AND_ASSIGN(MessagePumpUIApplication);
};

#else

// modes that would otherwise make the UI unresponsive. E.g., menu fade out.
class BASE_EXPORT ScopedPumpMessagesInPrivateModes {
 public:
  ScopedPumpMessagesInPrivateModes();
  ~ScopedPumpMessagesInPrivateModes();

  int GetModeMaskForTest();

 private:
  DISALLOW_COPY_AND_ASSIGN(ScopedPumpMessagesInPrivateModes);
};

class MessagePumpNSApplication : public MessagePumpCFRunLoopBase {
 public:
  MessagePumpNSApplication();
  ~MessagePumpNSApplication() override;

  void DoRun(Delegate* delegate) override;
  bool DoQuit() override;

 private:
  friend class ScopedPumpMessagesInPrivateModes;

  void EnterExitRunLoop(CFRunLoopActivity activity) override;




  bool running_own_loop_;


  bool quit_pending_;

  DISALLOW_COPY_AND_ASSIGN(MessagePumpNSApplication);
};

class MessagePumpCrApplication : public MessagePumpNSApplication {
 public:
  MessagePumpCrApplication();
  ~MessagePumpCrApplication() override;

 protected:


  AutoreleasePoolType* CreateAutoreleasePool() override;

 private:
  DISALLOW_COPY_AND_ASSIGN(MessagePumpCrApplication);
};
#endif  // !defined(OS_IOS)

class BASE_EXPORT MessagePumpMac {
 public:








  static std::unique_ptr<MessagePump> Create();

#if !defined(OS_IOS)





  static bool UsingCrApp();


  static bool IsHandlingSendEvent();
#endif  // !defined(OS_IOS)

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(MessagePumpMac);
};

// as kCFRunLoopCommonModes.
extern const CFStringRef BASE_EXPORT kMessageLoopExclusiveRunLoopMode;

}  // namespace base

#endif  // BASE_MESSAGE_LOOP_MESSAGE_PUMP_MAC_H_
