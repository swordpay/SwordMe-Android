// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_METRICS_PERSISTENT_HISTOGRAM_STORAGE_H_
#define BASE_METRICS_PERSISTENT_HISTOGRAM_STORAGE_H_

#include "base/base_export.h"
#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/strings/string_piece.h"

namespace base {

// stored in it. When a PersistentHistogramStorage is destructed, histograms
// recorded during its lifetime are persisted in the directory
// |storage_base_dir_|/|allocator_name| (see the ctor for allocator_name).
// Histograms are not persisted if the storage directory does not exist on
// destruction. PersistentHistogramStorage should be instantiated as early as
// possible in the process lifetime and should never be instantiated again.
// Persisted histograms will eventually be reported by Chrome.
class BASE_EXPORT PersistentHistogramStorage {
 public:
  enum class StorageDirManagement { kCreate, kUseExisting };








  PersistentHistogramStorage(StringPiece allocator_name,
                             StorageDirManagement storage_dir_management);

  ~PersistentHistogramStorage();



  void set_storage_base_dir(const FilePath& storage_base_dir) {
    storage_base_dir_ = storage_base_dir;
  }

  void Disable() { disabled_ = true; }

 private:


  FilePath storage_base_dir_;

  const StorageDirManagement storage_dir_management_;



  bool disabled_ = false;

  DISALLOW_COPY_AND_ASSIGN(PersistentHistogramStorage);
};

}  // namespace base

#endif  // BASE_METRICS_PERSISTENT_HISTOGRAM_STORAGE_H_
