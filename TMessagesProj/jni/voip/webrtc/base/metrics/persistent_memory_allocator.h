// Copyright (c) 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_METRICS_PERSISTENT_MEMORY_ALLOCATOR_H_
#define BASE_METRICS_PERSISTENT_MEMORY_ALLOCATOR_H_

#include <stdint.h>

#include <atomic>
#include <memory>
#include <type_traits>

#include "base/atomicops.h"
#include "base/base_export.h"
#include "base/files/file_path.h"
#include "base/gtest_prod_util.h"
#include "base/macros.h"
#include "base/memory/shared_memory_mapping.h"
#include "base/strings/string_piece.h"

namespace base {

class HistogramBase;
class MemoryMappedFile;

// to some storage or shared across multiple processes. This class resides
// under base/metrics because it was written for that purpose. It is,
// however, fully general-purpose and can be freely moved to base/memory
// if other uses are found.
//
// This class provides for thread-secure (i.e. safe against other threads
// or processes that may be compromised and thus have malicious intent)
// allocation of memory within a designated block and also a mechanism by
// which other threads can learn of these allocations.
//
// There is (currently) no way to release an allocated block of data because
// doing so would risk invalidating pointers held by other processes and
// greatly complicate the allocation algorithm.
//
// Construction of this object can accept new, clean (i.e. zeroed) memory
// or previously initialized memory. In the first case, construction must
// be allowed to complete before letting other allocators attach to the same
// segment. In other words, don't share the segment until at least one
// allocator has been attached to it.
//
// Note that memory not in active use is not accessed so it is possible to
// use virtual memory, including memory-mapped files, as backing storage with
// the OS "pinning" new (zeroed) physical RAM pages only as they are needed.
//
// OBJECTS: Although the allocator can be used in a "malloc" sense, fetching
// character arrays and manipulating that memory manually, the better way is
// generally to use the "object" methods to create and manage allocations. In
// this way the sizing, type-checking, and construction are all automatic. For
// this to work, however, every type of stored object must define two public
// "constexpr" values, kPersistentTypeId and kExpectedInstanceSize, as such:
//
// struct MyPersistentObjectType {
//     // SHA1(MyPersistentObjectType): Increment this if structure changes!
//     static constexpr uint32_t kPersistentTypeId = 0x3E15F6DE + 1;
//
//     // Expected size for 32/64-bit check. Update this if structure changes!
//     static constexpr size_t kExpectedInstanceSize = 20;
//
//     ...
// };
//
// kPersistentTypeId: This value is an arbitrary identifier that allows the
//   identification of these objects in the allocator, including the ability
//   to find them via iteration. The number is arbitrary but using the first
//   four bytes of the SHA1 hash of the type name means that there shouldn't
//   be any conflicts with other types that may also be stored in the memory.
//   The fully qualified name (e.g. base::debug::MyPersistentObjectType) could
//   be used to generate the hash if the type name seems common. Use a command
//   like this to get the hash: echo -n "MyPersistentObjectType" | sha1sum
//   If the structure layout changes, ALWAYS increment this number so that
//   newer versions of the code don't try to interpret persistent data written
//   by older versions with a different layout.
//
// kExpectedInstanceSize: This value is the hard-coded number that matches
//   what sizeof(T) would return. By providing it explicitly, the allocator can
//   verify that the structure is compatible between both 32-bit and 64-bit
//   versions of the code.
//
// Using New manages the memory and then calls the default constructor for the
// object. Given that objects are persistent, no destructor is ever called
// automatically though a caller can explicitly call Delete to destruct it and
// change the type to something indicating it is no longer in use.
//
// Though persistent memory segments are transferrable between programs built
// for different natural word widths, they CANNOT be exchanged between CPUs
// of different endianess. Attempts to do so will simply see the existing data
// as corrupt and refuse to access any of it.
class BASE_EXPORT PersistentMemoryAllocator {
 public:
  typedef uint32_t Reference;




  enum MemoryState : uint8_t {

    MEMORY_UNINITIALIZED = 0,

    MEMORY_INITIALIZED = 1,





    MEMORY_DELETED = 2,


    MEMORY_USER_DEFINED = 100,
  };














  class BASE_EXPORT Iterator {
   public:




    explicit Iterator(const PersistentMemoryAllocator* allocator);





    Iterator(const PersistentMemoryAllocator* allocator,
             Reference starting_after);

    void Reset();

    void Reset(Reference starting_after);



    Reference GetLast();




    Reference GetNext(uint32_t* type_return);




    Reference GetNextOfType(uint32_t type_match);

    template <typename T>
    Reference GetNextOfType() {
      return GetNextOfType(T::kPersistentTypeId);
    }

    template <typename T>
    const T* GetNextOfObject() {
      return GetAsObject<T>(GetNextOfType<T>());
    }







    template <typename T>
    const T* GetAsObject(Reference ref) const {
      return allocator_->GetAsObject<T>(ref);
    }

    template <typename T>
    const T* GetAsArray(Reference ref, uint32_t type_id, size_t count) const {
      return allocator_->GetAsArray<T>(ref, type_id, count);
    }



    Reference GetAsReference(const void* memory, uint32_t type_id) const {
      return allocator_->GetAsReference(memory, type_id);
    }

    template <typename T>
    Reference GetAsReference(const T* obj) const {
      return allocator_->GetAsReference(obj);
    }

   private:

    const PersistentMemoryAllocator* allocator_;

    std::atomic<Reference> last_record_;

    std::atomic<uint32_t> record_count_;

    DISALLOW_COPY_AND_ASSIGN(Iterator);
  };

  struct MemoryInfo {
    size_t total;
    size_t free;
  };

  enum : Reference {

    kReferenceNull = 0,
  };

  enum : uint32_t {

    kTypeIdAny = 0x00000000,


    kTypeIdTransitioning = 0xFFFFFFFF,
  };

  enum : size_t {
    kSizeAny = 1  // Constant indicating that any array size is acceptable.
  };


  static const base::FilePath::CharType kFileExtension[];



























  PersistentMemoryAllocator(void* base, size_t size, size_t page_size,
                            uint64_t id, base::StringPiece name,
                            bool readonly);
  virtual ~PersistentMemoryAllocator();





  static bool IsMemoryAcceptable(const void* data, size_t size,
                                 size_t page_size, bool readonly);

  uint64_t Id() const;

  const char* Name() const;

  bool IsReadonly() const { return readonly_; }

  void SetMemoryState(uint8_t memory_state);
  uint8_t GetMemoryState() const;









  void CreateTrackingHistograms(base::StringPiece name);










  void Flush(bool sync);



  const void* data() const { return const_cast<const char*>(mem_base_); }
  size_t length() const { return mem_size_; }
  size_t size() const { return mem_size_; }
  size_t used() const;









































  template <typename T>
  T* GetAsObject(Reference ref) {
    static_assert(std::is_standard_layout<T>::value, "only standard objects");
    static_assert(!std::is_array<T>::value, "use GetAsArray<>()");
    static_assert(T::kExpectedInstanceSize == sizeof(T), "inconsistent size");
    return const_cast<T*>(reinterpret_cast<volatile T*>(
        GetBlockData(ref, T::kPersistentTypeId, sizeof(T))));
  }
  template <typename T>
  const T* GetAsObject(Reference ref) const {
    static_assert(std::is_standard_layout<T>::value, "only standard objects");
    static_assert(!std::is_array<T>::value, "use GetAsArray<>()");
    static_assert(T::kExpectedInstanceSize == sizeof(T), "inconsistent size");
    return const_cast<const T*>(reinterpret_cast<const volatile T*>(
        GetBlockData(ref, T::kPersistentTypeId, sizeof(T))));
  }











  template <typename T>
  T* GetAsArray(Reference ref, uint32_t type_id, size_t count) {
    static_assert(std::is_fundamental<T>::value, "use GetAsObject<>()");
    return const_cast<T*>(reinterpret_cast<volatile T*>(
        GetBlockData(ref, type_id, count * sizeof(T))));
  }
  template <typename T>
  const T* GetAsArray(Reference ref, uint32_t type_id, size_t count) const {
    static_assert(std::is_fundamental<T>::value, "use GetAsObject<>()");
    return const_cast<const char*>(reinterpret_cast<const volatile T*>(
        GetBlockData(ref, type_id, count * sizeof(T))));
  }



  Reference GetAsReference(const void* memory, uint32_t type_id) const;




  size_t GetAllocSize(Reference ref) const;
















  uint32_t GetType(Reference ref) const;
  bool ChangeType(Reference ref,
                  uint32_t to_type_id,
                  uint32_t from_type_id,
                  bool clear);








  void MakeIterable(Reference ref);




  void GetMemoryInfo(MemoryInfo* meminfo) const;



  void SetCorrupt() const;



  bool IsCorrupt() const;

  bool IsFull() const;



  void UpdateTrackingHistograms();







  Reference Allocate(size_t size, uint32_t type_id);






  template <typename T>
  T* New(size_t size) {
    if (size < sizeof(T))
      size = sizeof(T);
    Reference ref = Allocate(size, T::kPersistentTypeId);
    void* mem =
        const_cast<void*>(GetBlockData(ref, T::kPersistentTypeId, size));
    if (!mem)
      return nullptr;
    DCHECK_EQ(0U, reinterpret_cast<uintptr_t>(mem) & (alignof(T) - 1));
    return new (mem) T();
  }
  template <typename T>
  T* New() {
    return New<T>(sizeof(T));
  }









  template <typename T>
  T* New(Reference ref, uint32_t from_type_id, bool clear) {
    DCHECK_LE(sizeof(T), GetAllocSize(ref)) << "alloc not big enough for obj";



    void* mem = const_cast<void*>(GetBlockData(ref, 0, sizeof(T)));
    if (!mem)
      return nullptr;


    DCHECK_EQ(0U, reinterpret_cast<uintptr_t>(mem) & (alignof(T) - 1));





    if (!ChangeType(ref, kTypeIdTransitioning, from_type_id, clear))
      return nullptr;


    T* obj = new (mem) T();


    bool success =
        ChangeType(ref, T::kPersistentTypeId, kTypeIdTransitioning, false);
    DCHECK(success);
    return obj;
  }


  template <typename T>
  void Delete(T* obj, uint32_t new_type) {

    Reference ref = GetAsReference<T>(obj);




    if (!ChangeType(ref, kTypeIdTransitioning, T::kPersistentTypeId, false))
      return;

    obj->~T();


    bool success = ChangeType(ref, new_type, kTypeIdTransitioning, false);
    DCHECK(success);
  }
  template <typename T>
  void Delete(T* obj) {
    Delete<T>(obj, 0);
  }

  template <typename T>
  Reference GetAsReference(const T* obj) const {
    return GetAsReference(obj, T::kPersistentTypeId);
  }

  template <typename T>
  void MakeIterable(const T* obj) {
    MakeIterable(GetAsReference<T>(obj));
  }

 protected:
  enum MemoryType {
    MEM_EXTERNAL,
    MEM_MALLOC,
    MEM_VIRTUAL,
    MEM_SHARED,
    MEM_FILE,
  };

  struct Memory {
    Memory(void* b, MemoryType t) : base(b), type(t) {}

    void* base;
    MemoryType type;
  };



  PersistentMemoryAllocator(Memory memory, size_t size, size_t page_size,
                            uint64_t id, base::StringPiece name,
                            bool readonly);

  virtual void FlushPartial(size_t length, bool sync);

  volatile char* const mem_base_;  // Memory base. (char so sizeof guaranteed 1)
  const MemoryType mem_type_;      // Type of memory allocation.
  const uint32_t mem_size_;        // Size of entire memory segment.
  const uint32_t mem_page_;        // Page size allocations shouldn't cross.
  const size_t vm_page_size_;      // The page size used by the OS.

 private:
  struct SharedMetadata;
  struct BlockHeader;
  static const uint32_t kAllocAlignment;
  static const Reference kReferenceQueue;



  const SharedMetadata* shared_meta() const {
    return reinterpret_cast<const SharedMetadata*>(
        const_cast<const char*>(mem_base_));
  }
  SharedMetadata* shared_meta() {
    return reinterpret_cast<SharedMetadata*>(const_cast<char*>(mem_base_));
  }

  Reference AllocateImpl(size_t size, uint32_t type_id);

  const volatile BlockHeader* GetBlock(Reference ref, uint32_t type_id,
                                       uint32_t size, bool queue_ok,
                                       bool free_ok) const;
  volatile BlockHeader* GetBlock(Reference ref, uint32_t type_id, uint32_t size,
                                 bool queue_ok, bool free_ok) {
      return const_cast<volatile BlockHeader*>(
          const_cast<const PersistentMemoryAllocator*>(this)->GetBlock(
              ref, type_id, size, queue_ok, free_ok));
  }

  const volatile void* GetBlockData(Reference ref, uint32_t type_id,
                                    uint32_t size) const;
  volatile void* GetBlockData(Reference ref, uint32_t type_id,
                              uint32_t size) {
      return const_cast<volatile void*>(
          const_cast<const PersistentMemoryAllocator*>(this)->GetBlockData(
              ref, type_id, size));
  }

  void RecordError(int error) const;

  const bool readonly_;                // Indicates access to read-only memory.
  mutable std::atomic<bool> corrupt_;  // Local version of "corrupted" flag.

  HistogramBase* allocs_histogram_;  // Histogram recording allocs.
  HistogramBase* used_histogram_;    // Histogram recording used space.
  HistogramBase* errors_histogram_;  // Histogram recording errors.

  friend class PersistentMemoryAllocatorTest;
  FRIEND_TEST_ALL_PREFIXES(PersistentMemoryAllocatorTest, AllocateAndIterate);
  DISALLOW_COPY_AND_ASSIGN(PersistentMemoryAllocator);
};

// heap. It is generally used when some kind of "death rattle" handler will
// save the contents to persistent storage during process shutdown. It is
// also useful for testing.
class BASE_EXPORT LocalPersistentMemoryAllocator
    : public PersistentMemoryAllocator {
 public:
  LocalPersistentMemoryAllocator(size_t size, uint64_t id,
                                 base::StringPiece name);
  ~LocalPersistentMemoryAllocator() override;

 private:



  static Memory AllocateLocalMemory(size_t size);

  static void DeallocateLocalMemory(void* memory, size_t size, MemoryType type);

  DISALLOW_COPY_AND_ASSIGN(LocalPersistentMemoryAllocator);
};

// allocation from it. The allocator takes ownership of the mapping object.
class BASE_EXPORT WritableSharedPersistentMemoryAllocator
    : public PersistentMemoryAllocator {
 public:
  WritableSharedPersistentMemoryAllocator(
      base::WritableSharedMemoryMapping memory,
      uint64_t id,
      base::StringPiece name);
  ~WritableSharedPersistentMemoryAllocator() override;




  static bool IsSharedMemoryAcceptable(
      const base::WritableSharedMemoryMapping& memory);

 private:
  base::WritableSharedMemoryMapping shared_memory_;

  DISALLOW_COPY_AND_ASSIGN(WritableSharedPersistentMemoryAllocator);
};

// allocation from it. The allocator takes ownership of the mapping object.
class BASE_EXPORT ReadOnlySharedPersistentMemoryAllocator
    : public PersistentMemoryAllocator {
 public:
  ReadOnlySharedPersistentMemoryAllocator(
      base::ReadOnlySharedMemoryMapping memory,
      uint64_t id,
      base::StringPiece name);
  ~ReadOnlySharedPersistentMemoryAllocator() override;




  static bool IsSharedMemoryAcceptable(
      const base::ReadOnlySharedMemoryMapping& memory);

 private:
  base::ReadOnlySharedMemoryMapping shared_memory_;

  DISALLOW_COPY_AND_ASSIGN(ReadOnlySharedPersistentMemoryAllocator);
};

#if !defined(OS_NACL)  // NACL doesn't support any kind of file access in build.
// This allocator takes a memory-mapped file object and performs allocation
// from it. The allocator takes ownership of the file object.
class BASE_EXPORT FilePersistentMemoryAllocator
    : public PersistentMemoryAllocator {
 public:



  FilePersistentMemoryAllocator(std::unique_ptr<MemoryMappedFile> file,
                                size_t max_size,
                                uint64_t id,
                                base::StringPiece name,
                                bool read_only);
  ~FilePersistentMemoryAllocator() override;




  static bool IsFileAcceptable(const MemoryMappedFile& file, bool read_only);






  void Cache();

 protected:

  void FlushPartial(size_t length, bool sync) override;

 private:
  std::unique_ptr<MemoryMappedFile> mapped_file_;

  DISALLOW_COPY_AND_ASSIGN(FilePersistentMemoryAllocator);
};
#endif  // !defined(OS_NACL)

// time. This allows for potential users of an allocation to be decoupled
// from the logic that defines it. In addition, there can be multiple users
// of the same allocation or any region thereof that are guaranteed to always
// use the same space. It's okay to copy/move these objects.
//
// This is a top-level class instead of an inner class of the PMA so that it
// can be forward-declared in other header files without the need to include
// the full contents of this file.
class BASE_EXPORT DelayedPersistentAllocation {
 public:
  using Reference = PersistentMemoryAllocator::Reference;


















  DelayedPersistentAllocation(PersistentMemoryAllocator* allocator,
                              subtle::Atomic32* ref,
                              uint32_t type,
                              size_t size,
                              bool make_iterable);
  DelayedPersistentAllocation(PersistentMemoryAllocator* allocator,
                              subtle::Atomic32* ref,
                              uint32_t type,
                              size_t size,
                              size_t offset,
                              bool make_iterable);
  DelayedPersistentAllocation(PersistentMemoryAllocator* allocator,
                              std::atomic<Reference>* ref,
                              uint32_t type,
                              size_t size,
                              bool make_iterable);
  DelayedPersistentAllocation(PersistentMemoryAllocator* allocator,
                              std::atomic<Reference>* ref,
                              uint32_t type,
                              size_t size,
                              size_t offset,
                              bool make_iterable);
  ~DelayedPersistentAllocation();








  void* Get() const;




  Reference reference() const {
    return reference_->load(std::memory_order_relaxed);
  }

 private:



  PersistentMemoryAllocator* const allocator_;


  const uint32_t type_;
  const uint32_t size_;
  const uint32_t offset_;

  const bool make_iterable_;




  volatile std::atomic<Reference>* const reference_;

};

}  // namespace base

#endif  // BASE_METRICS_PERSISTENT_MEMORY_ALLOCATOR_H_
