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

#include <google/protobuf/extension_set.h>
#include <google/protobuf/unittest.pb.h>
#include <google/protobuf/test_util.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/wire_format.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/testing/googletest.h>
#include <gtest/gtest.h>
#include <google/protobuf/stubs/stl_util-inl.h>

namespace google {
namespace protobuf {
namespace internal {
namespace {

// except that it uses extensions rather than regular fields.

TEST(ExtensionSetTest, Defaults) {

  unittest::TestAllExtensions message;

  TestUtil::ExpectExtensionsClear(message);



  EXPECT_EQ(&unittest::OptionalGroup_extension::default_instance(),
            &message.GetExtension(unittest::optionalgroup_extension));
  EXPECT_EQ(&unittest::TestAllTypes::NestedMessage::default_instance(),
            &message.GetExtension(unittest::optional_nested_message_extension));
  EXPECT_EQ(&unittest::ForeignMessage::default_instance(),
            &message.GetExtension(
              unittest::optional_foreign_message_extension));
  EXPECT_EQ(&unittest_import::ImportMessage::default_instance(),
            &message.GetExtension(unittest::optional_import_message_extension));
}

TEST(ExtensionSetTest, Accessors) {


  unittest::TestAllExtensions message;

  TestUtil::SetAllExtensions(&message);
  TestUtil::ExpectAllExtensionsSet(message);

  TestUtil::ModifyRepeatedExtensions(&message);
  TestUtil::ExpectRepeatedExtensionsModified(message);
}

TEST(ExtensionSetTest, Clear) {


  unittest::TestAllExtensions message;

  TestUtil::SetAllExtensions(&message);
  message.Clear();
  TestUtil::ExpectExtensionsClear(message);




  EXPECT_NE(&unittest::OptionalGroup_extension::default_instance(),
            &message.GetExtension(unittest::optionalgroup_extension));
  EXPECT_NE(&unittest::TestAllTypes::NestedMessage::default_instance(),
            &message.GetExtension(unittest::optional_nested_message_extension));
  EXPECT_NE(&unittest::ForeignMessage::default_instance(),
            &message.GetExtension(
              unittest::optional_foreign_message_extension));
  EXPECT_NE(&unittest_import::ImportMessage::default_instance(),
            &message.GetExtension(unittest::optional_import_message_extension));


  TestUtil::SetAllExtensions(&message);
  TestUtil::ExpectAllExtensionsSet(message);
}

TEST(ExtensionSetTest, ClearOneField) {


  unittest::TestAllExtensions message;

  TestUtil::SetAllExtensions(&message);
  int64 original_value =
    message.GetExtension(unittest::optional_int64_extension);

  message.ClearExtension(unittest::optional_int64_extension);
  EXPECT_FALSE(message.HasExtension(unittest::optional_int64_extension));
  EXPECT_EQ(0, message.GetExtension(unittest::optional_int64_extension));

  EXPECT_TRUE(message.HasExtension(unittest::optional_int32_extension));
  EXPECT_TRUE(message.HasExtension(unittest::optional_uint32_extension));

  message.SetExtension(unittest::optional_int64_extension, original_value);
  TestUtil::ExpectAllExtensionsSet(message);
}

TEST(ExtensionSetTest, CopyFrom) {
  unittest::TestAllExtensions message1, message2;
  string data;

  TestUtil::SetAllExtensions(&message1);
  message2.CopyFrom(message1);
  TestUtil::ExpectAllExtensionsSet(message2);
}

TEST(ExtensionSetTest, CopyFromUpcasted) {
  unittest::TestAllExtensions message1, message2;
  string data;
  const Message& upcasted_message = message1;

  TestUtil::SetAllExtensions(&message1);
  message2.CopyFrom(upcasted_message);
  TestUtil::ExpectAllExtensionsSet(message2);
}

TEST(ExtensionSetTest, SwapWithEmpty) {
  unittest::TestAllExtensions message1, message2;
  TestUtil::SetAllExtensions(&message1);

  TestUtil::ExpectAllExtensionsSet(message1);
  TestUtil::ExpectExtensionsClear(message2);
  message1.Swap(&message2);
  TestUtil::ExpectAllExtensionsSet(message2);
  TestUtil::ExpectExtensionsClear(message1);
}

TEST(ExtensionSetTest, SwapWithSelf) {
  unittest::TestAllExtensions message;
  TestUtil::SetAllExtensions(&message);

  TestUtil::ExpectAllExtensionsSet(message);
  message.Swap(&message);
  TestUtil::ExpectAllExtensionsSet(message);
}

TEST(ExtensionSetTest, SerializationToArray) {






  unittest::TestAllExtensions source;
  unittest::TestAllTypes destination;
  TestUtil::SetAllExtensions(&source);
  int size = source.ByteSize();
  string data;
  data.resize(size);
  uint8* target = reinterpret_cast<uint8*>(string_as_array(&data));
  uint8* end = source.SerializeWithCachedSizesToArray(target);
  EXPECT_EQ(size, end - target);
  EXPECT_TRUE(destination.ParseFromString(data));
  TestUtil::ExpectAllFieldsSet(destination);
}

TEST(ExtensionSetTest, SerializationToStream) {







  unittest::TestAllExtensions source;
  unittest::TestAllTypes destination;
  TestUtil::SetAllExtensions(&source);
  int size = source.ByteSize();
  string data;
  data.resize(size);
  {
    io::ArrayOutputStream array_stream(string_as_array(&data), size, 1);
    io::CodedOutputStream output_stream(&array_stream);
    source.SerializeWithCachedSizes(&output_stream);
    ASSERT_FALSE(output_stream.HadError());
  }
  EXPECT_TRUE(destination.ParseFromString(data));
  TestUtil::ExpectAllFieldsSet(destination);
}

TEST(ExtensionSetTest, PackedSerializationToArray) {






  unittest::TestPackedExtensions source;
  unittest::TestPackedTypes destination;
  TestUtil::SetPackedExtensions(&source);
  int size = source.ByteSize();
  string data;
  data.resize(size);
  uint8* target = reinterpret_cast<uint8*>(string_as_array(&data));
  uint8* end = source.SerializeWithCachedSizesToArray(target);
  EXPECT_EQ(size, end - target);
  EXPECT_TRUE(destination.ParseFromString(data));
  TestUtil::ExpectPackedFieldsSet(destination);
}

TEST(ExtensionSetTest, PackedSerializationToStream) {







  unittest::TestPackedExtensions source;
  unittest::TestPackedTypes destination;
  TestUtil::SetPackedExtensions(&source);
  int size = source.ByteSize();
  string data;
  data.resize(size);
  {
    io::ArrayOutputStream array_stream(string_as_array(&data), size, 1);
    io::CodedOutputStream output_stream(&array_stream);
    source.SerializeWithCachedSizes(&output_stream);
    ASSERT_FALSE(output_stream.HadError());
  }
  EXPECT_TRUE(destination.ParseFromString(data));
  TestUtil::ExpectPackedFieldsSet(destination);
}

TEST(ExtensionSetTest, Parsing) {

  unittest::TestAllTypes source;
  unittest::TestAllExtensions destination;
  string data;

  TestUtil::SetAllFields(&source);
  source.SerializeToString(&data);
  EXPECT_TRUE(destination.ParseFromString(data));
  TestUtil::ExpectAllExtensionsSet(destination);
}

TEST(ExtensionSetTest, PackedParsing) {

  unittest::TestPackedTypes source;
  unittest::TestPackedExtensions destination;
  string data;

  TestUtil::SetPackedFields(&source);
  source.SerializeToString(&data);
  EXPECT_TRUE(destination.ParseFromString(data));
  TestUtil::ExpectPackedExtensionsSet(destination);
}

TEST(ExtensionSetTest, IsInitialized) {


  unittest::TestAllExtensions message;

  EXPECT_TRUE(message.IsInitialized());

  message.MutableExtension(unittest::TestRequired::single);
  EXPECT_FALSE(message.IsInitialized());

  message.MutableExtension(unittest::TestRequired::single)->set_a(1);
  EXPECT_FALSE(message.IsInitialized());
  message.MutableExtension(unittest::TestRequired::single)->set_b(2);
  EXPECT_FALSE(message.IsInitialized());
  message.MutableExtension(unittest::TestRequired::single)->set_c(3);
  EXPECT_TRUE(message.IsInitialized());

  message.AddExtension(unittest::TestRequired::multi);
  EXPECT_FALSE(message.IsInitialized());

  message.MutableExtension(unittest::TestRequired::multi, 0)->set_a(1);
  EXPECT_FALSE(message.IsInitialized());
  message.MutableExtension(unittest::TestRequired::multi, 0)->set_b(2);
  EXPECT_FALSE(message.IsInitialized());
  message.MutableExtension(unittest::TestRequired::multi, 0)->set_c(3);
  EXPECT_TRUE(message.IsInitialized());
}

TEST(ExtensionSetTest, MutableString) {

  unittest::TestAllExtensions message;

  message.MutableExtension(unittest::optional_string_extension)->assign("foo");
  EXPECT_TRUE(message.HasExtension(unittest::optional_string_extension));
  EXPECT_EQ("foo", message.GetExtension(unittest::optional_string_extension));

  message.AddExtension(unittest::repeated_string_extension)->assign("bar");
  ASSERT_EQ(1, message.ExtensionSize(unittest::repeated_string_extension));
  EXPECT_EQ("bar",
            message.GetExtension(unittest::repeated_string_extension, 0));
}

TEST(ExtensionSetTest, SpaceUsedExcludingSelf) {


#define TEST_SCALAR_EXTENSIONS_SPACE_USED(type, value)                        \
  do {                                                                        \
    unittest::TestAllExtensions message;                                      \
    const int base_size = message.SpaceUsed();                                \
    message.SetExtension(unittest::optional_##type##_extension, value);       \
    int min_expected_size = base_size +                                       \
        sizeof(message.GetExtension(unittest::optional_##type##_extension));  \
    EXPECT_LE(min_expected_size, message.SpaceUsed());                        \
  } while (0)

  TEST_SCALAR_EXTENSIONS_SPACE_USED(int32   , 101);
  TEST_SCALAR_EXTENSIONS_SPACE_USED(int64   , 102);
  TEST_SCALAR_EXTENSIONS_SPACE_USED(uint32  , 103);
  TEST_SCALAR_EXTENSIONS_SPACE_USED(uint64  , 104);
  TEST_SCALAR_EXTENSIONS_SPACE_USED(sint32  , 105);
  TEST_SCALAR_EXTENSIONS_SPACE_USED(sint64  , 106);
  TEST_SCALAR_EXTENSIONS_SPACE_USED(fixed32 , 107);
  TEST_SCALAR_EXTENSIONS_SPACE_USED(fixed64 , 108);
  TEST_SCALAR_EXTENSIONS_SPACE_USED(sfixed32, 109);
  TEST_SCALAR_EXTENSIONS_SPACE_USED(sfixed64, 110);
  TEST_SCALAR_EXTENSIONS_SPACE_USED(float   , 111);
  TEST_SCALAR_EXTENSIONS_SPACE_USED(double  , 112);
  TEST_SCALAR_EXTENSIONS_SPACE_USED(bool    , true);
#undef TEST_SCALAR_EXTENSIONS_SPACE_USED
  {
    unittest::TestAllExtensions message;
    const int base_size = message.SpaceUsed();
    message.SetExtension(unittest::optional_nested_enum_extension,
                         unittest::TestAllTypes::FOO);
    int min_expected_size = base_size +
        sizeof(message.GetExtension(unittest::optional_nested_enum_extension));
    EXPECT_LE(min_expected_size, message.SpaceUsed());
  }
  {


    unittest::TestAllExtensions message;
    const int base_size = message.SpaceUsed();
    const string s("this is a fairly large string that will cause some "
                   "allocation in order to store it in the extension");
    message.SetExtension(unittest::optional_string_extension, s);
    int min_expected_size = base_size + s.length();
    EXPECT_LE(min_expected_size, message.SpaceUsed());
  }
  {

    unittest::TestAllExtensions message;
    const int base_size = message.SpaceUsed();
    unittest::ForeignMessage foreign;
    foreign.set_c(42);
    message.MutableExtension(unittest::optional_foreign_message_extension)->
        CopyFrom(foreign);
    int min_expected_size = base_size + foreign.SpaceUsed();
    EXPECT_LE(min_expected_size, message.SpaceUsed());
  }










#define TEST_REPEATED_EXTENSIONS_SPACE_USED(type, cpptype, value)              \
  do {                                                                         \
    unittest::TestAllExtensions message;                                       \
    const int base_size = message.SpaceUsed();                                 \
    int min_expected_size = sizeof(RepeatedField<cpptype>) + base_size;        \
    message.AddExtension(unittest::repeated_##type##_extension, value);        \
    message.ClearExtension(unittest::repeated_##type##_extension);             \
    const int empty_repeated_field_size = message.SpaceUsed();                 \
    EXPECT_LE(min_expected_size, empty_repeated_field_size) << #type;          \
    message.AddExtension(unittest::repeated_##type##_extension, value);        \
    message.AddExtension(unittest::repeated_##type##_extension, value);        \
    EXPECT_EQ(empty_repeated_field_size, message.SpaceUsed()) << #type;        \
    message.ClearExtension(unittest::repeated_##type##_extension);             \
    for (int i = 0; i < 16; ++i) {                                             \
      message.AddExtension(unittest::repeated_##type##_extension, value);      \
    }                                                                          \
    int expected_size = sizeof(cpptype) * 16 + empty_repeated_field_size;      \
    EXPECT_EQ(expected_size, message.SpaceUsed()) << #type;                    \
  } while (0)

  TEST_REPEATED_EXTENSIONS_SPACE_USED(int32   , int32 , 101);
  TEST_REPEATED_EXTENSIONS_SPACE_USED(int64   , int64 , 102);
  TEST_REPEATED_EXTENSIONS_SPACE_USED(uint32  , uint32, 103);
  TEST_REPEATED_EXTENSIONS_SPACE_USED(uint64  , uint64, 104);
  TEST_REPEATED_EXTENSIONS_SPACE_USED(sint32  , int32 , 105);
  TEST_REPEATED_EXTENSIONS_SPACE_USED(sint64  , int64 , 106);
  TEST_REPEATED_EXTENSIONS_SPACE_USED(fixed32 , uint32, 107);
  TEST_REPEATED_EXTENSIONS_SPACE_USED(fixed64 , uint64, 108);
  TEST_REPEATED_EXTENSIONS_SPACE_USED(sfixed32, int32 , 109);
  TEST_REPEATED_EXTENSIONS_SPACE_USED(sfixed64, int64 , 110);
  TEST_REPEATED_EXTENSIONS_SPACE_USED(float   , float , 111);
  TEST_REPEATED_EXTENSIONS_SPACE_USED(double  , double, 112);
  TEST_REPEATED_EXTENSIONS_SPACE_USED(bool    , bool  , true);
  TEST_REPEATED_EXTENSIONS_SPACE_USED(nested_enum, int,
                                      unittest::TestAllTypes::FOO);
#undef TEST_REPEATED_EXTENSIONS_SPACE_USED

  {
    unittest::TestAllExtensions message;
    const int base_size = message.SpaceUsed();
    int min_expected_size = sizeof(RepeatedPtrField<string>) + base_size;
    const string value(256, 'x');



    for (int i = 0; i < 16; ++i) {
      message.AddExtension(unittest::repeated_string_extension, value);
    }
    min_expected_size += (sizeof(value) + value.size()) * 16;
    EXPECT_LE(min_expected_size, message.SpaceUsed());
  }

  {
    unittest::TestAllExtensions message;
    const int base_size = message.SpaceUsed();
    int min_expected_size = sizeof(RepeatedPtrField<unittest::ForeignMessage>) +
        base_size;
    unittest::ForeignMessage prototype;
    prototype.set_c(2);
    for (int i = 0; i < 16; ++i) {
      message.AddExtension(unittest::repeated_foreign_message_extension)->
          CopyFrom(prototype);
    }
    min_expected_size += 16 * prototype.SpaceUsed();
    EXPECT_LE(min_expected_size, message.SpaceUsed());
  }
}

#ifdef GTEST_HAS_DEATH_TEST

TEST(ExtensionSetTest, InvalidEnumDeath) {
  unittest::TestAllExtensions message;
  EXPECT_DEBUG_DEATH(
    message.SetExtension(unittest::optional_foreign_enum_extension,
                         static_cast<unittest::ForeignEnum>(53)),
    "IsValid");
}

#endif  // GTEST_HAS_DEATH_TEST

TEST(ExtensionSetTest, DynamicExtensions) {


  FileDescriptorProto dynamic_proto;
  dynamic_proto.set_name("dynamic_extensions_test.proto");
  dynamic_proto.add_dependency(
      unittest::TestAllExtensions::descriptor()->file()->name());
  dynamic_proto.set_package("dynamic_extensions");


  const Descriptor* template_descriptor =
      unittest::TestDynamicExtensions::descriptor();
  DescriptorProto template_descriptor_proto;
  template_descriptor->CopyTo(&template_descriptor_proto);
  dynamic_proto.mutable_message_type()->MergeFrom(
      template_descriptor_proto.nested_type());
  dynamic_proto.mutable_enum_type()->MergeFrom(
      template_descriptor_proto.enum_type());
  dynamic_proto.mutable_extension()->MergeFrom(
      template_descriptor_proto.field());

  for (int i = 0; i < dynamic_proto.extension_size(); i++) {

    FieldDescriptorProto* extension = dynamic_proto.mutable_extension(i);
    extension->set_extendee(
        unittest::TestAllExtensions::descriptor()->full_name());


    string prefix = "." + template_descriptor->full_name() + ".";
    if (extension->has_type_name()) {
      string* type_name = extension->mutable_type_name();
      if (HasPrefixString(*type_name, prefix)) {
        type_name->replace(0, prefix.size(), ".dynamic_extensions.");
      }
    }
  }

  DescriptorPool dynamic_pool(DescriptorPool::generated_pool());
  const FileDescriptor* file = dynamic_pool.BuildFile(dynamic_proto);
  ASSERT_TRUE(file != NULL);
  DynamicMessageFactory dynamic_factory(&dynamic_pool);
  dynamic_factory.SetDelegateToGeneratedFactory(true);



  string data;
  {
    unittest::TestDynamicExtensions message;
    message.set_scalar_extension(123);
    message.set_enum_extension(unittest::FOREIGN_BAR);
    message.set_dynamic_enum_extension(
        unittest::TestDynamicExtensions::DYNAMIC_BAZ);
    message.mutable_message_extension()->set_c(456);
    message.mutable_dynamic_message_extension()->set_dynamic_field(789);
    message.add_repeated_extension("foo");
    message.add_repeated_extension("bar");
    message.add_packed_extension(12);
    message.add_packed_extension(-34);
    message.add_packed_extension(56);
    message.add_packed_extension(-78);


    message.mutable_unknown_fields()->AddVarint(
      unittest::TestDynamicExtensions::kDynamicEnumExtensionFieldNumber,
      12345);

    message.mutable_unknown_fields()->AddLengthDelimited(54321, "unknown");

    message.SerializeToString(&data);
  }

  unittest::TestAllExtensions message;
  {
    io::ArrayInputStream raw_input(data.data(), data.size());
    io::CodedInputStream input(&raw_input);
    input.SetExtensionRegistry(&dynamic_pool, &dynamic_factory);
    ASSERT_TRUE(message.ParseFromCodedStream(&input));
    ASSERT_TRUE(input.ConsumedEntireMessage());
  }

  EXPECT_EQ(
    "[dynamic_extensions.scalar_extension]: 123\n"
    "[dynamic_extensions.enum_extension]: FOREIGN_BAR\n"
    "[dynamic_extensions.dynamic_enum_extension]: DYNAMIC_BAZ\n"
    "[dynamic_extensions.message_extension] {\n"
    "  c: 456\n"
    "}\n"
    "[dynamic_extensions.dynamic_message_extension] {\n"
    "  dynamic_field: 789\n"
    "}\n"
    "[dynamic_extensions.repeated_extension]: \"foo\"\n"
    "[dynamic_extensions.repeated_extension]: \"bar\"\n"
    "[dynamic_extensions.packed_extension]: 12\n"
    "[dynamic_extensions.packed_extension]: -34\n"
    "[dynamic_extensions.packed_extension]: 56\n"
    "[dynamic_extensions.packed_extension]: -78\n"
    "2002: 12345\n"
    "54321: \"unknown\"\n",
    message.DebugString());



  EXPECT_TRUE(message.SerializeAsString() == data);

  {
    unittest::TestAllExtensions message2;
    io::ArrayInputStream raw_input(data.data(), data.size());
    io::CodedInputStream input(&raw_input);
    input.SetExtensionRegistry(&dynamic_pool, &dynamic_factory);
    ASSERT_TRUE(WireFormat::ParseAndMergePartial(&input, &message2));
    ASSERT_TRUE(input.ConsumedEntireMessage());
    EXPECT_EQ(message.DebugString(), message2.DebugString());
  }

  {
    const FieldDescriptor* message_extension =
        file->FindExtensionByName("message_extension");
    ASSERT_TRUE(message_extension != NULL);
    const Message& sub_message =
        message.GetReflection()->GetMessage(message, message_extension);
    const unittest::ForeignMessage* typed_sub_message =
        dynamic_cast<const unittest::ForeignMessage*>(&sub_message);
    ASSERT_TRUE(typed_sub_message != NULL);
    EXPECT_EQ(456, typed_sub_message->c());
  }


  {
    const FieldDescriptor* dynamic_message_extension =
        file->FindExtensionByName("dynamic_message_extension");
    ASSERT_TRUE(dynamic_message_extension != NULL);
    const Message& parent = unittest::TestAllExtensions::default_instance();
    const Message& sub_message =
        parent.GetReflection()->GetMessage(parent, dynamic_message_extension,
                                           &dynamic_factory);
    const Message* prototype =
        dynamic_factory.GetPrototype(dynamic_message_extension->message_type());
    EXPECT_EQ(prototype, &sub_message);
  }
}

}  // namespace
}  // namespace internal
}  // namespace protobuf
}  // namespace google
