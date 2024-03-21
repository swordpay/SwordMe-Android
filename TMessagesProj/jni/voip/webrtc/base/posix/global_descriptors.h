// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_POSIX_GLOBAL_DESCRIPTORS_H_
#define BASE_POSIX_GLOBAL_DESCRIPTORS_H_

#include "build/build_config.h"

#include <vector>
#include <utility>

#include <stdint.h>

#include "base/files/memory_mapped_file.h"
#include "base/files/scoped_file.h"
#include "base/memory/singleton.h"

namespace base {

// numbers before execing a child; stdin, stdout and stderr are ubiqutous
// examples.
//
// However, when using a zygote model, this becomes troublesome. Since the
// descriptors which need to be in these slots generally aren't known, any code
// could open a resource and take one of the reserved descriptors. Simply
// overwriting the slot isn't a viable solution.
//
// We could try to fill the reserved slots as soon as possible, but this is a
// fragile solution since global constructors etc are able to open files.
//
// Instead, we retreat from the idea of installing descriptors in specific
// slots and add a layer of indirection in the form of this singleton object.
// It maps from an abstract key to a descriptor. If independent modules each
// need to define keys, then values should be chosen randomly so as not to
// collide.
//
// Note that this class is deprecated and passing file descriptor should ideally
// be done through the command line and using FileDescriptorStore.
// See https://crbugs.com/detail?id=692619
class BASE_EXPORT GlobalDescriptors {
 public:
  typedef uint32_t Key;
  struct Descriptor {
    Descriptor(Key key, int fd);
    Descriptor(Key key, int fd, base::MemoryMappedFile::Region region);

    Key key;

    int fd;

    base::MemoryMappedFile::Region region;
  };
  typedef std::vector<Descriptor> Mapping;


  static const int kBaseDescriptor = 3;  // 0, 1, 2 are already taken.

  static GlobalDescriptors* GetInstance();

  int Get(Key key) const;

  int MaybeGet(Key key) const;



  base::ScopedFD TakeFD(Key key, base::MemoryMappedFile::Region* region);

  base::MemoryMappedFile::Region GetRegion(Key key) const;


  void Set(Key key, int fd);

  void Set(Key key, int fd, base::MemoryMappedFile::Region region);

  void Reset(const Mapping& mapping);

 private:
  friend struct DefaultSingletonTraits<GlobalDescriptors>;
  GlobalDescriptors();
  ~GlobalDescriptors();

  Mapping descriptors_;
};

}  // namespace base

#endif  // BASE_POSIX_GLOBAL_DESCRIPTORS_H_
