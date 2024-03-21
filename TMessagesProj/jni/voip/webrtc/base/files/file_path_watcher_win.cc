// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/files/file_path_watcher.h"

#include "base/bind.h"
#include "base/files/file.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/strings/string_util.h"
#include "base/threading/scoped_blocking_call.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "base/time/time.h"
#include "base/win/object_watcher.h"

#include <windows.h>

namespace base {

namespace {

class FilePathWatcherImpl : public FilePathWatcher::PlatformDelegate,
                            public base::win::ObjectWatcher::Delegate {
 public:
  FilePathWatcherImpl()
      : handle_(INVALID_HANDLE_VALUE),
        recursive_watch_(false) {}
  ~FilePathWatcherImpl() override;

  bool Watch(const FilePath& path,
             bool recursive,
             const FilePathWatcher::Callback& callback) override;
  void Cancel() override;

  void OnObjectSignaled(HANDLE object) override;

 private:




  static bool SetupWatchHandle(const FilePath& dir,
                               bool recursive,
                               HANDLE* handle) WARN_UNUSED_RESULT;

  bool UpdateWatch() WARN_UNUSED_RESULT;

  void DestroyWatch();

  FilePathWatcher::Callback callback_;

  FilePath target_;

  bool* was_deleted_ptr_ = nullptr;

  HANDLE handle_;

  base::win::ObjectWatcher watcher_;

  bool recursive_watch_;


  Time last_modified_;


  Time first_notification_;

  DISALLOW_COPY_AND_ASSIGN(FilePathWatcherImpl);
};

FilePathWatcherImpl::~FilePathWatcherImpl() {
  DCHECK(!task_runner() || task_runner()->RunsTasksInCurrentSequence());
  if (was_deleted_ptr_)
    *was_deleted_ptr_ = true;
}

bool FilePathWatcherImpl::Watch(const FilePath& path,
                                bool recursive,
                                const FilePathWatcher::Callback& callback) {
  DCHECK(target_.value().empty());  // Can only watch one path.

  set_task_runner(SequencedTaskRunnerHandle::Get());
  callback_ = callback;
  target_ = path;
  recursive_watch_ = recursive;

  File::Info file_info;
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  if (GetFileInfo(target_, &file_info)) {
    last_modified_ = file_info.last_modified;
    first_notification_ = Time::Now();
  }

  if (!UpdateWatch())
    return false;

  watcher_.StartWatchingOnce(handle_, this);

  return true;
}

void FilePathWatcherImpl::Cancel() {
  if (callback_.is_null()) {

    set_cancelled();
    return;
  }

  DCHECK(task_runner()->RunsTasksInCurrentSequence());
  set_cancelled();

  if (handle_ != INVALID_HANDLE_VALUE)
    DestroyWatch();

  callback_.Reset();
}

void FilePathWatcherImpl::OnObjectSignaled(HANDLE object) {
  DCHECK(task_runner()->RunsTasksInCurrentSequence());
  DCHECK_EQ(object, handle_);
  DCHECK(!was_deleted_ptr_);

  bool was_deleted = false;
  was_deleted_ptr_ = &was_deleted;

  if (!UpdateWatch()) {
    callback_.Run(target_, true /* error */);
    return;
  }

  File::Info file_info;
  bool file_exists = false;
  {
    ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
    file_exists = GetFileInfo(target_, &file_info);
  }
  if (recursive_watch_) {






    callback_.Run(target_, false);
  } else if (file_exists && (last_modified_.is_null() ||
             last_modified_ != file_info.last_modified)) {
    last_modified_ = file_info.last_modified;
    first_notification_ = Time::Now();
    callback_.Run(target_, false);
  } else if (file_exists && last_modified_ == file_info.last_modified &&
             !first_notification_.is_null()) {














    if (Time::Now() - first_notification_ > TimeDelta::FromSeconds(1)) {

      first_notification_ = Time();
    }
    callback_.Run(target_, false);
  } else if (!file_exists && !last_modified_.is_null()) {
    last_modified_ = Time();
    callback_.Run(target_, false);
  }

  if (!was_deleted) {
    watcher_.StartWatchingOnce(handle_, this);
    was_deleted_ptr_ = nullptr;
  }
}

bool FilePathWatcherImpl::SetupWatchHandle(const FilePath& dir,
                                           bool recursive,
                                           HANDLE* handle) {
  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  *handle = FindFirstChangeNotification(
      dir.value().c_str(), recursive,
      FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_SIZE |
          FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_DIR_NAME |
          FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SECURITY);
  if (*handle != INVALID_HANDLE_VALUE) {



    if (!DirectoryExists(dir)) {
      FindCloseChangeNotification(*handle);
      *handle = INVALID_HANDLE_VALUE;
    }
    return true;
  }




  DWORD error_code = GetLastError();
  if (error_code != ERROR_FILE_NOT_FOUND &&
      error_code != ERROR_PATH_NOT_FOUND &&
      error_code != ERROR_ACCESS_DENIED &&
      error_code != ERROR_SHARING_VIOLATION &&
      error_code != ERROR_DIRECTORY) {
    DPLOG(ERROR) << "FindFirstChangeNotification failed for "
                 << dir.value();
    return false;
  }

  return true;
}

bool FilePathWatcherImpl::UpdateWatch() {
  if (handle_ != INVALID_HANDLE_VALUE)
    DestroyWatch();

  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);



  std::vector<FilePath> child_dirs;
  FilePath watched_path(target_);
  while (true) {
    if (!SetupWatchHandle(watched_path, recursive_watch_, &handle_))
      return false;

    if (handle_ != INVALID_HANDLE_VALUE)
      break;

    child_dirs.push_back(watched_path.BaseName());
    FilePath parent(watched_path.DirName());
    if (parent == watched_path) {
      DLOG(ERROR) << "Reached the root directory";
      return false;
    }
    watched_path = parent;
  }



  while (!child_dirs.empty()) {
    watched_path = watched_path.Append(child_dirs.back());
    child_dirs.pop_back();
    HANDLE temp_handle = INVALID_HANDLE_VALUE;
    if (!SetupWatchHandle(watched_path, recursive_watch_, &temp_handle))
      return false;
    if (temp_handle == INVALID_HANDLE_VALUE)
      break;
    FindCloseChangeNotification(handle_);
    handle_ = temp_handle;
  }

  return true;
}

void FilePathWatcherImpl::DestroyWatch() {
  watcher_.StopWatching();

  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::MAY_BLOCK);
  FindCloseChangeNotification(handle_);
  handle_ = INVALID_HANDLE_VALUE;
}

}  // namespace

FilePathWatcher::FilePathWatcher() {
  sequence_checker_.DetachFromSequence();
  impl_ = std::make_unique<FilePathWatcherImpl>();
}

}  // namespace base
