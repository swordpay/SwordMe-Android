// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_METRICS_PERSISTENT_HISTOGRAM_ALLOCATOR_H_
#define BASE_METRICS_PERSISTENT_HISTOGRAM_ALLOCATOR_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/atomicops.h"
#include "base/base_export.h"
#include "base/feature_list.h"
#include "base/metrics/histogram_base.h"
#include "base/metrics/persistent_memory_allocator.h"
#include "base/process/process_handle.h"
#include "base/strings/string_piece.h"
#include "base/synchronization/lock.h"

namespace base {

class BucketRanges;
class FilePath;
class PersistentSampleMapRecords;
class PersistentSparseHistogramDataManager;
class WritableSharedMemoryRegion;

BASE_EXPORT extern const Feature kPersistentHistogramsFeature;

// to separately iterate over the entire memory segment. Though this class
// will generally be accessed through the PersistentHistogramAllocator above,
// it can be used independently on any PersistentMemoryAllocator (making it
// useable for testing). This object supports only one instance of a sparse
// histogram for a given id. Tests that create multiple identical histograms,
// perhaps to simulate multiple processes, should create a separate manager
// for each.
class BASE_EXPORT PersistentSparseHistogramDataManager {
 public:


  explicit PersistentSparseHistogramDataManager(
      PersistentMemoryAllocator* allocator);

  ~PersistentSparseHistogramDataManager();





  PersistentSampleMapRecords* UseSampleMapRecords(uint64_t id,
                                                  const void* user);


  template <typename T>
  T* GetAsObject(PersistentMemoryAllocator::Reference ref) {
    return allocator_->GetAsObject<T>(ref);
  }

 private:
  friend class PersistentSampleMapRecords;

  PersistentSampleMapRecords* GetSampleMapRecordsWhileLocked(uint64_t id)
      EXCLUSIVE_LOCKS_REQUIRED(lock_);






  bool LoadRecords(PersistentSampleMapRecords* sample_map_records);

  PersistentMemoryAllocator* allocator_;

  PersistentMemoryAllocator::Iterator record_iterator_ GUARDED_BY(lock_);

  std::map<uint64_t, std::unique_ptr<PersistentSampleMapRecords>>
      sample_records_ GUARDED_BY(lock_);

  base::Lock lock_;

  DISALLOW_COPY_AND_ASSIGN(PersistentSparseHistogramDataManager);
};

// that underlies a persistent SparseHistogram object. It is broken out into a
// top-level class so that it can be forward-declared in other header files
// rather than include this entire file as would be necessary if it were
// declared within the PersistentSparseHistogramDataManager class above.
class BASE_EXPORT PersistentSampleMapRecords {
 public:




  PersistentSampleMapRecords(PersistentSparseHistogramDataManager* data_manager,
                             uint64_t sample_map_id);

  ~PersistentSampleMapRecords();


  PersistentSampleMapRecords* Acquire(const void* user);

  void Release(const void* user);



  PersistentMemoryAllocator::Reference GetNext();


  PersistentMemoryAllocator::Reference CreateNew(HistogramBase::Sample value);






  template <typename T>
  T* GetAsObject(PersistentMemoryAllocator::Reference ref) {
    return data_manager_->GetAsObject<T>(ref);
  }

 private:
  friend PersistentSparseHistogramDataManager;

  PersistentSparseHistogramDataManager* data_manager_;

  const uint64_t sample_map_id_;


  const void* user_ = nullptr;


  size_t seen_ = 0;





  std::vector<PersistentMemoryAllocator::Reference> records_;



  std::vector<PersistentMemoryAllocator::Reference> found_;

  DISALLOW_COPY_AND_ASSIGN(PersistentSampleMapRecords);
};

class BASE_EXPORT PersistentHistogramAllocator {
 public:



  using Reference = PersistentMemoryAllocator::Reference;



  class BASE_EXPORT Iterator {
   public:


    explicit Iterator(PersistentHistogramAllocator* allocator);



    std::unique_ptr<HistogramBase> GetNext() { return GetNextWithIgnore(0); }


    std::unique_ptr<HistogramBase> GetNextWithIgnore(Reference ignore);

   private:

    PersistentHistogramAllocator* allocator_;


    PersistentMemoryAllocator::Iterator memory_iter_;

    DISALLOW_COPY_AND_ASSIGN(Iterator);
  };


  explicit PersistentHistogramAllocator(
      std::unique_ptr<PersistentMemoryAllocator> memory);
  virtual ~PersistentHistogramAllocator();



  PersistentMemoryAllocator* memory_allocator() {
    return memory_allocator_.get();
  }


  uint64_t Id() const { return memory_allocator_->Id(); }
  const char* Name() const { return memory_allocator_->Name(); }
  const void* data() const { return memory_allocator_->data(); }
  size_t length() const { return memory_allocator_->length(); }
  size_t size() const { return memory_allocator_->size(); }
  size_t used() const { return memory_allocator_->used(); }





  std::unique_ptr<HistogramBase> GetHistogram(Reference ref);


  std::unique_ptr<HistogramBase> AllocateHistogram(
      HistogramType histogram_type,
      const std::string& name,
      int minimum,
      int maximum,
      const BucketRanges* bucket_ranges,
      int32_t flags,
      Reference* ref_ptr);



  void FinalizeHistogram(Reference ref, bool registered);




  void MergeHistogramDeltaToStatisticsRecorder(HistogramBase* histogram);




  void MergeHistogramFinalDeltaToStatisticsRecorder(
      const HistogramBase* histogram);





  PersistentSampleMapRecords* UseSampleMapRecords(uint64_t id,
                                                  const void* user);









  void CreateTrackingHistograms(StringPiece name);
  void UpdateTrackingHistograms();


  void ClearLastCreatedReferenceForTesting();

 protected:


  struct PersistentHistogramData;


  PersistentHistogramAllocator::Reference last_created() {
    return subtle::NoBarrier_Load(&last_created_);
  }


  std::unique_ptr<HistogramBase> GetNextHistogramWithIgnore(Iterator* iter,
                                                            Reference ignore);

 private:

  std::unique_ptr<HistogramBase> CreateHistogram(
      PersistentHistogramData* histogram_data_ptr);



  HistogramBase* GetOrCreateStatisticsRecorderHistogram(
      const HistogramBase* histogram);

  std::unique_ptr<PersistentMemoryAllocator> memory_allocator_;

  PersistentSparseHistogramDataManager sparse_histogram_data_manager_;



  subtle::Atomic32 last_created_ = 0;

  DISALLOW_COPY_AND_ASSIGN(PersistentHistogramAllocator);
};

// global scale, collecting histograms created through standard macros and
// the FactoryGet() method.
class BASE_EXPORT GlobalHistogramAllocator
    : public PersistentHistogramAllocator {
 public:
  ~GlobalHistogramAllocator() override;


  static void CreateWithPersistentMemory(void* base,
                                         size_t size,
                                         size_t page_size,
                                         uint64_t id,
                                         StringPiece name);


  static void CreateWithLocalMemory(size_t size, uint64_t id, StringPiece name);

#if !defined(OS_NACL)





  static bool CreateWithFile(const FilePath& file_path,
                             size_t size,
                             uint64_t id,
                             StringPiece name);





  static bool CreateWithActiveFile(const FilePath& base_path,
                                   const FilePath& active_path,
                                   const FilePath& spare_path,
                                   size_t size,
                                   uint64_t id,
                                   StringPiece name);




  static bool CreateWithActiveFileInDir(const FilePath& dir,
                                        size_t size,
                                        uint64_t id,
                                        StringPiece name);

  static FilePath ConstructFilePath(const FilePath& dir, StringPiece name);

  static FilePath ConstructFilePathForUploadDir(const FilePath& dir,
                                                StringPiece name,
                                                base::Time stamp,
                                                ProcessId pid);

  static bool ParseFilePath(const FilePath& path,
                            std::string* out_name,
                            Time* out_stamp,
                            ProcessId* out_pid);






  static void ConstructFilePaths(const FilePath& dir,
                                 StringPiece name,
                                 FilePath* out_base_path,
                                 FilePath* out_active_path,
                                 FilePath* out_spare_path);



  static void ConstructFilePathsForUploadDir(const FilePath& active_dir,
                                             const FilePath& upload_dir,
                                             const std::string& name,
                                             FilePath* out_upload_path,
                                             FilePath* out_active_path,
                                             FilePath* out_spare_path);


  static bool CreateSpareFile(const FilePath& spare_path, size_t size);


  static bool CreateSpareFileInDir(const FilePath& dir_path,
                                   size_t size,
                                   StringPiece name);
#endif




  static void CreateWithSharedMemoryRegion(
      const WritableSharedMemoryRegion& region);






  static void Set(std::unique_ptr<GlobalHistogramAllocator> allocator);


  static GlobalHistogramAllocator* Get();




  static std::unique_ptr<GlobalHistogramAllocator> ReleaseForTesting();


  void SetPersistentLocation(const FilePath& location);


  const FilePath& GetPersistentLocation() const;




  bool WriteToPersistentLocation();


  void DeletePersistentLocation();

 private:
  friend class StatisticsRecorder;

  explicit GlobalHistogramAllocator(
      std::unique_ptr<PersistentMemoryAllocator> memory);






  void ImportHistogramsToStatisticsRecorder();

  static FilePath MakeMetricsFilePath(const FilePath& dir, StringPiece name);


  Iterator import_iterator_;

  FilePath persistent_location_;

  DISALLOW_COPY_AND_ASSIGN(GlobalHistogramAllocator);
};

}  // namespace base

#endif  // BASE_METRICS_PERSISTENT_HISTOGRAM_ALLOCATOR_H__
