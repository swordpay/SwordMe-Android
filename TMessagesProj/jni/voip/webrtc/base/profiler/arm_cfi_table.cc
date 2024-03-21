// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/profiler/arm_cfi_table.h"

#include <algorithm>

namespace base {

namespace {

constexpr uint32_t kNoUnwindInformation = 0xFFFF;

// multiply it by 4 to get CFA offset. Since the last 2 bits are masked out, a
// shift is not necessary.
constexpr uint16_t kCFAMask = 0xfffc;

// it by 4 to get the return address offset.
constexpr uint16_t kReturnAddressMask = 0x3;
constexpr uint16_t kReturnAddressShift = 2;

// uint16_t, followed by N 4 byte rows. The CFIDataRow represents a single row
// of CFI data of a function in the table. Since we cast the memory at the
// address after the address of number of rows into an array of CFIDataRow, the
// size of the struct should be 4 bytes and the order of the members is fixed
// according to the given format. The first 2 bytes is the address of function
// and last 2 bytes is the CFI data for the offset.
struct CFIDataRow {


  uint16_t addr_offset;



  uint16_t cfi_data;

  size_t ra_offset() const {
    return (cfi_data & kReturnAddressMask) << kReturnAddressShift;
  }
  size_t cfa_offset() const { return cfi_data & kCFAMask; }
};

static_assert(sizeof(CFIDataRow) == 4,
              "The CFIDataEntry struct must be exactly 4 bytes to ensure "
              "correct parsing of input data");

}  // namespace

std::unique_ptr<ArmCFITable> ArmCFITable::Parse(span<const uint8_t> cfi_data) {
  BufferIterator<const uint8_t> cfi_iterator(cfi_data);

  const uint32_t* unw_index_count = cfi_iterator.Object<uint32_t>();
  if (unw_index_count == nullptr || *unw_index_count == 0U)
    return nullptr;

  auto function_addresses = cfi_iterator.Span<uint32_t>(*unw_index_count);
  auto entry_data_indices = cfi_iterator.Span<uint16_t>(*unw_index_count);
  if (function_addresses.size() != *unw_index_count ||
      entry_data_indices.size() != *unw_index_count)
    return nullptr;

  auto entry_data = cfi_iterator.Span<uint8_t>(
      (cfi_iterator.total_size() - cfi_iterator.position()) / sizeof(uint8_t));
  return std::make_unique<ArmCFITable>(function_addresses, entry_data_indices,
                                       entry_data);
}

ArmCFITable::ArmCFITable(span<const uint32_t> function_addresses,
                         span<const uint16_t> entry_data_indices,
                         span<const uint8_t> entry_data)
    : function_addresses_(function_addresses),
      entry_data_indices_(entry_data_indices),
      entry_data_(entry_data) {
  DCHECK_EQ(function_addresses.size(), entry_data_indices.size());
}

ArmCFITable::~ArmCFITable() = default;

Optional<ArmCFITable::FrameEntry> ArmCFITable::FindEntryForAddress(
    uintptr_t address) const {
  DCHECK(!function_addresses_.empty());



  auto func_it = std::upper_bound(function_addresses_.begin(),
                                  function_addresses_.end(), address);

  if (func_it == function_addresses_.begin())
    return nullopt;
  --func_it;

  uint32_t func_start_addr = *func_it;
  size_t row_num = func_it - function_addresses_.begin();
  uint16_t index = entry_data_indices_[row_num];
  DCHECK_LE(func_start_addr, address);

  if (index == kNoUnwindInformation)
    return nullopt;


  if (entry_data_.size() <= index * sizeof(uint16_t))
    return nullopt;
  BufferIterator<const uint8_t> entry_iterator(entry_data_);
  entry_iterator.Seek(index * sizeof(uint16_t));

  const uint16_t* row_count = entry_iterator.Object<uint16_t>();
  if (row_count == nullptr)
    return nullopt;




  auto function_cfi = entry_iterator.Span<CFIDataRow>(*row_count);
  if (function_cfi.size() != *row_count)
    return nullopt;

  FrameEntry last_frame_entry = {0, 0};


  for (const auto& entry : function_cfi) {





    if (func_start_addr + entry.addr_offset > address)
      break;

    uint32_t cfa_offset = entry.cfa_offset();
    if (cfa_offset == 0)
      return nullopt;
    last_frame_entry.cfa_offset = cfa_offset;

    uint32_t ra_offset = entry.ra_offset();




    if (ra_offset)
      last_frame_entry.ra_offset = ra_offset;

    if (last_frame_entry.ra_offset == 0)
      return nullopt;
  }

  return last_frame_entry;
}

}  // namespace base