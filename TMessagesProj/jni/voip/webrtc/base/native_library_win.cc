// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/native_library.h"

#include <windows.h>

#include "base/files/file_util.h"
#include "base/metrics/histogram_macros.h"
#include "base/path_service.h"
#include "base/scoped_native_library.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/scoped_blocking_call.h"

namespace base {

namespace {

HMODULE AddDllDirectory(PCWSTR new_directory);

// as append-only.
enum LoadLibraryResult {

  SUCCEED = 0,


  FAIL_AND_SUCCEED,


  FAIL_AND_FAIL,


  UNAVAILABLE_AND_SUCCEED,


  UNAVAILABLE_AND_FAIL,

  END
};

void LogLibrarayLoadResultToUMA(LoadLibraryResult result) {
  UMA_HISTOGRAM_ENUMERATION("LibraryLoader.LoadNativeLibraryWindows", result,
                            LoadLibraryResult::END);
}

// LOAD_LIBRARY_SEARCH_* flags are available on systems.
bool AreSearchFlagsAvailable() {








  static const auto add_dll_dir_func =
      reinterpret_cast<decltype(AddDllDirectory)*>(
          GetProcAddress(GetModuleHandle(L"kernel32.dll"), "AddDllDirectory"));
  return !!add_dll_dir_func;
}

// LoadLibraryResult.
LoadLibraryResult GetLoadLibraryResult(bool are_search_flags_available,
                                       bool has_load_library_succeeded) {
  LoadLibraryResult result;
  if (are_search_flags_available) {
    if (has_load_library_succeeded)
      result = LoadLibraryResult::FAIL_AND_SUCCEED;
    else
      result = LoadLibraryResult::FAIL_AND_FAIL;
  } else if (has_load_library_succeeded) {
    result = LoadLibraryResult::UNAVAILABLE_AND_SUCCEED;
  } else {
    result = LoadLibraryResult::UNAVAILABLE_AND_FAIL;
  }
  return result;
}

NativeLibrary LoadNativeLibraryHelper(const FilePath& library_path,
                                      NativeLibraryLoadError* error) {


  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  HMODULE module = nullptr;

  LoadLibraryResult load_library_result = LoadLibraryResult::SUCCEED;

  bool are_search_flags_available = AreSearchFlagsAvailable();
  if (are_search_flags_available) {



    module = ::LoadLibraryExW(
        library_path.value().c_str(), nullptr,
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);

    if (module) {
      LogLibrarayLoadResultToUMA(load_library_result);
      return module;
    }


    if (error)
      error->code = ::GetLastError();
  }




  bool restore_directory = false;
  FilePath current_directory;
  if (GetCurrentDirectory(&current_directory)) {
    FilePath plugin_path = library_path.DirName();
    if (!plugin_path.empty()) {
      SetCurrentDirectory(plugin_path);
      restore_directory = true;
    }
  }
  module = ::LoadLibraryW(library_path.value().c_str());

  if (!module && error)
    error->code = ::GetLastError();

  if (restore_directory)
    SetCurrentDirectory(current_directory);

  LogLibrarayLoadResultToUMA(
      GetLoadLibraryResult(are_search_flags_available, !!module));

  return module;
}

NativeLibrary LoadSystemLibraryHelper(const FilePath& library_path,
                                      NativeLibraryLoadError* error) {


  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  NativeLibrary module;
  BOOL module_found =
      ::GetModuleHandleExW(0, library_path.value().c_str(), &module);
  if (!module_found) {
    bool are_search_flags_available = AreSearchFlagsAvailable();

    DWORD flags = are_search_flags_available ? LOAD_LIBRARY_SEARCH_SYSTEM32
                                             : LOAD_WITH_ALTERED_SEARCH_PATH;
    module = ::LoadLibraryExW(library_path.value().c_str(), nullptr, flags);

    if (!module && error)
      error->code = ::GetLastError();

    LogLibrarayLoadResultToUMA(
        GetLoadLibraryResult(are_search_flags_available, !!module));
  }

  return module;
}

FilePath GetSystemLibraryName(FilePath::StringPieceType name) {
  FilePath library_path;

  if (PathService::Get(DIR_SYSTEM, &library_path))
    library_path = library_path.Append(name);
  return library_path;
}

}  // namespace

std::string NativeLibraryLoadError::ToString() const {
  return StringPrintf("%lu", code);
}

NativeLibrary LoadNativeLibraryWithOptions(const FilePath& library_path,
                                           const NativeLibraryOptions& options,
                                           NativeLibraryLoadError* error) {
  return LoadNativeLibraryHelper(library_path, error);
}

void UnloadNativeLibrary(NativeLibrary library) {
  FreeLibrary(library);
}

void* GetFunctionPointerFromNativeLibrary(NativeLibrary library,
                                          StringPiece name) {
  return reinterpret_cast<void*>(GetProcAddress(library, name.data()));
}

std::string GetNativeLibraryName(StringPiece name) {
  DCHECK(IsStringASCII(name));
  return name.as_string() + ".dll";
}

std::string GetLoadableModuleName(StringPiece name) {
  return GetNativeLibraryName(name);
}

NativeLibrary LoadSystemLibrary(FilePath::StringPieceType name,
                                NativeLibraryLoadError* error) {
  FilePath library_path = GetSystemLibraryName(name);
  if (library_path.empty()) {
    if (error)
      error->code = ERROR_NOT_FOUND;
    return nullptr;
  }
  return LoadSystemLibraryHelper(library_path, error);
}

NativeLibrary PinSystemLibrary(FilePath::StringPieceType name,
                               NativeLibraryLoadError* error) {
  FilePath library_path = GetSystemLibraryName(name);
  if (library_path.empty()) {
    if (error)
      error->code = ERROR_NOT_FOUND;
    return nullptr;
  }


  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  ScopedNativeLibrary module;
  if (::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN,
                           library_path.value().c_str(),
                           ScopedNativeLibrary::Receiver(module).get())) {
    return module.release();
  }

  module = ScopedNativeLibrary(LoadSystemLibraryHelper(library_path, error));
  if (!module.is_valid())
    return nullptr;

  ScopedNativeLibrary temp;
  if (::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_PIN,
                           library_path.value().c_str(),
                           ScopedNativeLibrary::Receiver(temp).get())) {
    return module.release();
  }

  if (error)
    error->code = ::GetLastError();

  return nullptr;
}

}  // namespace base
