//
// Copyright 2017 The Abseil Authors.
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
// -----------------------------------------------------------------------------
// File: string_view.h
// -----------------------------------------------------------------------------
//
// This file contains the definition of the `absl::string_view` class. A
// `string_view` points to a contiguous span of characters, often part or all of
// another `std::string`, double-quoted string literal, character array, or even
// another `string_view`.
//
// This `absl::string_view` abstraction is designed to be a drop-in
// replacement for the C++17 `std::string_view` abstraction.
#ifndef ABSL_STRINGS_STRING_VIEW_H_
#define ABSL_STRINGS_STRING_VIEW_H_

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <string>

#include "absl/base/attributes.h"
#include "absl/base/config.h"
#include "absl/base/internal/throw_delegate.h"
#include "absl/base/macros.h"
#include "absl/base/optimization.h"
#include "absl/base/port.h"

#ifdef ABSL_USES_STD_STRING_VIEW

#include <string_view>  // IWYU pragma: export

namespace absl {
ABSL_NAMESPACE_BEGIN
using string_view = std::string_view;
ABSL_NAMESPACE_END
}  // namespace absl

#else  // ABSL_USES_STD_STRING_VIEW

#if ABSL_HAVE_BUILTIN(__builtin_memcmp) ||        \
    (defined(__GNUC__) && !defined(__clang__)) || \
    (defined(_MSC_VER) && _MSC_VER >= 1928)
#define ABSL_INTERNAL_STRING_VIEW_MEMCMP __builtin_memcmp
#else  // ABSL_HAVE_BUILTIN(__builtin_memcmp)
#define ABSL_INTERNAL_STRING_VIEW_MEMCMP memcmp
#endif  // ABSL_HAVE_BUILTIN(__builtin_memcmp)

namespace absl {
ABSL_NAMESPACE_BEGIN

//
// A `string_view` provides a lightweight view into the string data provided by
// a `std::string`, double-quoted string literal, character array, or even
// another `string_view`. A `string_view` does *not* own the string to which it
// points, and that data cannot be modified through the view.
//
// You can use `string_view` as a function or method parameter anywhere a
// parameter can receive a double-quoted string literal, `const char*`,
// `std::string`, or another `absl::string_view` argument with no need to copy
// the string data. Systematic use of `string_view` within function arguments
// reduces data copies and `strlen()` calls.
//
// Because of its small size, prefer passing `string_view` by value:
//
//   void MyFunction(absl::string_view arg);
//
// If circumstances require, you may also pass one by const reference:
//
//   void MyFunction(const absl::string_view& arg);  // not preferred
//
// Passing by value generates slightly smaller code for many architectures.
//
// In either case, the source data of the `string_view` must outlive the
// `string_view` itself.
//
// A `string_view` is also suitable for local variables if you know that the
// lifetime of the underlying object is longer than the lifetime of your
// `string_view` variable. However, beware of binding a `string_view` to a
// temporary value:
//
//   // BAD use of string_view: lifetime problem
//   absl::string_view sv = obj.ReturnAString();
//
//   // GOOD use of string_view: str outlives sv
//   std::string str = obj.ReturnAString();
//   absl::string_view sv = str;
//
// Due to lifetime issues, a `string_view` is sometimes a poor choice for a
// return value and usually a poor choice for a data member. If you do use a
// `string_view` this way, it is your responsibility to ensure that the object
// pointed to by the `string_view` outlives the `string_view`.
//
// A `string_view` may represent a whole string or just part of a string. For
// example, when splitting a string, `std::vector<absl::string_view>` is a
// natural data type for the output.
//
// For another example, a Cord is a non-contiguous, potentially very
// long string-like object.  The Cord class has an interface that iteratively
// provides string_view objects that point to the successive pieces of a Cord
// object.
//
// When constructed from a source which is NUL-terminated, the `string_view`
// itself will not include the NUL-terminator unless a specific size (including
// the NUL) is passed to the constructor. As a result, common idioms that work
// on NUL-terminated strings do not work on `string_view` objects. If you write
// code that scans a `string_view`, you must check its length rather than test
// for nul, for example. Note, however, that nuls may still be embedded within
// a `string_view` explicitly.
//
// You may create a null `string_view` in two ways:
//
//   absl::string_view sv;
//   absl::string_view sv(nullptr, 0);
//
// For the above, `sv.data() == nullptr`, `sv.length() == 0`, and
// `sv.empty() == true`. Also, if you create a `string_view` with a non-null
// pointer then `sv.data() != nullptr`. Thus, you can use `string_view()` to
// signal an undefined value that is different from other `string_view` values
// in a similar fashion to how `const char* p1 = nullptr;` is different from
// `const char* p2 = "";`. However, in practice, it is not recommended to rely
// on this behavior.
//
// Be careful not to confuse a null `string_view` with an empty one. A null
// `string_view` is an empty `string_view`, but some empty `string_view`s are
// not null. Prefer checking for emptiness over checking for null.
//
// There are many ways to create an empty string_view:
//
//   const char* nullcp = nullptr;
//   // string_view.size() will return 0 in all cases.
//   absl::string_view();
//   absl::string_view(nullcp, 0);
//   absl::string_view("");
//   absl::string_view("", 0);
//   absl::string_view("abcdef", 0);
//   absl::string_view("abcdef" + 6, 0);
//
// All empty `string_view` objects whether null or not, are equal:
//
//   absl::string_view() == absl::string_view("", 0)
//   absl::string_view(nullptr, 0) == absl::string_view("abcdef"+6, 0)
class string_view {
 public:
  using traits_type = std::char_traits<char>;
  using value_type = char;
  using pointer = char*;
  using const_pointer = const char*;
  using reference = char&;
  using const_reference = const char&;
  using const_iterator = const char*;
  using iterator = const_iterator;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reverse_iterator = const_reverse_iterator;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;

  static constexpr size_type npos = static_cast<size_type>(-1);

  constexpr string_view() noexcept : ptr_(nullptr), length_(0) {}


  template <typename Allocator>
  string_view(  // NOLINT(runtime/explicit)
      const std::basic_string<char, std::char_traits<char>, Allocator>& str
          ABSL_ATTRIBUTE_LIFETIME_BOUND) noexcept




      : string_view(str.data(), str.size(), SkipCheckLengthTag{}) {}




  constexpr string_view(const char* str)  // NOLINT(runtime/explicit)
      : ptr_(str), length_(str ? StrlenInternal(str) : 0) {}

  constexpr string_view(const char* data, size_type len)
      : ptr_(data), length_(CheckLengthInternal(len)) {}








  constexpr const_iterator begin() const noexcept { return ptr_; }





  constexpr const_iterator end() const noexcept { return ptr_ + length_; }




  constexpr const_iterator cbegin() const noexcept { return begin(); }





  constexpr const_iterator cend() const noexcept { return end(); }




  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }





  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }




  const_reverse_iterator crbegin() const noexcept { return rbegin(); }





  const_reverse_iterator crend() const noexcept { return rend(); }




  constexpr size_type size() const noexcept { return length_; }



  constexpr size_type length() const noexcept { return size(); }



  constexpr size_type max_size() const noexcept { return kMaxSize; }



  constexpr bool empty() const noexcept { return length_ == 0; }




  constexpr const_reference operator[](size_type i) const {
    return ABSL_HARDENING_ASSERT(i < size()), ptr_[i];
  }





  constexpr const_reference at(size_type i) const {
    return ABSL_PREDICT_TRUE(i < size())
               ? ptr_[i]
               : ((void)base_internal::ThrowStdOutOfRange(
                      "absl::string_view::at"),
                  ptr_[i]);
  }



  constexpr const_reference front() const {
    return ABSL_HARDENING_ASSERT(!empty()), ptr_[0];
  }



  constexpr const_reference back() const {
    return ABSL_HARDENING_ASSERT(!empty()), ptr_[size() - 1];
  }







  constexpr const_pointer data() const noexcept { return ptr_; }





  constexpr void remove_prefix(size_type n) {
    ABSL_HARDENING_ASSERT(n <= length_);
    ptr_ += n;
    length_ -= n;
  }




  constexpr void remove_suffix(size_type n) {
    ABSL_HARDENING_ASSERT(n <= length_);
    length_ -= n;
  }



  constexpr void swap(string_view& s) noexcept {
    auto t = *this;
    *this = s;
    s = t;
  }


  template <typename A>
  explicit operator std::basic_string<char, traits_type, A>() const {
    if (!data()) return {};
    return std::basic_string<char, traits_type, A>(data(), size());
  }




  size_type copy(char* buf, size_type n, size_type pos = 0) const {
    if (ABSL_PREDICT_FALSE(pos > length_)) {
      base_internal::ThrowStdOutOfRange("absl::string_view::copy");
    }
    size_type rlen = (std::min)(length_ - pos, n);
    if (rlen > 0) {
      const char* start = ptr_ + pos;
      traits_type::copy(buf, start, rlen);
    }
    return rlen;
  }






  constexpr string_view substr(size_type pos = 0, size_type n = npos) const {
    return ABSL_PREDICT_FALSE(pos > length_)
               ? (base_internal::ThrowStdOutOfRange(
                      "absl::string_view::substr"),
                  string_view())
               : string_view(ptr_ + pos, Min(n, length_ - pos));
  }






  constexpr int compare(string_view x) const noexcept {
    return CompareImpl(length_, x.length_,
                       Min(length_, x.length_) == 0
                           ? 0
                           : ABSL_INTERNAL_STRING_VIEW_MEMCMP(
                                 ptr_, x.ptr_, Min(length_, x.length_)));
  }


  constexpr int compare(size_type pos1, size_type count1, string_view v) const {
    return substr(pos1, count1).compare(v);
  }


  constexpr int compare(size_type pos1, size_type count1, string_view v,
                        size_type pos2, size_type count2) const {
    return substr(pos1, count1).compare(v.substr(pos2, count2));
  }


  constexpr int compare(const char* s) const { return compare(string_view(s)); }


  constexpr int compare(size_type pos1, size_type count1, const char* s) const {
    return substr(pos1, count1).compare(string_view(s));
  }


  constexpr int compare(size_type pos1, size_type count1, const char* s,
                        size_type count2) const {
    return substr(pos1, count1).compare(string_view(s, count2));
  }






  size_type find(string_view s, size_type pos = 0) const noexcept;


  size_type find(char c, size_type pos = 0) const noexcept;


  size_type find(const char* s, size_type pos, size_type count) const {
    return find(string_view(s, count), pos);
  }


  size_type find(const char* s, size_type pos = 0) const {
    return find(string_view(s), pos);
  }





  size_type rfind(string_view s, size_type pos = npos) const noexcept;


  size_type rfind(char c, size_type pos = npos) const noexcept;


  size_type rfind(const char* s, size_type pos, size_type count) const {
    return rfind(string_view(s, count), pos);
  }


  size_type rfind(const char* s, size_type pos = npos) const {
    return rfind(string_view(s), pos);
  }





  size_type find_first_of(string_view s, size_type pos = 0) const noexcept;


  size_type find_first_of(char c, size_type pos = 0) const noexcept {
    return find(c, pos);
  }


  size_type find_first_of(const char* s, size_type pos,
                                    size_type count) const {
    return find_first_of(string_view(s, count), pos);
  }


  size_type find_first_of(const char* s, size_type pos = 0) const {
    return find_first_of(string_view(s), pos);
  }





  size_type find_last_of(string_view s, size_type pos = npos) const noexcept;


  size_type find_last_of(char c, size_type pos = npos) const noexcept {
    return rfind(c, pos);
  }


  size_type find_last_of(const char* s, size_type pos, size_type count) const {
    return find_last_of(string_view(s, count), pos);
  }


  size_type find_last_of(const char* s, size_type pos = npos) const {
    return find_last_of(string_view(s), pos);
  }





  size_type find_first_not_of(string_view s, size_type pos = 0) const noexcept;


  size_type find_first_not_of(char c, size_type pos = 0) const noexcept;


  size_type find_first_not_of(const char* s, size_type pos,
                              size_type count) const {
    return find_first_not_of(string_view(s, count), pos);
  }


  size_type find_first_not_of(const char* s, size_type pos = 0) const {
    return find_first_not_of(string_view(s), pos);
  }





  size_type find_last_not_of(string_view s,
                             size_type pos = npos) const noexcept;


  size_type find_last_not_of(char c, size_type pos = npos) const noexcept;


  size_type find_last_not_of(const char* s, size_type pos,
                             size_type count) const {
    return find_last_not_of(string_view(s, count), pos);
  }


  size_type find_last_not_of(const char* s, size_type pos = npos) const {
    return find_last_not_of(string_view(s), pos);
  }

 private:


  struct SkipCheckLengthTag {};
  string_view(const char* data, size_type len, SkipCheckLengthTag) noexcept
      : ptr_(data), length_(len) {}

  static constexpr size_type kMaxSize =
      (std::numeric_limits<difference_type>::max)();

  static constexpr size_type CheckLengthInternal(size_type len) {
    return ABSL_HARDENING_ASSERT(len <= kMaxSize), len;
  }

  static constexpr size_type StrlenInternal(const char* str) {
#if defined(_MSC_VER) && _MSC_VER >= 1910 && !defined(__clang__)

    const char* begin = str;
    while (*str != '\0') ++str;
    return str - begin;
#elif ABSL_HAVE_BUILTIN(__builtin_strlen) || \
    (defined(__GNUC__) && !defined(__clang__))




    return __builtin_strlen(str);
#else
    return str ? strlen(str) : 0;
#endif
  }

  static constexpr size_t Min(size_type length_a, size_type length_b) {
    return length_a < length_b ? length_a : length_b;
  }

  static constexpr int CompareImpl(size_type length_a, size_type length_b,
                                   int compare_result) {
    return compare_result == 0 ? static_cast<int>(length_a > length_b) -
                                     static_cast<int>(length_a < length_b)
                               : (compare_result < 0 ? -1 : 1);
  }

  const char* ptr_;
  size_type length_;
};

// one of the arguments is a literal, the compiler can elide a lot of the
// following comparisons.
constexpr bool operator==(string_view x, string_view y) noexcept {
  return x.size() == y.size() &&
         (x.empty() ||
          ABSL_INTERNAL_STRING_VIEW_MEMCMP(x.data(), y.data(), x.size()) == 0);
}

constexpr bool operator!=(string_view x, string_view y) noexcept {
  return !(x == y);
}

constexpr bool operator<(string_view x, string_view y) noexcept {
  return x.compare(y) < 0;
}

constexpr bool operator>(string_view x, string_view y) noexcept {
  return y < x;
}

constexpr bool operator<=(string_view x, string_view y) noexcept {
  return !(y < x);
}

constexpr bool operator>=(string_view x, string_view y) noexcept {
  return !(x < y);
}

std::ostream& operator<<(std::ostream& o, string_view piece);

ABSL_NAMESPACE_END
}  // namespace absl

#undef ABSL_INTERNAL_STRING_VIEW_MEMCMP

#endif  // ABSL_USES_STD_STRING_VIEW

namespace absl {
ABSL_NAMESPACE_BEGIN

//
// Like `s.substr(pos, n)`, but clips `pos` to an upper bound of `s.size()`.
// Provided because std::string_view::substr throws if `pos > size()`
inline string_view ClippedSubstr(string_view s, size_t pos,
                                 size_t n = string_view::npos) {
  pos = (std::min)(pos, static_cast<size_t>(s.size()));
  return s.substr(pos, n);
}

//
// Creates an `absl::string_view` from a pointer `p` even if it's null-valued.
// This function should be used where an `absl::string_view` can be created from
// a possibly-null pointer.
constexpr string_view NullSafeStringView(const char* p) {
  return p ? string_view(p) : string_view();
}

ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STRINGS_STRING_VIEW_H_
