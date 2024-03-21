// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/files/file_descriptor_watcher_posix.h"

#include <utility>

#include "base/bind.h"
#include "base/callback_helpers.h"
#include "base/memory/ptr_util.h"
#include "base/message_loop/message_loop_current.h"
#include "base/message_loop/message_pump_for_io.h"
#include "base/no_destructor.h"
#include "base/sequenced_task_runner.h"
#include "base/single_thread_task_runner.h"
#include "base/synchronization/waitable_event.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "base/threading/thread_checker.h"
#include "base/threading/thread_local.h"
#include "base/threading/thread_restrictions.h"

namespace base {

namespace {

ThreadLocalPointer<FileDescriptorWatcher>& GetTlsFdWatcher() {
  static NoDestructor<ThreadLocalPointer<FileDescriptorWatcher>> tls_fd_watcher;
  return *tls_fd_watcher;
}

}  // namespace

class FileDescriptorWatcher::Controller::Watcher
    : public MessagePumpForIO::FdWatcher,
      public MessageLoopCurrent::DestructionObserver {
 public:
  Watcher(WeakPtr<Controller> controller, MessagePumpForIO::Mode mode, int fd);
  ~Watcher() override;

  void StartWatching();

 private:
  friend class FileDescriptorWatcher;

  void OnFileCanReadWithoutBlocking(int fd) override;
  void OnFileCanWriteWithoutBlocking(int fd) override;

  void WillDestroyCurrentMessageLoop() override;

  MessagePumpForIO::FdWatchController fd_watch_controller_;


  const scoped_refptr<SequencedTaskRunner> callback_task_runner_ =
      SequencedTaskRunnerHandle::Get();



  WeakPtr<Controller> controller_;


  const MessagePumpForIO::Mode mode_;

  const int fd_;


  ThreadChecker thread_checker_;


  bool registered_as_destruction_observer_ = false;

  DISALLOW_COPY_AND_ASSIGN(Watcher);
};

FileDescriptorWatcher::Controller::Watcher::Watcher(
    WeakPtr<Controller> controller,
    MessagePumpForIO::Mode mode,
    int fd)
    : fd_watch_controller_(FROM_HERE),
      controller_(controller),
      mode_(mode),
      fd_(fd) {
  DCHECK(callback_task_runner_);
  thread_checker_.DetachFromThread();
}

FileDescriptorWatcher::Controller::Watcher::~Watcher() {
  DCHECK(thread_checker_.CalledOnValidThread());
  MessageLoopCurrentForIO::Get()->RemoveDestructionObserver(this);
}

void FileDescriptorWatcher::Controller::Watcher::StartWatching() {
  DCHECK(thread_checker_.CalledOnValidThread());
  DCHECK(MessageLoopCurrentForIO::IsSet());

  const bool watch_success =
      MessageLoopCurrentForIO::Get()->WatchFileDescriptor(
          fd_, false, mode_, &fd_watch_controller_, this);
  DCHECK(watch_success) << "Failed to watch fd=" << fd_;

  if (!registered_as_destruction_observer_) {
    MessageLoopCurrentForIO::Get()->AddDestructionObserver(this);
    registered_as_destruction_observer_ = true;
  }
}

void FileDescriptorWatcher::Controller::Watcher::OnFileCanReadWithoutBlocking(
    int fd) {
  DCHECK_EQ(fd_, fd);
  DCHECK_EQ(MessagePumpForIO::WATCH_READ, mode_);
  DCHECK(thread_checker_.CalledOnValidThread());

  callback_task_runner_->PostTask(
      FROM_HERE, BindOnce(&Controller::RunCallback, controller_));
}

void FileDescriptorWatcher::Controller::Watcher::OnFileCanWriteWithoutBlocking(
    int fd) {
  DCHECK_EQ(fd_, fd);
  DCHECK_EQ(MessagePumpForIO::WATCH_WRITE, mode_);
  DCHECK(thread_checker_.CalledOnValidThread());

  callback_task_runner_->PostTask(
      FROM_HERE, BindOnce(&Controller::RunCallback, controller_));
}

void FileDescriptorWatcher::Controller::Watcher::
    WillDestroyCurrentMessageLoop() {
  DCHECK(thread_checker_.CalledOnValidThread());

  if (callback_task_runner_->RunsTasksInCurrentSequence()) {


    controller_->watcher_.reset();
  } else {






    delete this;
  }
}

FileDescriptorWatcher::Controller::Controller(MessagePumpForIO::Mode mode,
                                              int fd,
                                              const RepeatingClosure& callback)
    : callback_(callback),
      io_thread_task_runner_(GetTlsFdWatcher().Get()->io_thread_task_runner()) {
  DCHECK(!callback_.is_null());
  DCHECK(io_thread_task_runner_);
  watcher_ = std::make_unique<Watcher>(weak_factory_.GetWeakPtr(), mode, fd);
  StartWatching();
}

FileDescriptorWatcher::Controller::~Controller() {
  DCHECK(sequence_checker_.CalledOnValidSequence());

  if (io_thread_task_runner_->BelongsToCurrentThread()) {

    watcher_.reset();
  } else {





















    WaitableEvent done;
    io_thread_task_runner_->PostTask(
        FROM_HERE, BindOnce(
                       [](Watcher* watcher, ScopedClosureRunner closure) {


                         delete watcher;

                       },
                       Unretained(watcher_.release()),
                       ScopedClosureRunner(BindOnce(&WaitableEvent::Signal,
                                                    Unretained(&done)))));
    ScopedAllowBaseSyncPrimitivesOutsideBlockingScope allow;
    done.Wait();
  }


}

void FileDescriptorWatcher::Controller::StartWatching() {
  DCHECK(sequence_checker_.CalledOnValidSequence());
  if (io_thread_task_runner_->BelongsToCurrentThread()) {

    watcher_->StartWatching();
  } else {




    io_thread_task_runner_->PostTask(
        FROM_HERE,
        BindOnce(&Watcher::StartWatching, Unretained(watcher_.get())));
  }
}

void FileDescriptorWatcher::Controller::RunCallback() {
  DCHECK(sequence_checker_.CalledOnValidSequence());

  WeakPtr<Controller> weak_this = weak_factory_.GetWeakPtr();

  callback_.Run();

  if (weak_this)
    StartWatching();
}

FileDescriptorWatcher::FileDescriptorWatcher(
    scoped_refptr<SingleThreadTaskRunner> io_thread_task_runner)
    : io_thread_task_runner_(std::move(io_thread_task_runner)) {
  DCHECK(!GetTlsFdWatcher().Get());
  GetTlsFdWatcher().Set(this);
}

FileDescriptorWatcher::~FileDescriptorWatcher() {
  GetTlsFdWatcher().Set(nullptr);
}

std::unique_ptr<FileDescriptorWatcher::Controller>
FileDescriptorWatcher::WatchReadable(int fd, const RepeatingClosure& callback) {
  return WrapUnique(new Controller(MessagePumpForIO::WATCH_READ, fd, callback));
}

std::unique_ptr<FileDescriptorWatcher::Controller>
FileDescriptorWatcher::WatchWritable(int fd, const RepeatingClosure& callback) {
  return WrapUnique(
      new Controller(MessagePumpForIO::WATCH_WRITE, fd, callback));
}

#if DCHECK_IS_ON()
void FileDescriptorWatcher::AssertAllowed() {
  DCHECK(GetTlsFdWatcher().Get());
}
#endif

}  // namespace base
