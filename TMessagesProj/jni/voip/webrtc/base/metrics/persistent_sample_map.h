// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// by the SparseHistogram class to store samples in persistent memory which
// allows it to be shared between processes or live across restarts.

#ifndef BASE_METRICS_PERSISTENT_SAMPLE_MAP_H_
#define BASE_METRICS_PERSISTENT_SAMPLE_MAP_H_

#include <stdint.h>

#include <map>
#include <memory>

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "base/metrics/histogram_base.h"
#include "base/metrics/histogram_samples.h"
#include "base/metrics/persistent_memory_allocator.h"

namespace base {

class PersistentHistogramAllocator;
class PersistentSampleMapRecords;

// structures. Changes here likely need to be duplicated there.
class BASE_EXPORT PersistentSampleMap : public HistogramSamples {
 public:


  PersistentSampleMap(uint64_t id,
                      PersistentHistogramAllocator* allocator,
                      Metadata* meta);

  ~PersistentSampleMap() override;

  void Accumulate(HistogramBase::Sample value,
                  HistogramBase::Count count) override;
  HistogramBase::Count GetCount(HistogramBase::Sample value) const override;
  HistogramBase::Count TotalCount() const override;
  std::unique_ptr<SampleCountIterator> Iterator() const override;



  static PersistentMemoryAllocator::Reference GetNextPersistentRecord(
      PersistentMemoryAllocator::Iterator& iterator,
      uint64_t* sample_map_id);


  static PersistentMemoryAllocator::Reference CreatePersistentRecord(
      PersistentMemoryAllocator* allocator,
      uint64_t sample_map_id,
      HistogramBase::Sample value);

 protected:

  bool AddSubtractImpl(SampleCountIterator* iter, Operator op) override;


  HistogramBase::Count* GetSampleCountStorage(HistogramBase::Sample value);


  HistogramBase::Count* GetOrCreateSampleCountStorage(
      HistogramBase::Sample value);

 private:


  PersistentSampleMapRecords* GetRecords();







  HistogramBase::Count* ImportSamples(HistogramBase::Sample until_value,
                                      bool import_everything);



  std::map<HistogramBase::Sample, HistogramBase::Count*> sample_counts_;


  PersistentHistogramAllocator* allocator_;




  PersistentSampleMapRecords* records_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(PersistentSampleMap);
};

}  // namespace base

#endif  // BASE_METRICS_PERSISTENT_SAMPLE_MAP_H_
