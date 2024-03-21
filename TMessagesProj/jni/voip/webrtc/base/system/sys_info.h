// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_SYSTEM_SYS_INFO_H_
#define BASE_SYSTEM_SYS_INFO_H_

#include <stddef.h>
#include <stdint.h>

#include <map>
#include <string>

#include "base/base_export.h"
#include "base/callback_forward.h"
#include "base/files/file_path.h"
#include "base/gtest_prod_util.h"
#include "base/time/time.h"
#include "build/build_config.h"

namespace base {

namespace debug {
FORWARD_DECLARE_TEST(SystemMetricsTest, ParseMeminfo);
}

struct SystemMemoryInfoKB;

class BASE_EXPORT SysInfo {
 public:

  static int NumberOfProcessors();

  static int64_t AmountOfPhysicalMemory();





  static int64_t AmountOfAvailablePhysicalMemory();



  static int64_t AmountOfVirtualMemory();

  static int AmountOfPhysicalMemoryMB() {
    return static_cast<int>(AmountOfPhysicalMemory() / 1024 / 1024);
  }


  static int AmountOfVirtualMemoryMB() {
    return static_cast<int>(AmountOfVirtualMemory() / 1024 / 1024);
  }


  static int64_t AmountOfFreeDiskSpace(const FilePath& path);


  static int64_t AmountOfTotalDiskSpace(const FilePath& path);

  static TimeDelta Uptime();





  static std::string HardwareModelName();

  struct HardwareInfo {
    std::string manufacturer;
    std::string model;




    std::string serial_number;

    bool operator==(const HardwareInfo& rhs) const {
      return manufacturer == rhs.manufacturer && model == rhs.model &&
             serial_number == rhs.serial_number;
    }
  };




  static void GetHardwareInfo(base::OnceCallback<void(HardwareInfo)> callback);

  static std::string OperatingSystemName();

  static std::string OperatingSystemVersion();






  static void OperatingSystemVersionNumbers(int32_t* major_version,
                                            int32_t* minor_version,
                                            int32_t* bugfix_version);




  static std::string OperatingSystemArchitecture();




  static std::string CPUModelName();


  static size_t VMAllocationGranularity();

#if defined(OS_CHROMEOS)

  static bool GetLsbReleaseValue(const std::string& key, std::string* value);














  static std::string GetLsbReleaseBoard();


  static Time GetLsbReleaseTime();

  static bool IsRunningOnChromeOS();

  static void SetChromeOSVersionInfoForTest(const std::string& lsb_release,
                                            const Time& lsb_release_time);

  static std::string KernelVersion();
#endif  // defined(OS_CHROMEOS)

#if defined(OS_ANDROID)

  static std::string GetAndroidBuildCodename();

  static std::string GetAndroidBuildID();

  static std::string GetAndroidHardwareEGL();

  static int DalvikHeapSizeMB();
  static int DalvikHeapGrowthLimitMB();
#endif  // defined(OS_ANDROID)

#if defined(OS_IOS)





  static std::string GetIOSBuildNumber();
#endif  // defined(OS_IOS)










  static bool IsLowEndDevice();

 private:
  FRIEND_TEST_ALL_PREFIXES(SysInfoTest, AmountOfAvailablePhysicalMemory);
  FRIEND_TEST_ALL_PREFIXES(debug::SystemMetricsTest, ParseMeminfo);

  static int64_t AmountOfPhysicalMemoryImpl();
  static int64_t AmountOfAvailablePhysicalMemoryImpl();
  static bool IsLowEndDeviceImpl();
  static HardwareInfo GetHardwareInfoSync();

#if defined(OS_LINUX) || defined(OS_ANDROID) || defined(OS_AIX)
  static int64_t AmountOfAvailablePhysicalMemory(
      const SystemMemoryInfoKB& meminfo);
#endif
};

}  // namespace base

#endif  // BASE_SYSTEM_SYS_INFO_H_
