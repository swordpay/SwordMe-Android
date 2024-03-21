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
//
// -----------------------------------------------------------------------------
// File: status.h
// -----------------------------------------------------------------------------
//
// This header file defines the Abseil `status` library, consisting of:
//
//   * An `absl::Status` class for holding error handling information
//   * A set of canonical `absl::StatusCode` error codes, and associated
//     utilities for generating and propagating status codes.
//   * A set of helper functions for creating status codes and checking their
//     values
//
// Within Google, `absl::Status` is the primary mechanism for communicating
// errors in C++, and is used to represent error state in both in-process
// library calls as well as RPC calls. Some of these errors may be recoverable,
// but others may not. Most functions that can produce a recoverable error
// should be designed to return an `absl::Status` (or `absl::StatusOr`).
//
// Example:
//
// absl::Status myFunction(absl::string_view fname, ...) {
//   ...
//   // encounter error
//   if (error condition) {
//     return absl::InvalidArgumentError("bad mode");
//   }
//   // else, return OK
//   return absl::OkStatus();
// }
//
// An `absl::Status` is designed to either return "OK" or one of a number of
// different error codes, corresponding to typical error conditions.
// In almost all cases, when using `absl::Status` you should use the canonical
// error codes (of type `absl::StatusCode`) enumerated in this header file.
// These canonical codes are understood across the codebase and will be
// accepted across all API and RPC boundaries.
#ifndef ABSL_STATUS_STATUS_H_
#define ABSL_STATUS_STATUS_H_

#include <ostream>
#include <string>
#include <utility>

#include "absl/functional/function_ref.h"
#include "absl/status/internal/status_internal.h"
#include "absl/strings/cord.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

namespace absl {
ABSL_NAMESPACE_BEGIN

//
// An `absl::StatusCode` is an enumerated type indicating either no error ("OK")
// or an error condition. In most cases, an `absl::Status` indicates a
// recoverable error, and the purpose of signalling an error is to indicate what
// action to take in response to that error. These error codes map to the proto
// RPC error codes indicated in https://cloud.google.com/apis/design/errors.
//
// The errors listed below are the canonical errors associated with
// `absl::Status` and are used throughout the codebase. As a result, these
// error codes are somewhat generic.
//
// In general, try to return the most specific error that applies if more than
// one error may pertain. For example, prefer `kOutOfRange` over
// `kFailedPrecondition` if both codes apply. Similarly prefer `kNotFound` or
// `kAlreadyExists` over `kFailedPrecondition`.
//
// Because these errors may cross RPC boundaries, these codes are tied to the
// `google.rpc.Code` definitions within
// https://github.com/googleapis/googleapis/blob/master/google/rpc/code.proto
// The string value of these RPC codes is denoted within each enum below.
//
// If your error handling code requires more context, you can attach payloads
// to your status. See `absl::Status::SetPayload()` and
// `absl::Status::GetPayload()` below.
enum class StatusCode : int {






  kOk = 0,




  kCancelled = 1,






  kUnknown = 2,








  kInvalidArgument = 3,







  kDeadlineExceeded = 4,









  kNotFound = 5,





  kAlreadyExists = 6,












  kPermissionDenied = 7,





  kResourceExhausted = 8,



















  kFailedPrecondition = 9,








  kAborted = 10,


















  kOutOfRange = 11,





  kUnimplemented = 12,





  kInternal = 13,









  kUnavailable = 14,





  kDataLoss = 15,





  kUnauthenticated = 16,









  kDoNotUseReservedForFutureExpansionUseDefaultInSwitchInstead_ = 20
};

//
// Returns the name for the status code, or "" if it is an unknown value.
std::string StatusCodeToString(StatusCode code);

//
// Streams StatusCodeToString(code) to `os`.
std::ostream& operator<<(std::ostream& os, StatusCode code);

//
// An `absl::StatusToStringMode` is an enumerated type indicating how
// `absl::Status::ToString()` should construct the output string for a non-ok
// status.
enum class StatusToStringMode : int {


  kWithNoExtraData = 0,

  kWithPayload = 1 << 0,

  kWithEverything = ~kWithNoExtraData,

  kDefault = kWithPayload,
};

// following operations must be provided:
inline constexpr StatusToStringMode operator&(StatusToStringMode lhs,
                                              StatusToStringMode rhs) {
  return static_cast<StatusToStringMode>(static_cast<int>(lhs) &
                                         static_cast<int>(rhs));
}
inline constexpr StatusToStringMode operator|(StatusToStringMode lhs,
                                              StatusToStringMode rhs) {
  return static_cast<StatusToStringMode>(static_cast<int>(lhs) |
                                         static_cast<int>(rhs));
}
inline constexpr StatusToStringMode operator^(StatusToStringMode lhs,
                                              StatusToStringMode rhs) {
  return static_cast<StatusToStringMode>(static_cast<int>(lhs) ^
                                         static_cast<int>(rhs));
}
inline constexpr StatusToStringMode operator~(StatusToStringMode arg) {
  return static_cast<StatusToStringMode>(~static_cast<int>(arg));
}
inline StatusToStringMode& operator&=(StatusToStringMode& lhs,
                                      StatusToStringMode rhs) {
  lhs = lhs & rhs;
  return lhs;
}
inline StatusToStringMode& operator|=(StatusToStringMode& lhs,
                                      StatusToStringMode rhs) {
  lhs = lhs | rhs;
  return lhs;
}
inline StatusToStringMode& operator^=(StatusToStringMode& lhs,
                                      StatusToStringMode rhs) {
  lhs = lhs ^ rhs;
  return lhs;
}

//
// The `absl::Status` class is generally used to gracefully handle errors
// across API boundaries (and in particular across RPC boundaries). Some of
// these errors may be recoverable, but others may not. Most
// functions which can produce a recoverable error should be designed to return
// either an `absl::Status` (or the similar `absl::StatusOr<T>`, which holds
// either an object of type `T` or an error).
//
// API developers should construct their functions to return `absl::OkStatus()`
// upon success, or an `absl::StatusCode` upon another type of error (e.g
// an `absl::StatusCode::kInvalidArgument` error). The API provides convenience
// functions to construct each status code.
//
// Example:
//
// absl::Status myFunction(absl::string_view fname, ...) {
//   ...
//   // encounter error
//   if (error condition) {
//     // Construct an absl::StatusCode::kInvalidArgument error
//     return absl::InvalidArgumentError("bad mode");
//   }
//   // else, return OK
//   return absl::OkStatus();
// }
//
// Users handling status error codes should prefer checking for an OK status
// using the `ok()` member function. Handling multiple error codes may justify
// use of switch statement, but only check for error codes you know how to
// handle; do not try to exhaustively match against all canonical error codes.
// Errors that cannot be handled should be logged and/or propagated for higher
// levels to deal with. If you do use a switch statement, make sure that you
// also provide a `default:` switch case, so that code does not break as other
// canonical codes are added to the API.
//
// Example:
//
//   absl::Status result = DoSomething();
//   if (!result.ok()) {
//     LOG(ERROR) << result;
//   }
//
//   // Provide a default if switching on multiple error codes
//   switch (result.code()) {
//     // The user hasn't authenticated. Ask them to reauth
//     case absl::StatusCode::kUnauthenticated:
//       DoReAuth();
//       break;
//     // The user does not have permission. Log an error.
//     case absl::StatusCode::kPermissionDenied:
//       LOG(ERROR) << result;
//       break;
//     // Propagate the error otherwise.
//     default:
//       return true;
//   }
//
// An `absl::Status` can optionally include a payload with more information
// about the error. Typically, this payload serves one of several purposes:
//
//   * It may provide more fine-grained semantic information about the error to
//     facilitate actionable remedies.
//   * It may provide human-readable contexual information that is more
//     appropriate to display to an end user.
//
// Example:
//
//   absl::Status result = DoSomething();
//   // Inform user to retry after 30 seconds
//   // See more error details in googleapis/google/rpc/error_details.proto
//   if (absl::IsResourceExhausted(result)) {
//     google::rpc::RetryInfo info;
//     info.retry_delay().seconds() = 30;
//     // Payloads require a unique key (a URL to ensure no collisions with
//     // other payloads), and an `absl::Cord` to hold the encoded data.
//     absl::string_view url = "type.googleapis.com/google.rpc.RetryInfo";
//     result.SetPayload(url, info.SerializeAsCord());
//     return result;
//   }
//
// For documentation see https://abseil.io/docs/cpp/guides/status.
//
// Returned Status objects may not be ignored. status_internal.h has a forward
// declaration of the form
// class ABSL_MUST_USE_RESULT Status;
class Status final {
 public:




  Status();






  Status(absl::StatusCode code, absl::string_view msg);

  Status(const Status&);
  Status& operator=(const Status& x);


  Status(Status&&) noexcept;
  Status& operator=(Status&&);

  ~Status();















  void Update(const Status& new_status);
  void Update(Status&& new_status);





  ABSL_MUST_USE_RESULT bool ok() const;



  absl::StatusCode code() const;










  int raw_code() const;






  absl::string_view message() const;

  friend bool operator==(const Status&, const Status&);
  friend bool operator!=(const Status&, const Status&);











  std::string ToString(
      StatusToStringMode mode = StatusToStringMode::kDefault) const;





  void IgnoreError() const;



  friend void swap(Status& a, Status& b);



































  absl::optional<absl::Cord> GetPayload(absl::string_view type_url) const;






  void SetPayload(absl::string_view type_url, absl::Cord payload);




  bool ErasePayload(absl::string_view type_url);










  void ForEachPayload(
      absl::FunctionRef<void(absl::string_view, const absl::Cord&)> visitor)
      const;

 private:
  friend Status CancelledError();


  explicit Status(absl::StatusCode code);

  static void UnrefNonInlined(uintptr_t rep);
  static void Ref(uintptr_t rep);
  static void Unref(uintptr_t rep);


  void PrepareToModify();

  const status_internal::Payloads* GetPayloads() const;
  status_internal::Payloads* GetPayloads();

  static bool EqualsSlow(const absl::Status& a, const absl::Status& b);

  static constexpr const char kMovedFromString[] =
      "Status accessed after move.";

  static const std::string* EmptyString();
  static const std::string* MovedFromString();


  static bool IsInlined(uintptr_t rep);


  static bool IsMovedFrom(uintptr_t rep);
  static uintptr_t MovedFromRep();


  static uintptr_t CodeToInlinedRep(absl::StatusCode code);
  static absl::StatusCode InlinedRepToCode(uintptr_t rep);


  static uintptr_t PointerToRep(status_internal::StatusRep* r);
  static status_internal::StatusRep* RepToPointer(uintptr_t r);

  std::string ToStringSlow(StatusToStringMode mode) const;








  uintptr_t rep_;
};

//
// Returns an OK status, equivalent to a default constructed instance. Prefer
// usage of `absl::OkStatus()` when constructing such an OK status.
Status OkStatus();

//
// Prints a human-readable representation of `x` to `os`.
std::ostream& operator<<(std::ostream& os, const Status& x);

// IsAlreadyExists()
// IsCancelled()
// IsDataLoss()
// IsDeadlineExceeded()
// IsFailedPrecondition()
// IsInternal()
// IsInvalidArgument()
// IsNotFound()
// IsOutOfRange()
// IsPermissionDenied()
// IsResourceExhausted()
// IsUnauthenticated()
// IsUnavailable()
// IsUnimplemented()
// IsUnknown()
//
// These convenience functions return `true` if a given status matches the
// `absl::StatusCode` error code of its associated function.
ABSL_MUST_USE_RESULT bool IsAborted(const Status& status);
ABSL_MUST_USE_RESULT bool IsAlreadyExists(const Status& status);
ABSL_MUST_USE_RESULT bool IsCancelled(const Status& status);
ABSL_MUST_USE_RESULT bool IsDataLoss(const Status& status);
ABSL_MUST_USE_RESULT bool IsDeadlineExceeded(const Status& status);
ABSL_MUST_USE_RESULT bool IsFailedPrecondition(const Status& status);
ABSL_MUST_USE_RESULT bool IsInternal(const Status& status);
ABSL_MUST_USE_RESULT bool IsInvalidArgument(const Status& status);
ABSL_MUST_USE_RESULT bool IsNotFound(const Status& status);
ABSL_MUST_USE_RESULT bool IsOutOfRange(const Status& status);
ABSL_MUST_USE_RESULT bool IsPermissionDenied(const Status& status);
ABSL_MUST_USE_RESULT bool IsResourceExhausted(const Status& status);
ABSL_MUST_USE_RESULT bool IsUnauthenticated(const Status& status);
ABSL_MUST_USE_RESULT bool IsUnavailable(const Status& status);
ABSL_MUST_USE_RESULT bool IsUnimplemented(const Status& status);
ABSL_MUST_USE_RESULT bool IsUnknown(const Status& status);

// AlreadyExistsError()
// CancelledError()
// DataLossError()
// DeadlineExceededError()
// FailedPreconditionError()
// InternalError()
// InvalidArgumentError()
// NotFoundError()
// OutOfRangeError()
// PermissionDeniedError()
// ResourceExhaustedError()
// UnauthenticatedError()
// UnavailableError()
// UnimplementedError()
// UnknownError()
//
// These convenience functions create an `absl::Status` object with an error
// code as indicated by the associated function name, using the error message
// passed in `message`.
Status AbortedError(absl::string_view message);
Status AlreadyExistsError(absl::string_view message);
Status CancelledError(absl::string_view message);
Status DataLossError(absl::string_view message);
Status DeadlineExceededError(absl::string_view message);
Status FailedPreconditionError(absl::string_view message);
Status InternalError(absl::string_view message);
Status InvalidArgumentError(absl::string_view message);
Status NotFoundError(absl::string_view message);
Status OutOfRangeError(absl::string_view message);
Status PermissionDeniedError(absl::string_view message);
Status ResourceExhaustedError(absl::string_view message);
Status UnauthenticatedError(absl::string_view message);
Status UnavailableError(absl::string_view message);
Status UnimplementedError(absl::string_view message);
Status UnknownError(absl::string_view message);

//
// Returns the StatusCode for `error_number`, which should be an `errno` value.
// See https://en.cppreference.com/w/cpp/error/errno_macros and similar
// references.
absl::StatusCode ErrnoToStatusCode(int error_number);

//
// Convenience function that creates a `absl::Status` using an `error_number`,
// which should be an `errno` value.
Status ErrnoToStatus(int error_number, absl::string_view message);

// Implementation details follow
//------------------------------------------------------------------------------

inline Status::Status() : rep_(CodeToInlinedRep(absl::StatusCode::kOk)) {}

inline Status::Status(absl::StatusCode code) : rep_(CodeToInlinedRep(code)) {}

inline Status::Status(const Status& x) : rep_(x.rep_) { Ref(rep_); }

inline Status& Status::operator=(const Status& x) {
  uintptr_t old_rep = rep_;
  if (x.rep_ != old_rep) {
    Ref(x.rep_);
    rep_ = x.rep_;
    Unref(old_rep);
  }
  return *this;
}

inline Status::Status(Status&& x) noexcept : rep_(x.rep_) {
  x.rep_ = MovedFromRep();
}

inline Status& Status::operator=(Status&& x) {
  uintptr_t old_rep = rep_;
  if (x.rep_ != old_rep) {
    rep_ = x.rep_;
    x.rep_ = MovedFromRep();
    Unref(old_rep);
  }
  return *this;
}

inline void Status::Update(const Status& new_status) {
  if (ok()) {
    *this = new_status;
  }
}

inline void Status::Update(Status&& new_status) {
  if (ok()) {
    *this = std::move(new_status);
  }
}

inline Status::~Status() { Unref(rep_); }

inline bool Status::ok() const {
  return rep_ == CodeToInlinedRep(absl::StatusCode::kOk);
}

inline absl::string_view Status::message() const {
  return !IsInlined(rep_)
             ? RepToPointer(rep_)->message
             : (IsMovedFrom(rep_) ? absl::string_view(kMovedFromString)
                                  : absl::string_view());
}

inline bool operator==(const Status& lhs, const Status& rhs) {
  return lhs.rep_ == rhs.rep_ || Status::EqualsSlow(lhs, rhs);
}

inline bool operator!=(const Status& lhs, const Status& rhs) {
  return !(lhs == rhs);
}

inline std::string Status::ToString(StatusToStringMode mode) const {
  return ok() ? "OK" : ToStringSlow(mode);
}

inline void Status::IgnoreError() const {

}

inline void swap(absl::Status& a, absl::Status& b) {
  using std::swap;
  swap(a.rep_, b.rep_);
}

inline const status_internal::Payloads* Status::GetPayloads() const {
  return IsInlined(rep_) ? nullptr : RepToPointer(rep_)->payloads.get();
}

inline status_internal::Payloads* Status::GetPayloads() {
  return IsInlined(rep_) ? nullptr : RepToPointer(rep_)->payloads.get();
}

inline bool Status::IsInlined(uintptr_t rep) { return (rep & 1) == 0; }

inline bool Status::IsMovedFrom(uintptr_t rep) {
  return IsInlined(rep) && (rep & 2) != 0;
}

inline uintptr_t Status::MovedFromRep() {
  return CodeToInlinedRep(absl::StatusCode::kInternal) | 2;
}

inline uintptr_t Status::CodeToInlinedRep(absl::StatusCode code) {
  return static_cast<uintptr_t>(code) << 2;
}

inline absl::StatusCode Status::InlinedRepToCode(uintptr_t rep) {
  assert(IsInlined(rep));
  return static_cast<absl::StatusCode>(rep >> 2);
}

inline status_internal::StatusRep* Status::RepToPointer(uintptr_t rep) {
  assert(!IsInlined(rep));
  return reinterpret_cast<status_internal::StatusRep*>(rep - 1);
}

inline uintptr_t Status::PointerToRep(status_internal::StatusRep* rep) {
  return reinterpret_cast<uintptr_t>(rep) + 1;
}

inline void Status::Ref(uintptr_t rep) {
  if (!IsInlined(rep)) {
    RepToPointer(rep)->ref.fetch_add(1, std::memory_order_relaxed);
  }
}

inline void Status::Unref(uintptr_t rep) {
  if (!IsInlined(rep)) {
    UnrefNonInlined(rep);
  }
}

inline Status OkStatus() { return Status(); }

// and an empty message. It is provided only for efficiency, given that
// message-less kCancelled errors are common in the infrastructure.
inline Status CancelledError() { return Status(absl::StatusCode::kCancelled); }

ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STATUS_STATUS_H_
