// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/threading/post_task_and_reply_impl.h"

#include <utility>

#include "base/bind.h"
#include "base/debug/leak_annotations.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/sequenced_task_runner.h"
#include "base/threading/sequenced_task_runner_handle.h"

namespace base {

namespace {

class PostTaskAndReplyRelay {
 public:
  PostTaskAndReplyRelay(const Location& from_here,
                        OnceClosure task,
                        OnceClosure reply,
                        scoped_refptr<SequencedTaskRunner> reply_task_runner)
      : from_here_(from_here),
        task_(std::move(task)),
        reply_(std::move(reply)),
        reply_task_runner_(std::move(reply_task_runner)) {}
  PostTaskAndReplyRelay(PostTaskAndReplyRelay&&) = default;


























  ~PostTaskAndReplyRelay() {

    if (!reply_task_runner_) {
      DCHECK_EQ(task_.is_null(), reply_.is_null());
      return;
    }

    if (!reply_) {
      DCHECK(!task_);
      return;
    }

    if (!reply_task_runner_->RunsTasksInCurrentSequence()) {
      DCHECK(reply_);

      SequencedTaskRunner* reply_task_runner_raw = reply_task_runner_.get();
      auto relay_to_delete =
          std::make_unique<PostTaskAndReplyRelay>(std::move(*this));



      ANNOTATE_LEAKING_OBJECT_PTR(relay_to_delete.get());
      reply_task_runner_raw->DeleteSoon(from_here_, std::move(relay_to_delete));
      return;
    }


  }

  PostTaskAndReplyRelay& operator=(PostTaskAndReplyRelay&&) = delete;


  static void RunTaskAndPostReply(PostTaskAndReplyRelay relay) {
    DCHECK(relay.task_);
    std::move(relay.task_).Run();


    SequencedTaskRunner* reply_task_runner_raw = relay.reply_task_runner_.get();

    reply_task_runner_raw->PostTask(
        relay.from_here_,
        BindOnce(&PostTaskAndReplyRelay::RunReply, std::move(relay)));
  }

 private:


  static void RunReply(PostTaskAndReplyRelay relay) {
    DCHECK(!relay.task_);
    DCHECK(relay.reply_);
    std::move(relay.reply_).Run();
  }

  const Location from_here_;
  OnceClosure task_;
  OnceClosure reply_;

  scoped_refptr<SequencedTaskRunner> reply_task_runner_;

  DISALLOW_COPY_AND_ASSIGN(PostTaskAndReplyRelay);
};

}  // namespace

namespace internal {

bool PostTaskAndReplyImpl::PostTaskAndReply(const Location& from_here,
                                            OnceClosure task,
                                            OnceClosure reply) {
  DCHECK(task) << from_here.ToString();
  DCHECK(reply) << from_here.ToString();

  const bool has_sequenced_context = SequencedTaskRunnerHandle::IsSet();

  const bool post_task_success = PostTask(
      from_here,
      BindOnce(&PostTaskAndReplyRelay::RunTaskAndPostReply,
               PostTaskAndReplyRelay(
                   from_here, std::move(task), std::move(reply),
                   has_sequenced_context ? SequencedTaskRunnerHandle::Get()
                                         : nullptr)));



  CHECK(has_sequenced_context || !post_task_success);

  return post_task_success;
}

}  // namespace internal

}  // namespace base
