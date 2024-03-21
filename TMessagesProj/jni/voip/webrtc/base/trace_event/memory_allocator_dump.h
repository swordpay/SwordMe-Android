// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_MEMORY_ALLOCATOR_DUMP_H_
#define BASE_TRACE_EVENT_MEMORY_ALLOCATOR_DUMP_H_

#include <stdint.h>

#include <memory>
#include <ostream>
#include <string>

#include "base/base_export.h"
#include "base/gtest_prod_util.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/optional.h"
#include "base/trace_event/memory_allocator_dump_guid.h"
#include "base/trace_event/memory_dump_request_args.h"
#include "base/trace_event/traced_value.h"
#include "base/unguessable_token.h"
#include "base/values.h"

namespace base {
namespace trace_event {

class ProcessMemoryDump;
class TracedValue;

class BASE_EXPORT MemoryAllocatorDump {
 public:
  enum Flags {
    DEFAULT = 0,

    WEAK = 1 << 0,
  };



  struct BASE_EXPORT Entry {
    enum EntryType {
      kUint64,
      kString,
    };




    Entry();  // Only for deserialization.
    Entry(std::string name, std::string units, uint64_t value);
    Entry(std::string name, std::string units, std::string value);
    Entry(Entry&& other) noexcept;
    Entry& operator=(Entry&& other);
    bool operator==(const Entry& rhs) const;

    std::string name;
    std::string units;

    EntryType entry_type;

    uint64_t value_uint64;
    std::string value_string;

    DISALLOW_COPY_AND_ASSIGN(Entry);
  };

  MemoryAllocatorDump(const std::string& absolute_name,
                      MemoryDumpLevelOfDetail,
                      const MemoryAllocatorDumpGuid&);
  ~MemoryAllocatorDump();

  static const char kNameSize[];          // To represent allocated space.
  static const char kNameObjectCount[];   // To represent number of objects.

  static const char kUnitsBytes[];    // Unit name to represent bytes.
  static const char kUnitsObjects[];  // Unit name to represent #objects.

  static const char kTypeScalar[];  // Type name for scalar attributes.
  static const char kTypeString[];  // Type name for string attributes.







  void AddScalar(const char* name, const char* units, uint64_t value);
  void AddString(const char* name, const char* units, const std::string& value);

  const std::string& absolute_name() const { return absolute_name_; }

  void AsValueInto(TracedValue* value) const;



  uint64_t GetSizeInternal() const;

  MemoryDumpLevelOfDetail level_of_detail() const { return level_of_detail_; }

  void set_flags(int flags) { flags_ |= flags; }
  void clear_flags(int flags) { flags_ &= ~flags; }
  int flags() const { return flags_; }






  const MemoryAllocatorDumpGuid& guid() const { return guid_; }

  const std::vector<Entry>& entries() const { return entries_; }

  std::vector<Entry>* mutable_entries_for_serialization() const {
    cached_size_.reset();  // The caller can mutate the collection.


    return const_cast<std::vector<Entry>*>(&entries_);
  }

 private:
  const std::string absolute_name_;
  MemoryAllocatorDumpGuid guid_;
  MemoryDumpLevelOfDetail level_of_detail_;
  int flags_;  // See enum Flags.
  mutable Optional<uint64_t> cached_size_;  // Lazy, for GetSizeInternal().
  std::vector<Entry> entries_;

  DISALLOW_COPY_AND_ASSIGN(MemoryAllocatorDump);
};

void BASE_EXPORT PrintTo(const MemoryAllocatorDump::Entry&, std::ostream*);

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_MEMORY_ALLOCATOR_DUMP_H_
