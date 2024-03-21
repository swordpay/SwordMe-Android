// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PROFILER_ARM_CFI_TABLE_H_
#define BASE_PROFILER_ARM_CFI_TABLE_H_

#include <memory>

#include "base/containers/buffer_iterator.h"
#include "base/containers/span.h"
#include "base/macros.h"
#include "base/optional.h"

namespace base {

// Information (CFI) for Chrome, which contains tables for unwinding Chrome
// functions. For detailed description of the format, see
// extract_unwind_tables.py.
class BASE_EXPORT ArmCFITable {
 public:



  struct FrameEntry {



    uint16_t cfa_offset = 0;



    uint16_t ra_offset = 0;
  };


  static std::unique_ptr<ArmCFITable> Parse(span<const uint8_t> cfi_data);

  ArmCFITable(span<const uint32_t> function_addresses,
              span<const uint16_t> entry_data_indices,
              span<const uint8_t> entry_data);
  ~ArmCFITable();


  Optional<FrameEntry> FindEntryForAddress(uintptr_t address) const;

  size_t GetTableSizeForTesting() const { return function_addresses_.size(); }

 private:









  const span<const uint32_t> function_addresses_;
  const span<const uint16_t> entry_data_indices_;



  const span<const uint8_t> entry_data_;

  DISALLOW_COPY_AND_ASSIGN(ArmCFITable);
};

}  // namespace base

#endif  // BASE_PROFILER_ARM_CFI_TABLE_H_
