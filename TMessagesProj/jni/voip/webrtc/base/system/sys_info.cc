// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/system/sys_info.h"

#include <algorithm>

#include "base/base_switches.h"
#include "base/bind.h"
#include "base/callback.h"
#include "base/command_line.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/no_destructor.h"
#include "base/system/sys_info_internal.h"
#include "base/task/post_task.h"
#include "base/task/task_traits.h"
#include "base/task/thread_pool.h"
#include "base/task_runner_util.h"
#include "base/time/time.h"
#include "build/build_config.h"

namespace base {
namespace {
static const int kLowMemoryDeviceThresholdMB = 512;
}  // namespace

int64_t SysInfo::AmountOfPhysicalMemory() {
  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kEnableLowEndDeviceMode)) {
    return kLowMemoryDeviceThresholdMB * 1024 * 1024;
  }

  return AmountOfPhysicalMemoryImpl();
}

int64_t SysInfo::AmountOfAvailablePhysicalMemory() {
  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kEnableLowEndDeviceMode)) {


    size_t memory_used =
        AmountOfPhysicalMemoryImpl() - AmountOfAvailablePhysicalMemoryImpl();
    size_t memory_limit = kLowMemoryDeviceThresholdMB * 1024 * 1024;

    return memory_limit - std::min(memory_used, memory_limit);
  }

  return AmountOfAvailablePhysicalMemoryImpl();
}

bool SysInfo::IsLowEndDevice() {
  if (base::CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kEnableLowEndDeviceMode)) {
    return true;
  }

  return IsLowEndDeviceImpl();
}

#if !defined(OS_ANDROID)

bool DetectLowEndDevice() {
  CommandLine* command_line = CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(switches::kEnableLowEndDeviceMode))
    return true;
  if (command_line->HasSwitch(switches::kDisableLowEndDeviceMode))
    return false;

  int ram_size_mb = SysInfo::AmountOfPhysicalMemoryMB();
  return (ram_size_mb > 0 && ram_size_mb <= kLowMemoryDeviceThresholdMB);
}

bool SysInfo::IsLowEndDeviceImpl() {
  static base::NoDestructor<
      internal::LazySysInfoValue<bool, DetectLowEndDevice>>
      instance;
  return instance->value();
}
#endif

#if !defined(OS_MACOSX) && !defined(OS_ANDROID)
std::string SysInfo::HardwareModelName() {
  return std::string();
}
#endif

void SysInfo::GetHardwareInfo(base::OnceCallback<void(HardwareInfo)> callback) {
#if defined(OS_WIN)



  base::PostTaskAndReplyWithResult(
      base::ThreadPool::CreateCOMSTATaskRunner(
          {TaskShutdownBehavior::CONTINUE_ON_SHUTDOWN})
          .get(),
      FROM_HERE, base::BindOnce(&GetHardwareInfoSync), std::move(callback));
#elif defined(OS_ANDROID) || defined(OS_MACOSX)
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {}, base::BindOnce(&GetHardwareInfoSync), std::move(callback));
#elif defined(OS_LINUX)
  base::ThreadPool::PostTaskAndReplyWithResult(
      FROM_HERE, {base::MayBlock()}, base::BindOnce(&GetHardwareInfoSync),
      std::move(callback));
#else
  NOTIMPLEMENTED();
  base::ThreadPool::PostTask(
      FROM_HERE, {}, base::BindOnce(std::move(callback), HardwareInfo()));
#endif
}

base::TimeDelta SysInfo::Uptime() {



  int64_t uptime_in_microseconds = TimeTicks::Now().ToInternalValue();
  return base::TimeDelta::FromMicroseconds(uptime_in_microseconds);
}

}  // namespace base
