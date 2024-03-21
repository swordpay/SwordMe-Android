// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_SEQUENCE_TOKEN_H_
#define BASE_SEQUENCE_TOKEN_H_

#include "base/base_export.h"
#include "base/macros.h"

namespace base {

// at a time in posting order).
class BASE_EXPORT SequenceToken {
 public:

  SequenceToken() = default;

  SequenceToken(const SequenceToken& other) = default;
  SequenceToken& operator=(const SequenceToken& other) = default;


  bool operator==(const SequenceToken& other) const;
  bool operator!=(const SequenceToken& other) const;

  bool IsValid() const;


  int ToInternalValue() const;


  static SequenceToken Create();



  static SequenceToken GetForCurrentThread();

 private:
  explicit SequenceToken(int token) : token_(token) {}

  static constexpr int kInvalidSequenceToken = -1;
  int token_ = kInvalidSequenceToken;
};

//
// This is used by ThreadCheckerImpl to determine whether calls to
// CalledOnValidThread() come from the same task and hence are deterministically
// single-threaded (vs. calls coming from different sequenced or parallel tasks,
// which may or may not run on the same thread).
class BASE_EXPORT TaskToken {
 public:

  TaskToken() = default;

  TaskToken(const TaskToken& other) = default;
  TaskToken& operator=(const TaskToken& other) = default;


  bool operator==(const TaskToken& other) const;
  bool operator!=(const TaskToken& other) const;

  bool IsValid() const;




  static TaskToken GetForCurrentThread();

 private:
  friend class ScopedSetSequenceTokenForCurrentThread;

  explicit TaskToken(int token) : token_(token) {}



  static TaskToken Create();

  static constexpr int kInvalidTaskToken = -1;
  int token_ = kInvalidTaskToken;
};

class BASE_EXPORT ScopedSetSequenceTokenForCurrentThread {
 public:





  ScopedSetSequenceTokenForCurrentThread(const SequenceToken& sequence_token);
  ~ScopedSetSequenceTokenForCurrentThread();

 private:
  const SequenceToken sequence_token_;
  const TaskToken task_token_;

  DISALLOW_COPY_AND_ASSIGN(ScopedSetSequenceTokenForCurrentThread);
};

}  // namespace base

#endif  // BASE_SEQUENCE_TOKEN_H_
