// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_DEBUG_CRASH_LOGGING_H_
#define BASE_DEBUG_CRASH_LOGGING_H_

#include <stddef.h>

#include <memory>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/strings/string_piece.h"

namespace base {
namespace debug {

// provide additional debugging information beyond a stack trace. Crash keys
// have a name and a string value.
//
// The preferred API is //components/crash/core/common:crash_key, however not
// all clients can hold a direct dependency on that target. The API provided
// in this file indirects the dependency.
//
// Example usage:
//   static CrashKeyString* crash_key =
//       AllocateCrashKeyString("name", CrashKeySize::Size32);
//   SetCrashKeyString(crash_key, "value");
//   ClearCrashKeyString(crash_key);

// pre-determined values.
enum class CrashKeySize {
  Size32 = 32,
  Size64 = 64,
  Size256 = 256,
};

struct CrashKeyString;

// value up to length |size|. This will return null if the crash key system is
// not initialized.
BASE_EXPORT CrashKeyString* AllocateCrashKeyString(const char name[],
                                                   CrashKeySize size);

// if AllocateCrashKeyString() returned null. If |value| is longer than the
// size with which the key was allocated, it will be truncated.
BASE_EXPORT void SetCrashKeyString(CrashKeyString* crash_key,
                                   base::StringPiece value);

// null.
BASE_EXPORT void ClearCrashKeyString(CrashKeyString* crash_key);

// object, and clears it on destruction.
class BASE_EXPORT ScopedCrashKeyString {
 public:
  ScopedCrashKeyString(CrashKeyString* crash_key, base::StringPiece value);
  ~ScopedCrashKeyString();

 private:
  CrashKeyString* const crash_key_;

  DISALLOW_COPY_AND_ASSIGN(ScopedCrashKeyString);
};

// The following declarations are used to initialize the crash key system
// in //base by providing implementations for the above functions.

// API. This is implemented by a higher-layer component, and the instance is
// set using the function below.
class CrashKeyImplementation {
 public:
  virtual ~CrashKeyImplementation() = default;

  virtual CrashKeyString* Allocate(const char name[], CrashKeySize size) = 0;
  virtual void Set(CrashKeyString* crash_key, base::StringPiece value) = 0;
  virtual void Clear(CrashKeyString* crash_key) = 0;
};

// implementation, if it exists, with |impl|. The |impl| is copied into base.
BASE_EXPORT void SetCrashKeyImplementation(
    std::unique_ptr<CrashKeyImplementation> impl);

struct CrashKeyString {
  constexpr CrashKeyString(const char name[], CrashKeySize size)
      : name(name), size(size) {}
  const char* const name;
  const CrashKeySize size;
};

}  // namespace debug
}  // namespace base

#endif  // BASE_DEBUG_CRASH_LOGGING_H_
