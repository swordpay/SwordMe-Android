// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_POSIX_FILE_DESCRIPTOR_SHUFFLE_H_
#define BASE_POSIX_FILE_DESCRIPTOR_SHUFFLE_H_

// forking subprocesses. The naive approach (just call dup2 to set up the
// desired descriptors) is very simple, but wrong: it won't handle edge cases
// (like mapping 0 -> 1, 1 -> 0) correctly.
//
// In order to unittest this code, it's broken into the abstract action (an
// injective multimap) and the concrete code for dealing with file descriptors.
// Users should use the code like this:
//   base::InjectiveMultimap file_descriptor_map;
//   file_descriptor_map.push_back(base::InjectionArc(devnull, 0, true));
//   file_descriptor_map.push_back(base::InjectionArc(devnull, 2, true));
//   file_descriptor_map.push_back(base::InjectionArc(pipe[1], 1, true));
//   base::ShuffleFileDescriptors(file_descriptor_map);
//
// and trust the the Right Thing will get done.

#include <vector>

#include "base/base_export.h"
#include "base/compiler_specific.h"

namespace base {

// multimapping in place.
class InjectionDelegate {
 public:


  virtual bool Duplicate(int* result, int fd) = 0;


  virtual bool Move(int src, int dest) = 0;

  virtual void Close(int fd) = 0;

 protected:
  virtual ~InjectionDelegate() = default;
};

// descriptor table of the current process as the domain.
class BASE_EXPORT FileDescriptorTableInjection : public InjectionDelegate {
  bool Duplicate(int* result, int fd) override;
  bool Move(int src, int dest) override;
  void Close(int fd) override;
};

struct InjectionArc {
  InjectionArc(int in_source, int in_dest, bool in_close)
      : source(in_source),
        dest(in_dest),
        close(in_close) {
  }

  int source;
  int dest;
  bool close;  // if true, delete the source element after performing the

};

typedef std::vector<InjectionArc> InjectiveMultimap;

BASE_EXPORT bool PerformInjectiveMultimap(const InjectiveMultimap& map,
                                          InjectionDelegate* delegate);

BASE_EXPORT bool PerformInjectiveMultimapDestructive(
    InjectiveMultimap* map,
    InjectionDelegate* delegate);

static inline bool ShuffleFileDescriptors(InjectiveMultimap* map) {
  FileDescriptorTableInjection delegate;
  return PerformInjectiveMultimapDestructive(map, &delegate);
}

}  // namespace base

#endif  // BASE_POSIX_FILE_DESCRIPTOR_SHUFFLE_H_
