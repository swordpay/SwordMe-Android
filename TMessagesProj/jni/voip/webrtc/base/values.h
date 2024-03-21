// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// storing settings and other persistable data.
//
// A Value represents something that can be stored in JSON or passed to/from
// JavaScript. As such, it is NOT a generalized variant type, since only the
// types supported by JavaScript/JSON are supported.
//
// IN PARTICULAR this means that there is no support for int64_t or unsigned
// numbers. Writing JSON with such types would violate the spec. If you need
// something like this, either use a double or make a string value containing
// the number you want.
//
// NOTE: A Value parameter that is always a Value::STRING should just be passed
// as a std::string. Similarly for Values that are always Value::DICTIONARY
// (should be flat_map), Value::LIST (should be std::vector), et cetera.

#ifndef BASE_VALUES_H_
#define BASE_VALUES_H_

#include <stddef.h>
#include <stdint.h>

#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/base_export.h"
#include "base/containers/checked_iterators.h"
#include "base/containers/checked_range.h"
#include "base/containers/flat_map.h"
#include "base/containers/span.h"
#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "base/value_iterators.h"

namespace base {

class DictionaryValue;
class ListValue;
class Value;

// via passing the appropriate type or backing storage to the constructor.
//
// See the file-level comment above for more information.
//
// base::Value is currently in the process of being refactored. Design doc:
// https://docs.google.com/document/d/1uDLu5uTRlCWePxQUEHc8yNQdEoE1BDISYdpggWEABnw
//
// Previously (which is how most code that currently exists is written), Value
// used derived types to implement the individual data types, and base::Value
// was just a base class to refer to them. This required everything be heap
// allocated.
//
// OLD WAY:
//
//   std::unique_ptr<base::Value> GetFoo() {
//     std::unique_ptr<DictionaryValue> dict;
//     dict->SetString("mykey", foo);
//     return dict;
//   }
//
// The new design makes base::Value a variant type that holds everything in
// a union. It is now recommended to pass by value with std::move rather than
// use heap allocated values. The DictionaryValue and ListValue subclasses
// exist only as a compatibility shim that we're in the process of removing.
//
// NEW WAY:
//
//   base::Value GetFoo() {
//     base::Value dict(base::Value::Type::DICTIONARY);
//     dict.SetKey("mykey", base::Value(foo));
//     return dict;
//   }
class BASE_EXPORT Value {
 public:
  using BlobStorage = std::vector<uint8_t>;
  using DictStorage = flat_map<std::string, std::unique_ptr<Value>>;
  using ListStorage = std::vector<Value>;

  using ListView = CheckedContiguousRange<ListStorage>;
  using ConstListView = CheckedContiguousConstRange<ListStorage>;

  using DoubleStorage = struct { alignas(4) char v[sizeof(double)]; };

  enum class Type : unsigned char {
    NONE = 0,
    BOOLEAN,
    INTEGER,
    DOUBLE,
    STRING,
    BINARY,
    DICTIONARY,
    LIST,

    DEAD

  };





  static std::unique_ptr<Value> CreateWithCopiedBuffer(const char* buffer,
                                                       size_t size);

  static Value FromUniquePtrValue(std::unique_ptr<Value> val);
  static std::unique_ptr<Value> ToUniquePtrValue(Value val);
  static const DictionaryValue& AsDictionaryValue(const Value& val);
  static const ListValue& AsListValue(const Value& val);

  Value(Value&& that) noexcept;
  Value() noexcept {}  // A null value





  Value Clone() const;

  explicit Value(Type type);
  explicit Value(bool in_bool);
  explicit Value(int in_int);
  explicit Value(double in_double);




  explicit Value(const char* in_string);
  explicit Value(StringPiece in_string);
  explicit Value(std::string&& in_string) noexcept;
  explicit Value(const char16* in_string16);
  explicit Value(StringPiece16 in_string16);

  explicit Value(const std::vector<char>& in_blob);
  explicit Value(base::span<const uint8_t> in_blob);
  explicit Value(BlobStorage&& in_blob) noexcept;

  explicit Value(const DictStorage& in_dict);
  explicit Value(DictStorage&& in_dict) noexcept;

  explicit Value(span<const Value> in_list);
  explicit Value(ListStorage&& in_list) noexcept;

  Value& operator=(Value&& that) noexcept;

  ~Value();

  static const char* GetTypeName(Type type);

  Type type() const { return type_; }

  bool is_none() const { return type() == Type::NONE; }
  bool is_bool() const { return type() == Type::BOOLEAN; }
  bool is_int() const { return type() == Type::INTEGER; }
  bool is_double() const { return type() == Type::DOUBLE; }
  bool is_string() const { return type() == Type::STRING; }
  bool is_blob() const { return type() == Type::BINARY; }
  bool is_dict() const { return type() == Type::DICTIONARY; }
  bool is_list() const { return type() == Type::LIST; }

  bool GetBool() const;
  int GetInt() const;
  double GetDouble() const;  // Implicitly converts from int if necessary.
  const std::string& GetString() const;
  std::string& GetString();
  const BlobStorage& GetBlob() const;




  ListView GetList();
  ConstListView GetList() const;



  ListStorage TakeList();


  void Append(bool value);
  void Append(int value);
  void Append(double value);
  void Append(const char* value);
  void Append(StringPiece value);
  void Append(std::string&& value);
  void Append(const char16* value);
  void Append(StringPiece16 value);
  void Append(Value&& value);


  CheckedContiguousIterator<Value> Insert(
      CheckedContiguousConstIterator<Value> pos,
      Value&& value);



  bool EraseListIter(CheckedContiguousConstIterator<Value> iter);



  size_t EraseListValue(const Value& val);



  template <typename Predicate>
  size_t EraseListValueIf(Predicate pred) {
    CHECK(is_list());
    return base::EraseIf(list_, pred);
  }


  void ClearList();








  Value* FindKey(StringPiece key);
  const Value* FindKey(StringPiece key) const;









  Value* FindKeyOfType(StringPiece key, Type type);
  const Value* FindKeyOfType(StringPiece key, Type type) const;



  base::Optional<bool> FindBoolKey(StringPiece key) const;
  base::Optional<int> FindIntKey(StringPiece key) const;


  base::Optional<double> FindDoubleKey(StringPiece key) const;

  const std::string* FindStringKey(StringPiece key) const;
  std::string* FindStringKey(StringPiece key);

  const BlobStorage* FindBlobKey(StringPiece key) const;

  const Value* FindDictKey(StringPiece key) const;
  Value* FindDictKey(StringPiece key);

  const Value* FindListKey(StringPiece key) const;
  Value* FindListKey(StringPiece key);








  Value* SetKey(StringPiece key, Value&& value);

  Value* SetKey(std::string&& key, Value&& value);

  Value* SetKey(const char* key, Value&& value);




  Value* SetBoolKey(StringPiece key, bool val);
  Value* SetIntKey(StringPiece key, int val);
  Value* SetDoubleKey(StringPiece key, double val);
  Value* SetStringKey(StringPiece key, StringPiece val);


  Value* SetStringKey(StringPiece key, const char* val);
  Value* SetStringKey(StringPiece key, std::string&& val);
  Value* SetStringKey(StringPiece key, StringPiece16 val);








  bool RemoveKey(StringPiece key);








  Optional<Value> ExtractKey(StringPiece key);














  Value* FindPath(StringPiece path);
  const Value* FindPath(StringPiece path) const;










  Value* FindPath(std::initializer_list<StringPiece> path);
  Value* FindPath(span<const StringPiece> path);
  const Value* FindPath(std::initializer_list<StringPiece> path) const;
  const Value* FindPath(span<const StringPiece> path) const;






  Value* FindPathOfType(StringPiece path, Type type);
  const Value* FindPathOfType(StringPiece path, Type type) const;


  base::Optional<bool> FindBoolPath(StringPiece path) const;
  base::Optional<int> FindIntPath(StringPiece path) const;
  base::Optional<double> FindDoublePath(StringPiece path) const;
  const std::string* FindStringPath(StringPiece path) const;
  std::string* FindStringPath(StringPiece path);
  const BlobStorage* FindBlobPath(StringPiece path) const;
  Value* FindDictPath(StringPiece path);
  const Value* FindDictPath(StringPiece path) const;
  Value* FindListPath(StringPiece path);
  const Value* FindListPath(StringPiece path) const;


  Value* FindPathOfType(std::initializer_list<StringPiece> path, Type type);
  Value* FindPathOfType(span<const StringPiece> path, Type type);
  const Value* FindPathOfType(std::initializer_list<StringPiece> path,
                              Type type) const;
  const Value* FindPathOfType(span<const StringPiece> path, Type type) const;














  Value* SetPath(StringPiece path, Value&& value);


  Value* SetBoolPath(StringPiece path, bool value);
  Value* SetIntPath(StringPiece path, int value);
  Value* SetDoublePath(StringPiece path, double value);
  Value* SetStringPath(StringPiece path, StringPiece value);
  Value* SetStringPath(StringPiece path, const char* value);
  Value* SetStringPath(StringPiece path, std::string&& value);
  Value* SetStringPath(StringPiece path, StringPiece16 value);

  Value* SetPath(std::initializer_list<StringPiece> path, Value&& value);
  Value* SetPath(span<const StringPiece> path, Value&& value);










  bool RemovePath(StringPiece path);

  bool RemovePath(std::initializer_list<StringPiece> path);
  bool RemovePath(span<const StringPiece> path);











  Optional<Value> ExtractPath(StringPiece path);

  using dict_iterator_proxy = detail::dict_iterator_proxy;
  using const_dict_iterator_proxy = detail::const_dict_iterator_proxy;













  dict_iterator_proxy DictItems();
  const_dict_iterator_proxy DictItems() const;


  size_t DictSize() const;
  bool DictEmpty() const;






  void MergeDictionary(const Value* dictionary);





  bool GetAsBoolean(bool* out_value) const;

  bool GetAsInteger(int* out_value) const;

  bool GetAsDouble(double* out_value) const;

  bool GetAsString(std::string* out_value) const;
  bool GetAsString(string16* out_value) const;
  bool GetAsString(const Value** out_value) const;
  bool GetAsString(StringPiece* out_value) const;


  bool GetAsList(ListValue** out_value);
  bool GetAsList(const ListValue** out_value) const;

  bool GetAsDictionary(DictionaryValue** out_value);
  bool GetAsDictionary(const DictionaryValue** out_value) const;







  Value* DeepCopy() const;


  std::unique_ptr<Value> CreateDeepCopy() const;


  BASE_EXPORT friend bool operator==(const Value& lhs, const Value& rhs);
  BASE_EXPORT friend bool operator!=(const Value& lhs, const Value& rhs);
  BASE_EXPORT friend bool operator<(const Value& lhs, const Value& rhs);
  BASE_EXPORT friend bool operator>(const Value& lhs, const Value& rhs);
  BASE_EXPORT friend bool operator<=(const Value& lhs, const Value& rhs);
  BASE_EXPORT friend bool operator>=(const Value& lhs, const Value& rhs);



  bool Equals(const Value* other) const;


  size_t EstimateMemoryUsage() const;

 protected:








  Type type_ = Type::NONE;

  union {
    bool bool_value_;
    int int_value_;
    DoubleStorage double_value_;
    std::string string_value_;
    BlobStorage binary_value_;
    DictStorage dict_;
    ListStorage list_;
  };

 private:
  friend class ValuesTest_SizeOfValue_Test;
  double AsDoubleInternal() const;
  void InternalMoveConstructFrom(Value&& that);
  void InternalCleanup();


  Value* SetKeyInternal(StringPiece key, std::unique_ptr<Value>&& val_ptr);
  Value* SetPathInternal(StringPiece path, std::unique_ptr<Value>&& value_ptr);

  DISALLOW_COPY_AND_ASSIGN(Value);
};

// parsing for recursive access; see the comment at the top of the file. Keys
// are |std::string|s and should be UTF-8 encoded.
class BASE_EXPORT DictionaryValue : public Value {
 public:
  using const_iterator = DictStorage::const_iterator;
  using iterator = DictStorage::iterator;

  static std::unique_ptr<DictionaryValue> From(std::unique_ptr<Value> value);

  DictionaryValue();
  explicit DictionaryValue(const DictStorage& in_dict);
  explicit DictionaryValue(DictStorage&& in_dict) noexcept;


  bool HasKey(StringPiece key) const;

  size_t size() const { return dict_.size(); }

  bool empty() const { return dict_.empty(); }

  void Clear();









  Value* Set(StringPiece path, std::unique_ptr<Value> in_value);



  Value* SetBoolean(StringPiece path, bool in_value);

  Value* SetInteger(StringPiece path, int in_value);

  Value* SetDouble(StringPiece path, double in_value);

  Value* SetString(StringPiece path, StringPiece in_value);

  Value* SetString(StringPiece path, const string16& in_value);

  DictionaryValue* SetDictionary(StringPiece path,
                                 std::unique_ptr<DictionaryValue> in_value);

  ListValue* SetList(StringPiece path, std::unique_ptr<ListValue> in_value);



  Value* SetWithoutPathExpansion(StringPiece key,
                                 std::unique_ptr<Value> in_value);









  bool Get(StringPiece path, const Value** out_value) const;

  bool Get(StringPiece path, Value** out_value);





  bool GetBoolean(StringPiece path, bool* out_value) const;

  bool GetInteger(StringPiece path, int* out_value) const;



  bool GetDouble(StringPiece path, double* out_value) const;

  bool GetString(StringPiece path, std::string* out_value) const;

  bool GetString(StringPiece path, string16* out_value) const;

  bool GetStringASCII(StringPiece path, std::string* out_value) const;

  bool GetBinary(StringPiece path, const Value** out_value) const;

  bool GetBinary(StringPiece path, Value** out_value);

  bool GetDictionary(StringPiece path,
                     const DictionaryValue** out_value) const;

  bool GetDictionary(StringPiece path, DictionaryValue** out_value);

  bool GetList(StringPiece path, const ListValue** out_value) const;

  bool GetList(StringPiece path, ListValue** out_value);



  bool GetWithoutPathExpansion(StringPiece key, const Value** out_value) const;

  bool GetWithoutPathExpansion(StringPiece key, Value** out_value);

  bool GetBooleanWithoutPathExpansion(StringPiece key, bool* out_value) const;

  bool GetIntegerWithoutPathExpansion(StringPiece key, int* out_value) const;

  bool GetDoubleWithoutPathExpansion(StringPiece key, double* out_value) const;

  bool GetStringWithoutPathExpansion(StringPiece key,
                                     std::string* out_value) const;

  bool GetStringWithoutPathExpansion(StringPiece key,
                                     string16* out_value) const;

  bool GetDictionaryWithoutPathExpansion(
      StringPiece key,
      const DictionaryValue** out_value) const;

  bool GetDictionaryWithoutPathExpansion(StringPiece key,
                                         DictionaryValue** out_value);

  bool GetListWithoutPathExpansion(StringPiece key,
                                   const ListValue** out_value) const;

  bool GetListWithoutPathExpansion(StringPiece key, ListValue** out_value);








  bool Remove(StringPiece path, std::unique_ptr<Value>* out_value);



  bool RemoveWithoutPathExpansion(StringPiece key,
                                  std::unique_ptr<Value>* out_value);




  bool RemovePath(StringPiece path, std::unique_ptr<Value>* out_value);

  using Value::RemovePath;  // DictionaryValue::RemovePath shadows otherwise.


  std::unique_ptr<DictionaryValue> DeepCopyWithoutEmptyChildren() const;

  void Swap(DictionaryValue* other);



  class BASE_EXPORT Iterator {
   public:
    explicit Iterator(const DictionaryValue& target);
    Iterator(const Iterator& other);
    ~Iterator();

    bool IsAtEnd() const { return it_ == target_.dict_.end(); }
    void Advance() { ++it_; }

    const std::string& key() const { return it_->first; }
    const Value& value() const { return *it_->second; }

   private:
    const DictionaryValue& target_;
    DictStorage::const_iterator it_;
  };


  iterator begin() { return dict_.begin(); }
  iterator end() { return dict_.end(); }

  const_iterator begin() const { return dict_.begin(); }
  const_iterator end() const { return dict_.end(); }


  DictionaryValue* DeepCopy() const;


  std::unique_ptr<DictionaryValue> CreateDeepCopy() const;
};

class BASE_EXPORT ListValue : public Value {
 public:
  using const_iterator = ListView::const_iterator;
  using iterator = ListView::iterator;

  static std::unique_ptr<ListValue> From(std::unique_ptr<Value> value);

  ListValue();
  explicit ListValue(span<const Value> in_list);
  explicit ListValue(ListStorage&& in_list) noexcept;


  void Clear();


  size_t GetSize() const { return list_.size(); }


  bool empty() const { return list_.empty(); }


  void Reserve(size_t n);






  bool Set(size_t index, std::unique_ptr<Value> in_value);





  bool Get(size_t index, const Value** out_value) const;
  bool Get(size_t index, Value** out_value);





  bool GetBoolean(size_t index, bool* out_value) const;

  bool GetInteger(size_t index, int* out_value) const;



  bool GetDouble(size_t index, double* out_value) const;

  bool GetString(size_t index, std::string* out_value) const;
  bool GetString(size_t index, string16* out_value) const;

  bool GetDictionary(size_t index, const DictionaryValue** out_value) const;
  bool GetDictionary(size_t index, DictionaryValue** out_value);

  using Value::GetList;

  bool GetList(size_t index, const ListValue** out_value) const;
  bool GetList(size_t index, ListValue** out_value);






  bool Remove(size_t index, std::unique_ptr<Value>* out_value);




  bool Remove(const Value& value, size_t* index);





  iterator Erase(iterator iter, std::unique_ptr<Value>* out_value);

  using Value::Append;


  void Append(std::unique_ptr<Value> in_value);


  void AppendBoolean(bool in_value);
  void AppendInteger(int in_value);
  void AppendDouble(double in_value);
  void AppendString(StringPiece in_value);
  void AppendString(const string16& in_value);

  void AppendStrings(const std::vector<std::string>& in_values);
  void AppendStrings(const std::vector<string16>& in_values);



  bool AppendIfNotPresent(std::unique_ptr<Value> in_value);

  using Value::Insert;



  bool Insert(size_t index, std::unique_ptr<Value> in_value);




  const_iterator Find(const Value& value) const;


  void Swap(ListValue* other);


  iterator begin() { return GetList().begin(); }

  iterator end() { return GetList().end(); }

  const_iterator begin() const { return GetList().begin(); }

  const_iterator end() const { return GetList().end(); }


  ListValue* DeepCopy() const;


  std::unique_ptr<ListValue> CreateDeepCopy() const;
};

// Value objects.
class BASE_EXPORT ValueSerializer {
 public:
  virtual ~ValueSerializer();

  virtual bool Serialize(const Value& root) = 0;
};

// objects.
class BASE_EXPORT ValueDeserializer {
 public:
  virtual ~ValueDeserializer();






  virtual std::unique_ptr<Value> Deserialize(int* error_code,
                                             std::string* error_str) = 0;
};

// gtest uses this operator to print readable output on test failures, we must
// override each specific type. Otherwise, the default template implementation
// is preferred over an upcast.
BASE_EXPORT std::ostream& operator<<(std::ostream& out, const Value& value);

BASE_EXPORT inline std::ostream& operator<<(std::ostream& out,
                                            const DictionaryValue& value) {
  return out << static_cast<const Value&>(value);
}

BASE_EXPORT inline std::ostream& operator<<(std::ostream& out,
                                            const ListValue& value) {
  return out << static_cast<const Value&>(value);
}

BASE_EXPORT std::ostream& operator<<(std::ostream& out,
                                     const Value::Type& type);

}  // namespace base

#endif  // BASE_VALUES_H_
