// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/system/sys_info.h"

#include <dlfcn.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/system_properties.h>

#include "base/android/jni_android.h"
#include "base/android/sys_utils.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/system/sys_info_internal.h"

#if (__ANDROID_API__ >= 21 /* 5.0 - Lollipop */)

namespace {

typedef int(SystemPropertyGetFunction)(const char*, char*);

SystemPropertyGetFunction* DynamicallyLoadRealSystemPropertyGet() {

  void* handle = dlopen("libc.so", RTLD_NOLOAD);
  if (!handle) {
    LOG(FATAL) << "Cannot dlopen libc.so: " << dlerror();
  }
  SystemPropertyGetFunction* real_system_property_get =
      reinterpret_cast<SystemPropertyGetFunction*>(
          dlsym(handle, "__system_property_get"));
  if (!real_system_property_get) {
    LOG(FATAL) << "Cannot resolve __system_property_get(): " << dlerror();
  }
  return real_system_property_get;
}

static base::LazyInstance<base::internal::LazySysInfoValue<
    SystemPropertyGetFunction*,
    DynamicallyLoadRealSystemPropertyGet>>::Leaky
    g_lazy_real_system_property_get = LAZY_INSTANCE_INITIALIZER;

}  // namespace

// a hidden symbol in libc. Until we remove all calls of __system_property_get
// from Chrome we work around this by defining a weak stub here, which uses
// dlsym to but ensures that Chrome uses the real system
// implementatation when loaded.  http://crbug.com/392191.
BASE_EXPORT int __system_property_get(const char* name, char* value) {
  return g_lazy_real_system_property_get.Get().value()(name, value);
}

#endif

namespace {

// cannot be acquired. Use the latest Android release with a higher bug fix
// version to avoid unnecessarily comparison errors with the latest release.
// This should be manually kept up to date on each Android release.
const int kDefaultAndroidMajorVersion = 10;
const int kDefaultAndroidMinorVersion = 0;
const int kDefaultAndroidBugfixVersion = 99;

// Note if parse fails, the "default" version is returned as fallback.
void GetOsVersionStringAndNumbers(std::string* version_string,
                                  int32_t* major_version,
                                  int32_t* minor_version,
                                  int32_t* bugfix_version) {

  char os_version_str[PROP_VALUE_MAX];
  __system_property_get("ro.build.version.release", os_version_str);

  if (os_version_str[0]) {

    int num_read = sscanf(os_version_str, "%d.%d.%d", major_version,
                          minor_version, bugfix_version);

    if (num_read > 0) {

      if (num_read < 2)
        *minor_version = 0;
      if (num_read < 3)
        *bugfix_version = 0;
      *version_string = std::string(os_version_str);
      return;
    }
  }

  *major_version = kDefaultAndroidMajorVersion;
  *minor_version = kDefaultAndroidMinorVersion;
  *bugfix_version = kDefaultAndroidBugfixVersion;
  *version_string = ::base::StringPrintf("%d.%d.%d", *major_version,
                                         *minor_version, *bugfix_version);
}

// Returns a value in bytes.
// Returns -1 if the string could not be parsed.
int64_t ParseSystemPropertyBytes(const base::StringPiece& str) {
  const int64_t KB = 1024;
  const int64_t MB = 1024 * KB;
  const int64_t GB = 1024 * MB;
  if (str.size() == 0u)
    return -1;
  int64_t unit_multiplier = 1;
  size_t length = str.size();
  if (str[length - 1] == 'k') {
    unit_multiplier = KB;
    length--;
  } else if (str[length - 1] == 'm') {
    unit_multiplier = MB;
    length--;
  } else if (str[length - 1] == 'g') {
    unit_multiplier = GB;
    length--;
  }
  int64_t result = 0;
  bool parsed = base::StringToInt64(str.substr(0, length), &result);
  bool negative = result <= 0;
  bool overflow =
      result >= std::numeric_limits<int64_t>::max() / unit_multiplier;
  if (!parsed || negative || overflow)
    return -1;
  return result * unit_multiplier;
}

int GetDalvikHeapSizeMB() {
  char heap_size_str[PROP_VALUE_MAX];
  __system_property_get("dalvik.vm.heapsize", heap_size_str);



  const int64_t MB = 1024 * 1024;
  int64_t result = ParseSystemPropertyBytes(heap_size_str);
  if (result == -1) {

    LOG(ERROR) << "Can't parse dalvik.vm.heapsize: " << heap_size_str;
    result = base::SysInfo::AmountOfPhysicalMemoryMB() / 3;
  }
  result =
      std::min<int64_t>(std::max<int64_t>(32 * MB, result), 1024 * MB) / MB;
  return static_cast<int>(result);
}

int GetDalvikHeapGrowthLimitMB() {
  char heap_size_str[PROP_VALUE_MAX];
  __system_property_get("dalvik.vm.heapgrowthlimit", heap_size_str);



  const int64_t MB = 1024 * 1024;
  int64_t result = ParseSystemPropertyBytes(heap_size_str);
  if (result == -1) {

    LOG(ERROR) << "Can't parse dalvik.vm.heapgrowthlimit: " << heap_size_str;
    result = base::SysInfo::AmountOfPhysicalMemoryMB() / 6;
  }
  result = std::min<int64_t>(std::max<int64_t>(16 * MB, result), 512 * MB) / MB;
  return static_cast<int>(result);
}

std::string HardwareManufacturerName() {
  char device_model_str[PROP_VALUE_MAX];
  __system_property_get("ro.product.manufacturer", device_model_str);
  return std::string(device_model_str);
}

}  // anonymous namespace

namespace base {

std::string SysInfo::HardwareModelName() {
  char device_model_str[PROP_VALUE_MAX];
  __system_property_get("ro.product.model", device_model_str);
  return std::string(device_model_str);
}

std::string SysInfo::OperatingSystemName() {
  return "Android";
}

std::string SysInfo::OperatingSystemVersion() {
  std::string version_string;
  int32_t major, minor, bugfix;
  GetOsVersionStringAndNumbers(&version_string, &major, &minor, &bugfix);
  return version_string;
}

void SysInfo::OperatingSystemVersionNumbers(int32_t* major_version,
                                            int32_t* minor_version,
                                            int32_t* bugfix_version) {
  std::string version_string;
  GetOsVersionStringAndNumbers(&version_string, major_version, minor_version,
                               bugfix_version);
}

std::string SysInfo::GetAndroidBuildCodename() {
  char os_version_codename_str[PROP_VALUE_MAX];
  __system_property_get("ro.build.version.codename", os_version_codename_str);
  return std::string(os_version_codename_str);
}

std::string SysInfo::GetAndroidBuildID() {
  char os_build_id_str[PROP_VALUE_MAX];
  __system_property_get("ro.build.id", os_build_id_str);
  return std::string(os_build_id_str);
}

std::string SysInfo::GetAndroidHardwareEGL() {
  char os_hardware_egl_str[PROP_VALUE_MAX];
  __system_property_get("ro.hardware.egl", os_hardware_egl_str);
  return std::string(os_hardware_egl_str);
}

int SysInfo::DalvikHeapSizeMB() {
  static int heap_size = GetDalvikHeapSizeMB();
  return heap_size;
}

int SysInfo::DalvikHeapGrowthLimitMB() {
  static int heap_growth_limit = GetDalvikHeapGrowthLimitMB();
  return heap_growth_limit;
}

static base::LazyInstance<base::internal::LazySysInfoValue<
    bool,
    android::SysUtils::IsLowEndDeviceFromJni>>::Leaky g_lazy_low_end_device =
    LAZY_INSTANCE_INITIALIZER;

bool SysInfo::IsLowEndDeviceImpl() {









  if (!base::android::IsVMInitialized())
    return false;
  return g_lazy_low_end_device.Get().value();
}

SysInfo::HardwareInfo SysInfo::GetHardwareInfoSync() {
  HardwareInfo info;
  info.manufacturer = HardwareManufacturerName();
  info.model = HardwareModelName();
  DCHECK(IsStringUTF8(info.manufacturer));
  DCHECK(IsStringUTF8(info.model));
  return info;
}

}  // namespace base
