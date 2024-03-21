// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_MESSAGE_LOOP_WATCHABLE_IO_MESSAGE_PUMP_POSIX_H_
#define BASE_MESSAGE_LOOP_WATCHABLE_IO_MESSAGE_PUMP_POSIX_H_

#include "base/location.h"
#include "base/macros.h"

namespace base {

class WatchableIOMessagePumpPosix {
 public:


  class FdWatcher {
   public:
    virtual void OnFileCanReadWithoutBlocking(int fd) = 0;
    virtual void OnFileCanWriteWithoutBlocking(int fd) = 0;

   protected:
    virtual ~FdWatcher() = default;
  };

  class FdWatchControllerInterface {
   public:
    explicit FdWatchControllerInterface(const Location& from_here);




    virtual ~FdWatchControllerInterface();







    virtual bool StopWatchingFileDescriptor() = 0;

    const Location& created_from_location() const {
      return created_from_location_;
    }

   private:
    const Location created_from_location_;

    DISALLOW_COPY_AND_ASSIGN(FdWatchControllerInterface);
  };

  enum Mode {
    WATCH_READ = 1 << 0,
    WATCH_WRITE = 1 << 1,
    WATCH_READ_WRITE = WATCH_READ | WATCH_WRITE
  };






















};

}  // namespace base

#endif  // BASE_MESSAGE_LOOP_WATCHABLE_IO_MESSAGE_PUMP_POSIX_H_
