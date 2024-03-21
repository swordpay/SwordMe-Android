// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_FILES_FILE_TRACING_H_
#define BASE_FILES_FILE_TRACING_H_

#include <stdint.h>

#include "base/base_export.h"
#include "base/macros.h"

#define FILE_TRACING_PREFIX "File"

#define SCOPED_FILE_TRACE_WITH_SIZE(name, size) \
    FileTracing::ScopedTrace scoped_file_trace; \
    if (FileTracing::IsCategoryEnabled()) \
      scoped_file_trace.Initialize(FILE_TRACING_PREFIX "::" name, this, size)

#define SCOPED_FILE_TRACE(name) SCOPED_FILE_TRACE_WITH_SIZE(name, 0)

namespace base {

class File;
class FilePath;

class BASE_EXPORT FileTracing {
 public:

  static bool IsCategoryEnabled();

  class Provider {
   public:
    virtual ~Provider() = default;

    virtual bool FileTracingCategoryIsEnabled() const = 0;

    virtual void FileTracingEnable(const void* id) = 0;

    virtual void FileTracingDisable(const void* id) = 0;



    virtual void FileTracingEventBegin(const char* name,
                                       const void* id,
                                       const FilePath& path,
                                       int64_t size) = 0;

    virtual void FileTracingEventEnd(const char* name, const void* id) = 0;
  };

  static void SetProvider(Provider* provider);

  class ScopedEnabler {
   public:
    ScopedEnabler();
    ~ScopedEnabler();
  };

  class ScopedTrace {
   public:
    ScopedTrace();
    ~ScopedTrace();




    void Initialize(const char* name, const File* file, int64_t size);

   private:


    const void* id_;


    const char* name_;

    DISALLOW_COPY_AND_ASSIGN(ScopedTrace);
  };

  DISALLOW_COPY_AND_ASSIGN(FileTracing);
};

}  // namespace base

#endif  // BASE_FILES_FILE_TRACING_H_
