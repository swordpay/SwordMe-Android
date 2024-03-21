/*
 *  Copyright 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_RTC_ERROR_H_
#define API_RTC_ERROR_H_

#ifdef WEBRTC_UNIT_TEST
#include <ostream>
#endif  // WEBRTC_UNIT_TEST
#include <string>
#include <utility>  // For std::move.

#include "absl/types/optional.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/system/rtc_export.h"

namespace webrtc {

// may wish to act upon differently. These roughly map to DOMExceptions or
// RTCError "errorDetailEnum" values in the web API, as described in the
// comments below.
enum class RTCErrorType {

  NONE,


  UNSUPPORTED_OPERATION,


  UNSUPPORTED_PARAMETER,


  INVALID_PARAMETER,



  INVALID_RANGE,



  SYNTAX_ERROR,


  INVALID_STATE,


  INVALID_MODIFICATION,


  NETWORK_ERROR,



  RESOURCE_EXHAUSTED,


  INTERNAL_ERROR,




  OPERATION_ERROR_WITH_DATA,
};

// https://w3c.github.io/webrtc-pc/#rtcerrordetailtype-enum
enum class RTCErrorDetailType {
  NONE,
  DATA_CHANNEL_FAILURE,
  DTLS_FAILURE,
  FINGERPRINT_FAILURE,
  SCTP_FAILURE,
  SDP_SYNTAX_ERROR,
  HARDWARE_ENCODER_NOT_AVAILABLE,
  HARDWARE_ENCODER_ERROR,
};

// message, and possibly additional information specific to that error.
//
// Doesn't contain anything beyond a type and message now, but will in the
// future as more errors are implemented.
class RTC_EXPORT RTCError {
 public:


  RTCError() {}
  explicit RTCError(RTCErrorType type) : type_(type) {}

  RTCError(RTCErrorType type, std::string message)
      : type_(type), message_(std::move(message)) {}



  RTCError(const RTCError& other) = default;
  RTCError(RTCError&&) = default;
  RTCError& operator=(const RTCError& other) = default;
  RTCError& operator=(RTCError&&) = default;



  static RTCError OK();

  RTCErrorType type() const { return type_; }
  void set_type(RTCErrorType type) { type_ = type; }



  const char* message() const;

  void set_message(std::string message);

  RTCErrorDetailType error_detail() const { return error_detail_; }
  void set_error_detail(RTCErrorDetailType detail) { error_detail_ = detail; }
  absl::optional<uint16_t> sctp_cause_code() const { return sctp_cause_code_; }
  void set_sctp_cause_code(uint16_t cause_code) {
    sctp_cause_code_ = cause_code;
  }


  bool ok() const { return type_ == RTCErrorType::NONE; }

 private:
  RTCErrorType type_ = RTCErrorType::NONE;
  std::string message_;
  RTCErrorDetailType error_detail_ = RTCErrorDetailType::NONE;
  absl::optional<uint16_t> sctp_cause_code_;
};

// error type.
//
// Only intended to be used for logging/diagnostics. The returned char* points
// to literal string that lives for the whole duration of the program.
RTC_EXPORT const char* ToString(RTCErrorType error);
RTC_EXPORT const char* ToString(RTCErrorDetailType error);

#ifdef WEBRTC_UNIT_TEST
inline std::ostream& operator<<(  // no-presubmit-check TODO(webrtc:8982)
    std::ostream& stream,         // no-presubmit-check TODO(webrtc:8982)
    RTCErrorType error) {
  return stream << ToString(error);
}

inline std::ostream& operator<<(  // no-presubmit-check TODO(webrtc:8982)
    std::ostream& stream,         // no-presubmit-check TODO(webrtc:8982)
    RTCErrorDetailType error) {
  return stream << ToString(error);
}
#endif  // WEBRTC_UNIT_TEST

// message and log it. `message` should be a string literal or movable
// std::string.
#define LOG_AND_RETURN_ERROR_EX(type, message, severity)                     \
  {                                                                          \
    RTC_DCHECK(type != RTCErrorType::NONE);                                  \
    RTC_LOG(severity) << message << " (" << ::webrtc::ToString(type) << ")"; \
    return ::webrtc::RTCError(type, message);                                \
  }

#define LOG_AND_RETURN_ERROR(type, message) \
  LOG_AND_RETURN_ERROR_EX(type, message, LS_ERROR)

// models the concept of an object that is either a usable value, or an error
// Status explaining why such a value is not present. To this end RTCErrorOr<T>
// does not allow its RTCErrorType value to be RTCErrorType::NONE. This is
// enforced by a debug check in most cases.
//
// The primary use-case for RTCErrorOr<T> is as the return value of a function
// which may fail. For example, CreateRtpSender will fail if the parameters
// could not be successfully applied at the media engine level, but if
// successful will return a unique_ptr to an RtpSender.
//
// Example client usage for a RTCErrorOr<std::unique_ptr<T>>:
//
//  RTCErrorOr<std::unique_ptr<Foo>> result = FooFactory::MakeNewFoo(arg);
//  if (result.ok()) {
//    std::unique_ptr<Foo> foo = result.ConsumeValue();
//    foo->DoSomethingCool();
//  } else {
//    RTC_LOG(LS_ERROR) << result.error();
//  }
//
// Example factory implementation returning RTCErrorOr<std::unique_ptr<T>>:
//
//  RTCErrorOr<std::unique_ptr<Foo>> FooFactory::MakeNewFoo(int arg) {
//    if (arg <= 0) {
//      return RTCError(RTCErrorType::INVALID_RANGE, "Arg must be positive");
//    } else {
//      return std::unique_ptr<Foo>(new Foo(arg));
//    }
//  }
//
template <typename T>
class RTCErrorOr {


  template <typename U>
  friend class RTCErrorOr;

 public:
  typedef T element_type;




  RTCErrorOr() : error_(RTCErrorType::INTERNAL_ERROR) {}








  RTCErrorOr(RTCError&& error) : error_(std::move(error)) {  // NOLINT
    RTC_DCHECK(!error_.ok());
  }







  RTCErrorOr(const T& value) : value_(value) {}        // NOLINT
  RTCErrorOr(T&& value) : value_(std::move(value)) {}  // NOLINT



  RTCErrorOr(const RTCErrorOr& other) = delete;
  RTCErrorOr& operator=(const RTCErrorOr& other) = delete;





  RTCErrorOr(RTCErrorOr&& other)
      : error_(std::move(other.error_)), value_(std::move(other.value_)) {}
  RTCErrorOr& operator=(RTCErrorOr&& other) {
    error_ = std::move(other.error_);
    value_ = std::move(other.value_);
    return *this;
  }


  template <typename U>
  RTCErrorOr(RTCErrorOr<U> other)  // NOLINT
      : error_(std::move(other.error_)), value_(std::move(other.value_)) {}
  template <typename U>
  RTCErrorOr& operator=(RTCErrorOr<U> other) {
    error_ = std::move(other.error_);
    value_ = std::move(other.value_);
    return *this;
  }


  const RTCError& error() const { return error_; }



  RTCError MoveError() { return std::move(error_); }

  bool ok() const { return error_.ok(); }





  const T& value() const {
    RTC_DCHECK(ok());
    return value_;
  }
  T& value() {
    RTC_DCHECK(ok());
    return value_;
  }


  T MoveValue() {
    RTC_DCHECK(ok());
    return std::move(value_);
  }

 private:
  RTCError error_;
  T value_;
};

}  // namespace webrtc

#endif  // API_RTC_ERROR_H_
