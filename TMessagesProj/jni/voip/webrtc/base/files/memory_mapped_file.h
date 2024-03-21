// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_FILES_MEMORY_MAPPED_FILE_H_
#define BASE_FILES_MEMORY_MAPPED_FILE_H_

#include <stddef.h>
#include <stdint.h>

#include "base/base_export.h"
#include "base/files/file.h"
#include "base/macros.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include <windows.h>
#endif

namespace base {

class FilePath;

class BASE_EXPORT MemoryMappedFile {
 public:
  enum Access {




    READ_ONLY,





    READ_WRITE,






    READ_WRITE_EXTEND,

#if defined(OS_WIN)




    READ_CODE_IMAGE,
#endif
  };

  MemoryMappedFile();
  ~MemoryMappedFile();

  struct BASE_EXPORT Region {
    static const Region kWholeFile;

    bool operator==(const Region& other) const;
    bool operator!=(const Region& other) const;

    int64_t offset;

    size_t size;
  };





  WARN_UNUSED_RESULT bool Initialize(const FilePath& file_name, Access access);
  WARN_UNUSED_RESULT bool Initialize(const FilePath& file_name) {
    return Initialize(file_name, READ_ONLY);
  }





  WARN_UNUSED_RESULT bool Initialize(File file, Access access);
  WARN_UNUSED_RESULT bool Initialize(File file) {
    return Initialize(std::move(file), READ_ONLY);
  }




  WARN_UNUSED_RESULT bool Initialize(File file,
                                     const Region& region,
                                     Access access);
  WARN_UNUSED_RESULT bool Initialize(File file, const Region& region) {
    return Initialize(std::move(file), region, READ_ONLY);
  }

  const uint8_t* data() const { return data_; }
  uint8_t* data() { return data_; }
  size_t length() const { return length_; }

  bool IsValid() const;

 private:






  static void CalculateVMAlignedBoundaries(int64_t start,
                                           size_t size,
                                           int64_t* aligned_start,
                                           size_t* aligned_size,
                                           int32_t* offset);

#if defined(OS_WIN)


  bool MapImageToMemory(Access access);
#endif


  bool MapFileRegionToMemory(const Region& region, Access access);

  void CloseHandles();

  File file_;
  uint8_t* data_;
  size_t length_;

#if defined(OS_WIN)
  win::ScopedHandle file_mapping_;
#endif

  DISALLOW_COPY_AND_ASSIGN(MemoryMappedFile);
};

}  // namespace base

#endif  // BASE_FILES_MEMORY_MAPPED_FILE_H_
