// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_FILES_IMPORTANT_FILE_WRITER_H_
#define BASE_FILES_IMPORTANT_FILE_WRITER_H_

#include <string>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/sequence_checker.h"
#include "base/strings/string_piece.h"
#include "base/time/time.h"
#include "base/timer/timer.h"

namespace base {

class SequencedTaskRunner;

// *application* crash during write (implemented as create, flush, rename).
//
// As an added benefit, ImportantFileWriter makes it less likely that the file
// is corrupted by *system* crash, though even if the ImportantFileWriter call
// has already returned at the time of the crash it is not specified which
// version of the file (old or new) is preserved. And depending on system
// configuration (hardware and software) a significant likelihood of file
// corruption may remain, thus using ImportantFileWriter is not a valid
// substitute for file integrity checks and recovery codepaths for malformed
// files.
//
// Also note that ImportantFileWriter can be *really* slow (cf. File::Flush()
// for details) and thus please don't block shutdown on ImportantFileWriter.
class BASE_EXPORT ImportantFileWriter {
 public:


  class BASE_EXPORT DataSerializer {
   public:



    virtual bool SerializeData(std::string* data) = 0;

   protected:
    virtual ~DataSerializer() = default;
  };



  static bool WriteFileAtomically(const FilePath& path,
                                  StringPiece data,
                                  StringPiece histogram_suffix = StringPiece());





  ImportantFileWriter(const FilePath& path,
                      scoped_refptr<SequencedTaskRunner> task_runner,
                      const char* histogram_suffix = nullptr);

  ImportantFileWriter(const FilePath& path,
                      scoped_refptr<SequencedTaskRunner> task_runner,
                      TimeDelta interval,
                      const char* histogram_suffix = nullptr);


  ~ImportantFileWriter();

  const FilePath& path() const { return path_; }


  bool HasPendingWrite() const;


  void WriteNow(std::unique_ptr<std::string> data);






  void ScheduleWrite(DataSerializer* serializer);

  void DoScheduledWrite();








  void RegisterOnNextWriteCallbacks(
      OnceClosure before_next_write_callback,
      OnceCallback<void(bool success)> after_next_write_callback);

  TimeDelta commit_interval() const {
    return commit_interval_;
  }

  void SetTimerForTesting(OneShotTimer* timer_override);

 private:
  const OneShotTimer& timer() const {
    return timer_override_ ? *timer_override_ : timer_;
  }
  OneShotTimer& timer() { return timer_override_ ? *timer_override_ : timer_; }

  void ClearPendingWrite();

  OnceClosure before_next_write_callback_;
  OnceCallback<void(bool success)> after_next_write_callback_;

  const FilePath path_;

  const scoped_refptr<SequencedTaskRunner> task_runner_;

  OneShotTimer timer_;

  OneShotTimer* timer_override_ = nullptr;

  DataSerializer* serializer_;

  const TimeDelta commit_interval_;

  const std::string histogram_suffix_;

  SEQUENCE_CHECKER(sequence_checker_);

  WeakPtrFactory<ImportantFileWriter> weak_factory_{this};

  DISALLOW_COPY_AND_ASSIGN(ImportantFileWriter);
};

}  // namespace base

#endif  // BASE_FILES_IMPORTANT_FILE_WRITER_H_
