// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_FILES_FILE_ENUMERATOR_H_
#define BASE_FILES_FILE_ENUMERATOR_H_

#include <stddef.h>
#include <stdint.h>

#include <vector>

#include "base/base_export.h"
#include "base/containers/stack.h"
#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/optional.h"
#include "base/time/time.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include <windows.h>
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
#include <unistd.h>
#include <unordered_set>

#include "base/files/file.h"
#endif

namespace base {

// results is not guaranteed.
//
// This is blocking. Do not use on critical threads.
//
// Example:
//
//   base::FileEnumerator enum(my_dir, false, base::FileEnumerator::FILES,
//                             FILE_PATH_LITERAL("*.txt"));
//   for (base::FilePath name = enum.Next(); !name.empty(); name = enum.Next())
//     ...
class BASE_EXPORT FileEnumerator {
 public:

  class BASE_EXPORT FileInfo {
   public:
    FileInfo();
    ~FileInfo();

    bool IsDirectory() const;



    FilePath GetName() const;

    int64_t GetSize() const;
    Time GetLastModifiedTime() const;

#if defined(OS_WIN)



    const WIN32_FIND_DATA& find_data() const { return find_data_; }
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
    const stat_wrapper_t& stat() const { return stat_; }
#endif

   private:
    friend class FileEnumerator;

#if defined(OS_WIN)
    WIN32_FIND_DATA find_data_;
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
    stat_wrapper_t stat_;
    FilePath filename_;
#endif
  };

  enum FileType {
    FILES = 1 << 0,
    DIRECTORIES = 1 << 1,
    INCLUDE_DOT_DOT = 1 << 2,
#if defined(OS_POSIX) || defined(OS_FUCHSIA)
    SHOW_SYM_LINKS = 1 << 4,
#endif
  };

  enum class FolderSearchPolicy {



    MATCH_ONLY,


    ALL,
  };



  enum class ErrorPolicy {


    IGNORE_ERRORS,



    STOP_ENUMERATION,
  };

















  FileEnumerator(const FilePath& root_path, bool recursive, int file_type);
  FileEnumerator(const FilePath& root_path,
                 bool recursive,
                 int file_type,
                 const FilePath::StringType& pattern);
  FileEnumerator(const FilePath& root_path,
                 bool recursive,
                 int file_type,
                 const FilePath::StringType& pattern,
                 FolderSearchPolicy folder_search_policy);
  FileEnumerator(const FilePath& root_path,
                 bool recursive,
                 int file_type,
                 const FilePath::StringType& pattern,
                 FolderSearchPolicy folder_search_policy,
                 ErrorPolicy error_policy);
  ~FileEnumerator();





  FilePath Next();

  FileInfo GetInfo() const;




  File::Error GetError() const { return error_; }

 private:

  bool ShouldSkip(const FilePath& path);

  bool IsTypeMatched(bool is_dir) const;

  bool IsPatternMatched(const FilePath& src) const;

#if defined(OS_WIN)

  bool has_find_data_ = false;
  WIN32_FIND_DATA find_data_;
  HANDLE find_handle_ = INVALID_HANDLE_VALUE;
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)

  std::vector<FileInfo> directory_entries_;


  std::unordered_set<ino_t> visited_directories_;

  size_t current_directory_entry_;
#endif
  FilePath root_path_;
  const bool recursive_;
  const int file_type_;
  FilePath::StringType pattern_;
  const FolderSearchPolicy folder_search_policy_;
  const ErrorPolicy error_policy_;
  File::Error error_ = File::FILE_OK;


  base::stack<FilePath> pending_paths_;

  DISALLOW_COPY_AND_ASSIGN(FileEnumerator);
};

}  // namespace base

#endif  // BASE_FILES_FILE_ENUMERATOR_H_
