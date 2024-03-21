// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PICKLE_H_
#define BASE_PICKLE_H_

#include <stddef.h>
#include <stdint.h>

#include <string>

#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "base/gtest_prod_util.h"
#include "base/logging.h"
#include "base/memory/ref_counted.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"

namespace base {

class Pickle;

// while the PickleIterator object is in use.
class BASE_EXPORT PickleIterator {
 public:
  PickleIterator() : payload_(nullptr), read_index_(0), end_index_(0) {}
  explicit PickleIterator(const Pickle& pickle);





  bool ReadBool(bool* result) WARN_UNUSED_RESULT;
  bool ReadInt(int* result) WARN_UNUSED_RESULT;
  bool ReadLong(long* result) WARN_UNUSED_RESULT;
  bool ReadUInt16(uint16_t* result) WARN_UNUSED_RESULT;
  bool ReadUInt32(uint32_t* result) WARN_UNUSED_RESULT;
  bool ReadInt64(int64_t* result) WARN_UNUSED_RESULT;
  bool ReadUInt64(uint64_t* result) WARN_UNUSED_RESULT;
  bool ReadFloat(float* result) WARN_UNUSED_RESULT;
  bool ReadDouble(double* result) WARN_UNUSED_RESULT;
  bool ReadString(std::string* result) WARN_UNUSED_RESULT;

  bool ReadStringPiece(StringPiece* result) WARN_UNUSED_RESULT;
  bool ReadString16(string16* result) WARN_UNUSED_RESULT;

  bool ReadStringPiece16(StringPiece16* result) WARN_UNUSED_RESULT;




  bool ReadData(const char** data, int* length) WARN_UNUSED_RESULT;





  bool ReadBytes(const char** data, int length) WARN_UNUSED_RESULT;


  bool ReadLength(int* result) WARN_UNUSED_RESULT {
    return ReadInt(result) && *result >= 0;
  }


  bool SkipBytes(int num_bytes) WARN_UNUSED_RESULT {
    return !!GetReadPointerAndAdvance(num_bytes);
  }

  bool ReachedEnd() const { return read_index_ == end_index_; }

 private:

  template <typename Type>
  bool ReadBuiltinType(Type* result);


  void Advance(size_t size);

  template<typename Type>
  const char* GetReadPointerAndAdvance();


  const char* GetReadPointerAndAdvance(int num_bytes);


  const char* GetReadPointerAndAdvance(int num_elements,
                                       size_t size_element);

  const char* payload_;  // Start of our pickle's payload.
  size_t read_index_;  // Offset of the next readable byte in payload.
  size_t end_index_;  // Payload size.

  FRIEND_TEST_ALL_PREFIXES(PickleTest, GetReadPointerAndAdvance);
};

//
// The Pickle class supports appending primitive values (ints, strings, etc.)
// to a pickle instance.  The Pickle instance grows its internal memory buffer
// dynamically to hold the sequence of primitive values.   The internal memory
// buffer is exposed as the "data" of the Pickle.  This "data" can be passed
// to a Pickle object to initialize it for reading.
//
// When reading from a Pickle object, it is important for the consumer to know
// what value types to read and in what order to read them as the Pickle does
// not keep track of the type of data written to it.
//
// The Pickle's data has a header which contains the size of the Pickle's
// payload.  It can optionally support additional space in the header.  That
// space is controlled by the header_size parameter passed to the Pickle
// constructor.
//
class BASE_EXPORT Pickle {
 public:




  class BASE_EXPORT Attachment : public RefCountedThreadSafe<Attachment> {
   public:
    Attachment();

   protected:
    friend class RefCountedThreadSafe<Attachment>;
    virtual ~Attachment();

    DISALLOW_COPY_AND_ASSIGN(Attachment);
  };

  Pickle();



  explicit Pickle(int header_size);




  Pickle(const char* data, size_t data_len);

  Pickle(const Pickle& other);





  virtual ~Pickle();

  Pickle& operator=(const Pickle& other);

  size_t size() const { return header_size_ + header_->payload_size; }

  const void* data() const { return header_; }




  size_t GetTotalAllocatedSize() const;





  void WriteBool(bool value) { WriteInt(value ? 1 : 0); }
  void WriteInt(int value) { WritePOD(value); }
  void WriteLong(long value) {


    WritePOD(static_cast<int64_t>(value));
  }
  void WriteUInt16(uint16_t value) { WritePOD(value); }
  void WriteUInt32(uint32_t value) { WritePOD(value); }
  void WriteInt64(int64_t value) { WritePOD(value); }
  void WriteUInt64(uint64_t value) { WritePOD(value); }
  void WriteFloat(float value) { WritePOD(value); }
  void WriteDouble(double value) { WritePOD(value); }
  void WriteString(const StringPiece& value);
  void WriteString16(const StringPiece16& value);


  void WriteData(const char* data, int length);



  void WriteBytes(const void* data, int length);



  virtual bool WriteAttachment(scoped_refptr<Attachment> attachment);


  virtual bool ReadAttachment(base::PickleIterator* iter,
                              scoped_refptr<Attachment>* attachment) const;

  virtual bool HasAttachments() const;



  void Reserve(size_t additional_capacity);

  struct Header {
    uint32_t payload_size;  // Specifies the size of the payload.
  };



  template <class T>
  T* headerT() {
    DCHECK_EQ(header_size_, sizeof(T));
    return static_cast<T*>(header_);
  }
  template <class T>
  const T* headerT() const {
    DCHECK_EQ(header_size_, sizeof(T));
    return static_cast<const T*>(header_);
  }

  size_t payload_size() const {
    return header_ ? header_->payload_size : 0;
  }

  const char* payload() const {
    return reinterpret_cast<const char*>(header_) + header_size_;
  }


  const char* end_of_payload() const {

    return header_ ? payload() + payload_size() : NULL;
  }

 protected:


  size_t header_size() const { return header_size_; }

  char* mutable_payload() {
    return reinterpret_cast<char*>(header_) + header_size_;
  }

  size_t capacity_after_header() const {
    return capacity_after_header_;
  }


  void Resize(size_t new_capacity);





  void* ClaimBytes(size_t num_bytes);


  static const char* FindNext(size_t header_size,
                              const char* range_start,
                              const char* range_end);







  static bool PeekNext(size_t header_size,
                       const char* range_start,
                       const char* range_end,
                       size_t* pickle_size);

  static const int kPayloadUnit;

 private:
  friend class PickleIterator;

  Header* header_;
  size_t header_size_;  // Supports extra data between header and payload.


  size_t capacity_after_header_;


  size_t write_offset_;

  template<size_t length> void BASE_EXPORT WriteBytesStatic(const void* data);

  template <typename T> bool WritePOD(const T& data) {
    WriteBytesStatic<sizeof(data)>(&data);
    return true;
  }

  inline void* ClaimUninitializedBytesInternal(size_t num_bytes);
  inline void WriteBytesCommon(const void* data, size_t length);

  FRIEND_TEST_ALL_PREFIXES(PickleTest, DeepCopyResize);
  FRIEND_TEST_ALL_PREFIXES(PickleTest, Resize);
  FRIEND_TEST_ALL_PREFIXES(PickleTest, PeekNext);
  FRIEND_TEST_ALL_PREFIXES(PickleTest, PeekNextOverflow);
  FRIEND_TEST_ALL_PREFIXES(PickleTest, FindNext);
  FRIEND_TEST_ALL_PREFIXES(PickleTest, FindNextWithIncompleteHeader);
  FRIEND_TEST_ALL_PREFIXES(PickleTest, FindNextOverflow);
};

}  // namespace base

#endif  // BASE_PICKLE_H_
