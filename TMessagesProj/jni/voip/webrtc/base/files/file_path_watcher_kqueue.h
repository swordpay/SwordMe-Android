// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_FILES_FILE_PATH_WATCHER_KQUEUE_H_
#define BASE_FILES_FILE_PATH_WATCHER_KQUEUE_H_

#include <sys/event.h>

#include <memory>
#include <vector>

#include "base/files/file_descriptor_watcher_posix.h"
#include "base/files/file_path.h"
#include "base/files/file_path_watcher.h"
#include "base/macros.h"

namespace base {

// The Linux and Windows versions are able to detect:
// - file creation/deletion/modification in a watched directory
// - file creation/deletion/modification for a watched file
// - modifications to the paths to a watched object that would affect the
//   object such as renaming/attibute changes etc.
// The kqueue implementation will handle all of the items in the list above
// except for detecting modifications to files in a watched directory. It will
// detect the creation and deletion of files, just not the modification of
// files. It does however detect the attribute changes that the FSEvents impl
// would miss.
class FilePathWatcherKQueue : public FilePathWatcher::PlatformDelegate {
 public:
  FilePathWatcherKQueue();
  ~FilePathWatcherKQueue() override;

  bool Watch(const FilePath& path,
             bool recursive,
             const FilePathWatcher::Callback& callback) override;
  void Cancel() override;

 private:
  class EventData {
   public:
    EventData(const FilePath& path, const FilePath::StringType& subdir)
        : path_(path), subdir_(subdir) { }
    FilePath path_;  // Full path to this item.
    FilePath::StringType subdir_;  // Path to any sub item.
  };

  typedef std::vector<struct kevent> EventVector;

  void OnKQueueReadable();

  bool AreKeventValuesValid(struct kevent* kevents, int count);



  void HandleAttributesChange(const EventVector::iterator& event,
                              bool* target_file_affected,
                              bool* update_watches);



  void HandleDeleteOrMoveChange(const EventVector::iterator& event,
                                bool* target_file_affected,
                                bool* update_watches);



  void HandleCreateItemChange(const EventVector::iterator& event,
                              bool* target_file_affected,
                              bool* update_watches);



  bool UpdateWatches(bool* target_file_affected);



  static int EventsForPath(FilePath path, EventVector *events);

  static void ReleaseEvent(struct kevent& event);


  static uintptr_t FileDescriptorForPath(const FilePath& path);

  static const uintptr_t kNoFileDescriptor = static_cast<uintptr_t>(-1);

  static void CloseFileDescriptor(uintptr_t* fd);

  static bool IsKeventFileDescriptorOpen(const struct kevent& event) {
    return event.ident != kNoFileDescriptor;
  }

  static EventData* EventDataForKevent(const struct kevent& event) {
    return reinterpret_cast<EventData*>(event.udata);
  }

  EventVector events_;
  FilePathWatcher::Callback callback_;
  FilePath target_;
  int kqueue_;


  std::unique_ptr<FileDescriptorWatcher::Controller> kqueue_watch_controller_;

  DISALLOW_COPY_AND_ASSIGN(FilePathWatcherKQueue);
};

}  // namespace base

#endif  // BASE_FILES_FILE_PATH_WATCHER_KQUEUE_H_
