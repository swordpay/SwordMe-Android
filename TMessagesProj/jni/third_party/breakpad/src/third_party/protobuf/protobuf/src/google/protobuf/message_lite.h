// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//          kenton@google.com (Kenton Varda)
//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.
//
// Defines MessageLite, the abstract interface implemented by all (lite
// and non-lite) protocol message objects.

#ifndef GOOGLE_PROTOBUF_MESSAGE_LITE_H__
#define GOOGLE_PROTOBUF_MESSAGE_LITE_H__

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/io/coded_stream.h>

namespace google {
namespace protobuf {

//
// This interface is implemented by all protocol message objects.  Non-lite
// messages additionally implement the Message interface, which is a
// subclass of MessageLite.  Use MessageLite instead when you only need
// the subset of features which it supports -- namely, nothing that uses
// descriptors or reflection.  You can instruct the protocol compiler
// to generate classes which implement only MessageLite, not the full
// Message interface, by adding the following line to the .proto file:
//
//   option optimize_for = LITE_RUNTIME;
//
// This is particularly useful on resource-constrained systems where
// the full protocol buffers runtime library is too big.
//
// Note that on non-constrained systems (e.g. servers) when you need
// to link in lots of protocol definitions, a better way to reduce
// total code footprint is to use optimize_for = CODE_SIZE.  This
// will make the generated code smaller while still supporting all the
// same features (at the expense of speed).  optimize_for = LITE_RUNTIME
// is best when you only have a small number of message types linked
// into your binary, in which case the size of the protocol buffers
// runtime itself is the biggest problem.
class LIBPROTOBUF_EXPORT MessageLite {
 public:
  inline MessageLite() {}
  virtual ~MessageLite();


  virtual string GetTypeName() const = 0;


  virtual MessageLite* New() const = 0;





  virtual void Clear() = 0;

  virtual bool IsInitialized() const = 0;



  virtual string InitializationErrorString() const;


  virtual void CheckTypeAndMergeFrom(const MessageLite& other) = 0;






  bool ParseFromCodedStream(io::CodedInputStream* input);


  bool ParsePartialFromCodedStream(io::CodedInputStream* input);


  bool ParseFromZeroCopyStream(io::ZeroCopyInputStream* input);


  bool ParsePartialFromZeroCopyStream(io::ZeroCopyInputStream* input);



  bool ParseFromBoundedZeroCopyStream(io::ZeroCopyInputStream* input, int size);


  bool ParsePartialFromBoundedZeroCopyStream(io::ZeroCopyInputStream* input,
                                             int size);

  bool ParseFromString(const string& data);


  bool ParsePartialFromString(const string& data);

  bool ParseFromArray(const void* data, int size);


  bool ParsePartialFromArray(const void* data, int size);











  bool MergeFromCodedStream(io::CodedInputStream* input);





  virtual bool MergePartialFromCodedStream(io::CodedInputStream* input) = 0;






  bool SerializeToCodedStream(io::CodedOutputStream* output) const;

  bool SerializePartialToCodedStream(io::CodedOutputStream* output) const;


  bool SerializeToZeroCopyStream(io::ZeroCopyOutputStream* output) const;

  bool SerializePartialToZeroCopyStream(io::ZeroCopyOutputStream* output) const;


  bool SerializeToString(string* output) const;

  bool SerializePartialToString(string* output) const;


  bool SerializeToArray(void* data, int size) const;

  bool SerializePartialToArray(void* data, int size) const;






  string SerializeAsString() const;

  string SerializePartialAsString() const;


  bool AppendToString(string* output) const;

  bool AppendPartialToString(string* output) const;



  virtual int ByteSize() const = 0;



  virtual void SerializeWithCachedSizes(
      io::CodedOutputStream* output) const = 0;



  virtual uint8* SerializeWithCachedSizesToArray(uint8* target) const;











  virtual int GetCachedSize() const = 0;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(MessageLite);
};

}  // namespace protobuf

}  // namespace google
#endif  // GOOGLE_PROTOBUF_MESSAGE_LITE_H__
