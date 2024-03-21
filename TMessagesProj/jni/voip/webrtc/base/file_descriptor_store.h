// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_FILE_DESCRIPTOR_STORE_H_
#define BASE_FILE_DESCRIPTOR_STORE_H_

#include <map>
#include <string>

#include "base/files/memory_mapped_file.h"
#include "base/files/scoped_file.h"
#include "base/macros.h"

namespace base {

// that must be unique.
// It is used to share file descriptors from a process to its child.
class BASE_EXPORT FileDescriptorStore {
 public:
  struct Descriptor {
    Descriptor(const std::string& key, base::ScopedFD fd);
    Descriptor(const std::string& key,
               base::ScopedFD fd,
               base::MemoryMappedFile::Region region);
    Descriptor(Descriptor&& other);
    ~Descriptor();

    Descriptor& operator=(Descriptor&& other) = default;

    std::string key;

    base::ScopedFD fd;

    base::MemoryMappedFile::Region region;
  };
  using Mapping = std::map<std::string, Descriptor>;

  static FileDescriptorStore& GetInstance();


  base::ScopedFD TakeFD(const std::string& key,
                        base::MemoryMappedFile::Region* region);

  base::ScopedFD MaybeTakeFD(const std::string& key,
                             base::MemoryMappedFile::Region* region);


  void Set(const std::string& key, base::ScopedFD fd);

  void Set(const std::string& key,
           base::ScopedFD fd,
           base::MemoryMappedFile::Region region);

 private:
  FileDescriptorStore();
  ~FileDescriptorStore();

  Mapping descriptors_;

  DISALLOW_COPY_AND_ASSIGN(FileDescriptorStore);
};

}  // namespace base

#endif  // BASE_FILE_DESCRIPTOR_STORE_H_
