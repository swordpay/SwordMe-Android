// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_FILES_FILE_PATH_WATCHER_FSEVENTS_H_
#define BASE_FILES_FILE_PATH_WATCHER_FSEVENTS_H_

#include <CoreServices/CoreServices.h>
#include <stddef.h>

#include <vector>

#include "base/files/file_path.h"
#include "base/files/file_path_watcher.h"
#include "base/mac/scoped_dispatch_object.h"
#include "base/macros.h"
#include "base/memory/weak_ptr.h"

namespace base {

// There are trade-offs between the FSEvents implementation and a kqueue
// implementation. The biggest issues are that FSEvents on 10.6 sometimes drops
// events and kqueue does not trigger for modifications to a file in a watched
// directory. See file_path_watcher_mac.cc for the code that decides when to
// use which one.
class FilePathWatcherFSEvents : public FilePathWatcher::PlatformDelegate {
 public:
  FilePathWatcherFSEvents();
  ~FilePathWatcherFSEvents() override;

  bool Watch(const FilePath& path,
             bool recursive,
             const FilePathWatcher::Callback& callback) override;
  void Cancel() override;

 private:
  static void FSEventsCallback(ConstFSEventStreamRef stream,
                               void* event_watcher,
                               size_t num_events,
                               void* event_paths,
                               const FSEventStreamEventFlags flags[],
                               const FSEventStreamEventId event_ids[]);

  void OnFilePathsChanged(const std::vector<FilePath>& paths);



  void DispatchEvents(const std::vector<FilePath>& paths,
                      const FilePath& target,
                      const FilePath& resolved_target);


  void UpdateEventStream(FSEventStreamEventId start_event);


  bool ResolveTargetPath();

  void ReportError(const FilePath& target);

  void DestroyEventStream();

  void StartEventStream(FSEventStreamEventId start_event, const FilePath& path);


  FilePathWatcher::Callback callback_;

  ScopedDispatchObject<dispatch_queue_t> queue_;


  FilePath target_;


  FilePath resolved_target_;


  FSEventStreamRef fsevent_stream_;

  WeakPtrFactory<FilePathWatcherFSEvents> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(FilePathWatcherFSEvents);
};

}  // namespace base

#endif  // BASE_FILES_FILE_PATH_WATCHER_FSEVENTS_H_
