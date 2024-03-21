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
#ifndef ABSL_STATUS_INTERNAL_STATUSOR_INTERNAL_H_
#define ABSL_STATUS_INTERNAL_STATUSOR_INTERNAL_H_

#include <type_traits>
#include <utility>

#include "absl/base/attributes.h"
#include "absl/meta/type_traits.h"
#include "absl/status/status.h"
#include "absl/utility/utility.h"

namespace absl {
ABSL_NAMESPACE_BEGIN

template <typename T>
class ABSL_MUST_USE_RESULT StatusOr;

namespace internal_statusor {

// StatusOr<T>()`.
template <typename T, typename U, typename = void>
struct HasConversionOperatorToStatusOr : std::false_type {};

template <typename T, typename U>
void test(char (*)[sizeof(std::declval<U>().operator absl::StatusOr<T>())]);

template <typename T, typename U>
struct HasConversionOperatorToStatusOr<T, U, decltype(test<T, U>(0))>
    : std::true_type {};

template <typename T, typename U>
using IsConstructibleOrConvertibleFromStatusOr =
    absl::disjunction<std::is_constructible<T, StatusOr<U>&>,
                      std::is_constructible<T, const StatusOr<U>&>,
                      std::is_constructible<T, StatusOr<U>&&>,
                      std::is_constructible<T, const StatusOr<U>&&>,
                      std::is_convertible<StatusOr<U>&, T>,
                      std::is_convertible<const StatusOr<U>&, T>,
                      std::is_convertible<StatusOr<U>&&, T>,
                      std::is_convertible<const StatusOr<U>&&, T>>;

// `StatusOr<U>`.
template <typename T, typename U>
using IsConstructibleOrConvertibleOrAssignableFromStatusOr =
    absl::disjunction<IsConstructibleOrConvertibleFromStatusOr<T, U>,
                      std::is_assignable<T&, StatusOr<U>&>,
                      std::is_assignable<T&, const StatusOr<U>&>,
                      std::is_assignable<T&, StatusOr<U>&&>,
                      std::is_assignable<T&, const StatusOr<U>&&>>;

// when `U` is `StatusOr<V>` and `T` is constructible or convertible from `V`.
template <typename T, typename U>
struct IsDirectInitializationAmbiguous
    : public absl::conditional_t<
          std::is_same<absl::remove_cv_t<absl::remove_reference_t<U>>,
                       U>::value,
          std::false_type,
          IsDirectInitializationAmbiguous<
              T, absl::remove_cv_t<absl::remove_reference_t<U>>>> {};

template <typename T, typename V>
struct IsDirectInitializationAmbiguous<T, absl::StatusOr<V>>
    : public IsConstructibleOrConvertibleFromStatusOr<T, V> {};

// `StatusOr<T>::StatusOr(U&&)` should participate in overload resolution.
template <typename T, typename U>
using IsDirectInitializationValid = absl::disjunction<

    std::is_same<T, absl::remove_cv_t<absl::remove_reference_t<U>>>,
    absl::negation<absl::disjunction<
        std::is_same<absl::StatusOr<T>,
                     absl::remove_cv_t<absl::remove_reference_t<U>>>,
        std::is_same<absl::Status,
                     absl::remove_cv_t<absl::remove_reference_t<U>>>,
        std::is_same<absl::in_place_t,
                     absl::remove_cv_t<absl::remove_reference_t<U>>>,
        IsDirectInitializationAmbiguous<T, U>>>>;

// is equivalent to whether all the following conditions are met:
// 1. `U` is `StatusOr<V>`.
// 2. `T` is constructible and assignable from `V`.
// 3. `T` is constructible and assignable from `U` (i.e. `StatusOr<V>`).
// For example, the following code is considered ambiguous:
// (`T` is `bool`, `U` is `StatusOr<bool>`, `V` is `bool`)
//   StatusOr<bool> s1 = true;  // s1.ok() && s1.ValueOrDie() == true
//   StatusOr<bool> s2 = false;  // s2.ok() && s2.ValueOrDie() == false
//   s1 = s2;  // ambiguous, `s1 = s2.ValueOrDie()` or `s1 = bool(s2)`?
template <typename T, typename U>
struct IsForwardingAssignmentAmbiguous
    : public absl::conditional_t<
          std::is_same<absl::remove_cv_t<absl::remove_reference_t<U>>,
                       U>::value,
          std::false_type,
          IsForwardingAssignmentAmbiguous<
              T, absl::remove_cv_t<absl::remove_reference_t<U>>>> {};

template <typename T, typename U>
struct IsForwardingAssignmentAmbiguous<T, absl::StatusOr<U>>
    : public IsConstructibleOrConvertibleOrAssignableFromStatusOr<T, U> {};

// `StatusOr<T>::operator(U&&)` should participate in overload resolution.
template <typename T, typename U>
using IsForwardingAssignmentValid = absl::disjunction<

    std::is_same<T, absl::remove_cv_t<absl::remove_reference_t<U>>>,
    absl::negation<absl::disjunction<
        std::is_same<absl::StatusOr<T>,
                     absl::remove_cv_t<absl::remove_reference_t<U>>>,
        std::is_same<absl::Status,
                     absl::remove_cv_t<absl::remove_reference_t<U>>>,
        std::is_same<absl::in_place_t,
                     absl::remove_cv_t<absl::remove_reference_t<U>>>,
        IsForwardingAssignmentAmbiguous<T, U>>>>;

class Helper {
 public:

  static void HandleInvalidStatusCtorArg(Status*);
  ABSL_ATTRIBUTE_NORETURN static void Crash(const absl::Status& status);
};

// the constructor.
// This abstraction is here mostly for the gcc performance fix.
template <typename T, typename... Args>
ABSL_ATTRIBUTE_NONNULL(1) void PlacementNew(void* p, Args&&... args) {
  new (p) T(std::forward<Args>(args)...);
}

// We move all this to a base class to allow mixing with the appropriate
// TraitsBase specialization.
template <typename T>
class StatusOrData {
  template <typename U>
  friend class StatusOrData;

 public:
  StatusOrData() = delete;

  StatusOrData(const StatusOrData& other) {
    if (other.ok()) {
      MakeValue(other.data_);
      MakeStatus();
    } else {
      MakeStatus(other.status_);
    }
  }

  StatusOrData(StatusOrData&& other) noexcept {
    if (other.ok()) {
      MakeValue(std::move(other.data_));
      MakeStatus();
    } else {
      MakeStatus(std::move(other.status_));
    }
  }

  template <typename U>
  explicit StatusOrData(const StatusOrData<U>& other) {
    if (other.ok()) {
      MakeValue(other.data_);
      MakeStatus();
    } else {
      MakeStatus(other.status_);
    }
  }

  template <typename U>
  explicit StatusOrData(StatusOrData<U>&& other) {
    if (other.ok()) {
      MakeValue(std::move(other.data_));
      MakeStatus();
    } else {
      MakeStatus(std::move(other.status_));
    }
  }

  template <typename... Args>
  explicit StatusOrData(absl::in_place_t, Args&&... args)
      : data_(std::forward<Args>(args)...) {
    MakeStatus();
  }

  explicit StatusOrData(const T& value) : data_(value) {
    MakeStatus();
  }
  explicit StatusOrData(T&& value) : data_(std::move(value)) {
    MakeStatus();
  }

  template <typename U,
            absl::enable_if_t<std::is_constructible<absl::Status, U&&>::value,
                              int> = 0>
  explicit StatusOrData(U&& v) : status_(std::forward<U>(v)) {
    EnsureNotOk();
  }

  StatusOrData& operator=(const StatusOrData& other) {
    if (this == &other) return *this;
    if (other.ok())
      Assign(other.data_);
    else
      AssignStatus(other.status_);
    return *this;
  }

  StatusOrData& operator=(StatusOrData&& other) {
    if (this == &other) return *this;
    if (other.ok())
      Assign(std::move(other.data_));
    else
      AssignStatus(std::move(other.status_));
    return *this;
  }

  ~StatusOrData() {
    if (ok()) {
      status_.~Status();
      data_.~T();
    } else {
      status_.~Status();
    }
  }

  template <typename U>
  void Assign(U&& value) {
    if (ok()) {
      data_ = std::forward<U>(value);
    } else {
      MakeValue(std::forward<U>(value));
      status_ = OkStatus();
    }
  }

  template <typename U>
  void AssignStatus(U&& v) {
    Clear();
    status_ = static_cast<absl::Status>(std::forward<U>(v));
    EnsureNotOk();
  }

  bool ok() const { return status_.ok(); }

 protected:





  union {
    Status status_;
  };

  struct Dummy {};
  union {


    Dummy dummy_;
    T data_;
  };

  void Clear() {
    if (ok()) data_.~T();
  }

  void EnsureOk() const {
    if (ABSL_PREDICT_FALSE(!ok())) Helper::Crash(status_);
  }

  void EnsureNotOk() {
    if (ABSL_PREDICT_FALSE(ok())) Helper::HandleInvalidStatusCtorArg(&status_);
  }


  template <typename... Arg>
  void MakeValue(Arg&&... arg) {
    internal_statusor::PlacementNew<T>(&dummy_, std::forward<Arg>(arg)...);
  }


  template <typename... Args>
  void MakeStatus(Args&&... args) {
    internal_statusor::PlacementNew<Status>(&status_,
                                            std::forward<Args>(args)...);
  }
};

// operators in `StatusOr`. For example, `CopyCtorBase` will explicitly delete
// the copy constructor when T is not copy constructible and `StatusOr` will
// inherit that behavior implicitly.
template <typename T, bool = std::is_copy_constructible<T>::value>
struct CopyCtorBase {
  CopyCtorBase() = default;
  CopyCtorBase(const CopyCtorBase&) = default;
  CopyCtorBase(CopyCtorBase&&) = default;
  CopyCtorBase& operator=(const CopyCtorBase&) = default;
  CopyCtorBase& operator=(CopyCtorBase&&) = default;
};

template <typename T>
struct CopyCtorBase<T, false> {
  CopyCtorBase() = default;
  CopyCtorBase(const CopyCtorBase&) = delete;
  CopyCtorBase(CopyCtorBase&&) = default;
  CopyCtorBase& operator=(const CopyCtorBase&) = default;
  CopyCtorBase& operator=(CopyCtorBase&&) = default;
};

template <typename T, bool = std::is_move_constructible<T>::value>
struct MoveCtorBase {
  MoveCtorBase() = default;
  MoveCtorBase(const MoveCtorBase&) = default;
  MoveCtorBase(MoveCtorBase&&) = default;
  MoveCtorBase& operator=(const MoveCtorBase&) = default;
  MoveCtorBase& operator=(MoveCtorBase&&) = default;
};

template <typename T>
struct MoveCtorBase<T, false> {
  MoveCtorBase() = default;
  MoveCtorBase(const MoveCtorBase&) = default;
  MoveCtorBase(MoveCtorBase&&) = delete;
  MoveCtorBase& operator=(const MoveCtorBase&) = default;
  MoveCtorBase& operator=(MoveCtorBase&&) = default;
};

template <typename T, bool = std::is_copy_constructible<T>::value&&
                          std::is_copy_assignable<T>::value>
struct CopyAssignBase {
  CopyAssignBase() = default;
  CopyAssignBase(const CopyAssignBase&) = default;
  CopyAssignBase(CopyAssignBase&&) = default;
  CopyAssignBase& operator=(const CopyAssignBase&) = default;
  CopyAssignBase& operator=(CopyAssignBase&&) = default;
};

template <typename T>
struct CopyAssignBase<T, false> {
  CopyAssignBase() = default;
  CopyAssignBase(const CopyAssignBase&) = default;
  CopyAssignBase(CopyAssignBase&&) = default;
  CopyAssignBase& operator=(const CopyAssignBase&) = delete;
  CopyAssignBase& operator=(CopyAssignBase&&) = default;
};

template <typename T, bool = std::is_move_constructible<T>::value&&
                          std::is_move_assignable<T>::value>
struct MoveAssignBase {
  MoveAssignBase() = default;
  MoveAssignBase(const MoveAssignBase&) = default;
  MoveAssignBase(MoveAssignBase&&) = default;
  MoveAssignBase& operator=(const MoveAssignBase&) = default;
  MoveAssignBase& operator=(MoveAssignBase&&) = default;
};

template <typename T>
struct MoveAssignBase<T, false> {
  MoveAssignBase() = default;
  MoveAssignBase(const MoveAssignBase&) = default;
  MoveAssignBase(MoveAssignBase&&) = default;
  MoveAssignBase& operator=(const MoveAssignBase&) = default;
  MoveAssignBase& operator=(MoveAssignBase&&) = delete;
};

ABSL_ATTRIBUTE_NORETURN void ThrowBadStatusOrAccess(absl::Status status);

}  // namespace internal_statusor
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STATUS_INTERNAL_STATUSOR_INTERNAL_H_
