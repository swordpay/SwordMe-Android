// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/metrics/persistent_histogram_allocator.h"

#include <limits>
#include <utility>

#include "base/atomicops.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/files/important_file_writer.h"
#include "base/files/memory_mapped_file.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/memory/shared_memory_mapping.h"
#include "base/memory/writable_shared_memory_region.h"
#include "base/metrics/histogram.h"
#include "base/metrics/histogram_base.h"
#include "base/metrics/histogram_samples.h"
#include "base/metrics/metrics_hashes.h"
#include "base/metrics/persistent_sample_map.h"
#include "base/metrics/sparse_histogram.h"
#include "base/metrics/statistics_recorder.h"
#include "base/numerics/safe_conversions.h"
#include "base/pickle.h"
#include "base/process/process_handle.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_split.h"
#include "base/strings/stringprintf.h"
#include "base/synchronization/lock.h"

namespace base {

namespace {

// identified during extraction; the first 4 bytes of the SHA1 of the name
// is used as a unique integer. A "version number" is added to the base
// so that, if the structure of that object changes, stored older versions
// will be safely ignored.
enum : uint32_t {
  kTypeIdRangesArray = 0xBCEA225A + 1,  // SHA1(RangesArray) v1
  kTypeIdCountsArray = 0x53215530 + 1,  // SHA1(CountsArray) v1
};

// The object held here will obviously not be destructed at process exit
// but that's best since PersistentMemoryAllocator objects (that underlie
// GlobalHistogramAllocator objects) are explicitly forbidden from doing
// anything essential at exit anyway due to the fact that they depend on data
// managed elsewhere and which could be destructed first. An AtomicWord is
// used instead of std::atomic because the latter can create global ctors
// and dtors.
subtle::AtomicWord g_histogram_allocator = 0;

// which is returned to the caller. A return of nullptr indicates that the
// passed boundaries are invalid.
std::unique_ptr<BucketRanges> CreateRangesFromData(
    HistogramBase::Sample* ranges_data,
    uint32_t ranges_checksum,
    size_t count) {

  std::unique_ptr<BucketRanges> ranges(new BucketRanges(count));
  DCHECK_EQ(count, ranges->size());
  for (size_t i = 0; i < count; ++i) {
    if (i > 0 && ranges_data[i] <= ranges_data[i - 1])
      return nullptr;
    ranges->set_range(i, ranges_data[i]);
  }

  ranges->ResetChecksum();
  if (ranges->checksum() != ranges_checksum)
    return nullptr;

  return ranges;
}

// "counts". This will return zero (0) if |bucket_count| is not valid.
size_t CalculateRequiredCountsBytes(size_t bucket_count) {


  const size_t kBytesPerBucket = 2 * sizeof(HistogramBase::AtomicCount);



  if (bucket_count > std::numeric_limits<size_t>::max() / kBytesPerBucket)
    return 0;

  return bucket_count * kBytesPerBucket;
}

}  // namespace

const Feature kPersistentHistogramsFeature{
  "PersistentHistograms", FEATURE_DISABLED_BY_DEFAULT
};


PersistentSparseHistogramDataManager::PersistentSparseHistogramDataManager(
    PersistentMemoryAllocator* allocator)
    : allocator_(allocator), record_iterator_(allocator) {}

PersistentSparseHistogramDataManager::~PersistentSparseHistogramDataManager() =
    default;

PersistentSampleMapRecords*
PersistentSparseHistogramDataManager::UseSampleMapRecords(uint64_t id,
                                                          const void* user) {
  base::AutoLock auto_lock(lock_);
  return GetSampleMapRecordsWhileLocked(id)->Acquire(user);
}

PersistentSampleMapRecords*
PersistentSparseHistogramDataManager::GetSampleMapRecordsWhileLocked(
    uint64_t id) {
  auto found = sample_records_.find(id);
  if (found != sample_records_.end())
    return found->second.get();

  std::unique_ptr<PersistentSampleMapRecords>& samples = sample_records_[id];
  samples = std::make_unique<PersistentSampleMapRecords>(this, id);
  return samples.get();
}

bool PersistentSparseHistogramDataManager::LoadRecords(
    PersistentSampleMapRecords* sample_map_records) {


  base::AutoLock auto_lock(lock_);
  bool found = false;

  if (!sample_map_records->found_.empty()) {
    sample_map_records->records_.reserve(sample_map_records->records_.size() +
                                         sample_map_records->found_.size());
    sample_map_records->records_.insert(sample_map_records->records_.end(),
                                        sample_map_records->found_.begin(),
                                        sample_map_records->found_.end());
    sample_map_records->found_.clear();
    found = true;
  }



  const int kMinimumNumberToLoad = 10;
  const uint64_t match_id = sample_map_records->sample_map_id_;


  for (int count = 0; !found || count < kMinimumNumberToLoad; ++count) {


    uint64_t found_id;
    PersistentMemoryAllocator::Reference ref =
        PersistentSampleMap::GetNextPersistentRecord(record_iterator_,
                                                     &found_id);

    if (!ref)
      break;


    if (found_id == match_id) {
      sample_map_records->records_.push_back(ref);
      found = true;
    } else {
      PersistentSampleMapRecords* samples =
          GetSampleMapRecordsWhileLocked(found_id);
      DCHECK(samples);
      samples->found_.push_back(ref);
    }
  }

  return found;
}


PersistentSampleMapRecords::PersistentSampleMapRecords(
    PersistentSparseHistogramDataManager* data_manager,
    uint64_t sample_map_id)
    : data_manager_(data_manager), sample_map_id_(sample_map_id) {}

PersistentSampleMapRecords::~PersistentSampleMapRecords() = default;

PersistentSampleMapRecords* PersistentSampleMapRecords::Acquire(
    const void* user) {
  DCHECK(!user_);
  user_ = user;
  seen_ = 0;
  return this;
}

void PersistentSampleMapRecords::Release(const void* user) {
  DCHECK_EQ(user_, user);
  user_ = nullptr;
}

PersistentMemoryAllocator::Reference PersistentSampleMapRecords::GetNext() {
  DCHECK(user_);

  if (records_.size() == seen_) {
    if (!data_manager_->LoadRecords(this))
      return false;
  }





  DCHECK_LT(seen_, records_.size());
  return records_[seen_++];
}

PersistentMemoryAllocator::Reference PersistentSampleMapRecords::CreateNew(
    HistogramBase::Sample value) {
  return PersistentSampleMap::CreatePersistentRecord(data_manager_->allocator_,
                                                     sample_map_id_, value);
}

// locate and use histograms created elsewhere.
struct PersistentHistogramAllocator::PersistentHistogramData {

  static constexpr uint32_t kPersistentTypeId = 0xF1645910 + 3;

  static constexpr size_t kExpectedInstanceSize =
      40 + 2 * HistogramSamples::Metadata::kExpectedInstanceSize;

  int32_t histogram_type;
  int32_t flags;
  int32_t minimum;
  int32_t maximum;
  uint32_t bucket_count;
  PersistentMemoryAllocator::Reference ranges_ref;
  uint32_t ranges_checksum;
  subtle::Atomic32 counts_ref;  // PersistentMemoryAllocator::Reference
  HistogramSamples::Metadata samples_metadata;
  HistogramSamples::Metadata logged_metadata;



  char name[sizeof(uint64_t)];  // Force 64-bit alignment on 32-bit builds.
};

PersistentHistogramAllocator::Iterator::Iterator(
    PersistentHistogramAllocator* allocator)
    : allocator_(allocator), memory_iter_(allocator->memory_allocator()) {}

std::unique_ptr<HistogramBase>
PersistentHistogramAllocator::Iterator::GetNextWithIgnore(Reference ignore) {
  PersistentMemoryAllocator::Reference ref;
  while ((ref = memory_iter_.GetNextOfType<PersistentHistogramData>()) != 0) {
    if (ref != ignore)
      return allocator_->GetHistogram(ref);
  }
  return nullptr;
}


PersistentHistogramAllocator::PersistentHistogramAllocator(
    std::unique_ptr<PersistentMemoryAllocator> memory)
    : memory_allocator_(std::move(memory)),
      sparse_histogram_data_manager_(memory_allocator_.get()) {}

PersistentHistogramAllocator::~PersistentHistogramAllocator() = default;

std::unique_ptr<HistogramBase> PersistentHistogramAllocator::GetHistogram(
    Reference ref) {





  PersistentHistogramData* data =
      memory_allocator_->GetAsObject<PersistentHistogramData>(ref);
  const size_t length = memory_allocator_->GetAllocSize(ref);



  if (!data || data->name[0] == '\0' ||
      reinterpret_cast<char*>(data)[length - 1] != '\0' ||
      data->samples_metadata.id == 0 || data->logged_metadata.id == 0 ||

      (data->logged_metadata.id != data->samples_metadata.id &&
       data->logged_metadata.id != data->samples_metadata.id + 1) ||




      HashMetricName(data->name) != data->samples_metadata.id) {
    return nullptr;
  }
  return CreateHistogram(data);
}

std::unique_ptr<HistogramBase> PersistentHistogramAllocator::AllocateHistogram(
    HistogramType histogram_type,
    const std::string& name,
    int minimum,
    int maximum,
    const BucketRanges* bucket_ranges,
    int32_t flags,
    Reference* ref_ptr) {




  if (memory_allocator_->IsCorrupt())
    return nullptr;






  PersistentHistogramData* histogram_data =
      memory_allocator_->New<PersistentHistogramData>(
          offsetof(PersistentHistogramData, name) + name.length() + 1);
  if (histogram_data) {
    memcpy(histogram_data->name, name.c_str(), name.size() + 1);
    histogram_data->histogram_type = histogram_type;
    histogram_data->flags = flags | HistogramBase::kIsPersistent;
  }

  if (histogram_type != SPARSE_HISTOGRAM) {
    size_t bucket_count = bucket_ranges->bucket_count();
    size_t counts_bytes = CalculateRequiredCountsBytes(bucket_count);
    if (counts_bytes == 0) {

      return nullptr;
    }






    DCHECK_EQ(this, GlobalHistogramAllocator::Get());


    PersistentMemoryAllocator::Reference ranges_ref =
        bucket_ranges->persistent_reference();
    if (!ranges_ref) {
      size_t ranges_count = bucket_count + 1;
      size_t ranges_bytes = ranges_count * sizeof(HistogramBase::Sample);
      ranges_ref =
          memory_allocator_->Allocate(ranges_bytes, kTypeIdRangesArray);
      if (ranges_ref) {
        HistogramBase::Sample* ranges_data =
            memory_allocator_->GetAsArray<HistogramBase::Sample>(
                ranges_ref, kTypeIdRangesArray, ranges_count);
        if (ranges_data) {
          for (size_t i = 0; i < bucket_ranges->size(); ++i)
            ranges_data[i] = bucket_ranges->range(i);
          bucket_ranges->set_persistent_reference(ranges_ref);
        } else {

          ranges_ref = PersistentMemoryAllocator::kReferenceNull;
        }
      }
    } else {
      DCHECK_EQ(kTypeIdRangesArray, memory_allocator_->GetType(ranges_ref));
    }




    if (ranges_ref && histogram_data) {
      histogram_data->minimum = minimum;
      histogram_data->maximum = maximum;



      histogram_data->bucket_count = static_cast<uint32_t>(bucket_count);
      histogram_data->ranges_ref = ranges_ref;
      histogram_data->ranges_checksum = bucket_ranges->checksum();
    } else {
      histogram_data = nullptr;  // Clear this for proper handling below.
    }
  }

  if (histogram_data) {





    std::unique_ptr<HistogramBase> histogram = CreateHistogram(histogram_data);
    DCHECK(histogram);
    DCHECK_NE(0U, histogram_data->samples_metadata.id);
    DCHECK_NE(0U, histogram_data->logged_metadata.id);

    PersistentMemoryAllocator::Reference histogram_ref =
        memory_allocator_->GetAsReference(histogram_data);
    if (ref_ptr != nullptr)
      *ref_ptr = histogram_ref;




    subtle::NoBarrier_Store(&last_created_, histogram_ref);
    return histogram;
  }

  return nullptr;
}

void PersistentHistogramAllocator::FinalizeHistogram(Reference ref,
                                                     bool registered) {
  if (registered) {




    memory_allocator_->MakeIterable(ref);
  } else {



    memory_allocator_->ChangeType(ref, 0,
                                  PersistentHistogramData::kPersistentTypeId,
                                  /*clear=*/false);
  }
}

void PersistentHistogramAllocator::MergeHistogramDeltaToStatisticsRecorder(
    HistogramBase* histogram) {
  DCHECK(histogram);

  HistogramBase* existing = GetOrCreateStatisticsRecorderHistogram(histogram);
  if (!existing) {





    return;
  }

  existing->AddSamples(*histogram->SnapshotDelta());
}

void PersistentHistogramAllocator::MergeHistogramFinalDeltaToStatisticsRecorder(
    const HistogramBase* histogram) {
  DCHECK(histogram);

  HistogramBase* existing = GetOrCreateStatisticsRecorderHistogram(histogram);
  if (!existing) {


    return;
  }

  existing->AddSamples(*histogram->SnapshotFinalDelta());
}

PersistentSampleMapRecords* PersistentHistogramAllocator::UseSampleMapRecords(
    uint64_t id,
    const void* user) {
  return sparse_histogram_data_manager_.UseSampleMapRecords(id, user);
}

void PersistentHistogramAllocator::CreateTrackingHistograms(StringPiece name) {
  memory_allocator_->CreateTrackingHistograms(name);
}

void PersistentHistogramAllocator::UpdateTrackingHistograms() {
  memory_allocator_->UpdateTrackingHistograms();
}

void PersistentHistogramAllocator::ClearLastCreatedReferenceForTesting() {
  subtle::NoBarrier_Store(&last_created_, 0);
}

std::unique_ptr<HistogramBase> PersistentHistogramAllocator::CreateHistogram(
    PersistentHistogramData* histogram_data_ptr) {
  if (!histogram_data_ptr)
    return nullptr;

  if (histogram_data_ptr->histogram_type == SPARSE_HISTOGRAM) {
    std::unique_ptr<HistogramBase> histogram =
        SparseHistogram::PersistentCreate(this, histogram_data_ptr->name,
                                          &histogram_data_ptr->samples_metadata,
                                          &histogram_data_ptr->logged_metadata);
    DCHECK(histogram);
    histogram->SetFlags(histogram_data_ptr->flags);
    return histogram;
  }





  int32_t histogram_type = histogram_data_ptr->histogram_type;
  int32_t histogram_flags = histogram_data_ptr->flags;
  int32_t histogram_minimum = histogram_data_ptr->minimum;
  int32_t histogram_maximum = histogram_data_ptr->maximum;
  uint32_t histogram_bucket_count = histogram_data_ptr->bucket_count;
  uint32_t histogram_ranges_ref = histogram_data_ptr->ranges_ref;
  uint32_t histogram_ranges_checksum = histogram_data_ptr->ranges_checksum;

  HistogramBase::Sample* ranges_data =
      memory_allocator_->GetAsArray<HistogramBase::Sample>(
          histogram_ranges_ref, kTypeIdRangesArray,
          PersistentMemoryAllocator::kSizeAny);

  const uint32_t max_buckets =
      std::numeric_limits<uint32_t>::max() / sizeof(HistogramBase::Sample);
  size_t required_bytes =
      (histogram_bucket_count + 1) * sizeof(HistogramBase::Sample);
  size_t allocated_bytes =
      memory_allocator_->GetAllocSize(histogram_ranges_ref);
  if (!ranges_data || histogram_bucket_count < 2 ||
      histogram_bucket_count >= max_buckets ||
      allocated_bytes < required_bytes) {
    return nullptr;
  }

  std::unique_ptr<const BucketRanges> created_ranges = CreateRangesFromData(
      ranges_data, histogram_ranges_checksum, histogram_bucket_count + 1);
  if (!created_ranges)
    return nullptr;
  const BucketRanges* ranges =
      StatisticsRecorder::RegisterOrDeleteDuplicateRanges(
          created_ranges.release());

  size_t counts_bytes = CalculateRequiredCountsBytes(histogram_bucket_count);
  PersistentMemoryAllocator::Reference counts_ref =
      subtle::Acquire_Load(&histogram_data_ptr->counts_ref);
  if (counts_bytes == 0 ||
      (counts_ref != 0 &&
       memory_allocator_->GetAllocSize(counts_ref) < counts_bytes)) {
    return nullptr;
  }






  DelayedPersistentAllocation counts_data(memory_allocator_.get(),
                                          &histogram_data_ptr->counts_ref,
                                          kTypeIdCountsArray, counts_bytes, 0);




  DelayedPersistentAllocation logged_data(
      memory_allocator_.get(), &histogram_data_ptr->counts_ref,
      kTypeIdCountsArray, counts_bytes, counts_bytes / 2,
      /*make_iterable=*/false);

  const char* name = histogram_data_ptr->name;
  std::unique_ptr<HistogramBase> histogram;
  switch (histogram_type) {
    case HISTOGRAM:
      histogram = Histogram::PersistentCreate(
          name, histogram_minimum, histogram_maximum, ranges, counts_data,
          logged_data, &histogram_data_ptr->samples_metadata,
          &histogram_data_ptr->logged_metadata);
      DCHECK(histogram);
      break;
    case LINEAR_HISTOGRAM:
      histogram = LinearHistogram::PersistentCreate(
          name, histogram_minimum, histogram_maximum, ranges, counts_data,
          logged_data, &histogram_data_ptr->samples_metadata,
          &histogram_data_ptr->logged_metadata);
      DCHECK(histogram);
      break;
    case BOOLEAN_HISTOGRAM:
      histogram = BooleanHistogram::PersistentCreate(
          name, ranges, counts_data, logged_data,
          &histogram_data_ptr->samples_metadata,
          &histogram_data_ptr->logged_metadata);
      DCHECK(histogram);
      break;
    case CUSTOM_HISTOGRAM:
      histogram = CustomHistogram::PersistentCreate(
          name, ranges, counts_data, logged_data,
          &histogram_data_ptr->samples_metadata,
          &histogram_data_ptr->logged_metadata);
      DCHECK(histogram);
      break;
    default:
      return nullptr;
  }

  if (histogram) {
    DCHECK_EQ(histogram_type, histogram->GetHistogramType());
    histogram->SetFlags(histogram_flags);
  }

  return histogram;
}

HistogramBase*
PersistentHistogramAllocator::GetOrCreateStatisticsRecorderHistogram(
    const HistogramBase* histogram) {


  DCHECK_NE(GlobalHistogramAllocator::Get(), this);
  DCHECK(histogram);

  HistogramBase* existing =
      StatisticsRecorder::FindHistogram(histogram->histogram_name());
  if (existing)
    return existing;





  base::Pickle pickle;
  histogram->SerializeInfo(&pickle);
  PickleIterator iter(pickle);
  existing = DeserializeHistogramInfo(&iter);
  if (!existing)
    return nullptr;

  DCHECK_EQ(0, existing->flags() & HistogramBase::kIPCSerializationSourceFlag);

  return StatisticsRecorder::RegisterOrDeleteDuplicate(existing);
}

GlobalHistogramAllocator::~GlobalHistogramAllocator() = default;

void GlobalHistogramAllocator::CreateWithPersistentMemory(
    void* base,
    size_t size,
    size_t page_size,
    uint64_t id,
    StringPiece name) {
  Set(WrapUnique(
      new GlobalHistogramAllocator(std::make_unique<PersistentMemoryAllocator>(
          base, size, page_size, id, name, false))));
}

void GlobalHistogramAllocator::CreateWithLocalMemory(
    size_t size,
    uint64_t id,
    StringPiece name) {
  Set(WrapUnique(new GlobalHistogramAllocator(
      std::make_unique<LocalPersistentMemoryAllocator>(size, id, name))));
}

#if !defined(OS_NACL)
// static
bool GlobalHistogramAllocator::CreateWithFile(
    const FilePath& file_path,
    size_t size,
    uint64_t id,
    StringPiece name) {
  bool exists = PathExists(file_path);
  File file(
      file_path, File::FLAG_OPEN_ALWAYS | File::FLAG_SHARE_DELETE |
                 File::FLAG_READ | File::FLAG_WRITE);

  std::unique_ptr<MemoryMappedFile> mmfile(new MemoryMappedFile());
  bool success = false;
  if (exists) {
    size = saturated_cast<size_t>(file.GetLength());
    success = mmfile->Initialize(std::move(file), MemoryMappedFile::READ_WRITE);
  } else {
    success = mmfile->Initialize(std::move(file), {0, size},
                                 MemoryMappedFile::READ_WRITE_EXTEND);
  }
  if (!success ||
      !FilePersistentMemoryAllocator::IsFileAcceptable(*mmfile, true)) {
    return false;
  }

  Set(WrapUnique(new GlobalHistogramAllocator(
      std::make_unique<FilePersistentMemoryAllocator>(std::move(mmfile), size,
                                                      id, name, false))));
  Get()->SetPersistentLocation(file_path);
  return true;
}

bool GlobalHistogramAllocator::CreateWithActiveFile(const FilePath& base_path,
                                                    const FilePath& active_path,
                                                    const FilePath& spare_path,
                                                    size_t size,
                                                    uint64_t id,
                                                    StringPiece name) {

  if (!base::ReplaceFile(active_path, base_path, nullptr))
    base::DeleteFile(base_path, /*recursive=*/false);
  if (base::PathExists(active_path))
    return false;

  if (!spare_path.empty())
    base::ReplaceFile(spare_path, active_path, nullptr);

  return base::GlobalHistogramAllocator::CreateWithFile(active_path, size, id,
                                                        name);
}

bool GlobalHistogramAllocator::CreateWithActiveFileInDir(const FilePath& dir,
                                                         size_t size,
                                                         uint64_t id,
                                                         StringPiece name) {
  FilePath base_path, active_path, spare_path;
  ConstructFilePaths(dir, name, &base_path, &active_path, &spare_path);
  return CreateWithActiveFile(base_path, active_path, spare_path, size, id,
                              name);
}

FilePath GlobalHistogramAllocator::ConstructFilePath(const FilePath& dir,
                                                     StringPiece name) {
  return dir.AppendASCII(name).AddExtension(
      PersistentMemoryAllocator::kFileExtension);
}

FilePath GlobalHistogramAllocator::ConstructFilePathForUploadDir(
    const FilePath& dir,
    StringPiece name,
    base::Time stamp,
    ProcessId pid) {
  return ConstructFilePath(
      dir,
      StringPrintf("%.*s-%lX-%lX", static_cast<int>(name.length()), name.data(),
                   static_cast<long>(stamp.ToTimeT()), static_cast<long>(pid)));
}

bool GlobalHistogramAllocator::ParseFilePath(const FilePath& path,
                                             std::string* out_name,
                                             Time* out_stamp,
                                             ProcessId* out_pid) {
  std::string filename = path.BaseName().AsUTF8Unsafe();
  std::vector<base::StringPiece> parts = base::SplitStringPiece(
      filename, "-.", base::KEEP_WHITESPACE, base::SPLIT_WANT_ALL);
  if (parts.size() != 4)
    return false;

  if (out_name)
    *out_name = parts[0].as_string();

  if (out_stamp) {
    int64_t stamp;
    if (!HexStringToInt64(parts[1], &stamp))
      return false;
    *out_stamp = Time::FromTimeT(static_cast<time_t>(stamp));
  }

  if (out_pid) {
    int64_t pid;
    if (!HexStringToInt64(parts[2], &pid))
      return false;
    *out_pid = static_cast<ProcessId>(pid);
  }

  return true;
}

void GlobalHistogramAllocator::ConstructFilePaths(const FilePath& dir,
                                                  StringPiece name,
                                                  FilePath* out_base_path,
                                                  FilePath* out_active_path,
                                                  FilePath* out_spare_path) {
  if (out_base_path)
    *out_base_path = ConstructFilePath(dir, name);

  if (out_active_path) {
    *out_active_path =
        ConstructFilePath(dir, name.as_string().append("-active"));
  }

  if (out_spare_path) {
    *out_spare_path = ConstructFilePath(dir, name.as_string().append("-spare"));
  }
}

void GlobalHistogramAllocator::ConstructFilePathsForUploadDir(
    const FilePath& active_dir,
    const FilePath& upload_dir,
    const std::string& name,
    FilePath* out_upload_path,
    FilePath* out_active_path,
    FilePath* out_spare_path) {
  if (out_upload_path) {
    *out_upload_path = ConstructFilePathForUploadDir(
        upload_dir, name, Time::Now(), GetCurrentProcId());
  }

  if (out_active_path) {
    *out_active_path =
        ConstructFilePath(active_dir, name + std::string("-active"));
  }

  if (out_spare_path) {
    *out_spare_path =
        ConstructFilePath(active_dir, name + std::string("-spare"));
  }
}

bool GlobalHistogramAllocator::CreateSpareFile(const FilePath& spare_path,
                                               size_t size) {
  FilePath temp_spare_path = spare_path.AddExtension(FILE_PATH_LITERAL(".tmp"));
  bool success;
  {
    File spare_file(temp_spare_path, File::FLAG_CREATE_ALWAYS |
                                         File::FLAG_READ | File::FLAG_WRITE);
    success = spare_file.IsValid();

    if (success) {
      MemoryMappedFile mmfile;
      success = mmfile.Initialize(std::move(spare_file), {0, size},
                                  MemoryMappedFile::READ_WRITE_EXTEND);
    }
  }

  if (success)
    success = ReplaceFile(temp_spare_path, spare_path, nullptr);

  if (!success)
    DeleteFile(temp_spare_path, /*recursive=*/false);

  return success;
}

bool GlobalHistogramAllocator::CreateSpareFileInDir(const FilePath& dir,
                                                    size_t size,
                                                    StringPiece name) {
  FilePath spare_path;
  ConstructFilePaths(dir, name, nullptr, nullptr, &spare_path);
  return CreateSpareFile(spare_path, size);
}
#endif  // !defined(OS_NACL)

void GlobalHistogramAllocator::CreateWithSharedMemoryRegion(
    const WritableSharedMemoryRegion& region) {
  base::WritableSharedMemoryMapping mapping = region.Map();
  if (!mapping.IsValid() ||
      !WritableSharedPersistentMemoryAllocator::IsSharedMemoryAcceptable(
          mapping)) {
    return;
  }

  Set(WrapUnique(new GlobalHistogramAllocator(
      std::make_unique<WritableSharedPersistentMemoryAllocator>(
          std::move(mapping), 0, StringPiece()))));
}

void GlobalHistogramAllocator::Set(
    std::unique_ptr<GlobalHistogramAllocator> allocator) {



  CHECK(!subtle::NoBarrier_Load(&g_histogram_allocator));
  subtle::Release_Store(&g_histogram_allocator,
                        reinterpret_cast<uintptr_t>(allocator.release()));
  size_t existing = StatisticsRecorder::GetHistogramCount();

  DVLOG_IF(1, existing)
      << existing << " histograms were created before persistence was enabled.";
}

GlobalHistogramAllocator* GlobalHistogramAllocator::Get() {
  return reinterpret_cast<GlobalHistogramAllocator*>(
      subtle::Acquire_Load(&g_histogram_allocator));
}

std::unique_ptr<GlobalHistogramAllocator>
GlobalHistogramAllocator::ReleaseForTesting() {
  GlobalHistogramAllocator* histogram_allocator = Get();
  if (!histogram_allocator)
    return nullptr;
  PersistentMemoryAllocator* memory_allocator =
      histogram_allocator->memory_allocator();



  PersistentMemoryAllocator::Iterator iter(memory_allocator);
  const PersistentHistogramData* data;
  while ((data = iter.GetNextOfObject<PersistentHistogramData>()) != nullptr) {
    StatisticsRecorder::ForgetHistogramForTesting(data->name);
  }

  subtle::Release_Store(&g_histogram_allocator, 0);
  return WrapUnique(histogram_allocator);
}

void GlobalHistogramAllocator::SetPersistentLocation(const FilePath& location) {
  persistent_location_ = location;
}

const FilePath& GlobalHistogramAllocator::GetPersistentLocation() const {
  return persistent_location_;
}

bool GlobalHistogramAllocator::WriteToPersistentLocation() {
#if defined(OS_NACL)

  NOTREACHED();
  return false;
#else

  if (persistent_location_.empty()) {
    NOTREACHED() << "Could not write \"" << Name() << "\" persistent histograms"
                 << " to file because no location was set.";
    return false;
  }

  StringPiece contents(static_cast<const char*>(data()), used());
  if (!ImportantFileWriter::WriteFileAtomically(persistent_location_,
                                                contents)) {
    LOG(ERROR) << "Could not write \"" << Name() << "\" persistent histograms"
               << " to file: " << persistent_location_.value();
    return false;
  }

  return true;
#endif
}

void GlobalHistogramAllocator::DeletePersistentLocation() {
  memory_allocator()->SetMemoryState(PersistentMemoryAllocator::MEMORY_DELETED);

#if defined(OS_NACL)
  NOTREACHED();
#else
  if (persistent_location_.empty())
    return;




  File file(persistent_location_,
            File::FLAG_OPEN | File::FLAG_READ | File::FLAG_DELETE_ON_CLOSE);
#endif
}

GlobalHistogramAllocator::GlobalHistogramAllocator(
    std::unique_ptr<PersistentMemoryAllocator> memory)
    : PersistentHistogramAllocator(std::move(memory)),
      import_iterator_(this) {
}

void GlobalHistogramAllocator::ImportHistogramsToStatisticsRecorder() {






  Reference record_to_ignore = last_created();



  while (true) {
    std::unique_ptr<HistogramBase> histogram =
        import_iterator_.GetNextWithIgnore(record_to_ignore);
    if (!histogram)
      break;
    StatisticsRecorder::RegisterOrDeleteDuplicate(histogram.release());
  }
}

}  // namespace base
