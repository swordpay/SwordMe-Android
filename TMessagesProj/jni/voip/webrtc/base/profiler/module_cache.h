// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PROFILER_MODULE_CACHE_H_
#define BASE_PROFILER_MODULE_CACHE_H_

#include <memory>
#include <set>
#include <string>
#include <vector>

#include "base/base_export.h"
#include "base/containers/flat_set.h"
#include "base/files/file_path.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include <windows.h>
#endif

namespace base {

// address ranges.
//
// Cached lookup is necessary on Mac for performance, due to an inefficient
// dladdr implementation. See https://crrev.com/487092.
//
// Cached lookup is beneficial on Windows to minimize use of the loader
// lock. Note however that the cache retains a handle to looked-up modules for
// its lifetime, which may result in pinning modules in memory that were
// transiently loaded by the OS.
class BASE_EXPORT ModuleCache {
 public:


  class BASE_EXPORT Module {
   public:
    Module() = default;
    virtual ~Module() = default;

    Module(const Module&) = delete;
    Module& operator=(const Module&) = delete;

    virtual uintptr_t GetBaseAddress() const = 0;







    virtual std::string GetId() const = 0;


    virtual FilePath GetDebugBasename() const = 0;

    virtual size_t GetSize() const = 0;

    virtual bool IsNative() const = 0;
  };

  ModuleCache();
  ~ModuleCache();



  const Module* GetModuleForAddress(uintptr_t address);
  std::vector<const Module*> GetModules() const;














  void UpdateNonNativeModules(
      const std::vector<const Module*>& to_remove,
      std::vector<std::unique_ptr<const Module>> to_add);



  void AddCustomNativeModule(std::unique_ptr<const Module> module);

 private:





  struct ModuleAndAddressCompare {
    using is_transparent = void;
    bool operator()(const std::unique_ptr<const Module>& m1,
                    const std::unique_ptr<const Module>& m2) const;
    bool operator()(const std::unique_ptr<const Module>& m1,
                    uintptr_t address) const;
    bool operator()(uintptr_t address,
                    const std::unique_ptr<const Module>& m2) const;
  };


  static std::unique_ptr<const Module> CreateModuleForAddress(
      uintptr_t address);



  std::set<std::unique_ptr<const Module>, ModuleAndAddressCompare>
      native_modules_;








  base::flat_set<std::unique_ptr<const Module>, ModuleAndAddressCompare>
      non_native_modules_;






  std::vector<std::unique_ptr<const Module>> inactive_non_native_modules_;
};

}  // namespace base

#endif  // BASE_PROFILER_MODULE_CACHE_H_
