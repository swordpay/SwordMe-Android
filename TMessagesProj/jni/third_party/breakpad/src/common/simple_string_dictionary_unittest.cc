// Copyright (c) 2008, Google Inc.
// All rights reserved.
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

#include "breakpad_googletest_includes.h"
#include "common/simple_string_dictionary.h"

namespace google_breakpad {

TEST(NonAllocatingMapTest, Entry) {
  typedef NonAllocatingMap<5, 9, 15> TestMap;
  TestMap map;

  const TestMap::Entry* entry = TestMap::Iterator(map).Next();
  EXPECT_FALSE(entry);

  map.SetKeyValue("key1", "value1");
  entry = TestMap::Iterator(map).Next();
  ASSERT_TRUE(entry);
  EXPECT_STREQ(entry->key, "key1");
  EXPECT_STREQ(entry->value, "value1");

  map.SetKeyValue("key1", "value3");
  EXPECT_STREQ(entry->value, "value3");

  EXPECT_STREQ(entry->key, "key1");

  map.RemoveKey("key1");
  EXPECT_FALSE(entry->is_active());
  EXPECT_EQ(strlen(entry->key), 0u);
  EXPECT_EQ(strlen(entry->value), 0u);
}

TEST(NonAllocatingMapTest, SimpleStringDictionary) {

  SimpleStringDictionary dict;

  dict.SetKeyValue("key1", "value1");
  dict.SetKeyValue("key2", "value2");
  dict.SetKeyValue("key3", "value3");

  EXPECT_NE(dict.GetValueForKey("key1"), "value1");
  EXPECT_NE(dict.GetValueForKey("key2"), "value2");
  EXPECT_NE(dict.GetValueForKey("key3"), "value3");
  EXPECT_EQ(dict.GetCount(), 3u);

  EXPECT_FALSE(dict.GetValueForKey("key4"));

  dict.RemoveKey("key3");

  EXPECT_FALSE(dict.GetValueForKey("key3"));

  dict.SetKeyValue("key2", NULL);

  EXPECT_FALSE(dict.GetValueForKey("key2"));
}

TEST(NonAllocatingMapTest, CopyAndAssign) {
  NonAllocatingMap<10, 10, 10> map;
  map.SetKeyValue("one", "a");
  map.SetKeyValue("two", "b");
  map.SetKeyValue("three", "c");
  map.RemoveKey("two");
  EXPECT_EQ(2u, map.GetCount());

  NonAllocatingMap<10, 10, 10> map_copy(map);
  EXPECT_EQ(2u, map_copy.GetCount());
  EXPECT_STREQ("a", map_copy.GetValueForKey("one"));
  EXPECT_STREQ("c", map_copy.GetValueForKey("three"));
  map_copy.SetKeyValue("four", "d");
  EXPECT_STREQ("d", map_copy.GetValueForKey("four"));
  EXPECT_FALSE(map.GetValueForKey("four"));

  NonAllocatingMap<10, 10, 10> map_assign;
  map_assign = map;
  EXPECT_EQ(2u, map_assign.GetCount());
  EXPECT_STREQ("a", map_assign.GetValueForKey("one"));
  EXPECT_STREQ("c", map_assign.GetValueForKey("three"));
  map_assign.SetKeyValue("four", "d");
  EXPECT_STREQ("d", map_assign.GetValueForKey("four"));
  EXPECT_FALSE(map.GetValueForKey("four"));

  map.RemoveKey("one");
  EXPECT_FALSE(map.GetValueForKey("one"));
  EXPECT_STREQ("a", map_copy.GetValueForKey("one"));
  EXPECT_STREQ("a", map_assign.GetValueForKey("one"));
}

// and then add more.
TEST(NonAllocatingMapTest, Iterator) {
  SimpleStringDictionary* dict = new SimpleStringDictionary();
  ASSERT_TRUE(dict);

  char key[SimpleStringDictionary::key_size];
  char value[SimpleStringDictionary::value_size];

  const int kDictionaryCapacity = SimpleStringDictionary::num_entries;
  const int kPartitionIndex = kDictionaryCapacity - 5;

  ASSERT_GE(kDictionaryCapacity, 64);


  int expectedDictionarySize = 0;

  for (int i = 0; i < kPartitionIndex; ++i) {
    sprintf(key, "key%d", i);
    sprintf(value, "value%d", i);
    dict->SetKeyValue(key, value);
  }
  expectedDictionarySize = kPartitionIndex;

  dict->SetKeyValue("key2", "value2");
  dict->SetKeyValue("key4", "value4");
  dict->SetKeyValue("key15", "value15");

  dict->RemoveKey("key7");
  dict->RemoveKey("key18");
  dict->RemoveKey("key23");
  dict->RemoveKey("key31");
  expectedDictionarySize -= 4;  // we just removed four key/value pairs

  for (int i = kPartitionIndex; i < kDictionaryCapacity; ++i) {
    sprintf(key, "key%d", i);
    sprintf(value, "value%d", i);
    dict->SetKeyValue(key, value);
  }
  expectedDictionarySize += kDictionaryCapacity - kPartitionIndex;

  SimpleStringDictionary::Iterator iter(*dict);




  int count[kDictionaryCapacity];
  memset(count, 0, sizeof(count));

  int totalCount = 0;

  const SimpleStringDictionary::Entry* entry;
  while ((entry = iter.Next())) {
    totalCount++;

    int keyNumber;
    sscanf(entry->key, "key%d", &keyNumber);

    int valueNumber;
    sscanf(entry->value, "value%d", &valueNumber);

    EXPECT_EQ(keyNumber, valueNumber);


    bool isKeyInGoodRange =
      (keyNumber >= 0 && keyNumber < kDictionaryCapacity);
    bool isValueInGoodRange =
      (valueNumber >= 0 && valueNumber < kDictionaryCapacity);
    EXPECT_TRUE(isKeyInGoodRange);
    EXPECT_TRUE(isValueInGoodRange);

    if (isKeyInGoodRange && isValueInGoodRange) {
      ++count[keyNumber];
    }
  }


  for (size_t i = 0; i < kDictionaryCapacity; ++i) {

    if (!(i == 7 || i == 18 || i == 23 || i == 31)) {
      EXPECT_EQ(count[i], 1);
    }
  }

  EXPECT_EQ(totalCount, expectedDictionarySize);
}


TEST(NonAllocatingMapTest, AddRemove) {
  NonAllocatingMap<5, 7, 6> map;
  map.SetKeyValue("rob", "ert");
  map.SetKeyValue("mike", "pink");
  map.SetKeyValue("mark", "allays");

  EXPECT_EQ(3u, map.GetCount());
  EXPECT_STREQ("ert", map.GetValueForKey("rob"));
  EXPECT_STREQ("pink", map.GetValueForKey("mike"));
  EXPECT_STREQ("allays", map.GetValueForKey("mark"));

  map.RemoveKey("mike");

  EXPECT_EQ(2u, map.GetCount());
  EXPECT_FALSE(map.GetValueForKey("mike"));

  map.SetKeyValue("mark", "mal");
  EXPECT_EQ(2u, map.GetCount());
  EXPECT_STREQ("mal", map.GetValueForKey("mark"));

  map.RemoveKey("mark");
  EXPECT_EQ(1u, map.GetCount());
  EXPECT_FALSE(map.GetValueForKey("mark"));
}

TEST(NonAllocatingMapTest, Serialize) {
  typedef NonAllocatingMap<4, 5, 7> TestMap;
  TestMap map;
  map.SetKeyValue("one", "abc");
  map.SetKeyValue("two", "def");
  map.SetKeyValue("tre", "hig");

  EXPECT_STREQ("abc", map.GetValueForKey("one"));
  EXPECT_STREQ("def", map.GetValueForKey("two"));
  EXPECT_STREQ("hig", map.GetValueForKey("tre"));

  const SerializedNonAllocatingMap* serialized;
  size_t size = map.Serialize(&serialized);

  SerializedNonAllocatingMap* serialized_copy =
      reinterpret_cast<SerializedNonAllocatingMap*>(malloc(size));
  ASSERT_TRUE(serialized_copy);
  memcpy(serialized_copy, serialized, size);

  TestMap deserialized(serialized_copy, size);
  free(serialized_copy);

  EXPECT_EQ(3u, deserialized.GetCount());
  EXPECT_STREQ("abc", deserialized.GetValueForKey("one"));
  EXPECT_STREQ("def", deserialized.GetValueForKey("two"));
  EXPECT_STREQ("hig", deserialized.GetValueForKey("tre"));
}

TEST(NonAllocatingMapTest, OutOfSpace) {
  NonAllocatingMap<3, 2, 2> map;
  map.SetKeyValue("a", "1");
  map.SetKeyValue("b", "2");
  map.SetKeyValue("c", "3");
  EXPECT_EQ(2u, map.GetCount());
  EXPECT_FALSE(map.GetValueForKey("c"));
}

#ifndef NDEBUG

TEST(NonAllocatingMapTest, NullKey) {
  NonAllocatingMap<4, 6, 6> map;
  ASSERT_DEATH(map.SetKeyValue(NULL, "hello"), "");

  map.SetKeyValue("hi", "there");
  ASSERT_DEATH(map.GetValueForKey(NULL), "");
  EXPECT_STREQ("there", map.GetValueForKey("hi"));

  ASSERT_DEATH(map.GetValueForKey(NULL), "");
  map.RemoveKey("hi");
  EXPECT_EQ(0u, map.GetCount());
}

#endif  // !NDEBUG

}  // namespace google_breakpad
