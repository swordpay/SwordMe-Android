// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_TRACE_ARGUMENTS_H_
#define BASE_TRACE_EVENT_TRACE_ARGUMENTS_H_

#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <memory>
#include <string>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/trace_event/common/trace_event_common.h"

// identified by a name (a C string literal) and a value, which can be an
// integer, enum, floating point, boolean, string pointer or reference, or
// std::unique_ptr<ConvertableToTraceFormat> compatible values. Additionally,
// custom data types need to be supported, like time values or WTF::CString.
//
// TraceArguments is a helper class used to store 0 to 2 named arguments
// corresponding to an individual trace macro call. As efficiently as possible,
// and with the minimal amount of generated machine code (since this affects
// any TRACE macro call). Each argument has:
//
//  - A name (C string literal, e.g "dumps")
//  - An 8-bit type value, corresponding to the TRACE_VALUE_TYPE_XXX macros.
//  - A value, stored in a TraceValue union
//
// IMPORTANT: For a TRACE_VALUE_TYPE_CONVERTABLE types, the TraceArguments
// instance owns the pointed ConvertableToTraceFormat object, i.e. it will
// delete it automatically on destruction.
//
// TraceArguments instances should be built using one of specialized
// constructors declared below. One cannot modify an instance once it has
// been built, except for move operations, Reset() and destruction. Examples:
//
//    TraceArguments args;    // No arguments.
//    // args.size() == 0
//
//    TraceArguments("foo", 100);
//    // args.size() == 1
//    // args.types()[0] == TRACE_VALUE_TYPE_INT
//    // args.names()[0] == "foo"
//    // args.values()[0].as_int == 100
//
//    TraceArguments("bar", 1ULL);
//    // args.size() == 1
//    // args.types()[0] == TRACE_VALUE_TYPE_UINT
//    // args.names()[0] == "bar"
//    // args.values()[0].as_uint == 100
//
//    TraceArguments("foo", "Hello", "bar", "World");
//    // args.size() == 2
//    // args.types()[0] == TRACE_VALUE_TYPE_STRING
//    // args.types()[1] == TRACE_VALUE_TYPE_STRING
//    // args.names()[0] == "foo"
//    // args.names()[1] == "bar"
//    // args.values()[0].as_string == "Hello"
//    // args.values()[1].as_string == "World"
//
//    std::string some_string = ...;
//    TraceArguments("str1", some_string);
//    // args.size() == 1
//    // args.types()[0] == TRACE_VALUE_TYPE_COPY_STRING
//    // args.names()[0] == "str1"
//    // args.values()[0].as_string == some_string.c_str()
//
// Note that TRACE_VALUE_TYPE_COPY_STRING corresponds to string pointers
// that point to temporary values that may disappear soon. The
// TraceArguments::CopyStringTo() method can be used to copy their content
// into a StringStorage memory block, and update the |as_string| value pointers
// to it to avoid keeping any dangling pointers. This is used by TraceEvent
// to keep copies of such strings in the log after their initialization values
// have disappeared.
//
// The TraceStringWithCopy helper class can be used to initialize a value
// from a regular string pointer with TRACE_VALUE_TYPE_COPY_STRING too, as in:
//
//     const char str[] = "....";
//     TraceArguments("foo", str, "bar", TraceStringWithCopy(str));
//     // args.size() == 2
//     // args.types()[0] == TRACE_VALUE_TYPE_STRING
//     // args.types()[1] == TRACE_VALUE_TYPE_COPY_STRING
//     // args.names()[0] == "foo"
//     // args.names()[1] == "bar"
//     // args.values()[0].as_string == str
//     // args.values()[1].as_string == str
//
//     StringStorage storage;
//     args.CopyStringTo(&storage, false, nullptr, nullptr);
//     // args.size() == 2
//     // args.types()[0] == TRACE_VALUE_TYPE_STRING
//     // args.types()[1] == TRACE_VALUE_TYPE_COPY_STRING
//     // args.names()[0] == "foo"
//     // args.names()[1] == "bar"
//     // args.values()[0].as_string == str
//     // args.values()[1].as_string == Address inside |storage|.
//
// Initialization from a std::unique_ptr<ConvertableToTraceFormat>
// is supported but will move ownership of the pointer objects to the
// TraceArguments instance:
//
//     class MyConvertableType :
//         public base::trace_event::AsConvertableToTraceFormat {
//        ...
//     };
//
//     {
//       TraceArguments args("foo" , std::make_unique<MyConvertableType>(...));
//       // args.size() == 1
//       // args.values()[0].as_convertable == address of MyConvertable object.
//     } // Calls |args| destructor, which will delete the object too.
//
// Finally, it is possible to support initialization from custom values by
// specializing the TraceValue::Helper<> template struct as described below.
//
// This is how values of custom types like WTF::CString can be passed directly
// to trace macros.

namespace base {

class Time;
class TimeTicks;
class ThreadTicks;

namespace trace_event {

class TraceEventMemoryOverhead;

// class must implement this interface. Note that unlike other values,
// these objects will be owned by the TraceArguments instance that points
// to them.
class BASE_EXPORT ConvertableToTraceFormat {
 public:
  ConvertableToTraceFormat() = default;
  virtual ~ConvertableToTraceFormat() = default;




  virtual void AppendAsTraceFormat(std::string* out) const = 0;




  class BASE_EXPORT ProtoAppender {
   public:
    virtual ~ProtoAppender() = default;

    virtual void AddBuffer(uint8_t* begin, uint8_t* end) = 0;


    virtual size_t Finalize(uint32_t field_id) = 0;
  };
  virtual bool AppendToProto(ProtoAppender* appender);

  virtual void EstimateTraceMemoryOverhead(TraceEventMemoryOverhead* overhead);

 private:
  DISALLOW_COPY_AND_ASSIGN(ConvertableToTraceFormat);
};

const int kTraceMaxNumArgs = 2;

//
// This is a POD union for performance reason. Initialization from an
// explicit C++ trace argument should be performed with the Init()
// templated method described below.
//
// Initialization from custom types is possible by implementing a custom
// TraceValue::Helper<> instantiation as described below.
//
// IMPORTANT: Pointer storage inside a TraceUnion follows specific rules:
//
//   - |as_pointer| is for raw pointers that should be treated as a simple
//     address and will never be dereferenced. Associated with the
//     TRACE_VALUE_TYPE_POINTER type.
//
//   - |as_string| is for C-string pointers, associated with both
//     TRACE_VALUE_TYPE_STRING and TRACE_VALUE_TYPE_COPY_STRING. The former
//     indicates that the string pointer is persistent (e.g. a C string
//     literal), while the second indicates that the pointer belongs to a
//     temporary variable that may disappear soon. The TraceArguments class
//     provides a CopyStringTo() method to copy these strings into a
//     StringStorage instance, which is useful if the instance needs to
//     survive longer than the temporaries.
//
//   - |as_convertable| is equivalent to
//     std::unique_ptr<ConvertableToTraceFormat>, except that it is a pointer
//     to keep this union POD and avoid un-necessary declarations and potential
//     code generation. This means that its ownership is passed to the
//     TraceValue instance when Init(std::unique_ptr<ConvertableToTraceFormat>)
//     is called, and that it will be deleted by the containing TraceArguments
//     destructor, or Reset() method.
//
union BASE_EXPORT TraceValue {
  bool as_bool;
  unsigned long long as_uint;
  long long as_int;
  double as_double;
  const void* as_pointer;
  const char* as_string;
  ConvertableToTraceFormat* as_convertable;













  template <typename T>
  void Init(T&& value) {
    using ValueType = typename InnerType<T>::type;
    Helper<ValueType>::SetValue(this, std::forward<T>(value));
  }












  template <typename T>
  static TraceValue Make(T&& value) {
    TraceValue ret;
    ret.Init(std::forward<T>(value));
    return ret;
  }


  void AppendAsJSON(unsigned char type, std::string* out) const;



  void AppendAsString(unsigned char type, std::string* out) const;

 private:
  void Append(unsigned char type, bool as_json, std::string* out) const;


  template <typename T>
  struct InnerType {
    using type = typename std::remove_cv<typename std::remove_reference<
        typename std::decay<T>::type>::type>::type;
  };

 public:





































  template <typename T, class = void>
  struct Helper {};


  template <typename T>
  struct TypeFor {
    using ValueType = typename InnerType<T>::type;
    static const unsigned char value = Helper<ValueType>::kType;
  };




  template <typename T,
            class = decltype(TraceValue::Helper<
                             typename TraceValue::InnerType<T>::type>::kType)>
  struct TypeCheck {
    static const bool value = true;
  };
};

template <typename T>
struct TraceValue::Helper<
    T,
    typename std::enable_if<std::is_integral<T>::value ||
                            std::is_enum<T>::value>::type> {
  static constexpr unsigned char kType =
      std::is_signed<T>::value ? TRACE_VALUE_TYPE_INT : TRACE_VALUE_TYPE_UINT;
  static inline void SetValue(TraceValue* v, T value) {
    v->as_uint = static_cast<unsigned long long>(value);
  }
};

template <typename T>
struct TraceValue::
    Helper<T, typename std::enable_if<std::is_floating_point<T>::value>::type> {
  static constexpr unsigned char kType = TRACE_VALUE_TYPE_DOUBLE;
  static inline void SetValue(TraceValue* v, T value) { v->as_double = value; }
};

template <>
struct TraceValue::Helper<bool> {
  static constexpr unsigned char kType = TRACE_VALUE_TYPE_BOOL;
  static inline void SetValue(TraceValue* v, bool value) { v->as_bool = value; }
};

template <typename T>
struct TraceValue::Helper<T*> {
  static constexpr unsigned char kType = TRACE_VALUE_TYPE_POINTER;
  static inline void SetValue(TraceValue* v,
                              const typename std::decay<T>::type* value) {
    v->as_pointer = value;
  }
};

template <>
struct TraceValue::Helper<const char*> {
  static constexpr unsigned char kType = TRACE_VALUE_TYPE_STRING;
  static inline void SetValue(TraceValue* v, const char* value) {
    v->as_string = value;
  }
};

template <>
struct TraceValue::Helper<std::string> {
  static constexpr unsigned char kType = TRACE_VALUE_TYPE_COPY_STRING;
  static inline void SetValue(TraceValue* v, const std::string& value) {
    v->as_string = value.c_str();
  }
};

// |CONVERTABLE_TYPE| must be a type whose pointers can be converted to a
// ConvertableToTraceFormat* pointer as well (e.g. a derived class).
// IMPORTANT: This takes an std::unique_ptr<CONVERTABLE_TYPE> value, and takes
// ownership of the pointed object!
template <typename CONVERTABLE_TYPE>
struct TraceValue::Helper<std::unique_ptr<CONVERTABLE_TYPE>,
                          typename std::enable_if<std::is_convertible<
                              CONVERTABLE_TYPE*,
                              ConvertableToTraceFormat*>::value>::type> {
  static constexpr unsigned char kType = TRACE_VALUE_TYPE_CONVERTABLE;
  static inline void SetValue(TraceValue* v,
                              std::unique_ptr<CONVERTABLE_TYPE> value) {
    v->as_convertable = value.release();
  }
};

// a ToInternalValue() method.
template <typename T>
struct TraceValue::Helper<
    T,
    typename std::enable_if<std::is_same<T, base::Time>::value ||
                            std::is_same<T, base::TimeTicks>::value ||
                            std::is_same<T, base::ThreadTicks>::value>::type> {
  static constexpr unsigned char kType = TRACE_VALUE_TYPE_INT;
  static inline void SetValue(TraceValue* v, const T& value) {
    v->as_int = value.ToInternalValue();
  }
};

// The goal is to indicate that the C string is copyable, unlike the default
// Init(const char*) implementation. Usage is:
//
//    const char* str = ...;
//    v.Init(TraceStringWithCopy(str));
//
// Which will mark the string as TRACE_VALUE_TYPE_COPY_STRING, instead of
// TRACE_VALUE_TYPE_STRING.
//
class TraceStringWithCopy {
 public:
  explicit TraceStringWithCopy(const char* str) : str_(str) {}
  const char* str() const { return str_; }

 private:
  const char* str_;
};

template <>
struct TraceValue::Helper<TraceStringWithCopy> {
  static constexpr unsigned char kType = TRACE_VALUE_TYPE_COPY_STRING;
  static inline void SetValue(TraceValue* v, const TraceStringWithCopy& value) {
    v->as_string = value.str();
  }
};

class TraceArguments;

// TraceArguments instance (see below). When empty, this should only
// take the size of a pointer. Otherwise, this will point to a heap
// allocated block containing a size_t value followed by all characters
// in the storage area. For most cases, this is more efficient
// than using a std::unique_ptr<std::string> or an std::vector<char>.
class BASE_EXPORT StringStorage {
 public:
  constexpr StringStorage() = default;

  explicit StringStorage(size_t alloc_size) { Reset(alloc_size); }

  ~StringStorage() {
    if (data_)
      ::free(data_);
  }

  StringStorage(StringStorage&& other) noexcept : data_(other.data_) {
    other.data_ = nullptr;
  }

  StringStorage& operator=(StringStorage&& other) noexcept {
    if (this != &other) {
      if (data_)
        ::free(data_);
      data_ = other.data_;
      other.data_ = nullptr;
    }
    return *this;
  }



  void Reset(size_t alloc_size = 0);

  constexpr size_t size() const { return data_ ? data_->size : 0u; }
  constexpr const char* data() const { return data_ ? data_->chars : nullptr; }
  constexpr char* data() { return data_ ? data_->chars : nullptr; }

  constexpr const char* begin() const { return data(); }
  constexpr const char* end() const { return data() + size(); }
  inline char* begin() { return data(); }
  inline char* end() { return data() + size(); }

  constexpr bool empty() const { return size() == 0; }


  constexpr bool Contains(const void* ptr) const {
    const char* char_ptr = static_cast<const char*>(ptr);
    return (char_ptr >= begin() && char_ptr < end());
  }


  bool Contains(const TraceArguments& args) const;


  constexpr size_t EstimateTraceMemoryOverhead() const {
    return data_ ? sizeof(size_t) + data_->size : 0u;
  }

 private:





  struct Data {
    size_t size = 0;
    char chars[1];  // really |size| character items in storage.
  };




  Data* data_ = nullptr;
};

// each one of them having:
//   - a name, which is a constant char array literal.
//   - a type, as described by TRACE_VALUE_TYPE_XXX macros.
//   - a value, stored in a TraceValue union.
//
// IMPORTANT: For TRACE_VALUE_TYPE_CONVERTABLE, the value holds an owning
//            pointer to an AsConvertableToTraceFormat instance, which will
//            be destroyed with the array (or moved out of it when passed
//            to a TraceEvent instance).
//
// For TRACE_VALUE_TYPE_COPY_STRING, the value holds a const char* pointer
// whose content will be copied when creating a TraceEvent instance.
//
// IMPORTANT: Most constructors and the destructor are all inlined
// intentionally, in order to let the compiler remove un-necessary operations
// and reduce machine code.
//
class BASE_EXPORT TraceArguments {
 public:

  static constexpr size_t kMaxSize = 2;

  TraceArguments() : size_(0) {}

  template <typename T, class = decltype(TraceValue::TypeCheck<T>::value)>
  TraceArguments(const char* arg1_name, T&& arg1_value) : size_(1) {
    types_[0] = TraceValue::TypeFor<T>::value;
    names_[0] = arg1_name;
    values_[0].Init(std::forward<T>(arg1_value));
  }

  template <typename T1,
            typename T2,
            class = decltype(TraceValue::TypeCheck<T1>::value &&
                             TraceValue::TypeCheck<T2>::value)>
  TraceArguments(const char* arg1_name,
                 T1&& arg1_value,
                 const char* arg2_name,
                 T2&& arg2_value)
      : size_(2) {
    types_[0] = TraceValue::TypeFor<T1>::value;
    types_[1] = TraceValue::TypeFor<T2>::value;
    names_[0] = arg1_name;
    names_[1] = arg2_name;
    values_[0].Init(std::forward<T1>(arg1_value));
    values_[1].Init(std::forward<T2>(arg2_value));
  }


  TraceArguments(int num_args,
                 const char* const* arg_names,
                 const unsigned char* arg_types,
                 const unsigned long long* arg_values);


  template <typename CONVERTABLE_TYPE>
  TraceArguments(int num_args,
                 const char* const* arg_names,
                 const unsigned char* arg_types,
                 const unsigned long long* arg_values,
                 CONVERTABLE_TYPE* arg_convertables) {
    static int max_args = static_cast<int>(kMaxSize);
    if (num_args > max_args)
      num_args = max_args;
    size_ = static_cast<unsigned char>(num_args);
    for (size_t n = 0; n < size_; ++n) {
      types_[n] = arg_types[n];
      names_[n] = arg_names[n];
      if (arg_types[n] == TRACE_VALUE_TYPE_CONVERTABLE) {
        values_[n].Init(
            std::forward<CONVERTABLE_TYPE>(std::move(arg_convertables[n])));
      } else {
        values_[n].as_uint = arg_values[n];
      }
    }
  }

  ~TraceArguments() {
    for (size_t n = 0; n < size_; ++n) {
      if (types_[n] == TRACE_VALUE_TYPE_CONVERTABLE)
        delete values_[n].as_convertable;
    }
  }

  TraceArguments(const TraceArguments&) = delete;
  TraceArguments& operator=(const TraceArguments&) = delete;

  TraceArguments(TraceArguments&& other) noexcept {
    ::memcpy(this, &other, sizeof(*this));


    other.size_ = 0;
  }

  TraceArguments& operator=(TraceArguments&&) noexcept;

  size_t size() const { return size_; }
  const unsigned char* types() const { return types_; }
  const char* const* names() const { return names_; }
  const TraceValue* values() const { return values_; }

  void Reset();






  void CopyStringsTo(StringStorage* storage,
                     bool copy_all_strings,
                     const char** extra_string1,
                     const char** extra_string2);

  void AppendDebugString(std::string* out);

 private:
  unsigned char size_;
  unsigned char types_[kMaxSize];
  const char* names_[kMaxSize];
  TraceValue values_[kMaxSize];
};

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_TRACE_ARGUMENTS_H_
