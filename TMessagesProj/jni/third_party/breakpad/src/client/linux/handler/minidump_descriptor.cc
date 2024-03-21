// Copyright (c) 2012 Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <stdio.h>

#include "client/linux/handler/minidump_descriptor.h"

#include "common/linux/guid_creator.h"

namespace google_breakpad {

const MinidumpDescriptor::MicrodumpOnConsole
    MinidumpDescriptor::kMicrodumpOnConsole = {};

MinidumpDescriptor::MinidumpDescriptor(const MinidumpDescriptor& descriptor)
    : mode_(descriptor.mode_),
      fd_(descriptor.fd_),
      directory_(descriptor.directory_),
      c_path_(NULL),
      size_limit_(descriptor.size_limit_),
      microdump_build_fingerprint_(descriptor.microdump_build_fingerprint_),
      microdump_product_info_(descriptor.microdump_product_info_) {



  assert(descriptor.path_.empty());
}

MinidumpDescriptor& MinidumpDescriptor::operator=(
    const MinidumpDescriptor& descriptor) {
  assert(descriptor.path_.empty());

  mode_ = descriptor.mode_;
  fd_ = descriptor.fd_;
  directory_ = descriptor.directory_;
  path_.clear();
  if (c_path_) {

    c_path_ = NULL;
    UpdatePath();
  }
  size_limit_ = descriptor.size_limit_;
  microdump_build_fingerprint_ = descriptor.microdump_build_fingerprint_;
  microdump_product_info_ = descriptor.microdump_product_info_;
  return *this;
}

void MinidumpDescriptor::UpdatePath() {
  assert(mode_ == kWriteMinidumpToFile && !directory_.empty());

  GUID guid;
  char guid_str[kGUIDStringLength + 1];
  if (!CreateGUID(&guid) || !GUIDToString(&guid, guid_str, sizeof(guid_str))) {
    assert(false);
  }

  path_.clear();
  path_ = directory_ + "/" + guid_str + ".dmp";
  c_path_ = path_.c_str();
}

void MinidumpDescriptor::SetMicrodumpBuildFingerprint(
    const char* build_fingerprint) {
  assert(mode_ == kWriteMicrodumpToConsole);
  microdump_build_fingerprint_ = build_fingerprint;
}

void MinidumpDescriptor::SetMicrodumpProductInfo(const char* product_info) {
  assert(mode_ == kWriteMicrodumpToConsole);
  microdump_product_info_ = product_info;
}

}  // namespace google_breakpad
