// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_FILES_FILE_PROXY_H_
#define BASE_FILES_FILE_PROXY_H_

#include <stdint.h>

#include "base/base_export.h"
#include "base/callback_forward.h"
#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"

namespace base {

class TaskRunner;
class Time;

// same rules of the equivalent File method, as they are implemented by bouncing
// the operation to File using a TaskRunner.
//
// This class performs automatic proxying to close the underlying file at
// destruction.
//
// The TaskRunner is in charge of any sequencing of the operations, but a single
// operation can be proxied at a time, regardless of the use of a callback.
// In other words, having a sequence like
//
//   proxy.Write(...);
//   proxy.Write(...);
//
// means the second Write will always fail.
class BASE_EXPORT FileProxy : public SupportsWeakPtr<FileProxy> {
 public:



  using StatusCallback = OnceCallback<void(File::Error)>;
  using CreateTemporaryCallback =
      OnceCallback<void(File::Error, const FilePath&)>;
  using GetFileInfoCallback =
      OnceCallback<void(File::Error, const File::Info&)>;
  using ReadCallback =
      OnceCallback<void(File::Error, const char* data, int bytes_read)>;
  using WriteCallback = OnceCallback<void(File::Error, int bytes_written)>;

  FileProxy();
  explicit FileProxy(TaskRunner* task_runner);
  ~FileProxy();






  bool CreateOrOpen(const FilePath& file_path,
                    uint32_t file_flags,
                    StatusCallback callback);








  bool CreateTemporary(uint32_t additional_file_flags,
                       CreateTemporaryCallback callback);

  bool IsValid() const;


  bool created() const { return file_.created(); }


  void SetFile(File file);

  File TakeFile();


  File DuplicateFile();

  PlatformFile GetPlatformFile() const;


  bool Close(StatusCallback callback);


  bool GetInfo(GetFileInfoCallback callback);



  bool Read(int64_t offset, int bytes_to_read, ReadCallback callback);



  bool Write(int64_t offset,
             const char* buffer,
             int bytes_to_write,
             WriteCallback callback);


  bool SetTimes(Time last_access_time,
                Time last_modified_time,
                StatusCallback callback);


  bool SetLength(int64_t length, StatusCallback callback);


  bool Flush(StatusCallback callback);

 private:
  friend class FileHelper;
  TaskRunner* task_runner() { return task_runner_.get(); }

  scoped_refptr<TaskRunner> task_runner_;
  File file_;
  DISALLOW_COPY_AND_ASSIGN(FileProxy);
};

}  // namespace base

#endif  // BASE_FILES_FILE_PROXY_H_
