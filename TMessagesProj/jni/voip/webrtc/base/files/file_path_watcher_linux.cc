// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/files/file_path_watcher.h"

#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

#include "base/bind.h"
#include "base/files/file_enumerator.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/lazy_instance.h"
#include "base/location.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/memory/ptr_util.h"
#include "base/memory/weak_ptr.h"
#include "base/posix/eintr_wrapper.h"
#include "base/single_thread_task_runner.h"
#include "base/stl_util.h"
#include "base/synchronization/lock.h"
#include "base/threading/platform_thread.h"
#include "base/threading/scoped_blocking_call.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "base/trace_event/trace_event.h"

namespace base {

namespace {

constexpr char kInotifyMaxUserWatchesPath[] =
    "/proc/sys/fs/inotify/max_user_watches";

// FilePathWatchers for a user, than they might affect each other's inotify
// watchers limit.
constexpr int kExpectedFilePathWatchers = 16;

// /proc/sys/fs/inotify/max_user_watches fails.
constexpr int kDefaultInotifyMaxUserWatches = 8192;

class FilePathWatcherImpl;
class InotifyReader;

// instance. This is based on /proc/sys/fs/inotify/max_user_watches entry.
int GetMaxNumberOfInotifyWatches() {
  const static int max = []() {
    int max_number_of_inotify_watches = 0;

    std::ifstream in(kInotifyMaxUserWatchesPath);
    if (!in.is_open() || !(in >> max_number_of_inotify_watches)) {
      LOG(ERROR) << "Failed to read " << kInotifyMaxUserWatchesPath;
      return kDefaultInotifyMaxUserWatches / kExpectedFilePathWatchers;
    }

    return max_number_of_inotify_watches / kExpectedFilePathWatchers;
  }();
  return max;
}

class InotifyReaderThreadDelegate final : public PlatformThread::Delegate {
 public:
  explicit InotifyReaderThreadDelegate(int inotify_fd)
      : inotify_fd_(inotify_fd) {}
  ~InotifyReaderThreadDelegate() override = default;

 private:
  void ThreadMain() override;

  const int inotify_fd_;

  DISALLOW_COPY_AND_ASSIGN(InotifyReaderThreadDelegate);
};

// TODO(tony): It would be nice if this wasn't a singleton.
// http://crbug.com/38174
class InotifyReader {
 public:
  using Watch = int;  // Watch descriptor used by AddWatch() and RemoveWatch().
  static constexpr Watch kInvalidWatch = -1;
  static constexpr Watch kWatchLimitExceeded = -2;


  Watch AddWatch(const FilePath& path, FilePathWatcherImpl* watcher);

  void RemoveWatch(Watch watch, FilePathWatcherImpl* watcher);

  void OnInotifyEvent(const inotify_event* event);

 private:
  friend struct LazyInstanceTraitsBase<InotifyReader>;

  InotifyReader();




  bool StartThread();

  Lock lock_;

  std::unordered_map<Watch, std::set<FilePathWatcherImpl*>> watchers_;

  const int inotify_fd_;

  InotifyReaderThreadDelegate thread_delegate_;

  bool valid_ = false;

  DISALLOW_COPY_AND_ASSIGN(InotifyReader);
};

class FilePathWatcherImpl : public FilePathWatcher::PlatformDelegate {
 public:
  FilePathWatcherImpl();
  ~FilePathWatcherImpl() override;







  void OnFilePathChanged(InotifyReader::Watch fired_watch,
                         const FilePath::StringType& child,
                         bool created,
                         bool deleted,
                         bool is_dir);


  bool IncreaseWatch();


  void DecreaseWatch();

 private:
  void OnFilePathChangedOnOriginSequence(InotifyReader::Watch fired_watch,
                                         const FilePath::StringType& child,
                                         bool created,
                                         bool deleted,
                                         bool is_dir);


  bool Watch(const FilePath& path,
             bool recursive,
             const FilePathWatcher::Callback& callback) override;

  void Cancel() override;







  struct WatchEntry {
    explicit WatchEntry(const FilePath::StringType& dirname)
        : watch(InotifyReader::kInvalidWatch),
          subdir(dirname) {}

    InotifyReader::Watch watch;
    FilePath::StringType subdir;
    FilePath::StringType linkname;
  };


  void UpdateWatches();







  void UpdateRecursiveWatches(InotifyReader::Watch fired_watch, bool is_dir);

  void UpdateRecursiveWatchesForPath(const FilePath& path);


  void TrackWatchForRecursion(InotifyReader::Watch watch, const FilePath& path);

  void RemoveRecursiveWatches();


  void AddWatchForBrokenSymlink(const FilePath& path, WatchEntry* watch_entry);

  bool HasValidWatchVector() const;

  FilePathWatcher::Callback callback_;

  FilePath target_;

  bool recursive_ = false;



  std::vector<WatchEntry> watches_;

  int number_of_inotify_watches_ = 0;

  std::unordered_map<InotifyReader::Watch, FilePath> recursive_paths_by_watch_;
  std::map<FilePath, InotifyReader::Watch> recursive_watches_by_path_;




  WeakPtr<FilePathWatcherImpl> weak_ptr_;

  WeakPtrFactory<FilePathWatcherImpl> weak_factory_{this};

  DISALLOW_COPY_AND_ASSIGN(FilePathWatcherImpl);
};

LazyInstance<InotifyReader>::Leaky g_inotify_reader = LAZY_INSTANCE_INITIALIZER;

void InotifyReaderThreadDelegate::ThreadMain() {
  PlatformThread::SetName("inotify_reader");

  CHECK_LE(0, inotify_fd_);
  CHECK_GT(FD_SETSIZE, inotify_fd_);

  while (true) {
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(inotify_fd_, &rfds);

    int select_result =
        HANDLE_EINTR(select(inotify_fd_ + 1, &rfds, nullptr, nullptr, nullptr));
    if (select_result < 0) {
      DPLOG(WARNING) << "select failed";
      return;
    }

    int buffer_size;
    int ioctl_result = HANDLE_EINTR(ioctl(inotify_fd_, FIONREAD, &buffer_size));

    if (ioctl_result != 0) {
      DPLOG(WARNING) << "ioctl failed";
      return;
    }

    std::vector<char> buffer(buffer_size);

    ssize_t bytes_read =
        HANDLE_EINTR(read(inotify_fd_, &buffer[0], buffer_size));

    if (bytes_read < 0) {
      DPLOG(WARNING) << "read from inotify fd failed";
      return;
    }

    ssize_t i = 0;
    while (i < bytes_read) {
      inotify_event* event = reinterpret_cast<inotify_event*>(&buffer[i]);
      size_t event_size = sizeof(inotify_event) + event->len;
      DCHECK(i + event_size <= static_cast<size_t>(bytes_read));
      g_inotify_reader.Get().OnInotifyEvent(event);
      i += event_size;
    }
  }
}

InotifyReader::InotifyReader()
    : inotify_fd_(inotify_init()), thread_delegate_(inotify_fd_) {
  if (inotify_fd_ < 0) {
    PLOG(ERROR) << "inotify_init() failed";
    return;
  }

  if (!StartThread())
    return;

  valid_ = true;
}

bool InotifyReader::StartThread() {


  return PlatformThread::CreateNonJoinable(0, &thread_delegate_);
}

InotifyReader::Watch InotifyReader::AddWatch(
    const FilePath& path, FilePathWatcherImpl* watcher) {
  if (!valid_)
    return kInvalidWatch;

  AutoLock auto_lock(lock_);

  ScopedBlockingCall scoped_blocking_call(FROM_HERE, BlockingType::WILL_BLOCK);

  if (!watcher->IncreaseWatch())
    return kWatchLimitExceeded;
  Watch watch = inotify_add_watch(inotify_fd_, path.value().c_str(),
                                  IN_ATTRIB | IN_CREATE | IN_DELETE |
                                  IN_CLOSE_WRITE | IN_MOVE |
                                  IN_ONLYDIR);

  if (watch == kInvalidWatch) {

    watcher->DecreaseWatch();
    return kInvalidWatch;
  }

  watchers_[watch].insert(watcher);

  return watch;
}

void InotifyReader::RemoveWatch(Watch watch, FilePathWatcherImpl* watcher) {
  if (!valid_ || (watch == kInvalidWatch))
    return;

  AutoLock auto_lock(lock_);

  watchers_[watch].erase(watcher);
  watcher->DecreaseWatch();

  if (watchers_[watch].empty()) {
    watchers_.erase(watch);

    ScopedBlockingCall scoped_blocking_call(FROM_HERE,
                                            BlockingType::WILL_BLOCK);
    inotify_rm_watch(inotify_fd_, watch);
  }
}

void InotifyReader::OnInotifyEvent(const inotify_event* event) {
  if (event->mask & IN_IGNORED)
    return;

  FilePath::StringType child(event->len ? event->name : FILE_PATH_LITERAL(""));
  AutoLock auto_lock(lock_);

  auto& watcher_set = watchers_[event->wd];
  for (FilePathWatcherImpl* watcher : watcher_set) {
    watcher->OnFilePathChanged(
        event->wd, child, event->mask & (IN_CREATE | IN_MOVED_TO),
        event->mask & (IN_DELETE | IN_MOVED_FROM), event->mask & IN_ISDIR);
  }
}

FilePathWatcherImpl::FilePathWatcherImpl() {
  weak_ptr_ = weak_factory_.GetWeakPtr();
}

FilePathWatcherImpl::~FilePathWatcherImpl() {
  DCHECK(!task_runner() || task_runner()->RunsTasksInCurrentSequence());
}

void FilePathWatcherImpl::OnFilePathChanged(InotifyReader::Watch fired_watch,
                                            const FilePath::StringType& child,
                                            bool created,
                                            bool deleted,
                                            bool is_dir) {
  DCHECK(!task_runner()->RunsTasksInCurrentSequence());



  task_runner()->PostTask(
      FROM_HERE,
      BindOnce(&FilePathWatcherImpl::OnFilePathChangedOnOriginSequence,
               weak_ptr_, fired_watch, child, created, deleted, is_dir));
}

void FilePathWatcherImpl::OnFilePathChangedOnOriginSequence(
    InotifyReader::Watch fired_watch,
    const FilePath::StringType& child,
    bool created,
    bool deleted,
    bool is_dir) {
  DCHECK(task_runner()->RunsTasksInCurrentSequence());
  DCHECK(!watches_.empty());
  DCHECK(HasValidWatchVector());

  bool did_update = false;

  for (size_t i = 0; i < watches_.size(); ++i) {
    const WatchEntry& watch_entry = watches_[i];
    if (fired_watch != watch_entry.watch)
      continue;

    bool change_on_target_path =
        child.empty() ||
        (child == watch_entry.linkname) ||
        (child == watch_entry.subdir);

    bool target_changed;
    if (watch_entry.subdir.empty()) {




      target_changed = (watch_entry.linkname.empty() ||
                        child == watch_entry.linkname);
    } else {





      bool next_watch_may_be_for_target = watches_[i + 1].subdir.empty();
      if (next_watch_may_be_for_target) {


        target_changed = watch_entry.subdir == child;
      } else {


        target_changed = false;
      }
    }





    if (change_on_target_path && (created || deleted) && !did_update) {
      UpdateWatches();
      did_update = true;
    }







    if (target_changed ||
        (change_on_target_path && deleted) ||
        (change_on_target_path && created && PathExists(target_))) {
      if (!did_update) {
        UpdateRecursiveWatches(fired_watch, is_dir);
        did_update = true;
      }
      callback_.Run(target_, false /* error */);
      return;
    }
  }

  if (Contains(recursive_paths_by_watch_, fired_watch)) {
    if (!did_update)
      UpdateRecursiveWatches(fired_watch, is_dir);
    callback_.Run(target_, false /* error */);
  }
}

bool FilePathWatcherImpl::IncreaseWatch() {
  if (number_of_inotify_watches_ >= GetMaxNumberOfInotifyWatches()) {


    callback_.Run(target_, true /* error */);
    return false;
  }
  ++number_of_inotify_watches_;
  return true;
}

void FilePathWatcherImpl::DecreaseWatch() {
  --number_of_inotify_watches_;

  DCHECK_GE(number_of_inotify_watches_, 0);
}

bool FilePathWatcherImpl::Watch(const FilePath& path,
                                bool recursive,
                                const FilePathWatcher::Callback& callback) {
  DCHECK(target_.empty());

  set_task_runner(SequencedTaskRunnerHandle::Get());
  callback_ = callback;
  target_ = path;
  recursive_ = recursive;

  std::vector<FilePath::StringType> comps;
  target_.GetComponents(&comps);
  DCHECK(!comps.empty());
  for (size_t i = 1; i < comps.size(); ++i)
    watches_.push_back(WatchEntry(comps[i]));
  watches_.push_back(WatchEntry(FilePath::StringType()));
  UpdateWatches();
  return true;
}

void FilePathWatcherImpl::Cancel() {
  if (!callback_) {

    set_cancelled();
    return;
  }

  DCHECK(task_runner()->RunsTasksInCurrentSequence());
  DCHECK(!is_cancelled());

  set_cancelled();
  callback_.Reset();

  for (const auto& watch : watches_)
    g_inotify_reader.Get().RemoveWatch(watch.watch, this);
  watches_.clear();
  target_.clear();
  RemoveRecursiveWatches();
}

void FilePathWatcherImpl::UpdateWatches() {


  DCHECK(task_runner()->RunsTasksInCurrentSequence());
  DCHECK(HasValidWatchVector());

  FilePath path(FILE_PATH_LITERAL("/"));
  for (WatchEntry& watch_entry : watches_) {
    InotifyReader::Watch old_watch = watch_entry.watch;
    watch_entry.watch = InotifyReader::kInvalidWatch;
    watch_entry.linkname.clear();
    watch_entry.watch = g_inotify_reader.Get().AddWatch(path, this);
    if (watch_entry.watch == InotifyReader::kWatchLimitExceeded)
      break;
    if (watch_entry.watch == InotifyReader::kInvalidWatch) {




      if (IsLink(path))
        AddWatchForBrokenSymlink(path, &watch_entry);
    }
    if (old_watch != watch_entry.watch)
      g_inotify_reader.Get().RemoveWatch(old_watch, this);
    path = path.Append(watch_entry.subdir);
  }

  UpdateRecursiveWatches(InotifyReader::kInvalidWatch,
                         false /* is directory? */);
}

void FilePathWatcherImpl::UpdateRecursiveWatches(
    InotifyReader::Watch fired_watch,
    bool is_dir) {
  DCHECK(HasValidWatchVector());

  if (!recursive_)
    return;

  if (!DirectoryExists(target_)) {
    RemoveRecursiveWatches();
    return;
  }


  if (!Contains(recursive_paths_by_watch_, fired_watch) &&
      fired_watch != watches_.back().watch) {
    UpdateRecursiveWatchesForPath(target_);
    return;
  }

  if (!is_dir)
    return;

  const FilePath& changed_dir = Contains(recursive_paths_by_watch_, fired_watch)
                                    ? recursive_paths_by_watch_[fired_watch]
                                    : target_;

  auto start_it = recursive_watches_by_path_.lower_bound(changed_dir);
  auto end_it = start_it;
  for (; end_it != recursive_watches_by_path_.end(); ++end_it) {
    const FilePath& cur_path = end_it->first;
    if (!changed_dir.IsParent(cur_path))
      break;
    if (!DirectoryExists(cur_path))
      g_inotify_reader.Get().RemoveWatch(end_it->second, this);

    recursive_paths_by_watch_.erase(end_it->second);
  }
  recursive_watches_by_path_.erase(start_it, end_it);
  UpdateRecursiveWatchesForPath(changed_dir);
}

void FilePathWatcherImpl::UpdateRecursiveWatchesForPath(const FilePath& path) {
  DCHECK(recursive_);
  DCHECK(!path.empty());
  DCHECK(DirectoryExists(path));



  FileEnumerator enumerator(
      path,
      true /* recursive enumeration */,
      FileEnumerator::DIRECTORIES | FileEnumerator::SHOW_SYM_LINKS);
  for (FilePath current = enumerator.Next();
       !current.empty();
       current = enumerator.Next()) {
    DCHECK(enumerator.GetInfo().IsDirectory());

    if (!Contains(recursive_watches_by_path_, current)) {

      InotifyReader::Watch watch =
          g_inotify_reader.Get().AddWatch(current, this);
      if (watch == InotifyReader::kWatchLimitExceeded)
        break;
      TrackWatchForRecursion(watch, current);
    } else {

      InotifyReader::Watch old_watch = recursive_watches_by_path_[current];
      DCHECK_NE(InotifyReader::kInvalidWatch, old_watch);
      InotifyReader::Watch watch =
          g_inotify_reader.Get().AddWatch(current, this);
      if (watch == InotifyReader::kWatchLimitExceeded)
        break;
      if (watch != old_watch) {
        g_inotify_reader.Get().RemoveWatch(old_watch, this);
        recursive_paths_by_watch_.erase(old_watch);
        recursive_watches_by_path_.erase(current);
        TrackWatchForRecursion(watch, current);
      }
    }
  }
}

void FilePathWatcherImpl::TrackWatchForRecursion(InotifyReader::Watch watch,
                                                 const FilePath& path) {
  DCHECK(recursive_);
  DCHECK(!path.empty());
  DCHECK(target_.IsParent(path));

  if (watch == InotifyReader::kInvalidWatch)
    return;

  DCHECK(!Contains(recursive_paths_by_watch_, watch));
  DCHECK(!Contains(recursive_watches_by_path_, path));
  recursive_paths_by_watch_[watch] = path;
  recursive_watches_by_path_[path] = watch;
}

void FilePathWatcherImpl::RemoveRecursiveWatches() {
  if (!recursive_)
    return;

  for (const auto& it : recursive_paths_by_watch_)
    g_inotify_reader.Get().RemoveWatch(it.first, this);

  recursive_paths_by_watch_.clear();
  recursive_watches_by_path_.clear();
}

void FilePathWatcherImpl::AddWatchForBrokenSymlink(const FilePath& path,
                                                   WatchEntry* watch_entry) {
  DCHECK_EQ(InotifyReader::kInvalidWatch, watch_entry->watch);
  FilePath link;
  if (!ReadSymbolicLink(path, &link))
    return;

  if (!link.IsAbsolute())
    link = path.DirName().Append(link);




  InotifyReader::Watch watch =
      g_inotify_reader.Get().AddWatch(link.DirName(), this);
  if (watch == InotifyReader::kInvalidWatch) {



    DPLOG(WARNING) << "Watch failed for "  << link.DirName().value();
    return;
  }
  watch_entry->watch = watch;
  watch_entry->linkname = link.BaseName().value();
}

bool FilePathWatcherImpl::HasValidWatchVector() const {
  if (watches_.empty())
    return false;
  for (size_t i = 0; i < watches_.size() - 1; ++i) {
    if (watches_[i].subdir.empty())
      return false;
  }
  return watches_.back().subdir.empty();
}

}  // namespace

FilePathWatcher::FilePathWatcher() {
  sequence_checker_.DetachFromSequence();
  impl_ = std::make_unique<FilePathWatcherImpl>();
}

}  // namespace base
