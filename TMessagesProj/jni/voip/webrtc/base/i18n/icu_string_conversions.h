// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_I18N_ICU_STRING_CONVERSIONS_H_
#define BASE_I18N_ICU_STRING_CONVERSIONS_H_

#include <string>

#include "base/i18n/base_i18n_export.h"
#include "base/i18n/i18n_constants.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"

namespace base {

class OnStringConversionError {
 public:
  enum Type {

    FAIL,


    SKIP,



    SUBSTITUTE,
  };

 private:
  OnStringConversionError() = delete;
};

// encoding doesn't exist or the encoding fails (when on_error is FAIL),
// returns false.
BASE_I18N_EXPORT bool UTF16ToCodepage(base::StringPiece16 utf16,
                                      const char* codepage_name,
                                      OnStringConversionError::Type on_error,
                                      std::string* encoded);
BASE_I18N_EXPORT bool CodepageToUTF16(base::StringPiece encoded,
                                      const char* codepage_name,
                                      OnStringConversionError::Type on_error,
                                      string16* utf16);

// normalized.
BASE_I18N_EXPORT bool ConvertToUtf8AndNormalize(base::StringPiece text,
                                                const std::string& charset,
                                                std::string* result);

}  // namespace base

#endif  // BASE_I18N_ICU_STRING_CONVERSIONS_H_
