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

//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.
//
// Defines Message, the abstract interface implemented by non-lite
// protocol message objects.  Although it's possible to implement this
// interface manually, most users will use the protocol compiler to
// generate implementations.
//
// Example usage:
//
// Say you have a message defined as:
//
//   message Foo {
//     optional string text = 1;
//     repeated int32 numbers = 2;
//   }
//
// Then, if you used the protocol compiler to generate a class from the above
// definition, you could use it like so:
//
//   string data;  // Will store a serialized version of the message.
//
//   {
//     // Create a message and serialize it.
//     Foo foo;
//     foo.set_text("Hello World!");
//     foo.add_numbers(1);
//     foo.add_numbers(5);
//     foo.add_numbers(42);
//
//     foo.SerializeToString(&data);
//   }
//
//   {
//     // Parse the serialized message and check that it contains the
//     // correct data.
//     Foo foo;
//     foo.ParseFromString(data);
//
//     assert(foo.text() == "Hello World!");
//     assert(foo.numbers_size() == 3);
//     assert(foo.numbers(0) == 1);
//     assert(foo.numbers(1) == 5);
//     assert(foo.numbers(2) == 42);
//   }
//
//   {
//     // Same as the last block, but do it dynamically via the Message
//     // reflection interface.
//     Message* foo = new Foo;
//     Descriptor* descriptor = foo->GetDescriptor();
//
//     // Get the descriptors for the fields we're interested in and verify
//     // their types.
//     FieldDescriptor* text_field = descriptor->FindFieldByName("text");
//     assert(text_field != NULL);
//     assert(text_field->type() == FieldDescriptor::TYPE_STRING);
//     assert(text_field->label() == FieldDescriptor::TYPE_OPTIONAL);
//     FieldDescriptor* numbers_field = descriptor->FindFieldByName("numbers");
//     assert(numbers_field != NULL);
//     assert(numbers_field->type() == FieldDescriptor::TYPE_INT32);
//     assert(numbers_field->label() == FieldDescriptor::TYPE_REPEATED);
//
//     // Parse the message.
//     foo->ParseFromString(data);
//
//     // Use the reflection interface to examine the contents.
//     const Reflection* reflection = foo->GetReflection();
//     assert(reflection->GetString(foo, text_field) == "Hello World!");
//     assert(reflection->FieldSize(foo, numbers_field) == 3);
//     assert(reflection->GetRepeatedInt32(foo, numbers_field, 0) == 1);
//     assert(reflection->GetRepeatedInt32(foo, numbers_field, 1) == 5);
//     assert(reflection->GetRepeatedInt32(foo, numbers_field, 2) == 42);
//
//     delete foo;
//   }

#ifndef GOOGLE_PROTOBUF_MESSAGE_H__
#define GOOGLE_PROTOBUF_MESSAGE_H__

#include <vector>
#include <string>

#ifdef __DECCXX
// HP C++'s iosfwd doesn't work.
#include <iostream>
#else
#include <iosfwd>
#endif

#include <google/protobuf/message_lite.h>

#include <google/protobuf/stubs/common.h>


namespace google {
namespace protobuf {

class Message;
class Reflection;
class MessageFactory;

class Descriptor;            // descriptor.h
class FieldDescriptor;       // descriptor.h
class EnumDescriptor;        // descriptor.h
class EnumValueDescriptor;   // descriptor.h
namespace io {
  class ZeroCopyInputStream;   // zero_copy_stream.h
  class ZeroCopyOutputStream;  // zero_copy_stream.h
  class CodedInputStream;      // coded_stream.h
  class CodedOutputStream;     // coded_stream.h
}
class UnknownFieldSet;       // unknown_field_set.h

struct Metadata {
  const Descriptor* descriptor;
  const Reflection* reflection;
};

// proto-declared enum type.  Code generated by the protocol compiler
// will include specializations of this template for each enum type declared.
template <typename E>
const EnumDescriptor* GetEnumDescriptor();

//
// See also MessageLite, which contains most every-day operations.  Message
// adds descriptors and reflection on top of that.
//
// The methods of this class that are virtual but not pure-virtual have
// default implementations based on reflection.  Message classes which are
// optimized for speed will want to override these with faster implementations,
// but classes optimized for code size may be happy with keeping them.  See
// the optimize_for option in descriptor.proto.
class LIBPROTOBUF_EXPORT Message : public MessageLite {
 public:
  inline Message() {}
  virtual ~Message();




  virtual Message* New() const = 0;



  virtual void CopyFrom(const Message& from);




  virtual void MergeFrom(const Message& from);


  void CheckInitialized() const;




  void FindInitializationErrors(vector<string>* errors) const;


  string InitializationErrorString() const;










  virtual void DiscardUnknownFields();



  virtual int SpaceUsed() const;



  string DebugString() const;

  string ShortDebugString() const;

  string Utf8DebugString() const;

  void PrintDebugString() const;





  bool ParseFromFileDescriptor(int file_descriptor);


  bool ParsePartialFromFileDescriptor(int file_descriptor);


  bool ParseFromIstream(istream* input);


  bool ParsePartialFromIstream(istream* input);


  bool SerializeToFileDescriptor(int file_descriptor) const;

  bool SerializePartialToFileDescriptor(int file_descriptor) const;


  bool SerializeToOstream(ostream* output) const;

  bool SerializePartialToOstream(ostream* output) const;




  virtual string GetTypeName() const;
  virtual void Clear();
  virtual bool IsInitialized() const;
  virtual void CheckTypeAndMergeFrom(const MessageLite& other);
  virtual bool MergePartialFromCodedStream(io::CodedInputStream* input);
  virtual int ByteSize() const;
  virtual void SerializeWithCachedSizes(io::CodedOutputStream* output) const;

 private:







  virtual void SetCachedSize(int size) const;

 public:


  typedef google::protobuf::Reflection Reflection;


  const Descriptor* GetDescriptor() const { return GetMetadata().descriptor; }







  virtual const Reflection* GetReflection() const {
    return GetMetadata().reflection;
  }

 protected:



  virtual Metadata GetMetadata() const  = 0;


 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(Message);
};

// and modify the fields of a protocol message.  Their semantics are
// similar to the accessors the protocol compiler generates.
//
// To get the Reflection for a given Message, call Message::GetReflection().
//
// This interface is separate from Message only for efficiency reasons;
// the vast majority of implementations of Message will share the same
// implementation of Reflection (GeneratedMessageReflection,
// defined in generated_message.h), and all Messages of a particular class
// should share the same Reflection object (though you should not rely on
// the latter fact).
//
// There are several ways that these methods can be used incorrectly.  For
// example, any of the following conditions will lead to undefined
// results (probably assertion failures):
// - The FieldDescriptor is not a field of this message type.
// - The method called is not appropriate for the field's type.  For
//   each field type in FieldDescriptor::TYPE_*, there is only one
//   Get*() method, one Set*() method, and one Add*() method that is
//   valid for that type.  It should be obvious which (except maybe
//   for TYPE_BYTES, which are represented using strings in C++).
// - A Get*() or Set*() method for singular fields is called on a repeated
//   field.
// - GetRepeated*(), SetRepeated*(), or Add*() is called on a non-repeated
//   field.
// - The Message object passed to any method is not of the right type for
//   this Reflection object (i.e. message.GetReflection() != reflection).
//
// You might wonder why there is not any abstract representation for a field
// of arbitrary type.  E.g., why isn't there just a "GetField()" method that
// returns "const Field&", where "Field" is some class with accessors like
// "GetInt32Value()".  The problem is that someone would have to deal with
// allocating these Field objects.  For generated message classes, having to
// allocate space for an additional object to wrap every field would at least
// double the message's memory footprint, probably worse.  Allocating the
// objects on-demand, on the other hand, would be expensive and prone to
// memory leaks.  So, instead we ended up with this flat interface.
//
// TODO(kenton):  Create a utility class which callers can use to read and
//   write fields from a Reflection without paying attention to the type.
class LIBPROTOBUF_EXPORT Reflection {
 public:

  inline Reflection() {}
  virtual ~Reflection();



  virtual const UnknownFieldSet& GetUnknownFields(
      const Message& message) const = 0;



  virtual UnknownFieldSet* MutableUnknownFields(Message* message) const = 0;

  virtual int SpaceUsed(const Message& message) const = 0;

  virtual bool HasField(const Message& message,
                        const FieldDescriptor* field) const = 0;

  virtual int FieldSize(const Message& message,
                        const FieldDescriptor* field) const = 0;


  virtual void ClearField(Message* message,
                          const FieldDescriptor* field) const = 0;







  virtual void RemoveLast(Message* message,
                          const FieldDescriptor* field) const = 0;

  virtual void Swap(Message* message1, Message* message2) const = 0;

  virtual void SwapElements(Message* message,
                    const FieldDescriptor* field,
                    int index1,
                    int index2) const = 0;





  virtual void ListFields(const Message& message,
                          vector<const FieldDescriptor*>* output) const = 0;




  virtual int32  GetInt32 (const Message& message,
                           const FieldDescriptor* field) const = 0;
  virtual int64  GetInt64 (const Message& message,
                           const FieldDescriptor* field) const = 0;
  virtual uint32 GetUInt32(const Message& message,
                           const FieldDescriptor* field) const = 0;
  virtual uint64 GetUInt64(const Message& message,
                           const FieldDescriptor* field) const = 0;
  virtual float  GetFloat (const Message& message,
                           const FieldDescriptor* field) const = 0;
  virtual double GetDouble(const Message& message,
                           const FieldDescriptor* field) const = 0;
  virtual bool   GetBool  (const Message& message,
                           const FieldDescriptor* field) const = 0;
  virtual string GetString(const Message& message,
                           const FieldDescriptor* field) const = 0;
  virtual const EnumValueDescriptor* GetEnum(
      const Message& message, const FieldDescriptor* field) const = 0;

  virtual const Message& GetMessage(const Message& message,
                                    const FieldDescriptor* field,
                                    MessageFactory* factory = NULL) const = 0;















  virtual const string& GetStringReference(const Message& message,
                                           const FieldDescriptor* field,
                                           string* scratch) const = 0;



  virtual void SetInt32 (Message* message,
                         const FieldDescriptor* field, int32  value) const = 0;
  virtual void SetInt64 (Message* message,
                         const FieldDescriptor* field, int64  value) const = 0;
  virtual void SetUInt32(Message* message,
                         const FieldDescriptor* field, uint32 value) const = 0;
  virtual void SetUInt64(Message* message,
                         const FieldDescriptor* field, uint64 value) const = 0;
  virtual void SetFloat (Message* message,
                         const FieldDescriptor* field, float  value) const = 0;
  virtual void SetDouble(Message* message,
                         const FieldDescriptor* field, double value) const = 0;
  virtual void SetBool  (Message* message,
                         const FieldDescriptor* field, bool   value) const = 0;
  virtual void SetString(Message* message,
                         const FieldDescriptor* field,
                         const string& value) const = 0;
  virtual void SetEnum  (Message* message,
                         const FieldDescriptor* field,
                         const EnumValueDescriptor* value) const = 0;










  virtual Message* MutableMessage(Message* message,
                                  const FieldDescriptor* field,
                                  MessageFactory* factory = NULL) const = 0;



  virtual int32  GetRepeatedInt32 (const Message& message,
                                   const FieldDescriptor* field,
                                   int index) const = 0;
  virtual int64  GetRepeatedInt64 (const Message& message,
                                   const FieldDescriptor* field,
                                   int index) const = 0;
  virtual uint32 GetRepeatedUInt32(const Message& message,
                                   const FieldDescriptor* field,
                                   int index) const = 0;
  virtual uint64 GetRepeatedUInt64(const Message& message,
                                   const FieldDescriptor* field,
                                   int index) const = 0;
  virtual float  GetRepeatedFloat (const Message& message,
                                   const FieldDescriptor* field,
                                   int index) const = 0;
  virtual double GetRepeatedDouble(const Message& message,
                                   const FieldDescriptor* field,
                                   int index) const = 0;
  virtual bool   GetRepeatedBool  (const Message& message,
                                   const FieldDescriptor* field,
                                   int index) const = 0;
  virtual string GetRepeatedString(const Message& message,
                                   const FieldDescriptor* field,
                                   int index) const = 0;
  virtual const EnumValueDescriptor* GetRepeatedEnum(
      const Message& message,
      const FieldDescriptor* field, int index) const = 0;
  virtual const Message& GetRepeatedMessage(
      const Message& message,
      const FieldDescriptor* field, int index) const = 0;

  virtual const string& GetRepeatedStringReference(
      const Message& message, const FieldDescriptor* field,
      int index, string* scratch) const = 0;



  virtual void SetRepeatedInt32 (Message* message,
                                 const FieldDescriptor* field,
                                 int index, int32  value) const = 0;
  virtual void SetRepeatedInt64 (Message* message,
                                 const FieldDescriptor* field,
                                 int index, int64  value) const = 0;
  virtual void SetRepeatedUInt32(Message* message,
                                 const FieldDescriptor* field,
                                 int index, uint32 value) const = 0;
  virtual void SetRepeatedUInt64(Message* message,
                                 const FieldDescriptor* field,
                                 int index, uint64 value) const = 0;
  virtual void SetRepeatedFloat (Message* message,
                                 const FieldDescriptor* field,
                                 int index, float  value) const = 0;
  virtual void SetRepeatedDouble(Message* message,
                                 const FieldDescriptor* field,
                                 int index, double value) const = 0;
  virtual void SetRepeatedBool  (Message* message,
                                 const FieldDescriptor* field,
                                 int index, bool   value) const = 0;
  virtual void SetRepeatedString(Message* message,
                                 const FieldDescriptor* field,
                                 int index, const string& value) const = 0;
  virtual void SetRepeatedEnum(Message* message,
                               const FieldDescriptor* field, int index,
                               const EnumValueDescriptor* value) const = 0;


  virtual Message* MutableRepeatedMessage(
      Message* message, const FieldDescriptor* field, int index) const = 0;



  virtual void AddInt32 (Message* message,
                         const FieldDescriptor* field, int32  value) const = 0;
  virtual void AddInt64 (Message* message,
                         const FieldDescriptor* field, int64  value) const = 0;
  virtual void AddUInt32(Message* message,
                         const FieldDescriptor* field, uint32 value) const = 0;
  virtual void AddUInt64(Message* message,
                         const FieldDescriptor* field, uint64 value) const = 0;
  virtual void AddFloat (Message* message,
                         const FieldDescriptor* field, float  value) const = 0;
  virtual void AddDouble(Message* message,
                         const FieldDescriptor* field, double value) const = 0;
  virtual void AddBool  (Message* message,
                         const FieldDescriptor* field, bool   value) const = 0;
  virtual void AddString(Message* message,
                         const FieldDescriptor* field,
                         const string& value) const = 0;
  virtual void AddEnum  (Message* message,
                         const FieldDescriptor* field,
                         const EnumValueDescriptor* value) const = 0;

  virtual Message* AddMessage(Message* message,
                              const FieldDescriptor* field,
                              MessageFactory* factory = NULL) const = 0;



  virtual const FieldDescriptor* FindKnownExtensionByName(
      const string& name) const = 0;


  virtual const FieldDescriptor* FindKnownExtensionByNumber(
      int number) const = 0;

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(Reflection);
};

class LIBPROTOBUF_EXPORT MessageFactory {
 public:
  inline MessageFactory() {}
  virtual ~MessageFactory();


















  virtual const Message* GetPrototype(const Descriptor* type) = 0;














  static MessageFactory* generated_factory();








  static void InternalRegisterGeneratedFile(
      const char* filename, void (*register_messages)(const string&));



  static void InternalRegisterGeneratedMessage(const Descriptor* descriptor,
                                               const Message* prototype);

 private:
  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(MessageFactory);
};

}  // namespace protobuf

}  // namespace google
#endif  // GOOGLE_PROTOBUF_MESSAGE_H__
