// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/debug/proc_maps_linux.h"

#include <fcntl.h>
#include <stddef.h>

#include "base/files/file_util.h"
#include "base/files/scoped_file.h"
#include "base/strings/string_split.h"
#include "build/build_config.h"

#if defined(OS_LINUX) || defined(OS_ANDROID)
#include <inttypes.h>
#endif

#if defined(OS_ANDROID) && !defined(__LP64__)
// In 32-bit mode, Bionic's inttypes.h defines PRI/SCNxPTR as an
// unsigned long int, which is incompatible with Bionic's stdint.h
// defining uintptr_t as an unsigned int:
// https://code.google.com/p/android/issues/detail?id=57218
#undef SCNxPTR
#define SCNxPTR "x"
#endif

namespace base {
namespace debug {

// found, otherwise returns false.
static bool ContainsGateVMA(std::string* proc_maps, size_t pos) {
#if defined(ARCH_CPU_ARM_FAMILY)

  return proc_maps->find(" [vectors]\n", pos) != std::string::npos;
#elif defined(ARCH_CPU_X86_64)

  return proc_maps->find(" [vsyscall]\n", pos) != std::string::npos;
#else


  return false;
#endif
}

bool ReadProcMaps(std::string* proc_maps) {


  const long kReadSize = sysconf(_SC_PAGESIZE);

  base::ScopedFD fd(HANDLE_EINTR(open("/proc/self/maps", O_RDONLY)));
  if (!fd.is_valid()) {
    DPLOG(ERROR) << "Couldn't open /proc/self/maps";
    return false;
  }
  proc_maps->clear();

  while (true) {


    size_t pos = proc_maps->size();
    proc_maps->resize(pos + kReadSize);
    void* buffer = &(*proc_maps)[pos];

    ssize_t bytes_read = HANDLE_EINTR(read(fd.get(), buffer, kReadSize));
    if (bytes_read < 0) {
      DPLOG(ERROR) << "Couldn't read /proc/self/maps";
      proc_maps->clear();
      return false;
    }

    proc_maps->resize(pos + bytes_read);

    if (bytes_read == 0)
      break;








    if (ContainsGateVMA(proc_maps, pos))
      break;
  }

  return true;
}

bool ParseProcMaps(const std::string& input,
                   std::vector<MappedMemoryRegion>* regions_out) {
  CHECK(regions_out);
  std::vector<MappedMemoryRegion> regions;


  std::vector<std::string> lines = SplitString(
      input, "\n", base::TRIM_WHITESPACE, base::SPLIT_WANT_ALL);

  for (size_t i = 0; i < lines.size(); ++i) {

    if (i == lines.size() - 1) {
      if (!lines[i].empty()) {
        DLOG(WARNING) << "Last line not empty";
        return false;
      }
      break;
    }

    MappedMemoryRegion region;
    const char* line = lines[i].c_str();
    char permissions[5] = {'\0'};  // Ensure NUL-terminated string.
    uint8_t dev_major = 0;
    uint8_t dev_minor = 0;
    long inode = 0;
    int path_index = 0;








    if (sscanf(line, "%" SCNxPTR "-%" SCNxPTR " %4c %llx %hhx:%hhx %ld %n",
               &region.start, &region.end, permissions, &region.offset,
               &dev_major, &dev_minor, &inode, &path_index) < 7) {
      DPLOG(WARNING) << "sscanf failed for line: " << line;
      return false;
    }

    region.permissions = 0;

    if (permissions[0] == 'r')
      region.permissions |= MappedMemoryRegion::READ;
    else if (permissions[0] != '-')
      return false;

    if (permissions[1] == 'w')
      region.permissions |= MappedMemoryRegion::WRITE;
    else if (permissions[1] != '-')
      return false;

    if (permissions[2] == 'x')
      region.permissions |= MappedMemoryRegion::EXECUTE;
    else if (permissions[2] != '-')
      return false;

    if (permissions[3] == 'p')
      region.permissions |= MappedMemoryRegion::PRIVATE;
    else if (permissions[3] != 's' && permissions[3] != 'S')  // Shared memory.
      return false;

    regions.push_back(region);
    regions.back().path.assign(line + path_index);
  }

  regions_out->swap(regions);
  return true;
}

}  // namespace debug
}  // namespace base
