// Copyright 2018 The Abseil Authors.
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
//
//                           MOTIVATION AND TUTORIAL
//
// If you want to put in a single heap allocation N doubles followed by M ints,
// it's easy if N and M are known at compile time.
//
//   struct S {
//     double a[N];
//     int b[M];
//   };
//
//   S* p = new S;
//
// But what if N and M are known only in run time? Class template Layout to the
// rescue! It's a portable generalization of the technique known as struct hack.
//
//   // This object will tell us everything we need to know about the memory
//   // layout of double[N] followed by int[M]. It's structurally identical to
//   // size_t[2] that stores N and M. It's very cheap to create.
//   const Layout<double, int> layout(N, M);
//
//   // Allocate enough memory for both arrays. `AllocSize()` tells us how much
//   // memory is needed. We are free to use any allocation function we want as
//   // long as it returns aligned memory.
//   std::unique_ptr<unsigned char[]> p(new unsigned char[layout.AllocSize()]);
//
//   // Obtain the pointer to the array of doubles.
//   // Equivalent to `reinterpret_cast<double*>(p.get())`.
//   //
//   // We could have written layout.Pointer<0>(p) instead. If all the types are
//   // unique you can use either form, but if some types are repeated you must
//   // use the index form.
//   double* a = layout.Pointer<double>(p.get());
//
//   // Obtain the pointer to the array of ints.
//   // Equivalent to `reinterpret_cast<int*>(p.get() + N * 8)`.
//   int* b = layout.Pointer<int>(p);
//
// If we are unable to specify sizes of all fields, we can pass as many sizes as
// we can to `Partial()`. In return, it'll allow us to access the fields whose
// locations and sizes can be computed from the provided information.
// `Partial()` comes in handy when the array sizes are embedded into the
// allocation.
//
//   // size_t[1] containing N, size_t[1] containing M, double[N], int[M].
//   using L = Layout<size_t, size_t, double, int>;
//
//   unsigned char* Allocate(size_t n, size_t m) {
//     const L layout(1, 1, n, m);
//     unsigned char* p = new unsigned char[layout.AllocSize()];
//     *layout.Pointer<0>(p) = n;
//     *layout.Pointer<1>(p) = m;
//     return p;
//   }
//
//   void Use(unsigned char* p) {
//     // First, extract N and M.
//     // Specify that the first array has only one element. Using `prefix` we
//     // can access the first two arrays but not more.
//     constexpr auto prefix = L::Partial(1);
//     size_t n = *prefix.Pointer<0>(p);
//     size_t m = *prefix.Pointer<1>(p);
//
//     // Now we can get pointers to the payload.
//     const L layout(1, 1, n, m);
//     double* a = layout.Pointer<double>(p);
//     int* b = layout.Pointer<int>(p);
//   }
//
// The layout we used above combines fixed-size with dynamically-sized fields.
// This is quite common. Layout is optimized for this use case and generates
// optimal code. All computations that can be performed at compile time are
// indeed performed at compile time.
//
// Efficiency tip: The order of fields matters. In `Layout<T1, ..., TN>` try to
// ensure that `alignof(T1) >= ... >= alignof(TN)`. This way you'll have no
// padding in between arrays.
//
// You can manually override the alignment of an array by wrapping the type in
// `Aligned<T, N>`. `Layout<..., Aligned<T, N>, ...>` has exactly the same API
// and behavior as `Layout<..., T, ...>` except that the first element of the
// array of `T` is aligned to `N` (the rest of the elements follow without
// padding). `N` cannot be less than `alignof(T)`.
//
// `AllocSize()` and `Pointer()` are the most basic methods for dealing with
// memory layouts. Check out the reference or code below to discover more.
//
//                            EXAMPLE
//
//   // Immutable move-only string with sizeof equal to sizeof(void*). The
//   // string size and the characters are kept in the same heap allocation.
//   class CompactString {
//    public:
//     CompactString(const char* s = "") {
//       const size_t size = strlen(s);
//       // size_t[1] followed by char[size + 1].
//       const L layout(1, size + 1);
//       p_.reset(new unsigned char[layout.AllocSize()]);
//       // If running under ASAN, mark the padding bytes, if any, to catch
//       // memory errors.
//       layout.PoisonPadding(p_.get());
//       // Store the size in the allocation.
//       *layout.Pointer<size_t>(p_.get()) = size;
//       // Store the characters in the allocation.
//       memcpy(layout.Pointer<char>(p_.get()), s, size + 1);
//     }
//
//     size_t size() const {
//       // Equivalent to reinterpret_cast<size_t&>(*p).
//       return *L::Partial().Pointer<size_t>(p_.get());
//     }
//
//     const char* c_str() const {
//       // Equivalent to reinterpret_cast<char*>(p.get() + sizeof(size_t)).
//       // The argument in Partial(1) specifies that we have size_t[1] in front
//       // of the characters.
//       return L::Partial(1).Pointer<char>(p_.get());
//     }
//
//    private:
//     // Our heap allocation contains a size_t followed by an array of chars.
//     using L = Layout<size_t, char>;
//     std::unique_ptr<unsigned char[]> p_;
//   };
//
//   int main() {
//     CompactString s = "hello";
//     assert(s.size() == 5);
//     assert(strcmp(s.c_str(), "hello") == 0);
//   }
//
//                               DOCUMENTATION
//
// The interface exported by this file consists of:
// - class `Layout<>` and its public members.
// - The public members of class `internal_layout::LayoutImpl<>`. That class
//   isn't intended to be used directly, and its name and template parameter
//   list are internal implementation details, but the class itself provides
//   most of the functionality in this file. See comments on its members for
//   detailed documentation.
//
// `Layout<T1,... Tn>::Partial(count1,..., countm)` (where `m` <= `n`) returns a
// `LayoutImpl<>` object. `Layout<T1,..., Tn> layout(count1,..., countn)`
// creates a `Layout` object, which exposes the same functionality by inheriting
// from `LayoutImpl<>`.

#ifndef ABSL_CONTAINER_INTERNAL_LAYOUT_H_
#define ABSL_CONTAINER_INTERNAL_LAYOUT_H_

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <ostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include "absl/base/config.h"
#include "absl/meta/type_traits.h"
#include "absl/strings/str_cat.h"
#include "absl/types/span.h"
#include "absl/utility/utility.h"

#ifdef ABSL_HAVE_ADDRESS_SANITIZER
#include <sanitizer/asan_interface.h>
#endif

#if defined(__GXX_RTTI)
#define ABSL_INTERNAL_HAS_CXA_DEMANGLE
#endif

#ifdef ABSL_INTERNAL_HAS_CXA_DEMANGLE
#include <cxxabi.h>
#endif

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace container_internal {

// array. `Layout<..., Aligned<T, N>, ...>` has exactly the same API
// and behavior as `Layout<..., T, ...>` except that the first element of the
// array of `T` is aligned to `N` (the rest of the elements follow without
// padding).
//
// Requires: `N >= alignof(T)` and `N` is a power of 2.
template <class T, size_t N>
struct Aligned;

namespace internal_layout {

template <class T>
struct NotAligned {};

template <class T, size_t N>
struct NotAligned<const Aligned<T, N>> {
  static_assert(sizeof(T) == 0, "Aligned<T, N> cannot be const-qualified");
};

template <size_t>
using IntToSize = size_t;

template <class>
using TypeToSize = size_t;

template <class T>
struct Type : NotAligned<T> {
  using type = T;
};

template <class T, size_t N>
struct Type<Aligned<T, N>> {
  using type = T;
};

template <class T>
struct SizeOf : NotAligned<T>, std::integral_constant<size_t, sizeof(T)> {};

template <class T, size_t N>
struct SizeOf<Aligned<T, N>> : std::integral_constant<size_t, sizeof(T)> {};

template <class T>
struct AlignOf : NotAligned<T> {
  static constexpr size_t value = alignof(T);
};

template <class T, size_t N>
struct AlignOf<Aligned<T, N>> {
  static_assert(N % alignof(T) == 0,
                "Custom alignment can't be lower than the type's alignment");
  static constexpr size_t value = N;
};

template <class T, class... Ts>
using Contains = absl::disjunction<std::is_same<T, Ts>...>;

template <class From, class To>
using CopyConst =
    typename std::conditional<std::is_const<From>::value, const To, To>::type;

// MSVC.
template <class T>
using SliceType = Span<T>;

// being found by ADL.
namespace adl_barrier {

template <class Needle, class... Ts>
constexpr size_t Find(Needle, Needle, Ts...) {
  static_assert(!Contains<Needle, Ts...>(), "Duplicate element type");
  return 0;
}

template <class Needle, class T, class... Ts>
constexpr size_t Find(Needle, T, Ts...) {
  return adl_barrier::Find(Needle(), Ts()...) + 1;
}

constexpr bool IsPow2(size_t n) { return !(n & (n - 1)); }

// Requires: `m` is a power of two. It's enforced by IsLegalElementType below.
constexpr size_t Align(size_t n, size_t m) { return (n + m - 1) & ~(m - 1); }

constexpr size_t Min(size_t a, size_t b) { return b < a ? b : a; }

constexpr size_t Max(size_t a) { return a; }

template <class... Ts>
constexpr size_t Max(size_t a, size_t b, Ts... rest) {
  return adl_barrier::Max(b < a ? a : b, rest...);
}

template <class T>
std::string TypeName() {
  std::string out;
  int status = 0;
  char* demangled = nullptr;
#ifdef ABSL_INTERNAL_HAS_CXA_DEMANGLE
  demangled = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status);
#endif
  if (status == 0 && demangled != nullptr) {  // Demangling succeeded.
    absl::StrAppend(&out, "<", demangled, ">");
    free(demangled);
  } else {
#if defined(__GXX_RTTI) || defined(_CPPRTTI)
    absl::StrAppend(&out, "<", typeid(T).name(), ">");
#endif
  }
  return out;
}

}  // namespace adl_barrier

template <bool C>
using EnableIf = typename std::enable_if<C, int>::type;

template <class T>
using IsLegalElementType = std::integral_constant<
    bool, !std::is_reference<T>::value && !std::is_volatile<T>::value &&
              !std::is_reference<typename Type<T>::type>::value &&
              !std::is_volatile<typename Type<T>::type>::value &&
              adl_barrier::IsPow2(AlignOf<T>::value)>;

template <class Elements, class SizeSeq, class OffsetSeq>
class LayoutImpl;

//
// `Elements...` contains all template arguments of `Layout` that created this
// instance.
//
// `SizeSeq...` is `[0, NumSizes)` where `NumSizes` is the number of arguments
// passed to `Layout::Partial()` or `Layout::Layout()`.
//
// `OffsetSeq...` is `[0, NumOffsets)` where `NumOffsets` is
// `Min(sizeof...(Elements), NumSizes + 1)` (the number of arrays for which we
// can compute offsets).
template <class... Elements, size_t... SizeSeq, size_t... OffsetSeq>
class LayoutImpl<std::tuple<Elements...>, absl::index_sequence<SizeSeq...>,
                 absl::index_sequence<OffsetSeq...>> {
 private:
  static_assert(sizeof...(Elements) > 0, "At least one field is required");
  static_assert(absl::conjunction<IsLegalElementType<Elements>...>::value,
                "Invalid element type (see IsLegalElementType)");

  enum {
    NumTypes = sizeof...(Elements),
    NumSizes = sizeof...(SizeSeq),
    NumOffsets = sizeof...(OffsetSeq),
  };

  static_assert(NumOffsets == adl_barrier::Min(NumTypes, NumSizes + 1),
                "Internal error");
  static_assert(NumTypes > 0, "Internal error");


  template <class T>
  static constexpr size_t ElementIndex() {
    static_assert(Contains<Type<T>, Type<typename Type<Elements>::type>...>(),
                  "Type not found");
    return adl_barrier::Find(Type<T>(),
                             Type<typename Type<Elements>::type>()...);
  }

  template <size_t N>
  using ElementAlignment =
      AlignOf<typename std::tuple_element<N, std::tuple<Elements...>>::type>;

 public:

  using ElementTypes = std::tuple<typename Type<Elements>::type...>;

  template <size_t N>
  using ElementType = typename std::tuple_element<N, ElementTypes>::type;

  constexpr explicit LayoutImpl(IntToSize<SizeSeq>... sizes)
      : size_{sizes...} {}


  static constexpr size_t Alignment() {
    return adl_barrier::Max(AlignOf<Elements>::value...);
  }








  template <size_t N, EnableIf<N == 0> = 0>
  constexpr size_t Offset() const {
    return 0;
  }

  template <size_t N, EnableIf<N != 0> = 0>
  constexpr size_t Offset() const {
    static_assert(N < NumOffsets, "Index out of bounds");
    return adl_barrier::Align(
        Offset<N - 1>() + SizeOf<ElementType<N - 1>>::value * size_[N - 1],
        ElementAlignment<N>::value);
  }








  template <class T>
  constexpr size_t Offset() const {
    return Offset<ElementIndex<T>()>();
  }

  constexpr std::array<size_t, NumOffsets> Offsets() const {
    return {{Offset<OffsetSeq>()...}};
  }









  template <size_t N>
  constexpr size_t Size() const {
    static_assert(N < NumSizes, "Index out of bounds");
    return size_[N];
  }








  template <class T>
  constexpr size_t Size() const {
    return Size<ElementIndex<T>()>();
  }

  constexpr std::array<size_t, NumSizes> Sizes() const {
    return {{Size<SizeSeq>()...}};
  }












  template <size_t N, class Char>
  CopyConst<Char, ElementType<N>>* Pointer(Char* p) const {
    using C = typename std::remove_const<Char>::type;
    static_assert(
        std::is_same<C, char>() || std::is_same<C, unsigned char>() ||
            std::is_same<C, signed char>(),
        "The argument must be a pointer to [const] [signed|unsigned] char");
    constexpr size_t alignment = Alignment();
    (void)alignment;
    assert(reinterpret_cast<uintptr_t>(p) % alignment == 0);
    return reinterpret_cast<CopyConst<Char, ElementType<N>>*>(p + Offset<N>());
  }













  template <class T, class Char>
  CopyConst<Char, T>* Pointer(Char* p) const {
    return Pointer<ElementIndex<T>()>(p);
  }
















  template <class Char>
  std::tuple<CopyConst<
      Char, typename std::tuple_element<OffsetSeq, ElementTypes>::type>*...>
  Pointers(Char* p) const {
    return std::tuple<CopyConst<Char, ElementType<OffsetSeq>>*...>(
        Pointer<OffsetSeq>(p)...);
  }












  template <size_t N, class Char>
  SliceType<CopyConst<Char, ElementType<N>>> Slice(Char* p) const {
    return SliceType<CopyConst<Char, ElementType<N>>>(Pointer<N>(p), Size<N>());
  }












  template <class T, class Char>
  SliceType<CopyConst<Char, T>> Slice(Char* p) const {
    return Slice<ElementIndex<T>()>(p);
  }
















  template <class Char>
  std::tuple<SliceType<CopyConst<
      Char, typename std::tuple_element<SizeSeq, ElementTypes>::type>>...>
  Slices(Char* p) const {


    (void)p;
    return std::tuple<SliceType<CopyConst<Char, ElementType<SizeSeq>>>...>(
        Slice<SizeSeq>(p)...);
  }







  constexpr size_t AllocSize() const {
    static_assert(NumTypes == NumSizes, "You must specify sizes of all fields");
    return Offset<NumTypes - 1>() +
        SizeOf<ElementType<NumTypes - 1>>::value * size_[NumTypes - 1];
  }







  template <class Char, size_t N = NumOffsets - 1, EnableIf<N == 0> = 0>
  void PoisonPadding(const Char* p) const {
    Pointer<0>(p);  // verify the requirements on `Char` and `p`
  }

  template <class Char, size_t N = NumOffsets - 1, EnableIf<N != 0> = 0>
  void PoisonPadding(const Char* p) const {
    static_assert(N < NumOffsets, "Index out of bounds");
    (void)p;
#ifdef ABSL_HAVE_ADDRESS_SANITIZER
    PoisonPadding<Char, N - 1>(p);

    if (ElementAlignment<N - 1>::value % ElementAlignment<N>::value) {
      size_t start =
          Offset<N - 1>() + SizeOf<ElementType<N - 1>>::value * size_[N - 1];
      ASAN_POISON_MEMORY_REGION(p + start, Offset<N>() - start);
    }
#endif
  }
















  std::string DebugString() const {
    const auto offsets = Offsets();
    const size_t sizes[] = {SizeOf<ElementType<OffsetSeq>>::value...};
    const std::string types[] = {
        adl_barrier::TypeName<ElementType<OffsetSeq>>()...};
    std::string res = absl::StrCat("@0", types[0], "(", sizes[0], ")");
    for (size_t i = 0; i != NumOffsets - 1; ++i) {
      absl::StrAppend(&res, "[", size_[i], "]; @", offsets[i + 1], types[i + 1],
                      "(", sizes[i + 1], ")");
    }


    int last = static_cast<int>(NumSizes) - 1;
    if (NumTypes == NumSizes && last >= 0) {
      absl::StrAppend(&res, "[", size_[last], "]");
    }
    return res;
  }

 private:

  size_t size_[NumSizes > 0 ? NumSizes : 1];
};

template <size_t NumSizes, class... Ts>
using LayoutType = LayoutImpl<
    std::tuple<Ts...>, absl::make_index_sequence<NumSizes>,
    absl::make_index_sequence<adl_barrier::Min(sizeof...(Ts), NumSizes + 1)>>;

}  // namespace internal_layout

// another. See the top of the file for documentation.
//
// Check out the public API of internal_layout::LayoutImpl above. The type is
// internal to the library but its methods are public, and they are inherited
// by `Layout`.
template <class... Ts>
class Layout : public internal_layout::LayoutType<sizeof...(Ts), Ts...> {
 public:
  static_assert(sizeof...(Ts) > 0, "At least one field is required");
  static_assert(
      absl::conjunction<internal_layout::IsLegalElementType<Ts>...>::value,
      "Invalid element type (see IsLegalElementType)");

  template <size_t NumSizes>
  using PartialType = internal_layout::LayoutType<NumSizes, Ts...>;


























  template <class... Sizes>
  static constexpr PartialType<sizeof...(Sizes)> Partial(Sizes&&... sizes) {
    static_assert(sizeof...(Sizes) <= sizeof...(Ts), "");
    return PartialType<sizeof...(Sizes)>(absl::forward<Sizes>(sizes)...);
  }








  constexpr explicit Layout(internal_layout::TypeToSize<Ts>... sizes)
      : internal_layout::LayoutType<sizeof...(Ts), Ts...>(sizes...) {}
};

}  // namespace container_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_CONTAINER_INTERNAL_LAYOUT_H_
