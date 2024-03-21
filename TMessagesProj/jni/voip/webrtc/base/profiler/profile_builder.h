// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PROFILER_PROFILE_BUILDER_H_
#define BASE_PROFILER_PROFILE_BUILDER_H_

#include <memory>

#include "base/base_export.h"
#include "base/optional.h"
#include "base/profiler/frame.h"
#include "base/profiler/module_cache.h"
#include "base/time/time.h"

namespace base {

// the fly in whatever format is desired. Functions are invoked by the profiler
// on its own thread so must not block or perform expensive operations.
class BASE_EXPORT ProfileBuilder {
 public:
  ProfileBuilder() = default;
  virtual ~ProfileBuilder() = default;


  virtual ModuleCache* GetModuleCache() = 0;

  struct BASE_EXPORT MetadataItem {
    MetadataItem(uint64_t name_hash, Optional<int64_t> key, int64_t value);
    MetadataItem();

    MetadataItem(const MetadataItem& other);
    MetadataItem& operator=(const MetadataItem& other);

    uint64_t name_hash;

    Optional<int64_t> key;

    int64_t value;
  };

  static constexpr size_t MAX_METADATA_COUNT = 50;
  typedef std::array<MetadataItem, MAX_METADATA_COUNT> MetadataItemArray;

  class MetadataProvider {
   public:
    MetadataProvider() = default;
    virtual ~MetadataProvider() = default;

    virtual size_t GetItems(ProfileBuilder::MetadataItemArray* const items) = 0;
  };






  virtual void RecordMetadata(MetadataProvider* metadata_provider) {}






  virtual void ApplyMetadataRetrospectively(TimeTicks period_start,
                                            TimeTicks period_end,
                                            const MetadataItem& item) {}

  virtual void OnSampleCompleted(std::vector<Frame> frames,
                                 TimeTicks sample_timestamp) = 0;


  virtual void OnProfileCompleted(TimeDelta profile_duration,
                                  TimeDelta sampling_period) = 0;

 private:
  DISALLOW_COPY_AND_ASSIGN(ProfileBuilder);
};

}  // namespace base

#endif  // BASE_PROFILER_PROFILE_BUILDER_H_
