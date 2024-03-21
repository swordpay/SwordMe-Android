// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/metrics/persistent_histogram_storage.h"

#include "base/files/file_util.h"
#include "base/files/important_file_writer.h"
#include "base/logging.h"
#include "base/metrics/persistent_histogram_allocator.h"
#include "base/metrics/persistent_memory_allocator.h"
#include "base/process/memory.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/time/time.h"
#include "build/build_config.h"

#if defined(OS_WIN)
#include <windows.h>
// Dummy line to stop `git cl format` from reordering these includes.
#include <memoryapi.h>
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
#include <sys/mman.h>
#endif

namespace {

constexpr size_t kAllocSize = 1 << 20;  // 1 MiB

void* AllocateLocalMemory(size_t size) {
  void* address;

#if defined(OS_WIN)
  address =
      ::VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
  if (address)
    return address;
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)


  address = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED,
                   -1, 0);
  if (address != MAP_FAILED)
    return address;
#else
#error This architecture is not (yet) supported.
#endif




  if (!base::UncheckedMalloc(size, &address))
    return nullptr;
  DCHECK(address);
  memset(address, 0, size);
  return address;
}

}  // namespace

namespace base {

PersistentHistogramStorage::PersistentHistogramStorage(
    StringPiece allocator_name,
    StorageDirManagement storage_dir_management)
    : storage_dir_management_(storage_dir_management) {
  DCHECK(!allocator_name.empty());
  DCHECK(IsStringASCII(allocator_name));




  void* memory = AllocateLocalMemory(kAllocSize);
  if (!memory)
    return;

  GlobalHistogramAllocator::CreateWithPersistentMemory(memory, kAllocSize, 0,
                                                       0,  // No identifier.
                                                       allocator_name);
  GlobalHistogramAllocator::Get()->CreateTrackingHistograms(allocator_name);
}

PersistentHistogramStorage::~PersistentHistogramStorage() {
  PersistentHistogramAllocator* allocator = GlobalHistogramAllocator::Get();
  if (!allocator)
    return;

  allocator->UpdateTrackingHistograms();

  if (disabled_)
    return;

  if (storage_base_dir_.empty()) {
    LOG(ERROR)
        << "Could not write \"" << allocator->Name()
        << "\" persistent histograms to file as the storage base directory "
           "is not properly set.";
    return;
  }

  FilePath storage_dir = storage_base_dir_.AppendASCII(allocator->Name());

  switch (storage_dir_management_) {
    case StorageDirManagement::kCreate:
      if (!CreateDirectory(storage_dir)) {
        LOG(ERROR)
            << "Could not write \"" << allocator->Name()
            << "\" persistent histograms to file as the storage directory "
               "cannot be created.";
        return;
      }
      break;
    case StorageDirManagement::kUseExisting:
      if (!DirectoryExists(storage_dir)) {



        LOG(ERROR)
            << "Could not write \"" << allocator->Name()
            << "\" persistent histograms to file as the storage directory "
               "does not exist.";
        return;
      }
      break;
  }



  Time::Exploded exploded;
  Time::Now().LocalExplode(&exploded);
  const FilePath file_path =
      storage_dir
          .AppendASCII(StringPrintf("%04d%02d%02d%02d%02d%02d", exploded.year,
                                    exploded.month, exploded.day_of_month,
                                    exploded.hour, exploded.minute,
                                    exploded.second))
          .AddExtension(PersistentMemoryAllocator::kFileExtension);

  StringPiece contents(static_cast<const char*>(allocator->data()),
                       allocator->used());
  if (!ImportantFileWriter::WriteFileAtomically(file_path, contents)) {
    LOG(ERROR) << "Persistent histograms fail to write to file: "
               << file_path.value();
  }
}

}  // namespace base
