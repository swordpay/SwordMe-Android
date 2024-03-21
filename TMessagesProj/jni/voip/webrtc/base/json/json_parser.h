// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_JSON_JSON_PARSER_H_
#define BASE_JSON_JSON_PARSER_H_

#include <stddef.h>
#include <stdint.h>

#include <memory>
#include <string>

#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "base/gtest_prod_util.h"
#include "base/json/json_common.h"
#include "base/json/json_reader.h"
#include "base/macros.h"
#include "base/optional.h"
#include "base/strings/string_piece.h"

namespace base {

class Value;

namespace internal {

class JSONParserTest;

// to be used directly; it encapsulates logic that need not be exposed publicly.
//
// This parser guarantees O(n) time through the input string. Iteration happens
// on the byte level, with the functions ConsumeChars() and ConsumeChar(). The
// conversion from byte to JSON token happens without advancing the parser in
// GetNextToken/ParseToken, that is tokenization operates on the current parser
// position without advancing.
//
// Built on top of these are a family of Consume functions that iterate
// internally. Invariant: on entry of a Consume function, the parser is wound
// to the first byte of a valid JSON token. On exit, it is on the first byte
// after the token that was just consumed, which would likely be the first byte
// of the next token.
class BASE_EXPORT JSONParser {
 public:
  JSONParser(int options, size_t max_depth = kAbsoluteMaxDepth);
  ~JSONParser();




  Optional<Value> Parse(StringPiece input);

  JSONReader::JsonParseError error_code() const;

  std::string GetErrorMessage() const;


  int error_line() const;


  int error_column() const;

 private:
  enum Token {
    T_OBJECT_BEGIN,           // {
    T_OBJECT_END,             // }
    T_ARRAY_BEGIN,            // [
    T_ARRAY_END,              // ]
    T_STRING,
    T_NUMBER,
    T_BOOL_TRUE,              // true
    T_BOOL_FALSE,             // false
    T_NULL,                   // null
    T_LIST_SEPARATOR,         // ,
    T_OBJECT_PAIR_SEPARATOR,  // :
    T_END_OF_INPUT,
    T_INVALID_TOKEN,
  };





  class StringBuilder {
   public:

    StringBuilder();

    explicit StringBuilder(const char* pos);

    ~StringBuilder();

    StringBuilder& operator=(StringBuilder&& other);



    void Append(uint32_t point);



    void Convert();



    std::string DestructiveAsString();

   private:

    const char* pos_;

    size_t length_;


    base::Optional<std::string> string_;
  };


  Optional<StringPiece> PeekChars(size_t count);

  Optional<char> PeekChar();


  Optional<StringPiece> ConsumeChars(size_t count);

  Optional<char> ConsumeChar();

  const char* pos();


  Token GetNextToken();


  void EatWhitespaceAndComments();


  bool EatComment();

  Optional<Value> ParseNextToken();


  Optional<Value> ParseToken(Token token);


  Optional<Value> ConsumeDictionary();


  Optional<Value> ConsumeList();

  Optional<Value> ConsumeString();




  bool ConsumeStringRaw(StringBuilder* out);




  bool DecodeUTF16(uint32_t* out_code_point);


  Optional<Value> ConsumeNumber();


  bool ReadInt(bool allow_leading_zeros);


  Optional<Value> ConsumeLiteral();




  bool ConsumeIfMatch(StringPiece match);



  void ReportError(JSONReader::JsonParseError code, int column_adjust);


  static std::string FormatErrorMessage(int line, int column,
                                        const std::string& description);

  const int options_;

  const size_t max_depth_;

  StringPiece input_;

  int index_;

  size_t stack_depth_;

  int line_number_;

  int index_last_line_;

  JSONReader::JsonParseError error_code_;
  int error_line_;
  int error_column_;

  friend class JSONParserTest;
  FRIEND_TEST_ALL_PREFIXES(JSONParserTest, NextChar);
  FRIEND_TEST_ALL_PREFIXES(JSONParserTest, ConsumeDictionary);
  FRIEND_TEST_ALL_PREFIXES(JSONParserTest, ConsumeList);
  FRIEND_TEST_ALL_PREFIXES(JSONParserTest, ConsumeString);
  FRIEND_TEST_ALL_PREFIXES(JSONParserTest, ConsumeLiterals);
  FRIEND_TEST_ALL_PREFIXES(JSONParserTest, ConsumeNumbers);
  FRIEND_TEST_ALL_PREFIXES(JSONParserTest, ErrorMessages);
  FRIEND_TEST_ALL_PREFIXES(JSONParserTest, ReplaceInvalidCharacters);
  FRIEND_TEST_ALL_PREFIXES(JSONParserTest, ReplaceInvalidUTF16EscapeSequence);

  DISALLOW_COPY_AND_ASSIGN(JSONParser);
};

BASE_EXPORT extern const char kUnicodeReplacementString[];

}  // namespace internal
}  // namespace base

#endif  // BASE_JSON_JSON_PARSER_H_
