// Copyright 2020 The Abseil Authors.
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
// File: statusor.h
// -----------------------------------------------------------------------------
//
// An `absl::StatusOr<T>` represents a union of an `absl::Status` object
// and an object of type `T`. The `absl::StatusOr<T>` will either contain an
// object of type `T` (indicating a successful operation), or an error (of type
// `absl::Status`) explaining why such a value is not present.
//
// In general, check the success of an operation returning an
// `absl::StatusOr<T>` like you would an `absl::Status` by using the `ok()`
// member function.
//
// Example:
//
//   StatusOr<Foo> result = Calculation();
//   if (result.ok()) {
//     result->DoSomethingCool();
//   } else {
//     LOG(ERROR) << result.status();
//   }
#ifndef ABSL_STATUS_STATUSOR_H_
#define ABSL_STATUS_STATUSOR_H_

#include <exception>
#include <initializer_list>
#include <new>
#include <string>
#include <type_traits>
#include <utility>

#include "absl/base/attributes.h"
#include "absl/base/call_once.h"
#include "absl/meta/type_traits.h"
#include "absl/status/internal/statusor_internal.h"
#include "absl/status/status.h"
#include "absl/types/variant.h"
#include "absl/utility/utility.h"

namespace absl {
ABSL_NAMESPACE_BEGIN

//
// This class defines the type of object to throw (if exceptions are enabled),
// when accessing the value of an `absl::StatusOr<T>` object that does not
// contain a value. This behavior is analogous to that of
// `std::bad_optional_access` in the case of accessing an invalid
// `std::optional` value.
//
// Example:
//
// try {
//   absl::StatusOr<int> v = FetchInt();
//   DoWork(v.value());  // Accessing value() when not "OK" may throw
// } catch (absl::BadStatusOrAccess& ex) {
//   LOG(ERROR) << ex.status();
// }
class BadStatusOrAccess : public std::exception {
 public:
  explicit BadStatusOrAccess(absl::Status status);
  ~BadStatusOrAccess() override = default;

  BadStatusOrAccess(const BadStatusOrAccess& other);
  BadStatusOrAccess& operator=(const BadStatusOrAccess& other);
  BadStatusOrAccess(BadStatusOrAccess&& other);
  BadStatusOrAccess& operator=(BadStatusOrAccess&& other);








  const char* what() const noexcept override;




  const absl::Status& status() const;

 private:
  void InitWhat() const;

  absl::Status status_;
  mutable absl::once_flag init_what_;
  mutable std::string what_;
};

template <typename T>
#if ABSL_HAVE_CPP_ATTRIBUTE(nodiscard)
// TODO(b/176172494): ABSL_MUST_USE_RESULT should expand to the more strict
// [[nodiscard]]. For now, just use [[nodiscard]] directly when it is available.
class [[nodiscard]] StatusOr;
#else
class ABSL_MUST_USE_RESULT StatusOr;
#endif  // ABSL_HAVE_CPP_ATTRIBUTE(nodiscard)

//
// The `absl::StatusOr<T>` class template is a union of an `absl::Status` object
// and an object of type `T`. The `absl::StatusOr<T>` models an object that is
// either a usable object, or an error (of type `absl::Status`) explaining why
// such an object is not present. An `absl::StatusOr<T>` is typically the return
// value of a function which may fail.
//
// An `absl::StatusOr<T>` can never hold an "OK" status (an
// `absl::StatusCode::kOk` value); instead, the presence of an object of type
// `T` indicates success. Instead of checking for a `kOk` value, use the
// `absl::StatusOr<T>::ok()` member function. (It is for this reason, and code
// readability, that using the `ok()` function is preferred for `absl::Status`
// as well.)
//
// Example:
//
//   StatusOr<Foo> result = DoBigCalculationThatCouldFail();
//   if (result.ok()) {
//     result->DoSomethingCool();
//   } else {
//     LOG(ERROR) << result.status();
//   }
//
// Accessing the object held by an `absl::StatusOr<T>` should be performed via
// `operator*` or `operator->`, after a call to `ok()` confirms that the
// `absl::StatusOr<T>` holds an object of type `T`:
//
// Example:
//
//   absl::StatusOr<int> i = GetCount();
//   if (i.ok()) {
//     updated_total += *i
//   }
//
// NOTE: using `absl::StatusOr<T>::value()` when no valid value is present will
// throw an exception if exceptions are enabled or terminate the process when
// exceptions are not enabled.
//
// Example:
//
//   StatusOr<Foo> result = DoBigCalculationThatCouldFail();
//   const Foo& foo = result.value();    // Crash/exception if no value present
//   foo.DoSomethingCool();
//
// A `absl::StatusOr<T*>` can be constructed from a null pointer like any other
// pointer value, and the result will be that `ok()` returns `true` and
// `value()` returns `nullptr`. Checking the value of pointer in an
// `absl::StatusOr<T*>` generally requires a bit more care, to ensure both that
// a value is present and that value is not null:
//
//  StatusOr<std::unique_ptr<Foo>> result = FooFactory::MakeNewFoo(arg);
//  if (!result.ok()) {
//    LOG(ERROR) << result.status();
//  } else if (*result == nullptr) {
//    LOG(ERROR) << "Unexpected null pointer";
//  } else {
//    (*result)->DoSomethingCool();
//  }
//
// Example factory implementation returning StatusOr<T>:
//
//  StatusOr<Foo> FooFactory::MakeFoo(int arg) {
//    if (arg <= 0) {
//      return absl::Status(absl::StatusCode::kInvalidArgument,
//                          "Arg must be positive");
//    }
//    return Foo(arg);
//  }
template <typename T>
class StatusOr : private internal_statusor::StatusOrData<T>,
                 private internal_statusor::CopyCtorBase<T>,
                 private internal_statusor::MoveCtorBase<T>,
                 private internal_statusor::CopyAssignBase<T>,
                 private internal_statusor::MoveAssignBase<T> {
  template <typename U>
  friend class StatusOr;

  typedef internal_statusor::StatusOrData<T> Base;

 public:





  typedef T value_type;






  explicit StatusOr();

  StatusOr(const StatusOr&) = default;


  StatusOr& operator=(const StatusOr&) = default;

  StatusOr(StatusOr&&) = default;


  StatusOr& operator=(StatusOr&&) = default;







  template <
      typename U,
      absl::enable_if_t<
          absl::conjunction<
              absl::negation<std::is_same<T, U>>,
              std::is_constructible<T, const U&>,
              std::is_convertible<const U&, T>,
              absl::negation<
                  internal_statusor::IsConstructibleOrConvertibleFromStatusOr<
                      T, U>>>::value,
          int> = 0>
  StatusOr(const StatusOr<U>& other)  // NOLINT
      : Base(static_cast<const typename StatusOr<U>::Base&>(other)) {}
  template <
      typename U,
      absl::enable_if_t<
          absl::conjunction<
              absl::negation<std::is_same<T, U>>,
              std::is_constructible<T, const U&>,
              absl::negation<std::is_convertible<const U&, T>>,
              absl::negation<
                  internal_statusor::IsConstructibleOrConvertibleFromStatusOr<
                      T, U>>>::value,
          int> = 0>
  explicit StatusOr(const StatusOr<U>& other)
      : Base(static_cast<const typename StatusOr<U>::Base&>(other)) {}

  template <
      typename U,
      absl::enable_if_t<
          absl::conjunction<
              absl::negation<std::is_same<T, U>>, std::is_constructible<T, U&&>,
              std::is_convertible<U&&, T>,
              absl::negation<
                  internal_statusor::IsConstructibleOrConvertibleFromStatusOr<
                      T, U>>>::value,
          int> = 0>
  StatusOr(StatusOr<U>&& other)  // NOLINT
      : Base(static_cast<typename StatusOr<U>::Base&&>(other)) {}
  template <
      typename U,
      absl::enable_if_t<
          absl::conjunction<
              absl::negation<std::is_same<T, U>>, std::is_constructible<T, U&&>,
              absl::negation<std::is_convertible<U&&, T>>,
              absl::negation<
                  internal_statusor::IsConstructibleOrConvertibleFromStatusOr<
                      T, U>>>::value,
          int> = 0>
  explicit StatusOr(StatusOr<U>&& other)
      : Base(static_cast<typename StatusOr<U>::Base&&>(other)) {}


















  template <
      typename U,
      absl::enable_if_t<
          absl::conjunction<
              absl::negation<std::is_same<T, U>>,
              std::is_constructible<T, const U&>,
              std::is_assignable<T, const U&>,
              absl::negation<
                  internal_statusor::
                      IsConstructibleOrConvertibleOrAssignableFromStatusOr<
                          T, U>>>::value,
          int> = 0>
  StatusOr& operator=(const StatusOr<U>& other) {
    this->Assign(other);
    return *this;
  }
  template <
      typename U,
      absl::enable_if_t<
          absl::conjunction<
              absl::negation<std::is_same<T, U>>, std::is_constructible<T, U&&>,
              std::is_assignable<T, U&&>,
              absl::negation<
                  internal_statusor::
                      IsConstructibleOrConvertibleOrAssignableFromStatusOr<
                          T, U>>>::value,
          int> = 0>
  StatusOr& operator=(StatusOr<U>&& other) {
    this->Assign(std::move(other));
    return *this;
  }











  template <
      typename U = absl::Status,
      absl::enable_if_t<
          absl::conjunction<
              std::is_convertible<U&&, absl::Status>,
              std::is_constructible<absl::Status, U&&>,
              absl::negation<std::is_same<absl::decay_t<U>, absl::StatusOr<T>>>,
              absl::negation<std::is_same<absl::decay_t<U>, T>>,
              absl::negation<std::is_same<absl::decay_t<U>, absl::in_place_t>>,
              absl::negation<internal_statusor::HasConversionOperatorToStatusOr<
                  T, U&&>>>::value,
          int> = 0>
  StatusOr(U&& v) : Base(std::forward<U>(v)) {}

  template <
      typename U = absl::Status,
      absl::enable_if_t<
          absl::conjunction<
              absl::negation<std::is_convertible<U&&, absl::Status>>,
              std::is_constructible<absl::Status, U&&>,
              absl::negation<std::is_same<absl::decay_t<U>, absl::StatusOr<T>>>,
              absl::negation<std::is_same<absl::decay_t<U>, T>>,
              absl::negation<std::is_same<absl::decay_t<U>, absl::in_place_t>>,
              absl::negation<internal_statusor::HasConversionOperatorToStatusOr<
                  T, U&&>>>::value,
          int> = 0>
  explicit StatusOr(U&& v) : Base(std::forward<U>(v)) {}

  template <
      typename U = absl::Status,
      absl::enable_if_t<
          absl::conjunction<
              std::is_convertible<U&&, absl::Status>,
              std::is_constructible<absl::Status, U&&>,
              absl::negation<std::is_same<absl::decay_t<U>, absl::StatusOr<T>>>,
              absl::negation<std::is_same<absl::decay_t<U>, T>>,
              absl::negation<std::is_same<absl::decay_t<U>, absl::in_place_t>>,
              absl::negation<internal_statusor::HasConversionOperatorToStatusOr<
                  T, U&&>>>::value,
          int> = 0>
  StatusOr& operator=(U&& v) {
    this->AssignStatus(std::forward<U>(v));
    return *this;
  }















  template <
      typename U = T,
      typename = typename std::enable_if<absl::conjunction<
          std::is_constructible<T, U&&>, std::is_assignable<T&, U&&>,
          absl::disjunction<
              std::is_same<absl::remove_cv_t<absl::remove_reference_t<U>>, T>,
              absl::conjunction<
                  absl::negation<std::is_convertible<U&&, absl::Status>>,
                  absl::negation<internal_statusor::
                                     HasConversionOperatorToStatusOr<T, U&&>>>>,
          internal_statusor::IsForwardingAssignmentValid<T, U&&>>::value>::type>
  StatusOr& operator=(U&& v) {
    this->Assign(std::forward<U>(v));
    return *this;
  }


  template <typename... Args>
  explicit StatusOr(absl::in_place_t, Args&&... args);
  template <typename U, typename... Args>
  explicit StatusOr(absl::in_place_t, std::initializer_list<U> ilist,
                    Args&&... args);







  template <
      typename U = T,
      absl::enable_if_t<
          absl::conjunction<
              internal_statusor::IsDirectInitializationValid<T, U&&>,
              std::is_constructible<T, U&&>, std::is_convertible<U&&, T>,
              absl::disjunction<
                  std::is_same<absl::remove_cv_t<absl::remove_reference_t<U>>,
                               T>,
                  absl::conjunction<
                      absl::negation<std::is_convertible<U&&, absl::Status>>,
                      absl::negation<
                          internal_statusor::HasConversionOperatorToStatusOr<
                              T, U&&>>>>>::value,
          int> = 0>
  StatusOr(U&& u)  // NOLINT
      : StatusOr(absl::in_place, std::forward<U>(u)) {}

  template <
      typename U = T,
      absl::enable_if_t<
          absl::conjunction<
              internal_statusor::IsDirectInitializationValid<T, U&&>,
              absl::disjunction<
                  std::is_same<absl::remove_cv_t<absl::remove_reference_t<U>>,
                               T>,
                  absl::conjunction<
                      absl::negation<std::is_constructible<absl::Status, U&&>>,
                      absl::negation<
                          internal_statusor::HasConversionOperatorToStatusOr<
                              T, U&&>>>>,
              std::is_constructible<T, U&&>,
              absl::negation<std::is_convertible<U&&, T>>>::value,
          int> = 0>
  explicit StatusOr(U&& u)  // NOLINT
      : StatusOr(absl::in_place, std::forward<U>(u)) {}














  ABSL_MUST_USE_RESULT bool ok() const { return this->status_.ok(); }





  const Status& status() const&;
  Status status() &&;


























  const T& value() const& ABSL_ATTRIBUTE_LIFETIME_BOUND;
  T& value() & ABSL_ATTRIBUTE_LIFETIME_BOUND;
  const T&& value() const&& ABSL_ATTRIBUTE_LIFETIME_BOUND;
  T&& value() && ABSL_ATTRIBUTE_LIFETIME_BOUND;










  const T& operator*() const& ABSL_ATTRIBUTE_LIFETIME_BOUND;
  T& operator*() & ABSL_ATTRIBUTE_LIFETIME_BOUND;
  const T&& operator*() const&& ABSL_ATTRIBUTE_LIFETIME_BOUND;
  T&& operator*() && ABSL_ATTRIBUTE_LIFETIME_BOUND;







  const T* operator->() const ABSL_ATTRIBUTE_LIFETIME_BOUND;
  T* operator->() ABSL_ATTRIBUTE_LIFETIME_BOUND;













  template <typename U>
  T value_or(U&& default_value) const&;
  template <typename U>
  T value_or(U&& default_value) &&;





  void IgnoreError() const;




  template <typename... Args>
  T& emplace(Args&&... args) {
    if (ok()) {
      this->Clear();
      this->MakeValue(std::forward<Args>(args)...);
    } else {
      this->MakeValue(std::forward<Args>(args)...);
      this->status_ = absl::OkStatus();
    }
    return this->data_;
  }

  template <
      typename U, typename... Args,
      absl::enable_if_t<
          std::is_constructible<T, std::initializer_list<U>&, Args&&...>::value,
          int> = 0>
  T& emplace(std::initializer_list<U> ilist, Args&&... args) {
    if (ok()) {
      this->Clear();
      this->MakeValue(ilist, std::forward<Args>(args)...);
    } else {
      this->MakeValue(ilist, std::forward<Args>(args)...);
      this->status_ = absl::OkStatus();
    }
    return this->data_;
  }

 private:
  using internal_statusor::StatusOrData<T>::Assign;
  template <typename U>
  void Assign(const absl::StatusOr<U>& other);
  template <typename U>
  void Assign(absl::StatusOr<U>&& other);
};

//
// This operator checks the equality of two `absl::StatusOr<T>` objects.
template <typename T>
bool operator==(const StatusOr<T>& lhs, const StatusOr<T>& rhs) {
  if (lhs.ok() && rhs.ok()) return *lhs == *rhs;
  return lhs.status() == rhs.status();
}

//
// This operator checks the inequality of two `absl::StatusOr<T>` objects.
template <typename T>
bool operator!=(const StatusOr<T>& lhs, const StatusOr<T>& rhs) {
  return !(lhs == rhs);
}

// Implementation details for StatusOr<T>
//------------------------------------------------------------------------------

template <typename T>
StatusOr<T>::StatusOr() : Base(Status(absl::StatusCode::kUnknown, "")) {}

template <typename T>
template <typename U>
inline void StatusOr<T>::Assign(const StatusOr<U>& other) {
  if (other.ok()) {
    this->Assign(*other);
  } else {
    this->AssignStatus(other.status());
  }
}

template <typename T>
template <typename U>
inline void StatusOr<T>::Assign(StatusOr<U>&& other) {
  if (other.ok()) {
    this->Assign(*std::move(other));
  } else {
    this->AssignStatus(std::move(other).status());
  }
}
template <typename T>
template <typename... Args>
StatusOr<T>::StatusOr(absl::in_place_t, Args&&... args)
    : Base(absl::in_place, std::forward<Args>(args)...) {}

template <typename T>
template <typename U, typename... Args>
StatusOr<T>::StatusOr(absl::in_place_t, std::initializer_list<U> ilist,
                      Args&&... args)
    : Base(absl::in_place, ilist, std::forward<Args>(args)...) {}

template <typename T>
const Status& StatusOr<T>::status() const& {
  return this->status_;
}
template <typename T>
Status StatusOr<T>::status() && {
  return ok() ? OkStatus() : std::move(this->status_);
}

template <typename T>
const T& StatusOr<T>::value() const& {
  if (!this->ok()) internal_statusor::ThrowBadStatusOrAccess(this->status_);
  return this->data_;
}

template <typename T>
T& StatusOr<T>::value() & {
  if (!this->ok()) internal_statusor::ThrowBadStatusOrAccess(this->status_);
  return this->data_;
}

template <typename T>
const T&& StatusOr<T>::value() const&& {
  if (!this->ok()) {
    internal_statusor::ThrowBadStatusOrAccess(std::move(this->status_));
  }
  return std::move(this->data_);
}

template <typename T>
T&& StatusOr<T>::value() && {
  if (!this->ok()) {
    internal_statusor::ThrowBadStatusOrAccess(std::move(this->status_));
  }
  return std::move(this->data_);
}

template <typename T>
const T& StatusOr<T>::operator*() const& {
  this->EnsureOk();
  return this->data_;
}

template <typename T>
T& StatusOr<T>::operator*() & {
  this->EnsureOk();
  return this->data_;
}

template <typename T>
const T&& StatusOr<T>::operator*() const&& {
  this->EnsureOk();
  return std::move(this->data_);
}

template <typename T>
T&& StatusOr<T>::operator*() && {
  this->EnsureOk();
  return std::move(this->data_);
}

template <typename T>
const T* StatusOr<T>::operator->() const {
  this->EnsureOk();
  return &this->data_;
}

template <typename T>
T* StatusOr<T>::operator->() {
  this->EnsureOk();
  return &this->data_;
}

template <typename T>
template <typename U>
T StatusOr<T>::value_or(U&& default_value) const& {
  if (ok()) {
    return this->data_;
  }
  return std::forward<U>(default_value);
}

template <typename T>
template <typename U>
T StatusOr<T>::value_or(U&& default_value) && {
  if (ok()) {
    return std::move(this->data_);
  }
  return std::forward<U>(default_value);
}

template <typename T>
void StatusOr<T>::IgnoreError() const {

}

ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STATUS_STATUSOR_H_
