// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_STRINGS_STRING_TOKENIZER_H_
#define BASE_STRINGS_STRING_TOKENIZER_H_

#include <algorithm>
#include <string>

#include "base/strings/string_piece.h"

namespace base {

// iterator that with each step (see the Advance method) updates members that
// refer to the next token in the input string.  The user may optionally
// configure the tokenizer to return delimiters.
//
// EXAMPLE 1:
//
//   char input[] = "this is a test";
//   CStringTokenizer t(input, input + strlen(input), " ");
//   while (t.GetNext()) {
//     printf("%s\n", t.token().c_str());
//   }
//
// Output:
//
//   this
//   is
//   a
//   test
//
//
// EXAMPLE 2:
//
//   std::string input = "no-cache=\"foo, bar\", private";
//   StringTokenizer t(input, ", ");
//   t.set_quote_chars("\"");
//   while (t.GetNext()) {
//     printf("%s\n", t.token().c_str());
//   }
//
// Output:
//
//   no-cache="foo, bar"
//   private
//
//
// EXAMPLE 3:
//
//   bool next_is_option = false, next_is_value = false;
//   std::string input = "text/html; charset=UTF-8; foo=bar";
//   StringTokenizer t(input, "; =");
//   t.set_options(StringTokenizer::RETURN_DELIMS);
//   while (t.GetNext()) {
//     if (t.token_is_delim()) {
//       switch (*t.token_begin()) {
//         case ';':
//           next_is_option = true;
//           break;
//         case '=':
//           next_is_value = true;
//           break;
//       }
//     } else {
//       const char* label;
//       if (next_is_option) {
//         label = "option-name";
//         next_is_option = false;
//       } else if (next_is_value) {
//         label = "option-value";
//         next_is_value = false;
//       } else {
//         label = "mime-type";
//       }
//       printf("%s: %s\n", label, t.token().c_str());
//     }
//   }
//
//
template <class str, class const_iterator>
class StringTokenizerT {
 public:
  typedef typename str::value_type char_type;

  enum {

    RETURN_DELIMS = 1 << 0,



    RETURN_EMPTY_TOKENS = 1 << 1,
  };




  StringTokenizerT(const str& string,
                   const str& delims) {
    Init(string.begin(), string.end(), delims);
  }


  StringTokenizerT(str&&, const str& delims) = delete;

  StringTokenizerT(const_iterator string_begin,
                   const_iterator string_end,
                   const str& delims) {
    Init(string_begin, string_end, delims);
  }

  void set_options(int options) { options_ = options; }





  void set_quote_chars(const str& quotes) { quotes_ = quotes; }



  bool GetNext() {
    if (quotes_.empty() && options_ == 0)
      return QuickGetNext();
    else
      return FullGetNext();
  }

  void Reset() {
    token_end_ = start_pos_;
  }




  bool token_is_delim() const { return token_is_delim_; }


  const_iterator token_begin() const { return token_begin_; }
  const_iterator token_end() const { return token_end_; }
  str token() const { return str(token_begin_, token_end_); }
  BasicStringPiece<str> token_piece() const {
    return BasicStringPiece<str>(&*token_begin_,
                                 std::distance(token_begin_, token_end_));
  }

 private:
  void Init(const_iterator string_begin,
            const_iterator string_end,
            const str& delims) {
    start_pos_ = string_begin;
    token_begin_ = string_begin;
    token_end_ = string_begin;
    end_ = string_end;
    delims_ = delims;
    options_ = 0;
    token_is_delim_ = true;
  }



  bool QuickGetNext() {
    token_is_delim_ = false;
    for (;;) {
      token_begin_ = token_end_;
      if (token_end_ == end_) {
        token_is_delim_ = true;
        return false;
      }
      ++token_end_;
      if (delims_.find(*token_begin_) == str::npos)
        break;

    }
    while (token_end_ != end_ && delims_.find(*token_end_) == str::npos)
      ++token_end_;
    return true;
  }

  bool FullGetNext() {
    AdvanceState state;

    for (;;) {
      if (token_is_delim_) {











        token_is_delim_ = false;
        token_begin_ = token_end_;

        while (token_end_ != end_ && AdvanceOne(&state, *token_end_)) {
          ++token_end_;
        }

        if (token_begin_ != token_end_ || (options_ & RETURN_EMPTY_TOKENS))
          return true;
      }

      DCHECK(!token_is_delim_);














      token_is_delim_ = true;
      token_begin_ = token_end_;

      if (token_end_ == end_)
        return false;

      ++token_end_;
      if (options_ & RETURN_DELIMS)
        return true;
    }

    return false;
  }

  bool IsDelim(char_type c) const {
    return delims_.find(c) != str::npos;
  }

  bool IsQuote(char_type c) const {
    return quotes_.find(c) != str::npos;
  }

  struct AdvanceState {
    bool in_quote;
    bool in_escape;
    char_type quote_char;
    AdvanceState() : in_quote(false), in_escape(false), quote_char('\0') {}
  };

  bool AdvanceOne(AdvanceState* state, char_type c) {
    if (state->in_quote) {
      if (state->in_escape) {
        state->in_escape = false;
      } else if (c == '\\') {
        state->in_escape = true;
      } else if (c == state->quote_char) {
        state->in_quote = false;
      }
    } else {
      if (IsDelim(c))
        return false;
      state->in_quote = IsQuote(state->quote_char = c);
    }
    return true;
  }

  const_iterator start_pos_;
  const_iterator token_begin_;
  const_iterator token_end_;
  const_iterator end_;
  str delims_;
  str quotes_;
  int options_;
  bool token_is_delim_;
};

typedef StringTokenizerT<std::string, std::string::const_iterator>
    StringTokenizer;
typedef StringTokenizerT<string16, string16::const_iterator> String16Tokenizer;
typedef StringTokenizerT<std::string, const char*> CStringTokenizer;

}  // namespace base

#endif  // BASE_STRINGS_STRING_TOKENIZER_H_
