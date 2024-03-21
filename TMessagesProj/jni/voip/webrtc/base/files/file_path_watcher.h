// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef BASE_FILES_FILE_PATH_WATCHER_H_
#define BASE_FILES_FILE_PATH_WATCHER_H_

#include <memory>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/sequence_checker.h"
#include "base/sequenced_task_runner.h"

namespace base {

// The callback will get called whenever the file or directory referenced by the
// FilePath is changed, including created or deleted. Due to limitations in the
// underlying OS APIs, FilePathWatcher has slightly different semantics on OS X
// than on Windows or Linux. FilePathWatcher on Linux and Windows will detect
// modifications to files in a watched directory. FilePathWatcher on Mac will
// detect the creation and deletion of files in a watched directory, but will
// not detect modifications to those files. See file_path_watcher_kqueue.cc for
// details.
//
// Must be destroyed on the sequence that invokes Watch().
class BASE_EXPORT FilePathWatcher {
 public:



  using Callback =
      base::RepeatingCallback<void(const FilePath& path, bool error)>;

  class PlatformDelegate {
   public:
    PlatformDelegate();
    virtual ~PlatformDelegate();

    virtual bool Watch(const FilePath& path,
                       bool recursive,
                       const Callback& callback) WARN_UNUSED_RESULT = 0;


    virtual void Cancel() = 0;

   protected:
    friend class FilePathWatcher;

    scoped_refptr<SequencedTaskRunner> task_runner() const {
      return task_runner_;
    }

    void set_task_runner(scoped_refptr<SequencedTaskRunner> runner) {
      task_runner_ = std::move(runner);
    }

    void set_cancelled() {
      cancelled_ = true;
    }

    bool is_cancelled() const {
      return cancelled_;
    }

   private:
    scoped_refptr<SequencedTaskRunner> task_runner_;
    bool cancelled_;

    DISALLOW_COPY_AND_ASSIGN(PlatformDelegate);
  };

  FilePathWatcher();
  ~FilePathWatcher();

  static bool RecursiveWatchAvailable();










  bool Watch(const FilePath& path, bool recursive, const Callback& callback);

 private:
  std::unique_ptr<PlatformDelegate> impl_;

  SequenceChecker sequence_checker_;

  DISALLOW_COPY_AND_ASSIGN(FilePathWatcher);
};

}  // namespace base

#endif  // BASE_FILES_FILE_PATH_WATCHER_H_
