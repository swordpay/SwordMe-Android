// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/system/sys_info.h"

#include <stddef.h>
#include <stdint.h>
#include <sys/utsname.h>

#include "base/environment.h"
#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/lazy_instance.h"
#include "base/stl_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_split.h"
#include "base/strings/string_tokenizer.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/threading/thread_restrictions.h"

namespace base {

namespace {

const char* const kLinuxStandardBaseVersionKeys[] = {
    "CHROMEOS_RELEASE_VERSION", "GOOGLE_RELEASE", "DISTRIB_RELEASE",
};

const char kChromeOsReleaseNameKey[] = "CHROMEOS_RELEASE_NAME";

const char* const kChromeOsReleaseNames[] = {
    "Chrome OS", "Chromium OS",
};

const char kLinuxStandardBaseReleaseFile[] = "/etc/lsb-release";

const char kLsbReleaseKey[] = "LSB_RELEASE";
const char kLsbReleaseTimeKey[] = "LSB_RELEASE_TIME";  // Seconds since epoch

const char kLsbReleaseSourceKey[] = "lsb-release";
const char kLsbReleaseSourceEnv[] = "env";
const char kLsbReleaseSourceFile[] = "file";

class ChromeOSVersionInfo {
 public:
  ChromeOSVersionInfo() { Parse(); }

  void Parse() {
    lsb_release_map_.clear();
    major_version_ = 0;
    minor_version_ = 0;
    bugfix_version_ = 0;
    is_running_on_chromeos_ = false;

    std::string lsb_release, lsb_release_time_str;
    std::unique_ptr<Environment> env(Environment::Create());
    bool parsed_from_env =
        env->GetVar(kLsbReleaseKey, &lsb_release) &&
        env->GetVar(kLsbReleaseTimeKey, &lsb_release_time_str);
    if (parsed_from_env) {
      double us = 0;
      if (StringToDouble(lsb_release_time_str, &us))
        lsb_release_time_ = Time::FromDoubleT(us);
    } else {



      ThreadRestrictions::ScopedAllowIO allow_io;
      FilePath path(kLinuxStandardBaseReleaseFile);
      ReadFileToString(path, &lsb_release);
      File::Info fileinfo;
      if (GetFileInfo(path, &fileinfo))
        lsb_release_time_ = fileinfo.creation_time;
    }
    ParseLsbRelease(lsb_release);

    lsb_release_map_[kLsbReleaseSourceKey] =
        parsed_from_env ? kLsbReleaseSourceEnv : kLsbReleaseSourceFile;
  }

  bool GetLsbReleaseValue(const std::string& key, std::string* value) {
    LsbReleaseMap::const_iterator iter = lsb_release_map_.find(key);
    if (iter == lsb_release_map_.end())
      return false;
    *value = iter->second;
    return true;
  }

  void GetVersionNumbers(int32_t* major_version,
                         int32_t* minor_version,
                         int32_t* bugfix_version) {
    *major_version = major_version_;
    *minor_version = minor_version_;
    *bugfix_version = bugfix_version_;
  }

  const Time& lsb_release_time() const { return lsb_release_time_; }
  bool is_running_on_chromeos() const { return is_running_on_chromeos_; }

 private:
  void ParseLsbRelease(const std::string& lsb_release) {



    base::StringPairs pairs;
    SplitStringIntoKeyValuePairs(lsb_release, '=', '\n', &pairs);
    for (size_t i = 0; i < pairs.size(); ++i) {
      std::string key, value;
      TrimWhitespaceASCII(pairs[i].first, TRIM_ALL, &key);
      TrimWhitespaceASCII(pairs[i].second, TRIM_ALL, &value);
      if (key.empty())
        continue;
      lsb_release_map_[key] = value;
    }

    std::string version;
    for (size_t i = 0; i < base::size(kLinuxStandardBaseVersionKeys); ++i) {
      std::string key = kLinuxStandardBaseVersionKeys[i];
      if (GetLsbReleaseValue(key, &version) && !version.empty())
        break;
    }
    StringTokenizer tokenizer(version, ".");
    if (tokenizer.GetNext()) {
      StringToInt(tokenizer.token_piece(), &major_version_);
    }
    if (tokenizer.GetNext()) {
      StringToInt(tokenizer.token_piece(), &minor_version_);
    }
    if (tokenizer.GetNext()) {
      StringToInt(tokenizer.token_piece(), &bugfix_version_);
    }

    std::string release_name;
    if (GetLsbReleaseValue(kChromeOsReleaseNameKey, &release_name)) {
      for (size_t i = 0; i < base::size(kChromeOsReleaseNames); ++i) {
        if (release_name == kChromeOsReleaseNames[i]) {
          is_running_on_chromeos_ = true;
          break;
        }
      }
    }
  }

  using LsbReleaseMap = std::map<std::string, std::string>;
  Time lsb_release_time_;
  LsbReleaseMap lsb_release_map_;
  int32_t major_version_;
  int32_t minor_version_;
  int32_t bugfix_version_;
  bool is_running_on_chromeos_;
};

static LazyInstance<ChromeOSVersionInfo>::Leaky g_chrome_os_version_info =
    LAZY_INSTANCE_INITIALIZER;

ChromeOSVersionInfo& GetChromeOSVersionInfo() {
  return g_chrome_os_version_info.Get();
}

}  // namespace

void SysInfo::OperatingSystemVersionNumbers(int32_t* major_version,
                                            int32_t* minor_version,
                                            int32_t* bugfix_version) {
  return GetChromeOSVersionInfo().GetVersionNumbers(
      major_version, minor_version, bugfix_version);
}

std::string SysInfo::OperatingSystemVersion() {
  int32_t major, minor, bugfix;
  GetChromeOSVersionInfo().GetVersionNumbers(&major, &minor, &bugfix);
  return base::StringPrintf("%d.%d.%d", major, minor, bugfix);
}

std::string SysInfo::KernelVersion() {
  struct utsname info;
  if (uname(&info) < 0) {
    NOTREACHED();
    return std::string();
  }
  return std::string(info.release);
}

bool SysInfo::GetLsbReleaseValue(const std::string& key, std::string* value) {
  return GetChromeOSVersionInfo().GetLsbReleaseValue(key, value);
}

std::string SysInfo::GetLsbReleaseBoard() {
  const char kMachineInfoBoard[] = "CHROMEOS_RELEASE_BOARD";
  std::string board;
  if (!GetLsbReleaseValue(kMachineInfoBoard, &board))
    board = "unknown";
  return board;
}

Time SysInfo::GetLsbReleaseTime() {
  return GetChromeOSVersionInfo().lsb_release_time();
}

bool SysInfo::IsRunningOnChromeOS() {
  return GetChromeOSVersionInfo().is_running_on_chromeos();
}

void SysInfo::SetChromeOSVersionInfoForTest(const std::string& lsb_release,
                                            const Time& lsb_release_time) {
  std::unique_ptr<Environment> env(Environment::Create());
  env->SetVar(kLsbReleaseKey, lsb_release);
  env->SetVar(kLsbReleaseTimeKey, NumberToString(lsb_release_time.ToDoubleT()));
  g_chrome_os_version_info.Get().Parse();
}

}  // namespace base
