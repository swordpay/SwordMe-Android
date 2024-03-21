//
// Copyright 2019 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ABSL_FLAGS_INTERNAL_FLAG_H_
#define ABSL_FLAGS_INTERNAL_FLAG_H_

#include <stddef.h>
#include <stdint.h>

#include <atomic>
#include <cstring>
#include <memory>
#include <new>
#include <string>
#include <type_traits>
#include <typeinfo>

#include "absl/base/attributes.h"
#include "absl/base/call_once.h"
#include "absl/base/casts.h"
#include "absl/base/config.h"
#include "absl/base/optimization.h"
#include "absl/base/thread_annotations.h"
#include "absl/flags/commandlineflag.h"
#include "absl/flags/config.h"
#include "absl/flags/internal/commandlineflag.h"
#include "absl/flags/internal/registry.h"
#include "absl/flags/internal/sequence_lock.h"
#include "absl/flags/marshalling.h"
#include "absl/meta/type_traits.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "absl/utility/utility.h"

namespace absl {
ABSL_NAMESPACE_BEGIN

// Forward declaration of absl::Flag<T> public API.
namespace flags_internal {
template <typename T>
class Flag;
}  // namespace flags_internal

#if defined(_MSC_VER) && !defined(__clang__)
template <typename T>
class Flag;
#else
template <typename T>
using Flag = flags_internal::Flag<T>;
#endif

template <typename T>
ABSL_MUST_USE_RESULT T GetFlag(const absl::Flag<T>& flag);

template <typename T>
void SetFlag(absl::Flag<T>* flag, const T& v);

template <typename T, typename V>
void SetFlag(absl::Flag<T>* flag, const V& v);

template <typename U>
const CommandLineFlag& GetFlagReflectionHandle(const absl::Flag<U>& f);

// Flag value type operations, eg., parsing, copying, etc. are provided
// by function specific to that type with a signature matching FlagOpFn.

namespace flags_internal {

enum class FlagOp {
  kAlloc,
  kDelete,
  kCopy,
  kCopyConstruct,
  kSizeof,
  kFastTypeId,
  kRuntimeTypeId,
  kParse,
  kUnparse,
  kValueOffset,
};
using FlagOpFn = void* (*)(FlagOp, const void*, void*, void*);

template <typename T>
void* FlagOps(FlagOp op, const void* v1, void* v2, void* v3);

inline void* Alloc(FlagOpFn op) {
  return op(FlagOp::kAlloc, nullptr, nullptr, nullptr);
}
// Deletes memory interpreting obj as flag value type pointer.
inline void Delete(FlagOpFn op, void* obj) {
  op(FlagOp::kDelete, nullptr, obj, nullptr);
}
// Copies src to dst interpreting as flag value type pointers.
inline void Copy(FlagOpFn op, const void* src, void* dst) {
  op(FlagOp::kCopy, src, dst, nullptr);
}
// Construct a copy of flag value in a location pointed by dst
// based on src - pointer to the flag's value.
inline void CopyConstruct(FlagOpFn op, const void* src, void* dst) {
  op(FlagOp::kCopyConstruct, src, dst, nullptr);
}
// Makes a copy of flag value pointed by obj.
inline void* Clone(FlagOpFn op, const void* obj) {
  void* res = flags_internal::Alloc(op);
  flags_internal::CopyConstruct(op, obj, res);
  return res;
}
// Returns true if parsing of input text is successfull.
inline bool Parse(FlagOpFn op, absl::string_view text, void* dst,
                  std::string* error) {
  return op(FlagOp::kParse, &text, dst, error) != nullptr;
}
// Returns string representing supplied value.
inline std::string Unparse(FlagOpFn op, const void* val) {
  std::string result;
  op(FlagOp::kUnparse, val, &result, nullptr);
  return result;
}
// Returns size of flag value type.
inline size_t Sizeof(FlagOpFn op) {


  return static_cast<size_t>(reinterpret_cast<intptr_t>(
      op(FlagOp::kSizeof, nullptr, nullptr, nullptr)));
}
// Returns fast type id coresponding to the value type.
inline FlagFastTypeId FastTypeId(FlagOpFn op) {
  return reinterpret_cast<FlagFastTypeId>(
      op(FlagOp::kFastTypeId, nullptr, nullptr, nullptr));
}
// Returns fast type id coresponding to the value type.
inline const std::type_info* RuntimeTypeId(FlagOpFn op) {
  return reinterpret_cast<const std::type_info*>(
      op(FlagOp::kRuntimeTypeId, nullptr, nullptr, nullptr));
}
// Returns offset of the field value_ from the field impl_ inside of
// absl::Flag<T> data. Given FlagImpl pointer p you can get the
// location of the corresponding value as:
//      reinterpret_cast<char*>(p) + ValueOffset().
inline ptrdiff_t ValueOffset(FlagOpFn op) {


  return static_cast<ptrdiff_t>(reinterpret_cast<intptr_t>(
      op(FlagOp::kValueOffset, nullptr, nullptr, nullptr)));
}

template <typename T>
inline const std::type_info* GenRuntimeTypeId() {
#ifdef ABSL_INTERNAL_HAS_RTTI
  return &typeid(T);
#else
  return nullptr;
#endif
}

// Flag help auxiliary structs.

// or pointer to function generating it as well as enum descriminating two
// cases.
using HelpGenFunc = std::string (*)();

template <size_t N>
struct FixedCharArray {
  char value[N];

  template <size_t... I>
  static constexpr FixedCharArray<N> FromLiteralString(
      absl::string_view str, absl::index_sequence<I...>) {
    return (void)str, FixedCharArray<N>({{str[I]..., '\0'}});
  }
};

template <typename Gen, size_t N = Gen::Value().size()>
constexpr FixedCharArray<N + 1> HelpStringAsArray(int) {
  return FixedCharArray<N + 1>::FromLiteralString(
      Gen::Value(), absl::make_index_sequence<N>{});
}

template <typename Gen>
constexpr std::false_type HelpStringAsArray(char) {
  return std::false_type{};
}

union FlagHelpMsg {
  constexpr explicit FlagHelpMsg(const char* help_msg) : literal(help_msg) {}
  constexpr explicit FlagHelpMsg(HelpGenFunc help_gen) : gen_func(help_gen) {}

  const char* literal;
  HelpGenFunc gen_func;
};

enum class FlagHelpKind : uint8_t { kLiteral = 0, kGenFunc = 1 };

struct FlagHelpArg {
  FlagHelpMsg source;
  FlagHelpKind kind;
};

extern const char kStrippedFlagHelp[];

// way to pass Help argument to absl::Flag. We'll be passing
// AbslFlagHelpGenFor##name as Gen and integer 0 as a single argument to prefer
// first overload if possible. If help message is evaluatable on constexpr
// context We'll be able to make FixedCharArray out of it and we'll choose first
// overload. In this case the help message expression is immediately evaluated
// and is used to construct the absl::Flag. No additionl code is generated by
// ABSL_FLAG Otherwise SFINAE kicks in and first overload is dropped from the
// consideration, in which case the second overload will be used. The second
// overload does not attempt to evaluate the help message expression
// immediately and instead delays the evaluation by returing the function
// pointer (&T::NonConst) genering the help message when necessary. This is
// evaluatable in constexpr context, but the cost is an extra function being
// generated in the ABSL_FLAG code.
template <typename Gen, size_t N>
constexpr FlagHelpArg HelpArg(const FixedCharArray<N>& value) {
  return {FlagHelpMsg(value.value), FlagHelpKind::kLiteral};
}

template <typename Gen>
constexpr FlagHelpArg HelpArg(std::false_type) {
  return {FlagHelpMsg(&Gen::NonConst), FlagHelpKind::kGenFunc};
}

// Flag default value auxiliary structs.

// based on default value supplied in flag's definition)
using FlagDfltGenFunc = void (*)(void*);

union FlagDefaultSrc {
  constexpr explicit FlagDefaultSrc(FlagDfltGenFunc gen_func_arg)
      : gen_func(gen_func_arg) {}

#define ABSL_FLAGS_INTERNAL_DFLT_FOR_TYPE(T, name) \
  T name##_value;                                  \
  constexpr explicit FlagDefaultSrc(T value) : name##_value(value) {}  // NOLINT
  ABSL_FLAGS_INTERNAL_BUILTIN_TYPES(ABSL_FLAGS_INTERNAL_DFLT_FOR_TYPE)
#undef ABSL_FLAGS_INTERNAL_DFLT_FOR_TYPE

  void* dynamic_value;
  FlagDfltGenFunc gen_func;
};

enum class FlagDefaultKind : uint8_t {
  kDynamicValue = 0,
  kGenFunc = 1,
  kOneWord = 2  // for default values UP to one word in size
};

struct FlagDefaultArg {
  FlagDefaultSrc source;
  FlagDefaultKind kind;
};

// facilitate usage of {} as default value in ABSL_FLAG macro.
// TODO(rogeeff): Fix handling types with explicit constructors.
struct EmptyBraces {};

template <typename T>
constexpr T InitDefaultValue(T t) {
  return t;
}

template <typename T>
constexpr T InitDefaultValue(EmptyBraces) {
  return T{};
}

template <typename ValueT, typename GenT,
          typename std::enable_if<std::is_integral<ValueT>::value, int>::type =
              ((void)GenT{}, 0)>
constexpr FlagDefaultArg DefaultArg(int) {
  return {FlagDefaultSrc(GenT{}.value), FlagDefaultKind::kOneWord};
}

template <typename ValueT, typename GenT>
constexpr FlagDefaultArg DefaultArg(char) {
  return {FlagDefaultSrc(&GenT::Gen), FlagDefaultKind::kGenFunc};
}

// Flag current value auxiliary structs.

constexpr int64_t UninitializedFlagValue() {
  return static_cast<int64_t>(0xababababababababll);
}

template <typename T>
using FlagUseValueAndInitBitStorage = std::integral_constant<
    bool, absl::type_traits_internal::is_trivially_copyable<T>::value &&
              std::is_default_constructible<T>::value && (sizeof(T) < 8)>;

template <typename T>
using FlagUseOneWordStorage = std::integral_constant<
    bool, absl::type_traits_internal::is_trivially_copyable<T>::value &&
              (sizeof(T) <= 8)>;

template <class T>
using FlagUseSequenceLockStorage = std::integral_constant<
    bool, absl::type_traits_internal::is_trivially_copyable<T>::value &&
              (sizeof(T) > 8)>;

enum class FlagValueStorageKind : uint8_t {
  kValueAndInitBit = 0,
  kOneWordAtomic = 1,
  kSequenceLocked = 2,
  kAlignedBuffer = 3,
};

template <typename T>
static constexpr FlagValueStorageKind StorageKind() {
  return FlagUseValueAndInitBitStorage<T>::value
             ? FlagValueStorageKind::kValueAndInitBit
         : FlagUseOneWordStorage<T>::value
             ? FlagValueStorageKind::kOneWordAtomic
         : FlagUseSequenceLockStorage<T>::value
             ? FlagValueStorageKind::kSequenceLocked
             : FlagValueStorageKind::kAlignedBuffer;
}

struct FlagOneWordValue {
  constexpr explicit FlagOneWordValue(int64_t v) : value(v) {}
  std::atomic<int64_t> value;
};

template <typename T>
struct alignas(8) FlagValueAndInitBit {
  T value;


  uint8_t init;
};

template <typename T,
          FlagValueStorageKind Kind = flags_internal::StorageKind<T>()>
struct FlagValue;

template <typename T>
struct FlagValue<T, FlagValueStorageKind::kValueAndInitBit> : FlagOneWordValue {
  constexpr FlagValue() : FlagOneWordValue(0) {}
  bool Get(const SequenceLock&, T& dst) const {
    int64_t storage = value.load(std::memory_order_acquire);
    if (ABSL_PREDICT_FALSE(storage == 0)) {
      return false;
    }
    dst = absl::bit_cast<FlagValueAndInitBit<T>>(storage).value;
    return true;
  }
};

template <typename T>
struct FlagValue<T, FlagValueStorageKind::kOneWordAtomic> : FlagOneWordValue {
  constexpr FlagValue() : FlagOneWordValue(UninitializedFlagValue()) {}
  bool Get(const SequenceLock&, T& dst) const {
    int64_t one_word_val = value.load(std::memory_order_acquire);
    if (ABSL_PREDICT_FALSE(one_word_val == UninitializedFlagValue())) {
      return false;
    }
    std::memcpy(&dst, static_cast<const void*>(&one_word_val), sizeof(T));
    return true;
  }
};

template <typename T>
struct FlagValue<T, FlagValueStorageKind::kSequenceLocked> {
  bool Get(const SequenceLock& lock, T& dst) const {
    return lock.TryRead(&dst, value_words, sizeof(T));
  }

  static constexpr int kNumWords =
      flags_internal::AlignUp(sizeof(T), sizeof(uint64_t)) / sizeof(uint64_t);

  alignas(T) alignas(
      std::atomic<uint64_t>) std::atomic<uint64_t> value_words[kNumWords];
};

template <typename T>
struct FlagValue<T, FlagValueStorageKind::kAlignedBuffer> {
  bool Get(const SequenceLock&, T&) const { return false; }

  alignas(T) char value[sizeof(T)];
};

// Flag callback auxiliary structs.

// The callback is noexcept.
// TODO(rogeeff): add noexcept after C++17 support is added.
using FlagCallbackFunc = void (*)();

struct FlagCallback {
  FlagCallbackFunc func;
  absl::Mutex guard;  // Guard for concurrent callback invocations.
};

// Flag implementation, which does not depend on flag value type.
// The class encapsulates the Flag's data and access to it.

struct DynValueDeleter {
  explicit DynValueDeleter(FlagOpFn op_arg = nullptr);
  void operator()(void* ptr) const;

  FlagOpFn op;
};

class FlagState;

class FlagImpl final : public CommandLineFlag {
 public:
  constexpr FlagImpl(const char* name, const char* filename, FlagOpFn op,
                     FlagHelpArg help, FlagValueStorageKind value_kind,
                     FlagDefaultArg default_arg)
      : name_(name),
        filename_(filename),
        op_(op),
        help_(help.source),
        help_source_kind_(static_cast<uint8_t>(help.kind)),
        value_storage_kind_(static_cast<uint8_t>(value_kind)),
        def_kind_(static_cast<uint8_t>(default_arg.kind)),
        modified_(false),
        on_command_line_(false),
        callback_(nullptr),
        default_value_(default_arg.source),
        data_guard_{} {}

  int64_t ReadOneWord() const ABSL_LOCKS_EXCLUDED(*DataGuard());
  bool ReadOneBool() const ABSL_LOCKS_EXCLUDED(*DataGuard());
  void Read(void* dst) const override ABSL_LOCKS_EXCLUDED(*DataGuard());
  void Read(bool* value) const ABSL_LOCKS_EXCLUDED(*DataGuard()) {
    *value = ReadOneBool();
  }
  template <typename T,
            absl::enable_if_t<flags_internal::StorageKind<T>() ==
                                  FlagValueStorageKind::kOneWordAtomic,
                              int> = 0>
  void Read(T* value) const ABSL_LOCKS_EXCLUDED(*DataGuard()) {
    int64_t v = ReadOneWord();
    std::memcpy(value, static_cast<const void*>(&v), sizeof(T));
  }
  template <typename T,
            typename std::enable_if<flags_internal::StorageKind<T>() ==
                                        FlagValueStorageKind::kValueAndInitBit,
                                    int>::type = 0>
  void Read(T* value) const ABSL_LOCKS_EXCLUDED(*DataGuard()) {
    *value = absl::bit_cast<FlagValueAndInitBit<T>>(ReadOneWord()).value;
  }

  void Write(const void* src) ABSL_LOCKS_EXCLUDED(*DataGuard());

  void SetCallback(const FlagCallbackFunc mutation_callback)
      ABSL_LOCKS_EXCLUDED(*DataGuard());
  void InvokeCallback() const ABSL_EXCLUSIVE_LOCKS_REQUIRED(*DataGuard());






  void AssertValidType(FlagFastTypeId type_id,
                       const std::type_info* (*gen_rtti)()) const;

 private:
  template <typename T>
  friend class Flag;
  friend class FlagState;

  absl::Mutex* DataGuard() const
      ABSL_LOCK_RETURNED(reinterpret_cast<absl::Mutex*>(data_guard_));

  std::unique_ptr<void, DynValueDeleter> MakeInitValue() const
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(*DataGuard());

  void Init();





  template <typename StorageT>
  StorageT* OffsetValue() const;



  void* AlignedBufferValue() const;

  std::atomic<uint64_t>* AtomicBufferValue() const;


  std::atomic<int64_t>& OneWordValue() const;


  std::unique_ptr<void, DynValueDeleter> TryParse(absl::string_view value,
                                                  std::string& err) const
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(*DataGuard());

  void StoreValue(const void* src) ABSL_EXCLUSIVE_LOCKS_REQUIRED(*DataGuard());



  void ReadSequenceLockedData(void* dst) const
      ABSL_LOCKS_EXCLUDED(*DataGuard());

  FlagHelpKind HelpSourceKind() const {
    return static_cast<FlagHelpKind>(help_source_kind_);
  }
  FlagValueStorageKind ValueStorageKind() const {
    return static_cast<FlagValueStorageKind>(value_storage_kind_);
  }
  FlagDefaultKind DefaultKind() const
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(*DataGuard()) {
    return static_cast<FlagDefaultKind>(def_kind_);
  }

  absl::string_view Name() const override;
  std::string Filename() const override;
  std::string Help() const override;
  FlagFastTypeId TypeId() const override;
  bool IsSpecifiedOnCommandLine() const override
      ABSL_LOCKS_EXCLUDED(*DataGuard());
  std::string DefaultValue() const override ABSL_LOCKS_EXCLUDED(*DataGuard());
  std::string CurrentValue() const override ABSL_LOCKS_EXCLUDED(*DataGuard());
  bool ValidateInputValue(absl::string_view value) const override
      ABSL_LOCKS_EXCLUDED(*DataGuard());
  void CheckDefaultValueParsingRoundtrip() const override
      ABSL_LOCKS_EXCLUDED(*DataGuard());

  int64_t ModificationCount() const ABSL_EXCLUSIVE_LOCKS_REQUIRED(*DataGuard());



  std::unique_ptr<FlagStateInterface> SaveState() override
      ABSL_LOCKS_EXCLUDED(*DataGuard());


  bool RestoreState(const FlagState& flag_state)
      ABSL_LOCKS_EXCLUDED(*DataGuard());

  bool ParseFrom(absl::string_view value, FlagSettingMode set_mode,
                 ValueSource source, std::string& error) override
      ABSL_LOCKS_EXCLUDED(*DataGuard());


  const char* const name_;

  const char* const filename_;

  const FlagOpFn op_;

  const FlagHelpMsg help_;

  const uint8_t help_source_kind_ : 1;

  const uint8_t value_storage_kind_ : 2;

  uint8_t : 0;  // The bytes containing the const bitfields must not be




  uint8_t def_kind_ : 2;

  bool modified_ : 1 ABSL_GUARDED_BY(*DataGuard());

  bool on_command_line_ : 1 ABSL_GUARDED_BY(*DataGuard());

  absl::once_flag init_control_;

  flags_internal::SequenceLock seq_lock_;

  FlagCallback* callback_ ABSL_GUARDED_BY(*DataGuard());




  FlagDefaultSrc default_value_;







  alignas(absl::Mutex) mutable char data_guard_[sizeof(absl::Mutex)];
};

// The Flag object parameterized by the flag's value type. This class implements
// flag reflection handle interface.

template <typename T>
class Flag {
 public:
  constexpr Flag(const char* name, const char* filename, FlagHelpArg help,
                 const FlagDefaultArg default_arg)
      : impl_(name, filename, &FlagOps<T>, help,
              flags_internal::StorageKind<T>(), default_arg),
        value_() {}

  absl::string_view Name() const { return impl_.Name(); }
  std::string Filename() const { return impl_.Filename(); }
  std::string Help() const { return impl_.Help(); }

  bool IsSpecifiedOnCommandLine() const {
    return impl_.IsSpecifiedOnCommandLine();
  }
  std::string DefaultValue() const { return impl_.DefaultValue(); }
  std::string CurrentValue() const { return impl_.CurrentValue(); }

 private:
  template <typename, bool>
  friend class FlagRegistrar;
  friend class FlagImplPeer;

  T Get() const {

    union U {
      T value;
      U() {}
      ~U() { value.~T(); }
    };
    U u;

#if !defined(NDEBUG)
    impl_.AssertValidType(base_internal::FastTypeId<T>(), &GenRuntimeTypeId<T>);
#endif

    if (ABSL_PREDICT_FALSE(!value_.Get(impl_.seq_lock_, u.value))) {
      impl_.Read(&u.value);
    }
    return std::move(u.value);
  }
  void Set(const T& v) {
    impl_.AssertValidType(base_internal::FastTypeId<T>(), &GenRuntimeTypeId<T>);
    impl_.Write(&v);
  }

  const CommandLineFlag& Reflect() const { return impl_; }




  FlagImpl impl_;
  FlagValue<T> value_;
};

// Trampoline for friend access

class FlagImplPeer {
 public:
  template <typename T, typename FlagType>
  static T InvokeGet(const FlagType& flag) {
    return flag.Get();
  }
  template <typename FlagType, typename T>
  static void InvokeSet(FlagType& flag, const T& v) {
    flag.Set(v);
  }
  template <typename FlagType>
  static const CommandLineFlag& InvokeReflect(const FlagType& f) {
    return f.Reflect();
  }
};

// Implementation of Flag value specific operations routine.
template <typename T>
void* FlagOps(FlagOp op, const void* v1, void* v2, void* v3) {
  switch (op) {
    case FlagOp::kAlloc: {
      std::allocator<T> alloc;
      return std::allocator_traits<std::allocator<T>>::allocate(alloc, 1);
    }
    case FlagOp::kDelete: {
      T* p = static_cast<T*>(v2);
      p->~T();
      std::allocator<T> alloc;
      std::allocator_traits<std::allocator<T>>::deallocate(alloc, p, 1);
      return nullptr;
    }
    case FlagOp::kCopy:
      *static_cast<T*>(v2) = *static_cast<const T*>(v1);
      return nullptr;
    case FlagOp::kCopyConstruct:
      new (v2) T(*static_cast<const T*>(v1));
      return nullptr;
    case FlagOp::kSizeof:
      return reinterpret_cast<void*>(static_cast<uintptr_t>(sizeof(T)));
    case FlagOp::kFastTypeId:
      return const_cast<void*>(base_internal::FastTypeId<T>());
    case FlagOp::kRuntimeTypeId:
      return const_cast<std::type_info*>(GenRuntimeTypeId<T>());
    case FlagOp::kParse: {


      T temp(*static_cast<T*>(v2));
      if (!absl::ParseFlag<T>(*static_cast<const absl::string_view*>(v1), &temp,
                              static_cast<std::string*>(v3))) {
        return nullptr;
      }
      *static_cast<T*>(v2) = std::move(temp);
      return v2;
    }
    case FlagOp::kUnparse:
      *static_cast<std::string*>(v2) =
          absl::UnparseFlag<T>(*static_cast<const T*>(v1));
      return nullptr;
    case FlagOp::kValueOffset: {


      size_t round_to = alignof(FlagValue<T>);
      size_t offset =
          (sizeof(FlagImpl) + round_to - 1) / round_to * round_to;
      return reinterpret_cast<void*>(offset);
    }
  }
  return nullptr;
}

// This class facilitates Flag object registration and tail expression-based
// flag definition, for example:
// ABSL_FLAG(int, foo, 42, "Foo help").OnUpdate(NotifyFooWatcher);
struct FlagRegistrarEmpty {};
template <typename T, bool do_register>
class FlagRegistrar {
 public:
  explicit FlagRegistrar(Flag<T>& flag, const char* filename) : flag_(flag) {
    if (do_register)
      flags_internal::RegisterCommandLineFlag(flag_.impl_, filename);
  }

  FlagRegistrar OnUpdate(FlagCallbackFunc cb) && {
    flag_.impl_.SetCallback(cb);
    return *this;
  }



  operator FlagRegistrarEmpty() const { return {}; }  // NOLINT

 private:
  Flag<T>& flag_;  // Flag being registered (not owned).
};

}  // namespace flags_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_FLAGS_INTERNAL_FLAG_H_
