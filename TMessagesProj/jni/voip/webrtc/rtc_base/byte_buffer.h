/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_BYTE_BUFFER_H_
#define RTC_BASE_BYTE_BUFFER_H_

#include <stddef.h>
#include <stdint.h>

#include <string>

#include "absl/strings/string_view.h"
#include "rtc_base/buffer.h"
#include "rtc_base/byte_order.h"

namespace rtc {

template <class BufferClassT>
class ByteBufferWriterT {
 public:
  ByteBufferWriterT() { Construct(nullptr, kDefaultCapacity); }
  ByteBufferWriterT(const char* bytes, size_t len) { Construct(bytes, len); }

  ByteBufferWriterT(const ByteBufferWriterT&) = delete;
  ByteBufferWriterT& operator=(const ByteBufferWriterT&) = delete;

  const char* Data() const { return buffer_.data(); }
  size_t Length() const { return buffer_.size(); }
  size_t Capacity() const { return buffer_.capacity(); }


  void WriteUInt8(uint8_t val) {
    WriteBytes(reinterpret_cast<const char*>(&val), 1);
  }
  void WriteUInt16(uint16_t val) {
    uint16_t v = HostToNetwork16(val);
    WriteBytes(reinterpret_cast<const char*>(&v), 2);
  }
  void WriteUInt24(uint32_t val) {
    uint32_t v = HostToNetwork32(val);
    char* start = reinterpret_cast<char*>(&v);
    ++start;
    WriteBytes(start, 3);
  }
  void WriteUInt32(uint32_t val) {
    uint32_t v = HostToNetwork32(val);
    WriteBytes(reinterpret_cast<const char*>(&v), 4);
  }
  void WriteUInt64(uint64_t val) {
    uint64_t v = HostToNetwork64(val);
    WriteBytes(reinterpret_cast<const char*>(&v), 8);
  }



  void WriteUVarint(uint64_t val) {
    while (val >= 0x80) {


      char byte = static_cast<char>(val) | 0x80;
      WriteBytes(&byte, 1);
      val >>= 7;
    }
    char last_byte = static_cast<char>(val);
    WriteBytes(&last_byte, 1);
  }
  void WriteString(absl::string_view val) {
    WriteBytes(val.data(), val.size());
  }
  void WriteBytes(const char* val, size_t len) { buffer_.AppendData(val, len); }



  char* ReserveWriteBuffer(size_t len) {
    buffer_.SetSize(buffer_.size() + len);
    return buffer_.data();
  }

  void Resize(size_t size) { buffer_.SetSize(size); }

  void Clear() { buffer_.Clear(); }

 private:
  static constexpr size_t kDefaultCapacity = 4096;

  void Construct(const char* bytes, size_t size) {
    if (bytes) {
      buffer_.AppendData(bytes, size);
    } else {
      buffer_.EnsureCapacity(size);
    }
  }

  BufferClassT buffer_;


};

class ByteBufferWriter : public ByteBufferWriterT<BufferT<char>> {
 public:
  ByteBufferWriter();
  ByteBufferWriter(const char* bytes, size_t len);

  ByteBufferWriter(const ByteBufferWriter&) = delete;
  ByteBufferWriter& operator=(const ByteBufferWriter&) = delete;
};

// valid during the lifetime of the reader.
class ByteBufferReader {
 public:
  ByteBufferReader(const char* bytes, size_t len);

  explicit ByteBufferReader(const char* bytes);

  explicit ByteBufferReader(const Buffer& buf);

  explicit ByteBufferReader(const ByteBufferWriter& buf);

  ByteBufferReader(const ByteBufferReader&) = delete;
  ByteBufferReader& operator=(const ByteBufferReader&) = delete;

  const char* Data() const { return bytes_ + start_; }

  size_t Length() const { return end_ - start_; }


  bool ReadUInt8(uint8_t* val);
  bool ReadUInt16(uint16_t* val);
  bool ReadUInt24(uint32_t* val);
  bool ReadUInt32(uint32_t* val);
  bool ReadUInt64(uint64_t* val);
  bool ReadUVarint(uint64_t* val);
  bool ReadBytes(char* val, size_t len);


  bool ReadString(std::string* val, size_t len);




  bool Consume(size_t size);

 protected:
  void Construct(const char* bytes, size_t size);

  const char* bytes_;
  size_t size_;
  size_t start_;
  size_t end_;
};

}  // namespace rtc

#endif  // RTC_BASE_BYTE_BUFFER_H_
