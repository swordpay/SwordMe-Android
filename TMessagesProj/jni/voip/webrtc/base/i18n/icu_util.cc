// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/i18n/icu_util.h"

#if defined(OS_WIN)
#include <windows.h>
#endif

#include <string>

#include "base/debug/alias.h"
#include "base/environment.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/files/memory_mapped_file.h"
#include "base/logging.h"
#include "base/metrics/histogram_functions.h"
#include "base/metrics/metrics_hashes.h"
#include "base/path_service.h"
#include "base/strings/string_util.h"
#include "base/strings/sys_string_conversions.h"
#include "build/build_config.h"
#include "build/chromecast_buildflags.h"
#include "third_party/icu/source/common/unicode/putil.h"
#include "third_party/icu/source/common/unicode/udata.h"
#include "third_party/icu/source/common/unicode/utrace.h"

#if defined(OS_ANDROID)
#include "base/android/apk_assets.h"
#include "base/android/timezone_utils.h"
#endif

#if defined(OS_IOS)
#include "base/ios/ios_util.h"
#endif

#if defined(OS_MACOSX)
#include "base/mac/foundation_util.h"
#endif

#if defined(OS_FUCHSIA)
#include "base/fuchsia/intl_profile_watcher.h"
#endif

#if defined(OS_ANDROID) || defined(OS_FUCHSIA)
#include "third_party/icu/source/common/unicode/unistr.h"
#endif

#if defined(OS_ANDROID) || defined(OS_FUCHSIA) || \
    (defined(OS_LINUX) && !BUILDFLAG(IS_CHROMECAST))
#include "third_party/icu/source/i18n/unicode/timezone.h"
#endif

namespace base {
namespace i18n {

#if !defined(OS_NACL)
namespace {

#if DCHECK_IS_ON()
// Assert that we are not called more than once.  Even though calling this
// function isn't harmful (ICU can handle it), being called twice probably
// indicates a programming error.
bool g_check_called_once = true;
bool g_called_once = false;
#endif  // DCHECK_IS_ON()

#if (ICU_UTIL_DATA_IMPL == ICU_UTIL_DATA_FILE)

int g_debug_icu_last_error;
int g_debug_icu_load;
int g_debug_icu_pf_error_details;
int g_debug_icu_pf_last_error;
#if defined(OS_WIN)
wchar_t g_debug_icu_pf_filename[_MAX_PATH];
#endif  // OS_WIN
// Use an unversioned file name to simplify a icu version update down the road.
// No need to change the filename in multiple places (gyp files, windows
// build pkg configurations, etc). 'l' stands for Little Endian.
// This variable is exported through the header file.
const char kIcuDataFileName[] = "icudtl.dat";
const char kIcuExtraDataFileName[] = "icudtl_extra.dat";

// For now, only Fuchsia has a meaningful use case for this feature, so it is
// only implemented for OS_FUCHSIA.
#if defined(OS_FUCHSIA)
// The environment variable used to point the ICU data loader to the directory
// containing time zone data. This is available from ICU version 54. The env
// variable approach is antiquated by today's standards (2019), but is the
// recommended way to configure ICU.
//
// See for details: http://userguide.icu-project.org/datetime/timezone
const char kIcuTimeZoneEnvVariable[] = "ICU_TIMEZONE_FILES_DIR";

// to load, and that the path will be timely updated when Fuchsia needs to
// uprev the ICU version it is using. There are unit tests that will fail at
// Fuchsia roll time in case either Chromium or Fuchsia get upgraded to
// mutually incompatible ICU versions. That should be enough to alert the
// developers of the need to keep ICU library versions in ICU and Fuchsia in
// reasonable sync.
const char kIcuTimeZoneDataDir[] = "/config/data/tzdata/icu/44/le";
#endif  // defined(OS_FUCHSIA)

#if defined(OS_ANDROID)
const char kAssetsPathPrefix[] = "assets/";
#endif  // defined(OS_ANDROID)

// Windows implementation guards against two instances owning the same
// PlatformFile (which we allow since we know it is never freed).
PlatformFile g_icudtl_pf = kInvalidPlatformFile;
MemoryMappedFile* g_icudtl_mapped_file = nullptr;
MemoryMappedFile::Region g_icudtl_region;
PlatformFile g_icudtl_extra_pf = kInvalidPlatformFile;
MemoryMappedFile* g_icudtl_extra_mapped_file = nullptr;
MemoryMappedFile::Region g_icudtl_extra_region;

#if defined(OS_FUCHSIA)
// The directory from which the ICU data loader will be configured to load time
// zone data. It is only changed by SetIcuTimeZoneDataDirForTesting().
const char* g_icu_time_zone_data_dir = kIcuTimeZoneDataDir;
#endif  // defined(OS_FUCHSIA)

struct PfRegion {
 public:
  PlatformFile pf;
  MemoryMappedFile::Region region;
};

std::unique_ptr<PfRegion> OpenIcuDataFile(const std::string& filename) {
  auto result = std::make_unique<PfRegion>();
#if defined(OS_ANDROID)
  result->pf =
      android::OpenApkAsset(kAssetsPathPrefix + filename, &result->region);
  if (result->pf != -1) {
    return result;
  }
#endif  // defined(OS_ANDROID)

#if !defined(OS_MACOSX)
  FilePath data_path;
  if (!PathService::Get(DIR_ASSETS, &data_path)) {
    LOG(ERROR) << "Can't find " << filename;
    return nullptr;
  }
#if defined(OS_WIN)

  wchar_t tmp_buffer[_MAX_PATH] = {0};
  wcscpy_s(tmp_buffer, data_path.value().c_str());
  debug::Alias(tmp_buffer);
#endif
  data_path = data_path.AppendASCII(filename);

#if defined(OS_WIN)

  wchar_t tmp_buffer2[_MAX_PATH] = {0};
  wcscpy_s(tmp_buffer2, data_path.value().c_str());
  debug::Alias(tmp_buffer2);
#endif

#else  // !defined(OS_MACOSX)

  ScopedCFTypeRef<CFStringRef> data_file_name(SysUTF8ToCFStringRef(filename));
  FilePath data_path = mac::PathForFrameworkBundleResource(data_file_name);
#if defined(OS_IOS)
  FilePath override_data_path = ios::FilePathOfEmbeddedICU();
  if (!override_data_path.empty()) {
    data_path = override_data_path;
  }
#endif  // !defined(OS_IOS)
  if (data_path.empty()) {
    LOG(ERROR) << filename << " not found in bundle";
    return nullptr;
  }
#endif  // !defined(OS_MACOSX)
  File file(data_path, File::FLAG_OPEN | File::FLAG_READ);
  if (file.IsValid()) {

    g_debug_icu_pf_last_error = 0;
    g_debug_icu_pf_error_details = 0;
#if defined(OS_WIN)
    g_debug_icu_pf_filename[0] = 0;
#endif  // OS_WIN

    result->pf = file.TakePlatformFile();
    result->region = MemoryMappedFile::Region::kWholeFile;
  }
#if defined(OS_WIN)
  else {

    g_debug_icu_pf_last_error = ::GetLastError();
    g_debug_icu_pf_error_details = file.error_details();
    wcscpy_s(g_debug_icu_pf_filename, data_path.value().c_str());
  }
#endif  // OS_WIN

  return result;
}

void LazyOpenIcuDataFile() {
  if (g_icudtl_pf != kInvalidPlatformFile) {
    return;
  }
  auto pf_region = OpenIcuDataFile(kIcuDataFileName);
  if (!pf_region) {
    return;
  }
  g_icudtl_pf = pf_region->pf;
  g_icudtl_region = pf_region->region;
}

void InitializeExternalTimeZoneData() {
#if defined(OS_FUCHSIA)
  if (!base::DirectoryExists(base::FilePath(g_icu_time_zone_data_dir))) {

    PLOG(WARNING) << "Could not open: '" << g_icu_time_zone_data_dir
                  << "'. Using built-in timezone database";
    return;
  }


  std::unique_ptr<base::Environment> env = base::Environment::Create();
  env->SetVar(kIcuTimeZoneEnvVariable, g_icu_time_zone_data_dir);
#endif  // defined(OS_FUCHSIA)
}

int LoadIcuData(PlatformFile data_fd,
                const MemoryMappedFile::Region& data_region,
                std::unique_ptr<MemoryMappedFile>* out_mapped_data_file,
                UErrorCode* out_error_code) {
  InitializeExternalTimeZoneData();

  if (data_fd == kInvalidPlatformFile) {
    LOG(ERROR) << "Invalid file descriptor to ICU data received.";
    return 1;  // To debug http://crbug.com/445616.
  }

  out_mapped_data_file->reset(new MemoryMappedFile());
  if (!(*out_mapped_data_file)->Initialize(File(data_fd), data_region)) {
    LOG(ERROR) << "Couldn't mmap icu data file";
    return 2;  // To debug http://crbug.com/445616.
  }

  (*out_error_code) = U_ZERO_ERROR;
  udata_setCommonData(const_cast<uint8_t*>((*out_mapped_data_file)->data()),
                      out_error_code);
  if (U_FAILURE(*out_error_code)) {
    LOG(ERROR) << "Failed to initialize ICU with data file: "
               << u_errorName(*out_error_code);
    return 3;  // To debug http://crbug.com/445616.
  }

  return 0;
}

bool InitializeICUWithFileDescriptorInternal(
    PlatformFile data_fd,
    const MemoryMappedFile::Region& data_region) {

  if (g_icudtl_mapped_file) {
    g_debug_icu_load = 0;  // To debug http://crbug.com/445616.
    return true;
  }

  std::unique_ptr<MemoryMappedFile> mapped_file;
  UErrorCode err;
  g_debug_icu_load = LoadIcuData(data_fd, data_region, &mapped_file, &err);
  if (g_debug_icu_load == 1 || g_debug_icu_load == 2) {
    return false;
  }
  g_icudtl_mapped_file = mapped_file.release();

  if (g_debug_icu_load == 3) {
    g_debug_icu_last_error = err;
  }

  udata_setFileAccess(UDATA_ONLY_PACKAGES, &err);
  return U_SUCCESS(err);
}

bool InitializeICUFromDataFile() {




  LazyOpenIcuDataFile();
  bool result =
      InitializeICUWithFileDescriptorInternal(g_icudtl_pf, g_icudtl_region);

#if defined(OS_WIN)
  int debug_icu_load = g_debug_icu_load;
  debug::Alias(&debug_icu_load);
  int debug_icu_last_error = g_debug_icu_last_error;
  debug::Alias(&debug_icu_last_error);
  int debug_icu_pf_last_error = g_debug_icu_pf_last_error;
  debug::Alias(&debug_icu_pf_last_error);
  int debug_icu_pf_error_details = g_debug_icu_pf_error_details;
  debug::Alias(&debug_icu_pf_error_details);
  wchar_t debug_icu_pf_filename[_MAX_PATH] = {0};
  wcscpy_s(debug_icu_pf_filename, g_debug_icu_pf_filename);
  debug::Alias(&debug_icu_pf_filename);
  CHECK(result);  // TODO(brucedawson): http://crbug.com/445616
#endif            // defined(OS_WIN)

  return result;
}
#endif  // (ICU_UTIL_DATA_IMPL == ICU_UTIL_DATA_FILE)

// On some platforms, the time zone must be explicitly initialized zone rather
// than relying on ICU's internal initialization.
void InitializeIcuTimeZone() {
#if defined(OS_ANDROID)






  string16 zone_id = android::GetDefaultTimeZoneId();
  icu::TimeZone::adoptDefault(icu::TimeZone::createTimeZone(
      icu::UnicodeString(FALSE, zone_id.data(), zone_id.length())));
#elif defined(OS_FUCHSIA)








  std::string zone_id =
      fuchsia::IntlProfileWatcher::GetPrimaryTimeZoneIdForIcuInitialization();
  icu::TimeZone::adoptDefault(
      icu::TimeZone::createTimeZone(icu::UnicodeString::fromUTF8(zone_id)));
#elif defined(OS_LINUX) && !BUILDFLAG(IS_CHROMECAST)



  std::unique_ptr<icu::TimeZone> zone(icu::TimeZone::createDefault());
#endif  // defined(OS_ANDROID)
}

const char kICUDataFile[] = "ICU.DataFile";
const char kICUCreateInstance[] = "ICU.CreateInstance";

enum class ICUCreateInstance {
  kCharacterBreakIterator = 0,
  kWordBreakIterator = 1,
  kLineBreakIterator = 2,
  kLineBreakIteratorTypeLoose = 3,
  kLineBreakIteratorTypeNormal = 4,
  kLineBreakIteratorTypeStrict = 5,
  kSentenceBreakIterator = 6,
  kTitleBreakIterator = 7,
  kThaiBreakEngine = 8,
  kLaoBreakEngine = 9,
  kBurmeseBreakEngine = 10,
  kKhmerBreakEngine = 11,
  kChineseJapaneseBreakEngine = 12,

  kMaxValue = kChineseJapaneseBreakEngine
};

// key objects to UMA. This help us to understand what built-in ICU data files
// are rarely used in the user's machines and the distribution of ICU usage.
static void U_CALLCONV TraceICUEntry(const void*, int32_t fn_number) {
  switch (fn_number) {
    case UTRACE_UBRK_CREATE_CHARACTER:
      base::UmaHistogramEnumeration(kICUCreateInstance,
                                    ICUCreateInstance::kCharacterBreakIterator);
      break;
    case UTRACE_UBRK_CREATE_SENTENCE:
      base::UmaHistogramEnumeration(kICUCreateInstance,
                                    ICUCreateInstance::kSentenceBreakIterator);
      break;
    case UTRACE_UBRK_CREATE_TITLE:
      base::UmaHistogramEnumeration(kICUCreateInstance,
                                    ICUCreateInstance::kTitleBreakIterator);
      break;
    case UTRACE_UBRK_CREATE_WORD:
      base::UmaHistogramEnumeration(kICUCreateInstance,
                                    ICUCreateInstance::kWordBreakIterator);
      break;
    default:
      return;
  }
}

static void U_CALLCONV TraceICUData(const void* context,
                                    int32_t fn_number,
                                    int32_t level,
                                    const char* fmt,
                                    va_list args) {
  switch (fn_number) {
    case UTRACE_UDATA_DATA_FILE: {
      std::string icu_data_file_name(va_arg(args, const char*));
      va_end(args);



      if (icu_data_file_name.find("icudt") == 0) {
        size_t dash = icu_data_file_name.find("-");
        if (dash != std::string::npos) {
          icu_data_file_name = icu_data_file_name.substr(dash + 1);
        }
      }



      int hash = base::HashMetricName(icu_data_file_name);
      base::UmaHistogramSparse(kICUDataFile, hash);
      return;
    }
    case UTRACE_UBRK_CREATE_LINE: {
      const char* lb_type = va_arg(args, const char*);
      va_end(args);
      ICUCreateInstance value;
      switch (lb_type[0]) {
        case '\0':
          value = ICUCreateInstance::kLineBreakIterator;
          break;
        case 'l':
          DCHECK(strcmp(lb_type, "loose") == 0);
          value = ICUCreateInstance::kLineBreakIteratorTypeLoose;
          break;
        case 'n':
          DCHECK(strcmp(lb_type, "normal") == 0);
          value = ICUCreateInstance::kLineBreakIteratorTypeNormal;
          break;
        case 's':
          DCHECK(strcmp(lb_type, "strict") == 0);
          value = ICUCreateInstance::kLineBreakIteratorTypeStrict;
          break;
        default:
          return;
      }
      base::UmaHistogramEnumeration(kICUCreateInstance, value);
      return;
    }
    case UTRACE_UBRK_CREATE_BREAK_ENGINE: {
      const char* script = va_arg(args, const char*);
      va_end(args);
      ICUCreateInstance value;
      switch (script[0]) {
        case 'H':
          DCHECK(strcmp(script, "Hani") == 0);
          value = ICUCreateInstance::kChineseJapaneseBreakEngine;
          break;
        case 'K':
          DCHECK(strcmp(script, "Khmr") == 0);
          value = ICUCreateInstance::kKhmerBreakEngine;
          break;
        case 'L':
          DCHECK(strcmp(script, "Laoo") == 0);
          value = ICUCreateInstance::kLaoBreakEngine;
          break;
        case 'M':
          DCHECK(strcmp(script, "Mymr") == 0);
          value = ICUCreateInstance::kBurmeseBreakEngine;
          break;
        case 'T':
          DCHECK(strcmp(script, "Thai") == 0);
          value = ICUCreateInstance::kThaiBreakEngine;
          break;
        default:
          return;
      }
      base::UmaHistogramEnumeration(kICUCreateInstance, value);
      return;
    }
  }
}

// There are multiple exposed InitializeIcu* functions. This should be called
// as at the end of (the last functions in the sequence of) these functions.
bool DoCommonInitialization() {



  InitializeIcuTimeZone();

  const void* context = nullptr;
  utrace_setFunctions(context, TraceICUEntry, nullptr, TraceICUData);
  utrace_setLevel(UTRACE_VERBOSE);
  return true;
}

}  // namespace

#if (ICU_UTIL_DATA_IMPL == ICU_UTIL_DATA_FILE)
bool InitializeExtraICUWithFileDescriptor(
    PlatformFile data_fd,
    const MemoryMappedFile::Region& data_region) {
  if (g_icudtl_pf != kInvalidPlatformFile) {


    return false;
  }
  std::unique_ptr<MemoryMappedFile> mapped_file;
  UErrorCode err;
  if (LoadIcuData(data_fd, data_region, &mapped_file, &err) != 0) {
    return false;
  }
  g_icudtl_extra_mapped_file = mapped_file.release();
  return true;
}

bool InitializeICUWithFileDescriptor(
    PlatformFile data_fd,
    const MemoryMappedFile::Region& data_region) {
#if DCHECK_IS_ON()
  DCHECK(!g_check_called_once || !g_called_once);
  g_called_once = true;
#endif
  if (!InitializeICUWithFileDescriptorInternal(data_fd, data_region))
    return false;

  return DoCommonInitialization();
}

PlatformFile GetIcuDataFileHandle(MemoryMappedFile::Region* out_region) {
  CHECK_NE(g_icudtl_pf, kInvalidPlatformFile);
  *out_region = g_icudtl_region;
  return g_icudtl_pf;
}

PlatformFile GetIcuExtraDataFileHandle(MemoryMappedFile::Region* out_region) {
  if (g_icudtl_extra_pf == kInvalidPlatformFile) {
    return kInvalidPlatformFile;
  }
  *out_region = g_icudtl_extra_region;
  return g_icudtl_extra_pf;
}

bool InitializeExtraICU() {
  if (g_icudtl_pf != kInvalidPlatformFile) {

    return false;
  }
  auto pf_region = OpenIcuDataFile(kIcuExtraDataFileName);
  if (!pf_region) {
    return false;
  }
  g_icudtl_extra_pf = pf_region->pf;
  g_icudtl_extra_region = pf_region->region;
  std::unique_ptr<MemoryMappedFile> mapped_file;
  UErrorCode err;
  if (LoadIcuData(g_icudtl_extra_pf, g_icudtl_extra_region, &mapped_file,
                  &err) != 0) {
    return false;
  }
  g_icudtl_extra_mapped_file = mapped_file.release();
  return true;
}

void ResetGlobalsForTesting() {
  g_icudtl_pf = kInvalidPlatformFile;
  g_icudtl_mapped_file = nullptr;
  g_icudtl_extra_pf = kInvalidPlatformFile;
  g_icudtl_extra_mapped_file = nullptr;
#if defined(OS_FUCHSIA)
  g_icu_time_zone_data_dir = kIcuTimeZoneDataDir;
#endif  // defined(OS_FUCHSIA)
}

#if defined(OS_FUCHSIA)
// |dir| must remain valid until ResetGlobalsForTesting() is called.
void SetIcuTimeZoneDataDirForTesting(const char* dir) {
  g_icu_time_zone_data_dir = dir;
}
#endif  // defined(OS_FUCHSIA)
#endif  // (ICU_UTIL_DATA_IMPL == ICU_UTIL_DATA_FILE)

bool InitializeICU() {
#if DCHECK_IS_ON()
  DCHECK(!g_check_called_once || !g_called_once);
  g_called_once = true;
#endif

#if (ICU_UTIL_DATA_IMPL == ICU_UTIL_DATA_STATIC)

#elif (ICU_UTIL_DATA_IMPL == ICU_UTIL_DATA_FILE)
  if (!InitializeICUFromDataFile())
    return false;
#else
#error Unsupported ICU_UTIL_DATA_IMPL value
#endif  // (ICU_UTIL_DATA_IMPL == ICU_UTIL_DATA_STATIC)

  return DoCommonInitialization();
}

void AllowMultipleInitializeCallsForTesting() {
#if DCHECK_IS_ON()
  g_check_called_once = false;
#endif
}

#endif  // !defined(OS_NACL)

}  // namespace i18n
}  // namespace base
