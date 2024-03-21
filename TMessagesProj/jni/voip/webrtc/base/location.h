// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_LOCATION_H_
#define BASE_LOCATION_H_

#include <stddef.h>

#include <cassert>
#include <functional>
#include <string>

#include "base/base_export.h"
#include "base/debug/debugging_buildflags.h"
#include "base/hash/hash.h"
#include "build/build_config.h"

namespace base {

#if defined(__has_builtin)
// Clang allows detection of these builtins.
#define SUPPORTS_LOCATION_BUILTINS                                       \
  (__has_builtin(__builtin_FUNCTION) && __has_builtin(__builtin_FILE) && \
   __has_builtin(__builtin_LINE))
#elif defined(COMPILER_GCC) && __GNUC__ >= 7
// GCC has supported these for a long time, but they point at the function
// declaration in the case of default arguments, rather than at the call site.
#define SUPPORTS_LOCATION_BUILTINS 1
#else
#define SUPPORTS_LOCATION_BUILTINS 0
#endif

// significantly brought to life.
class BASE_EXPORT Location {
 public:
  Location();
  Location(const Location& other);



  Location(const char* file_name, const void* program_counter);



  Location(const char* function_name,
           const char* file_name,
           int line_number,
           const void* program_counter);


  bool operator==(const Location& other) const {
    return program_counter_ == other.program_counter_;
  }



  bool has_source_info() const { return function_name_ && file_name_; }


  const char* function_name() const { return function_name_; }


  const char* file_name() const { return file_name_; }


  int line_number() const { return line_number_; }



  const void* program_counter() const { return program_counter_; }


  std::string ToString() const;

#if !BUILDFLAG(FROM_HERE_USES_LOCATION_BUILTINS)
#if !BUILDFLAG(ENABLE_LOCATION_SOURCE)
  static Location CreateFromHere(const char* file_name);
#else
  static Location CreateFromHere(const char* function_name,
                                 const char* file_name,
                                 int line_number);
#endif
#endif

#if SUPPORTS_LOCATION_BUILTINS && BUILDFLAG(ENABLE_LOCATION_SOURCE)
  static Location Current(const char* function_name = __builtin_FUNCTION(),
                          const char* file_name = __builtin_FILE(),
                          int line_number = __builtin_LINE());
#elif SUPPORTS_LOCATION_BUILTINS
  static Location Current(const char* file_name = __builtin_FILE());
#else
  static Location Current();
#endif

 private:
  const char* function_name_ = nullptr;
  const char* file_name_ = nullptr;
  int line_number_ = -1;
  const void* program_counter_ = nullptr;
};

BASE_EXPORT const void* GetProgramCounter();

#if BUILDFLAG(FROM_HERE_USES_LOCATION_BUILTINS)

#define FROM_HERE ::base::Location::Current()

#elif BUILDFLAG(ENABLE_LOCATION_SOURCE)

#define FROM_HERE ::base::Location::CreateFromHere(__func__, __FILE__, __LINE__)

#else

#define FROM_HERE ::base::Location::CreateFromHere(__FILE__)

#endif

}  // namespace base

namespace std {

template <>
struct hash<::base::Location> {
  std::size_t operator()(const ::base::Location& loc) const {
    const void* program_counter = loc.program_counter();
    return base::FastHash(base::as_bytes(base::make_span(&program_counter, 1)));
  }
};

}  // namespace std

#endif  // BASE_LOCATION_H_
