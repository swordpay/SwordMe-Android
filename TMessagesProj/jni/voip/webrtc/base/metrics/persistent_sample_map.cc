// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/metrics/persistent_sample_map.h"

#include "base/logging.h"
#include "base/metrics/histogram_macros.h"
#include "base/metrics/persistent_histogram_allocator.h"
#include "base/numerics/safe_conversions.h"
#include "base/stl_util.h"

namespace base {

typedef HistogramBase::Count Count;
typedef HistogramBase::Sample Sample;

namespace {

// identical to that of SampleMapIterator but with different data structures.
// Changes here likely need to be duplicated there.
class PersistentSampleMapIterator : public SampleCountIterator {
 public:
  typedef std::map<HistogramBase::Sample, HistogramBase::Count*>
      SampleToCountMap;

  explicit PersistentSampleMapIterator(const SampleToCountMap& sample_counts);
  ~PersistentSampleMapIterator() override;

  bool Done() const override;
  void Next() override;
  void Get(HistogramBase::Sample* min,
           int64_t* max,
           HistogramBase::Count* count) const override;

 private:
  void SkipEmptyBuckets();

  SampleToCountMap::const_iterator iter_;
  const SampleToCountMap::const_iterator end_;
};

PersistentSampleMapIterator::PersistentSampleMapIterator(
    const SampleToCountMap& sample_counts)
    : iter_(sample_counts.begin()),
      end_(sample_counts.end()) {
  SkipEmptyBuckets();
}

PersistentSampleMapIterator::~PersistentSampleMapIterator() = default;

bool PersistentSampleMapIterator::Done() const {
  return iter_ == end_;
}

void PersistentSampleMapIterator::Next() {
  DCHECK(!Done());
  ++iter_;
  SkipEmptyBuckets();
}

void PersistentSampleMapIterator::Get(Sample* min,
                                      int64_t* max,
                                      Count* count) const {
  DCHECK(!Done());
  if (min)
    *min = iter_->first;
  if (max)
    *max = strict_cast<int64_t>(iter_->first) + 1;
  if (count)
    *count = *iter_->second;
}

void PersistentSampleMapIterator::SkipEmptyBuckets() {
  while (!Done() && *iter_->second == 0) {
    ++iter_;
  }
}

// memory allocator. The "id" must be unique across all maps held by an
// allocator or they will get attached to the wrong sample map.
struct SampleRecord {

  static constexpr uint32_t kPersistentTypeId = 0x8FE6A69F + 1;

  static constexpr size_t kExpectedInstanceSize = 16;

  uint64_t id;   // Unique identifier of owner.
  Sample value;  // The value for which this record holds a count.
  Count count;   // The count associated with the above value.
};

}  // namespace

PersistentSampleMap::PersistentSampleMap(
    uint64_t id,
    PersistentHistogramAllocator* allocator,
    Metadata* meta)
    : HistogramSamples(id, meta), allocator_(allocator) {}

PersistentSampleMap::~PersistentSampleMap() {
  if (records_)
    records_->Release(this);
}

void PersistentSampleMap::Accumulate(Sample value, Count count) {
#if 0  // TODO(bcwhite) Re-enable efficient version after crbug.com/682680.
  *GetOrCreateSampleCountStorage(value) += count;
#else
  Count* local_count_ptr = GetOrCreateSampleCountStorage(value);
  if (count < 0) {
    if (*local_count_ptr < -count)
      RecordNegativeSample(SAMPLES_ACCUMULATE_WENT_NEGATIVE, -count);
    else
      RecordNegativeSample(SAMPLES_ACCUMULATE_NEGATIVE_COUNT, -count);
    *local_count_ptr += count;
  } else {
    Sample old_value = *local_count_ptr;
    Sample new_value = old_value + count;
    *local_count_ptr = new_value;
    if ((new_value >= 0) != (old_value >= 0))
      RecordNegativeSample(SAMPLES_ACCUMULATE_OVERFLOW, count);
  }
#endif
  IncreaseSumAndCount(strict_cast<int64_t>(count) * value, count);
}

Count PersistentSampleMap::GetCount(Sample value) const {


  Count* count_pointer =
      const_cast<PersistentSampleMap*>(this)->GetSampleCountStorage(value);
  return count_pointer ? *count_pointer : 0;
}

Count PersistentSampleMap::TotalCount() const {


  const_cast<PersistentSampleMap*>(this)->ImportSamples(-1, true);

  Count count = 0;
  for (const auto& entry : sample_counts_) {
    count += *entry.second;
  }
  return count;
}

std::unique_ptr<SampleCountIterator> PersistentSampleMap::Iterator() const {


  const_cast<PersistentSampleMap*>(this)->ImportSamples(-1, true);
  return std::make_unique<PersistentSampleMapIterator>(sample_counts_);
}

PersistentMemoryAllocator::Reference
PersistentSampleMap::GetNextPersistentRecord(
    PersistentMemoryAllocator::Iterator& iterator,
    uint64_t* sample_map_id) {
  const SampleRecord* record = iterator.GetNextOfObject<SampleRecord>();
  if (!record)
    return 0;

  *sample_map_id = record->id;
  return iterator.GetAsReference(record);
}

PersistentMemoryAllocator::Reference
PersistentSampleMap::CreatePersistentRecord(
    PersistentMemoryAllocator* allocator,
    uint64_t sample_map_id,
    Sample value) {
  SampleRecord* record = allocator->New<SampleRecord>();
  if (!record) {
    NOTREACHED() << "full=" << allocator->IsFull()
                 << ", corrupt=" << allocator->IsCorrupt();
    return 0;
  }

  record->id = sample_map_id;
  record->value = value;
  record->count = 0;

  PersistentMemoryAllocator::Reference ref = allocator->GetAsReference(record);
  allocator->MakeIterable(ref);
  return ref;
}

bool PersistentSampleMap::AddSubtractImpl(SampleCountIterator* iter,
                                          Operator op) {
  Sample min;
  int64_t max;
  Count count;
  for (; !iter->Done(); iter->Next()) {
    iter->Get(&min, &max, &count);
    if (count == 0)
      continue;
    if (strict_cast<int64_t>(min) + 1 != max)
      return false;  // SparseHistogram only supports bucket with size 1.
    *GetOrCreateSampleCountStorage(min) +=
        (op == HistogramSamples::ADD) ? count : -count;
  }
  return true;
}

Count* PersistentSampleMap::GetSampleCountStorage(Sample value) {

  auto it = sample_counts_.find(value);
  if (it != sample_counts_.end())
    return it->second;

  return ImportSamples(value, false);
}

Count* PersistentSampleMap::GetOrCreateSampleCountStorage(Sample value) {

  Count* count_pointer = GetSampleCountStorage(value);
  if (count_pointer)
    return count_pointer;


  DCHECK(records_);
  PersistentMemoryAllocator::Reference ref = records_->CreateNew(value);
  if (!ref) {




    count_pointer = new Count(0);
    sample_counts_[value] = count_pointer;
    return count_pointer;
  }









  count_pointer = ImportSamples(value, false);
  DCHECK(count_pointer);
  return count_pointer;
}

PersistentSampleMapRecords* PersistentSampleMap::GetRecords() {





  if (!records_)
    records_ = allocator_->UseSampleMapRecords(id(), this);
  return records_;
}

Count* PersistentSampleMap::ImportSamples(Sample until_value,
                                          bool import_everything) {
  Count* found_count = nullptr;
  PersistentMemoryAllocator::Reference ref;
  PersistentSampleMapRecords* records = GetRecords();
  while ((ref = records->GetNext()) != 0) {
    SampleRecord* record = records->GetAsObject<SampleRecord>(ref);
    if (!record)
      continue;

    DCHECK_EQ(id(), record->id);

    if (!Contains(sample_counts_, record->value)) {

      sample_counts_[record->value] = &record->count;
    } else {



      DCHECK_EQ(0, record->count);
    }




    if (record->value == until_value) {
      if (!found_count)
        found_count = &record->count;
      if (!import_everything)
        break;
    }
  }

  return found_count;
}

}  // namespace base
