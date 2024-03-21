// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/files/file_util.h"

#include <windows.h>
#include <io.h>
#include <psapi.h>
#include <shellapi.h>
#include <shlobj.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <winsock2.h>

#include <algorithm>
#include <limits>
#include <string>

#include "base/debug/alias.h"
#include "base/files/file_enumerator.h"
#include "base/files/file_path.h"
#include "base/files/memory_mapped_file.h"
#include "base/guid.h"
#include "base/logging.h"
#include "base/metrics/histogram_functions.h"
#include "base/numerics/safe_conversions.h"
#include "base/process/process_handle.h"
#include "base/rand_util.h"
#include "base/stl_util.h"
#include "base/strings/strcat.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_piece.h"
#include "base/strings/string_util.h"
#include "base/strings/string_util_win.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/scoped_blocking_call.h"
#include "base/threading/scoped_thread_priority.h"
#include "base/time/time.h"
#include "base/win/scoped_handle.h"
#include "base/win/windows_types.h"
#include "base/win/windows_version.h"

namespace base {

namespace {

const DWORD kF_All =
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

// "Windows.PostOperationState.|operation|" indicating the state of |path|
// following the named operation. If |operation_succeeded| is true, the
// "operation succeeded" sample is recorded. Otherwise, the state of |path| is
// queried and the most meaningful sample is recorded.
void RecordPostOperationState(const FilePath& path,
                              StringPiece operation,
                              bool operation_succeeded) {



  enum class PostOperationState {
    kOperationSucceeded = 0,
    kFileNotFoundAfterFailure = 1,
    kPathNotFoundAfterFailure = 2,
    kAccessDeniedAfterFailure = 3,
    kNoAttributesAfterFailure = 4,
    kEmptyDirectoryAfterFailure = 5,
    kNonEmptyDirectoryAfterFailure = 6,
    kNotDirectoryAfterFailure = 7,
    kCount
  } metric = PostOperationState::kOperationSucceeded;

  if (!operation_succeeded) {
    const DWORD attributes = ::GetFileAttributes(path.value().c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES) {



      const DWORD error_code = ::GetLastError();
      if (error_code == ERROR_FILE_NOT_FOUND)
        metric = PostOperationState::kFileNotFoundAfterFailure;
      else if (error_code == ERROR_PATH_NOT_FOUND)
        metric = PostOperationState::kPathNotFoundAfterFailure;
      else if (error_code == ERROR_ACCESS_DENIED)
        metric = PostOperationState::kAccessDeniedAfterFailure;
      else
        metric = PostOperationState::kNoAttributesAfterFailure;
    } else if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
      if (IsDirectoryEmpty(path))
        metric = PostOperationState::kEmptyDirectoryAfterFailure;
      else
        metric = PostOperationState::kNonEmptyDirectoryAfterFailure;
    } else {
      metric = PostOperationState::kNotDirectoryAfterFailure;
    }
  }

  std::string histogram_name =
      base::StrCat({"Windows.PostOperationState.", operation});
  UmaHistogramEnumeration(histogram_name, metric, PostOperationState::kCount);
}

// "Windows.FilesystemError.|operation|".
void RecordFilesystemError(StringPiece operation, DWORD error) {
  std::string histogram_name =
      base::StrCat({"Windows.FilesystemError.", operation});
  UmaHistogramSparse(histogram_name, error);
}

// ERROR_FILE_NOT_FOUND or ERROR_PATH_NOT_FOUND. This is useful in cases where
// the absence of a file or path is a success condition (e.g., when attempting
// to delete an item in the filesystem).
DWORD ReturnLastErrorOrSuccessOnNotFound() {
  const DWORD error_code = ::GetLastError();
  return (error_code == ERROR_FILE_NOT_FOUND ||
          error_code == ERROR_PATH_NOT_FOUND)
             ? ERROR_SUCCESS
             : error_code;
}

// Returns ERROR_SUCCESS on success or the Windows error code corresponding to
// the first error encountered. ERROR_FILE_NOT_FOUND and ERROR_PATH_NOT_FOUND
// are considered success conditions, and are therefore never returned.
DWORD DeleteFileRecursive(const FilePath& path,
                          const FilePath::StringType& pattern,
                          bool recursive) {
  FileEnumerator traversal(path, false,
                           FileEnumerator::FILES | FileEnumerator::DIRECTORIES,
                           pattern);
  DWORD result = ERROR_SUCCESS;
  for (FilePath current = traversal.Next(); !current.empty();
       current = traversal.Next()) {

    FileEnumerator::FileInfo info = traversal.GetInfo();
    if ((info.find_data().dwFileAttributes & FILE_ATTRIBUTE_READONLY) &&
        (recursive || !info.IsDirectory())) {
      ::SetFileAttributes(
          current.value().c_str(),
          info.find_data().dwFileAttributes & ~FILE_ATTRIBUTE_READONLY);
    }

    DWORD this_result = ERROR_SUCCESS;
    if (info.IsDirectory()) {
      if (recursive) {
        this_result = DeleteFileRecursive(current, pattern, true);
        DCHECK_NE(static_cast<LONG>(this_result), ERROR_FILE_NOT_FOUND);
        DCHECK_NE(static_cast<LONG>(this_result), ERROR_PATH_NOT_FOUND);
        if (this_result == ERROR_SUCCESS &&
            !::RemoveDirectory(current.value().c_str())) {
          this_result = ReturnLastErrorOrSuccessOnNotFound();
        }
      }
    } else if (!::DeleteFile(current.value().c_str())) {
      this_result = ReturnLastErrorOrSuccessOnNotFound();
    }
    if (result == ERROR_SUCCESS)
      result = this_result;
  }
  return result;
}

// https://msdn.microsoft.com/library/yeby3zcb.aspx for details.
void AppendModeCharacter(wchar_t mode_char, std::wstring* mode) {
  size_t comma_pos = mode->find(L',');
  mode->insert(comma_pos == std::wstring::npos ? mode->length() : comma_pos, 1,
               mode_char);
}

bool DoCopyFile(const FilePath& from_path,
                const FilePath& to_path,
                bool fail_if_exists) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  if (from_path.ReferencesParent() || to_path.ReferencesParent())
    return false;


  if (from_path.value().length() >= MAX_PATH ||
      to_path.value().length() >= MAX_PATH) {
    return false;
  }


  SCOPED_MAY_LOAD_LIBRARY_AT_BACKGROUND_PRIORITY();




  const wchar_t* dest = to_path.value().c_str();
  if (!::CopyFile(from_path.value().c_str(), dest, fail_if_exists)) {

    return false;
  }
  DWORD attrs = GetFileAttributes(dest);
  if (attrs == INVALID_FILE_ATTRIBUTES) {
    return false;
  }
  if (attrs & FILE_ATTRIBUTE_READONLY) {
    SetFileAttributes(dest, attrs & ~FILE_ATTRIBUTE_READONLY);
  }
  return true;
}

bool DoCopyDirectory(const FilePath& from_path,
                     const FilePath& to_path,
                     bool recursive,
                     bool fail_if_exists) {





  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);


  if (from_path.value().length() >= MAX_PATH ||
      to_path.value().length() >= MAX_PATH) {
    return false;
  }

  FilePath real_to_path = to_path;
  if (PathExists(real_to_path)) {
    real_to_path = MakeAbsoluteFilePath(real_to_path);
    if (real_to_path.empty())
      return false;
  } else {
    real_to_path = MakeAbsoluteFilePath(real_to_path.DirName());
    if (real_to_path.empty())
      return false;
  }
  FilePath real_from_path = MakeAbsoluteFilePath(from_path);
  if (real_from_path.empty())
    return false;
  if (real_to_path == real_from_path || real_from_path.IsParent(real_to_path))
    return false;

  int traverse_type = FileEnumerator::FILES;
  if (recursive)
    traverse_type |= FileEnumerator::DIRECTORIES;
  FileEnumerator traversal(from_path, recursive, traverse_type);

  if (!PathExists(from_path)) {
    DLOG(ERROR) << "CopyDirectory() couldn't stat source directory: "
                << from_path.value().c_str();
    return false;
  }

  DCHECK(recursive || DirectoryExists(from_path));

  FilePath current = from_path;
  bool from_is_dir = DirectoryExists(from_path);
  bool success = true;
  FilePath from_path_base = from_path;
  if (recursive && DirectoryExists(to_path)) {


    from_path_base = from_path.DirName();
  }

  while (success && !current.empty()) {


    FilePath target_path(to_path);
    if (from_path_base != current) {
      if (!from_path_base.AppendRelativePath(current, &target_path)) {
        success = false;
        break;
      }
    }

    if (from_is_dir) {
      if (!DirectoryExists(target_path) &&
          !::CreateDirectory(target_path.value().c_str(), NULL)) {
        DLOG(ERROR) << "CopyDirectory() couldn't create directory: "
                    << target_path.value().c_str();
        success = false;
      }
    } else if (!DoCopyFile(current, target_path, fail_if_exists)) {
      DLOG(ERROR) << "CopyDirectory() couldn't create file: "
                  << target_path.value().c_str();
      success = false;
    }

    current = traversal.Next();
    if (!current.empty())
      from_is_dir = traversal.GetInfo().IsDirectory();
  }

  return success;
}

DWORD DoDeleteFile(const FilePath& path, bool recursive) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  if (path.empty())
    return ERROR_SUCCESS;

  if (path.value().length() >= MAX_PATH)
    return ERROR_BAD_PATHNAME;

  if (path.BaseName().value().find_first_of(FILE_PATH_LITERAL("*?")) !=
      FilePath::StringType::npos) {
    const DWORD error_code =
        DeleteFileRecursive(path.DirName(), path.BaseName().value(), recursive);
    DCHECK_NE(static_cast<LONG>(error_code), ERROR_FILE_NOT_FOUND);
    DCHECK_NE(static_cast<LONG>(error_code), ERROR_PATH_NOT_FOUND);
    return error_code;
  }

  const DWORD attr = ::GetFileAttributes(path.value().c_str());
  if (attr == INVALID_FILE_ATTRIBUTES)
    return ReturnLastErrorOrSuccessOnNotFound();

  if ((attr & FILE_ATTRIBUTE_READONLY) &&
      !::SetFileAttributes(path.value().c_str(),
                           attr & ~FILE_ATTRIBUTE_READONLY)) {

    return ReturnLastErrorOrSuccessOnNotFound();
  }

  if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
    return ::DeleteFile(path.value().c_str())
               ? ERROR_SUCCESS
               : ReturnLastErrorOrSuccessOnNotFound();
  }

  if (recursive) {
    const DWORD error_code =
        DeleteFileRecursive(path, FILE_PATH_LITERAL("*"), true);
    DCHECK_NE(static_cast<LONG>(error_code), ERROR_FILE_NOT_FOUND);
    DCHECK_NE(static_cast<LONG>(error_code), ERROR_PATH_NOT_FOUND);
    if (error_code != ERROR_SUCCESS)
      return error_code;
  }
  return ::RemoveDirectory(path.value().c_str())
             ? ERROR_SUCCESS
             : ReturnLastErrorOrSuccessOnNotFound();
}

bool DeleteFileAndRecordMetrics(const FilePath& path, bool recursive) {
  static constexpr char kRecursive[] = "DeleteFile.Recursive";
  static constexpr char kNonRecursive[] = "DeleteFile.NonRecursive";
  const StringPiece operation(recursive ? kRecursive : kNonRecursive);

  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);




  const DWORD error = DoDeleteFile(path, recursive);
  RecordPostOperationState(path, operation, error == ERROR_SUCCESS);
  if (error == ERROR_SUCCESS)
    return true;

  RecordFilesystemError(operation, error);
  return false;
}

}  // namespace

FilePath MakeAbsoluteFilePath(const FilePath& input) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  wchar_t file_path[MAX_PATH];
  if (!_wfullpath(file_path, input.value().c_str(), MAX_PATH))
    return FilePath();
  return FilePath(file_path);
}

bool DeleteFile(const FilePath& path, bool recursive) {
  return DeleteFileAndRecordMetrics(path, recursive);
}

bool DeleteFileRecursively(const FilePath& path) {
  return DeleteFileAndRecordMetrics(path, /*recursive=*/true);
}

bool DeleteFileAfterReboot(const FilePath& path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  if (path.value().length() >= MAX_PATH)
    return false;

  return ::MoveFileEx(path.value().c_str(), nullptr,
                      MOVEFILE_DELAY_UNTIL_REBOOT);
}

bool ReplaceFile(const FilePath& from_path,
                 const FilePath& to_path,
                 File::Error* error) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);


  if (::MoveFile(from_path.value().c_str(), to_path.value().c_str()))
    return true;
  File::Error move_error = File::OSErrorToFileError(GetLastError());

  FilePath::CharType from_path_str[MAX_PATH];
  base::wcslcpy(from_path_str, from_path.value().c_str(),
                base::size(from_path_str));
  base::debug::Alias(from_path_str);
  FilePath::CharType to_path_str[MAX_PATH];
  base::wcslcpy(to_path_str, to_path.value().c_str(), base::size(to_path_str));
  base::debug::Alias(to_path_str);




  if (::ReplaceFile(to_path.value().c_str(), from_path.value().c_str(), NULL,
                    REPLACEFILE_IGNORE_MERGE_ERRORS, NULL, NULL)) {
    return true;
  }



  if (error) {
    File::Error replace_error = File::OSErrorToFileError(GetLastError());
    *error = replace_error == File::FILE_ERROR_NOT_FOUND ? move_error
                                                         : replace_error;
  }
  return false;
}

bool CopyDirectory(const FilePath& from_path,
                   const FilePath& to_path,
                   bool recursive) {
  return DoCopyDirectory(from_path, to_path, recursive, false);
}

bool CopyDirectoryExcl(const FilePath& from_path,
                       const FilePath& to_path,
                       bool recursive) {
  return DoCopyDirectory(from_path, to_path, recursive, true);
}

bool PathExists(const FilePath& path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  return (GetFileAttributes(path.value().c_str()) != INVALID_FILE_ATTRIBUTES);
}

bool PathIsWritable(const FilePath& path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  HANDLE dir =
      CreateFile(path.value().c_str(), FILE_ADD_FILE, kF_All, NULL,
                 OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);

  if (dir == INVALID_HANDLE_VALUE)
    return false;

  CloseHandle(dir);
  return true;
}

bool DirectoryExists(const FilePath& path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  DWORD fileattr = GetFileAttributes(path.value().c_str());
  if (fileattr != INVALID_FILE_ATTRIBUTES)
    return (fileattr & FILE_ATTRIBUTE_DIRECTORY) != 0;
  return false;
}

bool GetTempDir(FilePath* path) {
  wchar_t temp_path[MAX_PATH + 1];
  DWORD path_len = ::GetTempPath(MAX_PATH, temp_path);
  if (path_len >= MAX_PATH || path_len <= 0)
    return false;



  *path = FilePath(temp_path).StripTrailingSeparators();
  return true;
}

FilePath GetHomeDir() {
  wchar_t result[MAX_PATH];
  if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT,
                                result)) &&
      result[0]) {
    return FilePath(result);
  }

  FilePath temp;
  if (GetTempDir(&temp))
    return temp;

  return FilePath(FILE_PATH_LITERAL("C:\\"));
}

bool CreateTemporaryFile(FilePath* path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  FilePath temp_file;

  if (!GetTempDir(path))
    return false;

  if (CreateTemporaryFileInDir(*path, &temp_file)) {
    *path = temp_file;
    return true;
  }

  return false;
}

// atomically.
// TODO(jrg): is there equivalent call to use on Windows instead of
// going 2-step?
FILE* CreateAndOpenTemporaryFileInDir(const FilePath& dir, FilePath* path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  if (!CreateTemporaryFileInDir(dir, path)) {
    return NULL;
  }



  return OpenFile(*path, "wb+");
}

bool CreateTemporaryFileInDir(const FilePath& dir, FilePath* temp_file) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);







  FilePath temp_name;
  bool create_file_success = false;


  for (int i = 0; i < 100; ++i) {
    temp_name =
        dir.Append(UTF8ToWide(GenerateGUID()) + FILE_PATH_LITERAL(".tmp"));
    File file(temp_name,
              File::FLAG_CREATE | File::FLAG_READ | File::FLAG_WRITE);
    if (file.IsValid()) {
      file.Close();
      create_file_success = true;
      break;
    }
  }

  if (!create_file_success) {
    DPLOG(WARNING) << "Failed to get temporary file name in " << dir.value();
    return false;
  }

  wchar_t long_temp_name[MAX_PATH + 1];
  DWORD long_name_len =
      GetLongPathName(temp_name.value().c_str(), long_temp_name, MAX_PATH);
  if (long_name_len > MAX_PATH || long_name_len == 0) {

    *temp_file = std::move(temp_name);
    return true;
  }

  FilePath::StringPieceType long_temp_name_str(long_temp_name, long_name_len);
  *temp_file = FilePath(long_temp_name_str);
  return true;
}

bool CreateTemporaryDirInDir(const FilePath& base_dir,
                             const FilePath::StringType& prefix,
                             FilePath* new_dir) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  FilePath path_to_create;

  for (int count = 0; count < 50; ++count) {


    std::wstring new_dir_name;
    new_dir_name.assign(prefix);
    new_dir_name.append(AsWString(NumberToString16(GetCurrentProcId())));
    new_dir_name.push_back('_');
    new_dir_name.append(AsWString(
        NumberToString16(RandInt(0, std::numeric_limits<int32_t>::max()))));

    path_to_create = base_dir.Append(new_dir_name);
    if (::CreateDirectory(path_to_create.value().c_str(), NULL)) {
      *new_dir = path_to_create;
      return true;
    }
  }

  return false;
}

bool CreateNewTempDirectory(const FilePath::StringType& prefix,
                            FilePath* new_temp_path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  FilePath system_temp_dir;
  if (!GetTempDir(&system_temp_dir))
    return false;

  return CreateTemporaryDirInDir(system_temp_dir, prefix, new_temp_path);
}

bool CreateDirectoryAndGetError(const FilePath& full_path,
                                File::Error* error) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  const wchar_t* const full_path_str = full_path.value().c_str();
  const DWORD fileattr = ::GetFileAttributes(full_path_str);
  if (fileattr != INVALID_FILE_ATTRIBUTES) {
    if ((fileattr & FILE_ATTRIBUTE_DIRECTORY) != 0) {
      DVLOG(1) << "CreateDirectory(" << full_path_str << "), "
               << "directory already exists.";
      return true;
    }
    DLOG(WARNING) << "CreateDirectory(" << full_path_str << "), "
                  << "conflicts with existing file.";
    if (error)
      *error = File::FILE_ERROR_NOT_A_DIRECTORY;
    ::SetLastError(ERROR_FILE_EXISTS);
    return false;
  }




  FilePath parent_path(full_path.DirName());
  if (parent_path.value() == full_path.value()) {
    if (error)
      *error = File::FILE_ERROR_NOT_FOUND;
    ::SetLastError(ERROR_FILE_NOT_FOUND);
    return false;
  }
  if (!CreateDirectoryAndGetError(parent_path, error)) {
    DLOG(WARNING) << "Failed to create one of the parent directories.";
    DCHECK(!error || *error != File::FILE_OK);
    return false;
  }

  if (::CreateDirectory(full_path_str, NULL))
    return true;

  const DWORD error_code = ::GetLastError();
  if (error_code == ERROR_ALREADY_EXISTS && DirectoryExists(full_path)) {




    return true;
  }
  if (error)
    *error = File::OSErrorToFileError(error_code);
  ::SetLastError(error_code);
  DPLOG(WARNING) << "Failed to create directory " << full_path_str;
  return false;
}

bool NormalizeFilePath(const FilePath& path, FilePath* real_path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  File file(path, File::FLAG_OPEN | File::FLAG_READ | File::FLAG_SHARE_DELETE);
  if (!file.IsValid())
    return false;

  constexpr int kMaxPathLength = MAX_PATH + 10;
  wchar_t native_file_path[kMaxPathLength];



  DWORD used_wchars = ::GetFinalPathNameByHandle(
      file.GetPlatformFile(), native_file_path, kMaxPathLength - 1,
      FILE_NAME_NORMALIZED | VOLUME_NAME_NT);

  if (used_wchars >= kMaxPathLength || used_wchars == 0)
    return false;



  return DevicePathToDriveLetterPath(
      FilePath(FilePath::StringPieceType(native_file_path, used_wchars)),
      real_path);
}

bool DevicePathToDriveLetterPath(const FilePath& nt_device_path,
                                 FilePath* out_drive_letter_path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  const int kDriveMappingSize = 1024;
  wchar_t drive_mapping[kDriveMappingSize] = {'\0'};
  if (!::GetLogicalDriveStrings(kDriveMappingSize - 1, drive_mapping)) {
    DLOG(ERROR) << "Failed to get drive mapping.";
    return false;
  }


  wchar_t* drive_map_ptr = drive_mapping;
  wchar_t device_path_as_string[MAX_PATH];
  wchar_t drive[] = FILE_PATH_LITERAL(" :");



  while (*drive_map_ptr) {
    drive[0] = drive_map_ptr[0];  // Copy the drive letter.

    if (QueryDosDevice(drive, device_path_as_string, MAX_PATH)) {
      FilePath device_path(device_path_as_string);
      if (device_path == nt_device_path ||
          device_path.IsParent(nt_device_path)) {
        *out_drive_letter_path =
            FilePath(drive + nt_device_path.value().substr(
                                 wcslen(device_path_as_string)));
        return true;
      }
    }


    while (*drive_map_ptr++) {}
  }



  return false;
}

FilePath MakeLongFilePath(const FilePath& input) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  DWORD path_long_len = ::GetLongPathName(input.value().c_str(), nullptr, 0);
  if (path_long_len == 0UL)
    return FilePath();

  std::wstring path_long_str;
  path_long_len = ::GetLongPathName(input.value().c_str(),
                                    WriteInto(&path_long_str, path_long_len),
                                    path_long_len);
  if (path_long_len == 0UL)
    return FilePath();

  return FilePath(path_long_str);
}

bool CreateWinHardLink(const FilePath& to_file, const FilePath& from_file) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  return ::CreateHardLink(to_file.value().c_str(), from_file.value().c_str(),
                          nullptr);
}

// them if we do decide to.
bool IsLink(const FilePath& file_path) {
  return false;
}

bool GetFileInfo(const FilePath& file_path, File::Info* results) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  WIN32_FILE_ATTRIBUTE_DATA attr;
  if (!GetFileAttributesEx(file_path.value().c_str(), GetFileExInfoStandard,
                           &attr)) {
    return false;
  }

  ULARGE_INTEGER size;
  size.HighPart = attr.nFileSizeHigh;
  size.LowPart = attr.nFileSizeLow;
  results->size = size.QuadPart;

  results->is_directory =
      (attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  results->last_modified = Time::FromFileTime(attr.ftLastWriteTime);
  results->last_accessed = Time::FromFileTime(attr.ftLastAccessTime);
  results->creation_time = Time::FromFileTime(attr.ftCreationTime);

  return true;
}

FILE* OpenFile(const FilePath& filename, const char* mode) {


  DCHECK(
      strchr(mode, 'N') == nullptr ||
      (strchr(mode, ',') != nullptr && strchr(mode, 'N') > strchr(mode, ',')));
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  std::wstring w_mode = UTF8ToWide(mode);
  AppendModeCharacter(L'N', &w_mode);
  return _wfsopen(filename.value().c_str(), w_mode.c_str(), _SH_DENYNO);
}

FILE* FileToFILE(File file, const char* mode) {
  if (!file.IsValid())
    return NULL;
  int fd =
      _open_osfhandle(reinterpret_cast<intptr_t>(file.GetPlatformFile()), 0);
  if (fd < 0)
    return NULL;
  file.TakePlatformFile();
  FILE* stream = _fdopen(fd, mode);
  if (!stream)
    _close(fd);
  return stream;
}

int ReadFile(const FilePath& filename, char* data, int max_size) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  win::ScopedHandle file(CreateFile(filename.value().c_str(), GENERIC_READ,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                    OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN,
                                    NULL));
  if (!file.IsValid())
    return -1;

  DWORD read;
  if (::ReadFile(file.Get(), data, max_size, &read, NULL))
    return read;

  return -1;
}

int WriteFile(const FilePath& filename, const char* data, int size) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  win::ScopedHandle file(CreateFile(filename.value().c_str(), GENERIC_WRITE, 0,
                                    NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
                                    NULL));
  if (!file.IsValid()) {
    DPLOG(WARNING) << "CreateFile failed for path " << filename.value();
    return -1;
  }

  DWORD written;
  BOOL result = ::WriteFile(file.Get(), data, size, &written, NULL);
  if (result && static_cast<int>(written) == size)
    return written;

  if (!result) {

    DPLOG(WARNING) << "writing file " << filename.value() << " failed";
  } else {

    DLOG(WARNING) << "wrote" << written << " bytes to " << filename.value()
                  << " expected " << size;
  }
  return -1;
}

bool AppendToFile(const FilePath& filename, const char* data, int size) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  win::ScopedHandle file(CreateFile(filename.value().c_str(), FILE_APPEND_DATA,
                                    0, NULL, OPEN_EXISTING, 0, NULL));
  if (!file.IsValid()) {
    VPLOG(1) << "CreateFile failed for path " << filename.value();
    return false;
  }

  DWORD written;
  BOOL result = ::WriteFile(file.Get(), data, size, &written, NULL);
  if (result && static_cast<int>(written) == size)
    return true;

  if (!result) {

    VPLOG(1) << "Writing file " << filename.value() << " failed";
  } else {

    VPLOG(1) << "Only wrote " << written << " out of " << size << " byte(s) to "
             << filename.value();
  }
  return false;
}

bool GetCurrentDirectory(FilePath* dir) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  wchar_t system_buffer[MAX_PATH];
  system_buffer[0] = 0;
  DWORD len = ::GetCurrentDirectory(MAX_PATH, system_buffer);
  if (len == 0 || len > MAX_PATH)
    return false;



  *dir = FilePath(FilePath::StringPieceType(system_buffer))
             .StripTrailingSeparators();
  return true;
}

bool SetCurrentDirectory(const FilePath& directory) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  return ::SetCurrentDirectory(directory.value().c_str()) != 0;
}

int GetMaximumPathComponentLength(const FilePath& path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);

  wchar_t volume_path[MAX_PATH];
  if (!GetVolumePathNameW(path.NormalizePathSeparators().value().c_str(),
                          volume_path, size(volume_path))) {
    return -1;
  }

  DWORD max_length = 0;
  if (!GetVolumeInformationW(volume_path, NULL, 0, NULL, &max_length, NULL,
                             NULL, 0)) {
    return -1;
  }

  size_t prefix = path.StripTrailingSeparators().value().size() + 1;


  int whole_path_limit = std::max(0, MAX_PATH - 1 - static_cast<int>(prefix));
  return std::min(whole_path_limit, static_cast<int>(max_length));
}

bool CopyFile(const FilePath& from_path, const FilePath& to_path) {
  return DoCopyFile(from_path, to_path, false);
}

bool SetNonBlocking(int fd) {
  unsigned long nonblocking = 1;
  if (ioctlsocket(fd, FIONBIO, &nonblocking) == 0)
    return true;
  return false;
}

namespace {

// supports Windows 7, so we need to check for the function's presence
// dynamically.
using PrefetchVirtualMemoryPtr = decltype(&::PrefetchVirtualMemory);

PrefetchVirtualMemoryPtr GetPrefetchVirtualMemoryPtr() {
  HMODULE kernel32_dll = ::GetModuleHandleA("kernel32.dll");
  return reinterpret_cast<decltype(&::PrefetchVirtualMemory)>(
      GetProcAddress(kernel32_dll, "PrefetchVirtualMemory"));
}

}  // namespace

bool PreReadFile(const FilePath& file_path,
                 bool is_executable,
                 int64_t max_bytes) {
  DCHECK_GE(max_bytes, 0);




  static PrefetchVirtualMemoryPtr prefetch_virtual_memory =
      GetPrefetchVirtualMemoryPtr();

  if (prefetch_virtual_memory == nullptr)
    return internal::PreReadFileSlow(file_path, max_bytes);

  if (max_bytes == 0) {


    return true;
  }

  MemoryMappedFile::Access access = is_executable
                                        ? MemoryMappedFile::READ_CODE_IMAGE
                                        : MemoryMappedFile::READ_ONLY;
  MemoryMappedFile mapped_file;
  if (!mapped_file.Initialize(file_path, access))
    return false;

  const ::SIZE_T length =
      std::min(base::saturated_cast<::SIZE_T>(max_bytes),
               base::saturated_cast<::SIZE_T>(mapped_file.length()));
  ::_WIN32_MEMORY_RANGE_ENTRY address_range = {mapped_file.data(), length};
  return (*prefetch_virtual_memory)(::GetCurrentProcess(),
                                    /*NumberOfEntries=*/1, &address_range,
                                    /*Flags=*/0);
}


namespace internal {

bool MoveUnsafe(const FilePath& from_path, const FilePath& to_path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);


  if (from_path.value().length() >= MAX_PATH ||
      to_path.value().length() >= MAX_PATH) {
    return false;
  }
  if (MoveFileEx(from_path.value().c_str(), to_path.value().c_str(),
                 MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING) != 0)
    return true;


  bool ret = false;
  DWORD last_error = ::GetLastError();

  if (DirectoryExists(from_path)) {



    ret = internal::CopyAndDeleteDirectory(from_path, to_path);
  }

  if (!ret) {


    ::SetLastError(last_error);
  }

  return ret;
}

bool CopyAndDeleteDirectory(const FilePath& from_path,
                            const FilePath& to_path) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  if (CopyDirectory(from_path, to_path, true)) {
    if (DeleteFileRecursively(from_path))
      return true;




  }
  return false;
}

}  // namespace internal
}  // namespace base
