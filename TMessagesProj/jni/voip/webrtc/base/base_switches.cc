// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/base_switches.h"
#include "build/build_config.h"

namespace switches {

const char kDisableBestEffortTasks[] = "disable-best-effort-tasks";

const char kDisableBreakpad[]               = "disable-breakpad";

const char kDisableFeatures[] = "disable-features";

const char kDisableLowEndDeviceMode[] = "disable-low-end-device-mode";

// processes cannot access to files needed to make this decision, this flag is
// generated internally.
const char kEnableCrashReporter[]           = "enable-crash-reporter";

const char kEnableFeatures[] = "enable-features";

const char kEnableLowEndDeviceMode[]        = "enable-low-end-device-mode";

// The argument is a list of name and value pairs, separated by slashes. If a
// trial name is prefixed with an asterisk, that trial will start activated.
// For example, the following argument defines two trials, with the second one
// activated: "GoogleNow/Enable/*MaterialDesignNTP/Default/" This option can
// also be used by the browser process to send the list of trials to a
// non-browser process, using the same format. See
// FieldTrialList::CreateTrialsFromString() in field_trial.h for details.
const char kForceFieldTrials[]              = "force-fieldtrials";

const char kFullMemoryCrashReport[] = "full-memory-crash-report";

// this to diagnose issues that are thought to be caused by
// TaskPriority::BEST_EFFORT execution fences. Note: Tasks posted to a
// non-BEST_EFFORT UpdateableSequencedTaskRunner whose priority is later lowered
// to BEST_EFFORT are not logged.
const char kLogBestEffortTasks[] = "log-best-effort-tasks";

const char kNoErrorDialogs[]                = "noerrdialogs";

// will only work if chrome has been built with the gn arg enable_profiling =
// true. The output will go to the value of kProfilingFile.
const char kProfilingAtStart[] = "profiling-at-start";

// been built with the gyp variable profiling=1 or gn arg enable_profiling=true.
//
//   {pid} if present will be replaced by the pid of the process.
//   {count} if present will be incremented each time a profile is generated
//           for this process.
// The default is chrome-profile-{pid} for the browser and test-profile-{pid}
// for tests.
const char kProfilingFile[] = "profiling-file";

// the data gets written on exit but cases exist where chromium doesn't exit
// cleanly (especially when using single-process). A time in seconds can be
// specified.
const char kProfilingFlush[] = "profiling-flush";

// to the test framework that the current process is a child process.
const char kTestChildProcess[] = "test-child-process";

// to the test framework that the current process should not initialize ICU to
// avoid creating any scoped handles too early in startup.
const char kTestDoNotInitializeIcu[] = "test-do-not-initialize-icu";

// --trace-to-file on its own sends to default categories.
const char kTraceToFile[] = "trace-to-file";

// go to a default file name.
const char kTraceToFileName[] = "trace-to-file-name";

// Normally positive values are used for V-logging levels.
const char kV[] = "v";

// given by --v.  E.g. "my_module=2,foo*=3" would change the logging
// level for all code in source files "my_module.*" and "foo*.*"
// ("-inl" suffixes are also disregarded for this matching).
//
// Any pattern containing a forward or backward slash will be tested
// against the whole pathname and not just the module.  E.g.,
// "*/foo/bar/*=2" would change the logging level for all code in
// source files under a "foo/bar" directory.
const char kVModule[] = "vmodule";

const char kWaitForDebugger[] = "wait-for-debugger";

#if defined(OS_WIN)
// Disable high-resolution timer on Windows.
const char kDisableHighResTimer[] = "disable-highres-timer";

const char kDisableUsbKeyboardDetect[]      = "disable-usb-keyboard-detect";
#endif

#if defined(OS_LINUX) && !defined(OS_CHROMEOS)
// The /dev/shm partition is too small in certain VM environments, causing
// Chrome to fail or crash (see http://crbug.com/715363). Use this flag to
// work-around this issue (a temporary directory will always be used to create
// anonymous shared memory files).
const char kDisableDevShmUsage[] = "disable-dev-shm-usage";
#endif

#if defined(OS_POSIX)
// Used for turning on Breakpad crash reporting in a debug environment where
// crash reporting is typically compiled but disabled.
const char kEnableCrashReporterForTesting[] =
    "enable-crash-reporter-for-testing";
#endif

#if defined(OS_ANDROID)
// Enables the reached code profiler that samples all threads in all processes
// to determine which functions are almost never executed.
const char kEnableReachedCodeProfiler[] = "enable-reached-code-profiler";
#endif

#if defined(OS_LINUX)
// Controls whether or not retired instruction counts are surfaced for threads
// in trace events on Linux.
//
// This flag requires the BPF sandbox to be disabled.
const char kEnableThreadInstructionCount[] = "enable-thread-instruction-count";
#endif

}  // namespace switches
