// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_FILES_SCOPED_TEMP_DIR_H_
#define BASE_FILES_SCOPED_TEMP_DIR_H_

// cleaned up (recursively) when this object goes out of scope.  Since deletion
// occurs during the destructor, no further error handling is possible if the
// directory fails to be deleted.  As a result, deletion is not guaranteed by
// this class.  (However note that, whenever possible, by default
// CreateUniqueTempDir creates the directory in a location that is
// automatically cleaned up on reboot, or at other appropriate times.)
//
// Multiple calls to the methods which establish a temporary directory
// (CreateUniqueTempDir, CreateUniqueTempDirUnderPath, and Set) must have
// intervening calls to Delete or Take, or the calls will fail.

#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "base/files/file_path.h"

namespace base {

class BASE_EXPORT ScopedTempDir {
 public:

  ScopedTempDir();

  ScopedTempDir(ScopedTempDir&&) noexcept;
  ScopedTempDir& operator=(ScopedTempDir&&);

  ~ScopedTempDir();


  bool CreateUniqueTempDir() WARN_UNUSED_RESULT;

  bool CreateUniqueTempDirUnderPath(const FilePath& path) WARN_UNUSED_RESULT;


  bool Set(const FilePath& path) WARN_UNUSED_RESULT;

  bool Delete() WARN_UNUSED_RESULT;


  FilePath Take();


  const FilePath& GetPath() const;

  bool IsValid() const;


  static const FilePath::CharType* GetTempDirPrefix();

 private:
  FilePath path_;
};

}  // namespace base

#endif  // BASE_FILES_SCOPED_TEMP_DIR_H_
