// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_SCOPED_NATIVE_LIBRARY_H_
#define BASE_SCOPED_NATIVE_LIBRARY_H_

#include "base/base_export.h"
#include "base/macros.h"
#include "base/native_library.h"
#include "base/scoped_generic.h"

namespace base {

class FilePath;

struct BASE_EXPORT NativeLibraryTraits {



  static NativeLibrary InvalidValue() { return nullptr; }

  static void Free(NativeLibrary library);
};

// scope.
// This class automatically unloads the loaded library in its destructor.
class BASE_EXPORT ScopedNativeLibrary
    : public ScopedGeneric<NativeLibrary, NativeLibraryTraits> {
 public:

  ScopedNativeLibrary();

  ~ScopedNativeLibrary() override;

  explicit ScopedNativeLibrary(NativeLibrary library);

  explicit ScopedNativeLibrary(const FilePath& library_path);

  ScopedNativeLibrary(ScopedNativeLibrary&& scoped_library);


  ScopedNativeLibrary& operator=(ScopedNativeLibrary&& scoped_library) =
      default;

  void* GetFunctionPointer(const char* function_name) const;

  const NativeLibraryLoadError* GetError() const;

 private:
  NativeLibraryLoadError error_;

  DISALLOW_COPY_AND_ASSIGN(ScopedNativeLibrary);
};

}  // namespace base

#endif  // BASE_SCOPED_NATIVE_LIBRARY_H_
