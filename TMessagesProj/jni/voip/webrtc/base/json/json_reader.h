// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// A JSON parser.  Converts strings of JSON into a Value object (see
// base/values.h).
// http://www.ietf.org/rfc/rfc4627.txt?number=4627
//
// Known limitations/deviations from the RFC:
// - Only knows how to parse ints within the range of a signed 32 bit int and
//   decimal numbers within a double.
// - Assumes input is encoded as UTF8.  The spec says we should allow UTF-16
//   (BE or LE) and UTF-32 (BE or LE) as well.
// - We limit nesting to 100 levels to prevent stack overflow (this is allowed
//   by the RFC).
// - A Unicode FAQ ("http://unicode.org/faq/utf_bom.html") writes a data
//   stream may start with a Unicode Byte-Order-Mark (U+FEFF), i.e. the input
//   UTF-8 string for the JSONReader::JsonToValue() function may start with a
//   UTF-8 BOM (0xEF, 0xBB, 0xBF).
//   To avoid the function from mis-treating a UTF-8 BOM as an invalid
//   character, the function skips a Unicode BOM at the beginning of the
//   Unicode string (converted from the input UTF-8 string) before parsing it.
//
// TODO(tc): Add a parsing option to to relax object keys being wrapped in
//   double quotes
// TODO(tc): Add an option to disable comment stripping

#ifndef BASE_JSON_JSON_READER_H_
#define BASE_JSON_JSON_READER_H_

#include <memory>
#include <string>

#include "base/base_export.h"
#include "base/json/json_common.h"
#include "base/optional.h"
#include "base/strings/string_piece.h"
#include "base/values.h"

namespace base {

namespace internal {
class JSONParser;
}

enum JSONParserOptions {


  JSON_PARSE_RFC = 0,

  JSON_ALLOW_TRAILING_COMMAS = 1 << 0,




  JSON_REPLACE_INVALID_CHARACTERS = 1 << 1,
};

class BASE_EXPORT JSONReader {
 public:

  enum JsonParseError {
    JSON_NO_ERROR = 0,
    JSON_INVALID_ESCAPE,
    JSON_SYNTAX_ERROR,
    JSON_UNEXPECTED_TOKEN,
    JSON_TRAILING_COMMA,
    JSON_TOO_MUCH_NESTING,
    JSON_UNEXPECTED_DATA_AFTER_ROOT,
    JSON_UNSUPPORTED_ENCODING,
    JSON_UNQUOTED_DICTIONARY_KEY,
    JSON_TOO_LARGE,
    JSON_UNREPRESENTABLE_NUMBER,
    JSON_PARSE_ERROR_COUNT
  };

  struct BASE_EXPORT ValueWithError {
    ValueWithError();
    ValueWithError(ValueWithError&& other);
    ValueWithError& operator=(ValueWithError&& other);
    ~ValueWithError();

    Optional<Value> value;


    JsonParseError error_code = JSON_NO_ERROR;
    std::string error_message;
    int error_line = 0;
    int error_column = 0;

    DISALLOW_COPY_AND_ASSIGN(ValueWithError);
  };

  static const char kInvalidEscape[];
  static const char kSyntaxError[];
  static const char kUnexpectedToken[];
  static const char kTrailingComma[];
  static const char kTooMuchNesting[];
  static const char kUnexpectedDataAfterRoot[];
  static const char kUnsupportedEncoding[];
  static const char kUnquotedDictionaryKey[];
  static const char kInputTooLarge[];
  static const char kUnrepresentableNumber[];

  JSONReader(int options = JSON_PARSE_RFC,
             size_t max_depth = internal::kAbsoluteMaxDepth);

  ~JSONReader();


  static Optional<Value> Read(StringPiece json,
                              int options = JSON_PARSE_RFC,
                              size_t max_depth = internal::kAbsoluteMaxDepth);





  static std::unique_ptr<Value> ReadDeprecated(
      StringPiece json,
      int options = JSON_PARSE_RFC,
      size_t max_depth = internal::kAbsoluteMaxDepth);



  static ValueWithError ReadAndReturnValueWithError(
      StringPiece json,
      int options = JSON_PARSE_RFC);





  static std::unique_ptr<Value> ReadAndReturnErrorDeprecated(
      StringPiece json,
      int options,  // JSONParserOptions
      int* error_code_out,
      std::string* error_msg_out,
      int* error_line_out = nullptr,
      int* error_column_out = nullptr);


  static std::string ErrorCodeToString(JsonParseError error_code);

  Optional<Value> ReadToValue(StringPiece json);


  std::unique_ptr<Value> ReadToValueDeprecated(StringPiece json);


  JsonParseError error_code() const;


  std::string GetErrorMessage() const;

 private:
  std::unique_ptr<internal::JSONParser> parser_;
};

}  // namespace base

#endif  // BASE_JSON_JSON_READER_H_
