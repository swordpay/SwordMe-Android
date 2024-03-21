// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_FILES_FILE_H_
#define BASE_FILES_FILE_H_

#include <stdint.h>

#include <string>

#include "base/base_export.h"
#include "base/containers/span.h"
#include "base/files/file_path.h"
#include "base/files/file_tracing.h"
#include "base/files/platform_file.h"
#include "base/macros.h"
#include "base/time/time.h"
#include "build/build_config.h"

#if defined(OS_POSIX) || defined(OS_FUCHSIA)
#include <sys/stat.h>
#endif

namespace base {

#if defined(OS_BSD) || defined(OS_MACOSX) || defined(OS_NACL) || \
  defined(OS_FUCHSIA) || (defined(OS_ANDROID) && __ANDROID_API__ < 21)
typedef struct stat stat_wrapper_t;
#elif defined(OS_POSIX)
typedef struct stat64 stat_wrapper_t;
#endif

// Note that this class does not provide any support for asynchronous IO, other
// than the ability to create asynchronous handles on Windows.
//
// Note about const: this class does not attempt to determine if the underlying
// file system object is affected by a particular method in order to consider
// that method const or not. Only methods that deal with member variables in an
// obvious non-modifying way are marked as const. Any method that forward calls
// to the OS is not considered const, even if there is no apparent change to
// member variables.
class BASE_EXPORT File {
 public:







  enum Flags {
    FLAG_OPEN = 1 << 0,            // Opens a file, only if it exists.
    FLAG_CREATE = 1 << 1,          // Creates a new file, only if it does not

    FLAG_OPEN_ALWAYS = 1 << 2,     // May create a new file.
    FLAG_CREATE_ALWAYS = 1 << 3,   // May overwrite an old file.
    FLAG_OPEN_TRUNCATED = 1 << 4,  // Opens a file and truncates it, only if it

    FLAG_READ = 1 << 5,
    FLAG_WRITE = 1 << 6,
    FLAG_APPEND = 1 << 7,
    FLAG_EXCLUSIVE_READ = 1 << 8,  // EXCLUSIVE is opposite of Windows SHARE.
    FLAG_EXCLUSIVE_WRITE = 1 << 9,
    FLAG_ASYNC = 1 << 10,
    FLAG_TEMPORARY = 1 << 11,  // Used on Windows only.
    FLAG_HIDDEN = 1 << 12,     // Used on Windows only.
    FLAG_DELETE_ON_CLOSE = 1 << 13,
    FLAG_WRITE_ATTRIBUTES = 1 << 14,     // Used on Windows only.
    FLAG_SHARE_DELETE = 1 << 15,         // Used on Windows only.
    FLAG_TERMINAL_DEVICE = 1 << 16,      // Serial port flags.
    FLAG_BACKUP_SEMANTICS = 1 << 17,     // Used on Windows only.
    FLAG_EXECUTE = 1 << 18,              // Used on Windows only.
    FLAG_SEQUENTIAL_SCAN = 1 << 19,      // Used on Windows only.
    FLAG_CAN_DELETE_ON_CLOSE = 1 << 20,  // Requests permission to delete a file


  };







  enum Error {
    FILE_OK = 0,
    FILE_ERROR_FAILED = -1,
    FILE_ERROR_IN_USE = -2,
    FILE_ERROR_EXISTS = -3,
    FILE_ERROR_NOT_FOUND = -4,
    FILE_ERROR_ACCESS_DENIED = -5,
    FILE_ERROR_TOO_MANY_OPENED = -6,
    FILE_ERROR_NO_MEMORY = -7,
    FILE_ERROR_NO_SPACE = -8,
    FILE_ERROR_NOT_A_DIRECTORY = -9,
    FILE_ERROR_INVALID_OPERATION = -10,
    FILE_ERROR_SECURITY = -11,
    FILE_ERROR_ABORT = -12,
    FILE_ERROR_NOT_A_FILE = -13,
    FILE_ERROR_NOT_EMPTY = -14,
    FILE_ERROR_INVALID_URL = -15,
    FILE_ERROR_IO = -16,

    FILE_ERROR_MAX = -17
  };

  enum Whence {
    FROM_BEGIN   = 0,
    FROM_CURRENT = 1,
    FROM_END     = 2
  };





  struct BASE_EXPORT Info {
    Info();
    ~Info();
#if defined(OS_POSIX) || defined(OS_FUCHSIA)

    void FromStat(const stat_wrapper_t& stat_info);
#endif

    int64_t size = 0;

    bool is_directory = false;


    bool is_symbolic_link = false;

    Time last_modified;

    Time last_accessed;

    Time creation_time;
  };

  File();


  File(const FilePath& path, uint32_t flags);

  explicit File(ScopedPlatformFile platform_file);
  explicit File(PlatformFile platform_file);



  File(ScopedPlatformFile platform_file, bool async);
  File(PlatformFile platform_file, bool async);

  explicit File(Error error_details);

  File(File&& other);

  ~File();

  File& operator=(File&& other);

  void Initialize(const FilePath& path, uint32_t flags);



  bool IsValid() const;



  bool created() const { return created_; }





  Error error_details() const { return error_details_; }

  PlatformFile GetPlatformFile() const;
  PlatformFile TakePlatformFile();

  void Close();



  int64_t Seek(Whence whence, int64_t offset);



  bool ReadAndCheck(int64_t offset, span<uint8_t> data);
  bool ReadAtCurrentPosAndCheck(span<uint8_t> data);
  bool WriteAndCheck(int64_t offset, span<const uint8_t> data);
  bool WriteAtCurrentPosAndCheck(span<const uint8_t> data);






  int Read(int64_t offset, char* data, int size);

  int ReadAtCurrentPos(char* data, int size);



  int ReadNoBestEffort(int64_t offset, char* data, int size);

  int ReadAtCurrentPosNoBestEffort(char* data, int size);






  int Write(int64_t offset, const char* data, int size);

  int WriteAtCurrentPos(const char* data, int size);


  int WriteAtCurrentPosNoBestEffort(const char* data, int size);

  int64_t GetLength();



  bool SetLength(int64_t length);












  bool Flush();

  bool SetTimes(Time last_access_time, Time last_modified_time);

  bool GetInfo(Info* info);

#if !defined(OS_FUCHSIA)  // Fuchsia's POSIX API does not support file locking.
  enum class LockMode {
    kShared,
    kExclusive,
  };




















  Error Lock(LockMode mode = LockMode::kExclusive);

  Error Unlock();

#endif  // !defined(OS_FUCHSIA)





  File Duplicate() const;

  bool async() const { return async_; }

#if defined(OS_WIN)































  bool DeleteOnClose(bool delete_on_close);
#endif

#if defined(OS_WIN)
  static Error OSErrorToFileError(DWORD last_error);
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
  static Error OSErrorToFileError(int saved_errno);
#endif




  static Error GetLastFileError();

  static std::string ErrorToString(Error error);

#if defined(OS_POSIX) || defined(OS_FUCHSIA)

  static int Stat(const char* path, stat_wrapper_t* sb);
  static int Fstat(int fd, stat_wrapper_t* sb);
  static int Lstat(const char* path, stat_wrapper_t* sb);
#endif

 private:
  friend class FileTracing::ScopedTrace;


  void DoInitialize(const FilePath& path, uint32_t flags);

  void SetPlatformFile(PlatformFile file);

  ScopedPlatformFile file_;


  FilePath tracing_path_;

  FileTracing::ScopedEnabler trace_enabler_;

  Error error_details_ = FILE_ERROR_FAILED;
  bool created_ = false;
  bool async_ = false;

  DISALLOW_COPY_AND_ASSIGN(File);
};

}  // namespace base

#endif  // BASE_FILES_FILE_H_
