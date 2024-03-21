// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// RFC-3629. In particular, it does not reject the invalid characters rejected
// by base::IsStringUTF8().
//
// The implementation detects errors on the first possible byte.

#ifndef BASE_I18N_STREAMING_UTF8_VALIDATOR_H_
#define BASE_I18N_STREAMING_UTF8_VALIDATOR_H_

#include <stddef.h>
#include <stdint.h>

#include <string>

#include "base/i18n/base_i18n_export.h"
#include "base/macros.h"

namespace base {

class BASE_I18N_EXPORT StreamingUtf8Validator {
 public:




  enum State {
    VALID_ENDPOINT,
    VALID_MIDPOINT,
    INVALID
  };

  StreamingUtf8Validator() : state_(0u) {}






  State AddBytes(const char* data, size_t size);

  void Reset();


  static bool Validate(const std::string& string);

 private:



  uint8_t state_;


  DISALLOW_COPY_AND_ASSIGN(StreamingUtf8Validator);
};

}  // namespace base

#endif  // BASE_I18N_STREAMING_UTF8_VALIDATOR_H_
