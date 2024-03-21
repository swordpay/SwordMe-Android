// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_POSIX_UNIX_DOMAIN_SOCKET_H_
#define BASE_POSIX_UNIX_DOMAIN_SOCKET_H_

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <vector>

#include "base/base_export.h"
#include "base/files/scoped_file.h"
#include "base/process/process_handle.h"
#include "build/build_config.h"

namespace base {

class Pickle;

#if !defined(OS_NACL_NONSFI)
// Creates a connected pair of UNIX-domain SOCK_SEQPACKET sockets, and passes
// ownership of the newly allocated file descriptors to |one| and |two|.
// Returns true on success.
bool BASE_EXPORT CreateSocketPair(ScopedFD* one, ScopedFD* two);
#endif

class BASE_EXPORT UnixDomainSocket {
 public:

  static const size_t kMaxFileDescriptors;

#if !defined(OS_NACL_NONSFI)



  static bool EnableReceiveProcessId(int fd);
#endif  // !defined(OS_NACL_NONSFI)


  static bool SendMsg(int fd,
                      const void* msg,
                      size_t length,
                      const std::vector<int>& fds);


  static ssize_t RecvMsg(int fd,
                         void* msg,
                         size_t length,
                         std::vector<ScopedFD>* fds);




  static ssize_t RecvMsgWithPid(int fd,
                                void* msg,
                                size_t length,
                                std::vector<ScopedFD>* fds,
                                ProcessId* pid);

#if !defined(OS_NACL_NONSFI)


















  static ssize_t SendRecvMsg(int fd,
                             uint8_t* reply,
                             unsigned reply_len,
                             int* result_fd,
                             const Pickle& request);


  static ssize_t SendRecvMsgWithFlags(int fd,
                                      uint8_t* reply,
                                      unsigned reply_len,
                                      int recvmsg_flags,
                                      int* result_fd,
                                      const Pickle& request);
#endif  // !defined(OS_NACL_NONSFI)
 private:

  static ssize_t RecvMsgWithFlags(int fd,
                                  void* msg,
                                  size_t length,
                                  int flags,
                                  std::vector<ScopedFD>* fds,
                                  ProcessId* pid);
};

}  // namespace base

#endif  // BASE_POSIX_UNIX_DOMAIN_SOCKET_H_
