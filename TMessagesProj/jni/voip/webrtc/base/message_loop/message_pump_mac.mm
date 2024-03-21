// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "base/message_loop/message_pump_mac.h"

#import <Foundation/Foundation.h>

#include <limits>
#include <memory>

#include "base/auto_reset.h"
#include "base/feature_list.h"
#include "base/logging.h"
#include "base/mac/call_with_eh_frame.h"
#include "base/mac/scoped_cftyperef.h"
#include "base/message_loop/timer_slack.h"
#include "base/run_loop.h"
#include "base/stl_util.h"
#include "base/time/time.h"
#include "build/build_config.h"

#if !defined(OS_IOS)
#import <AppKit/AppKit.h>
#endif  // !defined(OS_IOS)

namespace base {

const CFStringRef kMessageLoopExclusiveRunLoopMode =
    CFSTR("kMessageLoopExclusiveRunLoopMode");

namespace {

enum { kCommonModeMask = 0x1, kAllModesMask = 0xf };

// Currently just common and exclusive modes. Ideally, messages would be pumped
// in all modes, but that interacts badly with app modal dialogs (e.g. NSAlert).
enum { kNSApplicationModalSafeModeMask = 0x3 };

void NoOp(void* info) {
}

constexpr CFTimeInterval kCFTimeIntervalMax =
    std::numeric_limits<CFTimeInterval>::max();

#if !defined(OS_IOS)
// Set to true if MessagePumpMac::Create() is called before NSApp is
// initialized.  Only accessed from the main thread.
bool g_not_using_cr_app = false;

MessagePumpNSApplication* g_app_pump;

Feature kMessagePumpTimerInvalidation{"MessagePumpMacTimerInvalidation",
                                      FEATURE_ENABLED_BY_DEFAULT};

typedef struct __CFRuntimeBase {
  uintptr_t _cfisa;
  uint8_t _cfinfo[4];
  uint32_t _rc;
} CFRuntimeBase;

#if defined(__BIG_ENDIAN__)
#define __CF_BIG_ENDIAN__ 1
#define __CF_LITTLE_ENDIAN__ 0
#endif

#if defined(__LITTLE_ENDIAN__)
#define __CF_LITTLE_ENDIAN__ 1
#define __CF_BIG_ENDIAN__ 0
#endif

#define CF_INFO_BITS (!!(__CF_BIG_ENDIAN__)*3)

#define __CFBitfieldMask(N1, N2) \
  ((((UInt32)~0UL) << (31UL - (N1) + (N2))) >> (31UL - N1))
#define __CFBitfieldSetValue(V, N1, N2, X)   \
  ((V) = ((V) & ~__CFBitfieldMask(N1, N2)) | \
         (((X) << (N2)) & __CFBitfieldMask(N1, N2)))

// significantly reduce power use (see the explanation in
// RunDelayedWorkTimer()), however there is no public API for doing so.
// CFRuntime.h states that CFRuntimeBase can change from release to release
// and should not be accessed directly. The last known change of this struct
// occurred in 2008 in CF-476 / 10.5; unfortunately the source for 10.11 and
// 10.12 is not available for inspection at this time.
// CanInvalidateCFRunLoopTimers() will at least prevent us from invalidating
// timers if this function starts flipping the wrong bit on a future OS release.
void __ChromeCFRunLoopTimerSetValid(CFRunLoopTimerRef timer, bool valid) {
  __CFBitfieldSetValue(((CFRuntimeBase*)timer)->_cfinfo[CF_INFO_BITS], 3, 3,
                       valid);
}
#endif  // !defined(OS_IOS)

}  // namespace

// Avoids dirtying up the ScopedNSAutoreleasePool interface for the rare
// case where an autorelease pool needs to be passed in.
class MessagePumpScopedAutoreleasePool {
 public:
  explicit MessagePumpScopedAutoreleasePool(MessagePumpCFRunLoopBase* pump) :
      pool_(pump->CreateAutoreleasePool()) {
  }
   ~MessagePumpScopedAutoreleasePool() {
    [pool_ drain];
  }

 private:
  NSAutoreleasePool* pool_;
  DISALLOW_COPY_AND_ASSIGN(MessagePumpScopedAutoreleasePool);
};

class MessagePumpCFRunLoopBase::ScopedModeEnabler {
 public:
  ScopedModeEnabler(MessagePumpCFRunLoopBase* owner, int mode_index)
      : owner_(owner), mode_index_(mode_index) {
    CFRunLoopRef loop = owner_->run_loop_;
    CFRunLoopAddTimer(loop, owner_->delayed_work_timer_, mode());
    CFRunLoopAddSource(loop, owner_->work_source_, mode());
    CFRunLoopAddSource(loop, owner_->idle_work_source_, mode());
    CFRunLoopAddSource(loop, owner_->nesting_deferred_work_source_, mode());
    CFRunLoopAddObserver(loop, owner_->pre_wait_observer_, mode());
    CFRunLoopAddObserver(loop, owner_->pre_source_observer_, mode());
    CFRunLoopAddObserver(loop, owner_->enter_exit_observer_, mode());
  }

  ~ScopedModeEnabler() {
    CFRunLoopRef loop = owner_->run_loop_;
    CFRunLoopRemoveObserver(loop, owner_->enter_exit_observer_, mode());
    CFRunLoopRemoveObserver(loop, owner_->pre_source_observer_, mode());
    CFRunLoopRemoveObserver(loop, owner_->pre_wait_observer_, mode());
    CFRunLoopRemoveSource(loop, owner_->nesting_deferred_work_source_, mode());
    CFRunLoopRemoveSource(loop, owner_->idle_work_source_, mode());
    CFRunLoopRemoveSource(loop, owner_->work_source_, mode());
    CFRunLoopRemoveTimer(loop, owner_->delayed_work_timer_, mode());
  }






  const CFStringRef& mode() const {
    static const CFStringRef modes[] = {


        kCFRunLoopCommonModes,

        kMessageLoopExclusiveRunLoopMode,

        CFSTR("com.apple.hitoolbox.windows.windowfadingmode"),

        CFSTR("NSUnhighlightMenuRunLoopMode"),
    };
    static_assert(base::size(modes) == kNumModes, "mode size mismatch");
    static_assert((1 << kNumModes) - 1 == kAllModesMask,
                  "kAllModesMask not large enough");

    return modes[mode_index_];
  }

 private:
  MessagePumpCFRunLoopBase* const owner_;  // Weak. Owns this.
  const int mode_index_;

  DISALLOW_COPY_AND_ASSIGN(ScopedModeEnabler);
};

void MessagePumpCFRunLoopBase::Run(Delegate* delegate) {
  AutoReset<bool> auto_reset_keep_running(&keep_running_, true);


  int last_run_nesting_level = run_nesting_level_;
  run_nesting_level_ = nesting_level_ + 1;

  Delegate* last_delegate = delegate_;
  SetDelegate(delegate);

  ScheduleWork();
  DoRun(delegate);

  SetDelegate(last_delegate);
  run_nesting_level_ = last_run_nesting_level;
}

void MessagePumpCFRunLoopBase::Quit() {
  if (DoQuit())
    OnDidQuit();
}

void MessagePumpCFRunLoopBase::OnDidQuit() {
  keep_running_ = false;
}

void MessagePumpCFRunLoopBase::ScheduleWork() {
  CFRunLoopSourceSignal(work_source_);
  CFRunLoopWakeUp(run_loop_);
}

void MessagePumpCFRunLoopBase::ScheduleDelayedWork(
    const TimeTicks& delayed_work_time) {
  ScheduleDelayedWorkImpl(delayed_work_time - TimeTicks::Now());
}

void MessagePumpCFRunLoopBase::ScheduleDelayedWorkImpl(TimeDelta delta) {








  SetDelayedWorkTimerValid(true);

  if (timer_slack_ == TIMER_SLACK_MAXIMUM) {
    CFRunLoopTimerSetTolerance(delayed_work_timer_, delta.InSecondsF() * 0.5);
  } else {
    CFRunLoopTimerSetTolerance(delayed_work_timer_, 0);
  }
  CFRunLoopTimerSetNextFireDate(
      delayed_work_timer_, CFAbsoluteTimeGetCurrent() + delta.InSecondsF());
}

void MessagePumpCFRunLoopBase::SetTimerSlack(TimerSlack timer_slack) {
  timer_slack_ = timer_slack;
}

#if defined(OS_IOS)
void MessagePumpCFRunLoopBase::Attach(Delegate* delegate) {}

void MessagePumpCFRunLoopBase::Detach() {}
#endif  // OS_IOS

MessagePumpCFRunLoopBase::MessagePumpCFRunLoopBase(int initial_mode_mask)
    : delegate_(NULL),
      timer_slack_(base::TIMER_SLACK_NONE),
      nesting_level_(0),
      run_nesting_level_(0),
      deepest_nesting_level_(0),
      keep_running_(true),
      delegateless_work_(false),
      delegateless_idle_work_(false),
      allow_timer_invalidation_(true) {
  run_loop_ = CFRunLoopGetCurrent();
  CFRetain(run_loop_);



  CFRunLoopTimerContext timer_context = CFRunLoopTimerContext();
  timer_context.info = this;
  delayed_work_timer_ = CFRunLoopTimerCreate(NULL,                // allocator
                                             kCFTimeIntervalMax,  // fire time
                                             kCFTimeIntervalMax,  // interval
                                             0,                   // flags
                                             0,                   // priority
                                             RunDelayedWorkTimer,
                                             &timer_context);

  CFRunLoopSourceContext source_context = CFRunLoopSourceContext();
  source_context.info = this;
  source_context.perform = RunWorkSource;
  work_source_ = CFRunLoopSourceCreate(NULL,  // allocator
                                       1,     // priority
                                       &source_context);
  source_context.perform = RunIdleWorkSource;
  idle_work_source_ = CFRunLoopSourceCreate(NULL,  // allocator
                                            2,     // priority
                                            &source_context);
  source_context.perform = RunNestingDeferredWorkSource;
  nesting_deferred_work_source_ = CFRunLoopSourceCreate(NULL,  // allocator
                                                        0,     // priority
                                                        &source_context);

  CFRunLoopObserverContext observer_context = CFRunLoopObserverContext();
  observer_context.info = this;
  pre_wait_observer_ = CFRunLoopObserverCreate(NULL,  // allocator
                                               kCFRunLoopBeforeWaiting,
                                               true,  // repeat
                                               0,     // priority
                                               PreWaitObserver,
                                               &observer_context);
  pre_source_observer_ = CFRunLoopObserverCreate(NULL,  // allocator
                                                 kCFRunLoopBeforeSources,
                                                 true,  // repeat
                                                 0,     // priority
                                                 PreSourceObserver,
                                                 &observer_context);
  enter_exit_observer_ = CFRunLoopObserverCreate(NULL,  // allocator
                                                 kCFRunLoopEntry |
                                                     kCFRunLoopExit,
                                                 true,  // repeat
                                                 0,     // priority
                                                 EnterExitObserver,
                                                 &observer_context);
  SetModeMask(initial_mode_mask);
}

// lower on the run loop thread's stack when this object was created, the
// same number of run loops must be running when this object is destroyed.
MessagePumpCFRunLoopBase::~MessagePumpCFRunLoopBase() {
  SetModeMask(0);
  CFRelease(enter_exit_observer_);
  CFRelease(pre_source_observer_);
  CFRelease(pre_wait_observer_);
  CFRelease(nesting_deferred_work_source_);
  CFRelease(idle_work_source_);
  CFRelease(work_source_);
  CFRelease(delayed_work_timer_);
  CFRelease(run_loop_);
}

void MessagePumpCFRunLoopBase::SetDelegate(Delegate* delegate) {
  delegate_ = delegate;

  if (delegate) {



    if (delegateless_work_) {
      CFRunLoopSourceSignal(work_source_);
      delegateless_work_ = false;
    }
    if (delegateless_idle_work_) {
      CFRunLoopSourceSignal(idle_work_source_);
      delegateless_idle_work_ = false;
    }
  }
}

AutoreleasePoolType* MessagePumpCFRunLoopBase::CreateAutoreleasePool() {
  return [[NSAutoreleasePool alloc] init];
}

void MessagePumpCFRunLoopBase::SetModeMask(int mode_mask) {
  for (size_t i = 0; i < kNumModes; ++i) {
    bool enable = mode_mask & (0x1 << i);
    if (enable == !enabled_modes_[i]) {
      enabled_modes_[i] =
          enable ? std::make_unique<ScopedModeEnabler>(this, i) : nullptr;
    }
  }
}

int MessagePumpCFRunLoopBase::GetModeMask() const {
  int mask = 0;
  for (size_t i = 0; i < kNumModes; ++i)
    mask |= enabled_modes_[i] ? (0x1 << i) : 0;
  return mask;
}

#if !defined(OS_IOS)
// This function uses private API to modify a test timer's valid state and
// uses public API to confirm that the private API changed the correct bit.
// static
bool MessagePumpCFRunLoopBase::CanInvalidateCFRunLoopTimers() {
  if (!FeatureList::IsEnabled(kMessagePumpTimerInvalidation)) {
    return false;
  }

  CFRunLoopTimerContext timer_context = CFRunLoopTimerContext();
  timer_context.info = nullptr;
  ScopedCFTypeRef<CFRunLoopTimerRef> test_timer(
      CFRunLoopTimerCreate(NULL,                // allocator
                           kCFTimeIntervalMax,  // fire time
                           kCFTimeIntervalMax,  // interval
                           0,                   // flags
                           0,                   // priority
                           nullptr, &timer_context));

  if (!CFRunLoopTimerIsValid(test_timer)) {
    return false;
  }

  __ChromeCFRunLoopTimerSetValid(test_timer, false);
  if (CFRunLoopTimerIsValid(test_timer)) {
    return false;
  }

  __ChromeCFRunLoopTimerSetValid(test_timer, true);
  return CFRunLoopTimerIsValid(test_timer);
}
#endif  // !defined(OS_IOS)

void MessagePumpCFRunLoopBase::ChromeCFRunLoopTimerSetValid(
    CFRunLoopTimerRef timer,
    bool valid) {
#if !defined(OS_IOS)
  static bool can_invalidate_timers = CanInvalidateCFRunLoopTimers();
  if (can_invalidate_timers) {
    __ChromeCFRunLoopTimerSetValid(timer, valid);
  }
#endif  // !defined(OS_IOS)
}

void MessagePumpCFRunLoopBase::SetDelayedWorkTimerValid(bool valid) {
  if (allow_timer_invalidation_) {
    ChromeCFRunLoopTimerSetValid(delayed_work_timer_, valid);
  } else {
    pending_timer_validity_ = valid;
  }
}

void MessagePumpCFRunLoopBase::SetTimerInvalidationAllowed(bool allowed) {
  if (!allowed)
    ChromeCFRunLoopTimerSetValid(delayed_work_timer_, true);
  allow_timer_invalidation_ = allowed;
  if (allowed && pending_timer_validity_.has_value()) {
    SetDelayedWorkTimerValid(*pending_timer_validity_);
    pending_timer_validity_ = nullopt;
  }
}

// static
void MessagePumpCFRunLoopBase::RunDelayedWorkTimer(CFRunLoopTimerRef timer,
                                                   void* info) {
  MessagePumpCFRunLoopBase* self = static_cast<MessagePumpCFRunLoopBase*>(info);























  self->SetDelayedWorkTimerValid(false);


  base::mac::CallWithEHFrame(^{
    self->RunWork();
  });
}

// static
void MessagePumpCFRunLoopBase::RunWorkSource(void* info) {
  MessagePumpCFRunLoopBase* self = static_cast<MessagePumpCFRunLoopBase*>(info);
  base::mac::CallWithEHFrame(^{
    self->RunWork();
  });
}

bool MessagePumpCFRunLoopBase::RunWork() {
  if (!delegate_) {



    delegateless_work_ = true;
    return false;
  }
  if (!keep_running())
    return false;





  MessagePumpScopedAutoreleasePool autorelease_pool(this);

  Delegate::NextWorkInfo next_work_info = delegate_->DoSomeWork();

  if (next_work_info.is_immediate()) {
    CFRunLoopSourceSignal(work_source_);
    return true;
  }

  if (!next_work_info.delayed_run_time.is_max())
    ScheduleDelayedWorkImpl(next_work_info.remaining_delay());
  return false;
}

// static
void MessagePumpCFRunLoopBase::RunIdleWorkSource(void* info) {
  MessagePumpCFRunLoopBase* self = static_cast<MessagePumpCFRunLoopBase*>(info);
  base::mac::CallWithEHFrame(^{
    self->RunIdleWork();
  });
}

void MessagePumpCFRunLoopBase::RunIdleWork() {
  if (!delegate_) {



    delegateless_idle_work_ = true;
    return;
  }
  if (!keep_running())
    return;





  MessagePumpScopedAutoreleasePool autorelease_pool(this);


  bool did_work = delegate_->DoIdleWork();
  if (did_work)
    CFRunLoopSourceSignal(idle_work_source_);
}

// static
void MessagePumpCFRunLoopBase::RunNestingDeferredWorkSource(void* info) {
  MessagePumpCFRunLoopBase* self = static_cast<MessagePumpCFRunLoopBase*>(info);
  base::mac::CallWithEHFrame(^{
    self->RunNestingDeferredWork();
  });
}

void MessagePumpCFRunLoopBase::RunNestingDeferredWork() {
  if (!delegate_) {




    return;
  }

  if (RunWork()) {


    CFRunLoopSourceSignal(idle_work_source_);
  } else {
    RunIdleWork();
  }
}

void MessagePumpCFRunLoopBase::MaybeScheduleNestingDeferredWork() {






  if (deepest_nesting_level_ > nesting_level_) {
    deepest_nesting_level_ = nesting_level_;
    CFRunLoopSourceSignal(nesting_deferred_work_source_);
  }
}

// static
void MessagePumpCFRunLoopBase::PreWaitObserver(CFRunLoopObserverRef observer,
                                               CFRunLoopActivity activity,
                                               void* info) {
  MessagePumpCFRunLoopBase* self = static_cast<MessagePumpCFRunLoopBase*>(info);
  base::mac::CallWithEHFrame(^{

    self->RunIdleWork();




    self->MaybeScheduleNestingDeferredWork();
  });
}

// static
void MessagePumpCFRunLoopBase::PreSourceObserver(CFRunLoopObserverRef observer,
                                                 CFRunLoopActivity activity,
                                                 void* info) {
  MessagePumpCFRunLoopBase* self = static_cast<MessagePumpCFRunLoopBase*>(info);





  base::mac::CallWithEHFrame(^{
    self->MaybeScheduleNestingDeferredWork();
  });
}

// static
void MessagePumpCFRunLoopBase::EnterExitObserver(CFRunLoopObserverRef observer,
                                                 CFRunLoopActivity activity,
                                                 void* info) {
  MessagePumpCFRunLoopBase* self = static_cast<MessagePumpCFRunLoopBase*>(info);

  switch (activity) {
    case kCFRunLoopEntry:
      ++self->nesting_level_;
      if (self->nesting_level_ > self->deepest_nesting_level_) {
        self->deepest_nesting_level_ = self->nesting_level_;
      }
      break;

    case kCFRunLoopExit:
















      base::mac::CallWithEHFrame(^{
        self->MaybeScheduleNestingDeferredWork();
      });
      --self->nesting_level_;
      break;

    default:
      break;
  }

  base::mac::CallWithEHFrame(^{
    self->EnterExitRunLoop(activity);
  });
}

// implementation is a no-op.
void MessagePumpCFRunLoopBase::EnterExitRunLoop(CFRunLoopActivity activity) {
}

MessagePumpCFRunLoop::MessagePumpCFRunLoop()
    : MessagePumpCFRunLoopBase(kCommonModeMask), quit_pending_(false) {}

MessagePumpCFRunLoop::~MessagePumpCFRunLoop() {}

// running lower on the run loop thread's stack when this object was created,
// the same number of CFRunLoopRun loops must be running for the outermost call
// to Run.  Run/DoRun are reentrant after that point.
void MessagePumpCFRunLoop::DoRun(Delegate* delegate) {


  int result;
  do {
    MessagePumpScopedAutoreleasePool autorelease_pool(this);
    result = CFRunLoopRunInMode(kCFRunLoopDefaultMode,
                                kCFTimeIntervalMax,
                                false);
  } while (result != kCFRunLoopRunStopped && result != kCFRunLoopRunFinished);
}

bool MessagePumpCFRunLoop::DoQuit() {

  if (nesting_level() == run_nesting_level()) {

    CFRunLoopStop(run_loop());
    return true;
  } else {





    quit_pending_ = true;
    return false;
  }
}

void MessagePumpCFRunLoop::EnterExitRunLoop(CFRunLoopActivity activity) {
  if (activity == kCFRunLoopExit &&
      nesting_level() == run_nesting_level() &&
      quit_pending_) {




    CFRunLoopStop(run_loop());
    quit_pending_ = false;
    OnDidQuit();
  }
}

MessagePumpNSRunLoop::MessagePumpNSRunLoop()
    : MessagePumpCFRunLoopBase(kCommonModeMask) {
  CFRunLoopSourceContext source_context = CFRunLoopSourceContext();
  source_context.perform = NoOp;
  quit_source_ = CFRunLoopSourceCreate(NULL,  // allocator
                                       0,     // priority
                                       &source_context);
  CFRunLoopAddSource(run_loop(), quit_source_, kCFRunLoopCommonModes);
}

MessagePumpNSRunLoop::~MessagePumpNSRunLoop() {
  CFRunLoopRemoveSource(run_loop(), quit_source_, kCFRunLoopCommonModes);
  CFRelease(quit_source_);
}

void MessagePumpNSRunLoop::DoRun(Delegate* delegate) {
  while (keep_running()) {

    [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode
                             beforeDate:[NSDate distantFuture]];
  }
}

bool MessagePumpNSRunLoop::DoQuit() {
  CFRunLoopSourceSignal(quit_source_);
  CFRunLoopWakeUp(run_loop());
  return true;
}

#if defined(OS_IOS)
MessagePumpUIApplication::MessagePumpUIApplication()
    : MessagePumpCFRunLoopBase(kCommonModeMask), run_loop_(NULL) {}

MessagePumpUIApplication::~MessagePumpUIApplication() {}

void MessagePumpUIApplication::DoRun(Delegate* delegate) {
  NOTREACHED();
}

bool MessagePumpUIApplication::DoQuit() {
  NOTREACHED();
  return false;
}

void MessagePumpUIApplication::Attach(Delegate* delegate) {
  DCHECK(!run_loop_);
  run_loop_ = new RunLoop();
  CHECK(run_loop_->BeforeRun());
  SetDelegate(delegate);
}

void MessagePumpUIApplication::Detach() {
  DCHECK(run_loop_);
  run_loop_->AfterRun();
  SetDelegate(nullptr);
  run_loop_ = nullptr;
}

#else

ScopedPumpMessagesInPrivateModes::ScopedPumpMessagesInPrivateModes() {
  DCHECK(g_app_pump);
  DCHECK_EQ(kNSApplicationModalSafeModeMask, g_app_pump->GetModeMask());


  if ([NSApp modalWindow])
    return;
  g_app_pump->SetModeMask(kAllModesMask);

  g_app_pump->SetTimerInvalidationAllowed(false);
}

ScopedPumpMessagesInPrivateModes::~ScopedPumpMessagesInPrivateModes() {
  DCHECK(g_app_pump);
  g_app_pump->SetModeMask(kNSApplicationModalSafeModeMask);
  g_app_pump->SetTimerInvalidationAllowed(true);
}

int ScopedPumpMessagesInPrivateModes::GetModeMaskForTest() {
  return g_app_pump ? g_app_pump->GetModeMask() : -1;
}

MessagePumpNSApplication::MessagePumpNSApplication()
    : MessagePumpCFRunLoopBase(kNSApplicationModalSafeModeMask),
      running_own_loop_(false),
      quit_pending_(false) {
  DCHECK_EQ(nullptr, g_app_pump);
  g_app_pump = this;
}

MessagePumpNSApplication::~MessagePumpNSApplication() {
  DCHECK_EQ(this, g_app_pump);
  g_app_pump = nullptr;
}

void MessagePumpNSApplication::DoRun(Delegate* delegate) {
  bool last_running_own_loop_ = running_own_loop_;





  CHECK(NSApp);

  if (![NSApp isRunning]) {
    running_own_loop_ = false;

    [NSApp run];
  } else {
    running_own_loop_ = true;
    NSDate* distant_future = [NSDate distantFuture];
    while (keep_running()) {
      MessagePumpScopedAutoreleasePool autorelease_pool(this);
      NSEvent* event = [NSApp nextEventMatchingMask:NSAnyEventMask
                                          untilDate:distant_future
                                             inMode:NSDefaultRunLoopMode
                                            dequeue:YES];
      if (event) {
        [NSApp sendEvent:event];
      }
    }
  }

  running_own_loop_ = last_running_own_loop_;
}

bool MessagePumpNSApplication::DoQuit() {





  if (nesting_level() > run_nesting_level() &&
      [[NSApplication sharedApplication] modalWindow] != nil) {
    quit_pending_ = true;
    return false;
  }

  if (!running_own_loop_) {
    [[NSApplication sharedApplication] stop:nil];
  }

  [NSApp postEvent:[NSEvent otherEventWithType:NSApplicationDefined
                                      location:NSZeroPoint
                                 modifierFlags:0
                                     timestamp:0
                                  windowNumber:0
                                       context:NULL
                                       subtype:0
                                         data1:0
                                         data2:0]
           atStart:NO];
  return true;
}

void MessagePumpNSApplication::EnterExitRunLoop(CFRunLoopActivity activity) {


  if (activity == kCFRunLoopEntry && quit_pending_ &&
      nesting_level() <= run_nesting_level() &&
      [[NSApplication sharedApplication] modalWindow] == nil) {
    quit_pending_ = false;
    if (DoQuit())
      OnDidQuit();
  }
}

MessagePumpCrApplication::MessagePumpCrApplication() {
}

MessagePumpCrApplication::~MessagePumpCrApplication() {
}

// handling a UI event because various parts of AppKit depend on objects that
// are created while handling a UI event to be autoreleased in the event loop.
// An example of this is NSWindowController. When a window with a window
// controller is closed it goes through a stack like this:
// (Several stack frames elided for clarity)
//
// #0 [NSWindowController autorelease]
// #1 DoAClose
// #2 MessagePumpCFRunLoopBase::DoWork()
// #3 [NSRunLoop run]
// #4 [NSButton performClick:]
// #5 [NSWindow sendEvent:]
// #6 [NSApp sendEvent:]
// #7 [NSApp run]
//
// -performClick: spins a nested run loop. If the pool created in DoWork was a
// standard NSAutoreleasePool, it would release the objects that were
// autoreleased into it once DoWork released it. This would cause the window
// controller, which autoreleased itself in frame #0, to release itself, and
// possibly free itself. Unfortunately this window controller controls the
// window in frame #5. When the stack is unwound to frame #5, the window would
// no longer exists and crashes may occur. Apple gets around this by never
// releasing the pool it creates in frame #4, and letting frame #7 clean it up
// when it cleans up the pool that wraps frame #7. When an autorelease pool is
// released it releases all other pools that were created after it on the
// autorelease pool stack.
//
// CrApplication is responsible for setting handlingSendEvent to true just
// before it sends the event through the event handling mechanism, and
// returning it to its previous value once the event has been sent.
AutoreleasePoolType* MessagePumpCrApplication::CreateAutoreleasePool() {
  if (MessagePumpMac::IsHandlingSendEvent())
    return nil;
  return MessagePumpNSApplication::CreateAutoreleasePool();
}

bool MessagePumpMac::UsingCrApp() {
  DCHECK([NSThread isMainThread]);


  DCHECK(NSApp);

  if (g_not_using_cr_app)
    return false;

  return [NSApp conformsToProtocol:@protocol(CrAppProtocol)];
}

bool MessagePumpMac::IsHandlingSendEvent() {
  DCHECK([NSApp conformsToProtocol:@protocol(CrAppProtocol)]);
  NSObject<CrAppProtocol>* app = static_cast<NSObject<CrAppProtocol>*>(NSApp);
  return [app isHandlingSendEvent];
}
#endif  // !defined(OS_IOS)

std::unique_ptr<MessagePump> MessagePumpMac::Create() {
  if ([NSThread isMainThread]) {
#if defined(OS_IOS)
    return std::make_unique<MessagePumpUIApplication>();
#else
    if ([NSApp conformsToProtocol:@protocol(CrAppProtocol)])
      return std::make_unique<MessagePumpCrApplication>();




    [NSApplication sharedApplication];
    g_not_using_cr_app = true;
    return std::make_unique<MessagePumpNSApplication>();
#endif
  }

  return std::make_unique<MessagePumpNSRunLoop>();
}

}  // namespace base
