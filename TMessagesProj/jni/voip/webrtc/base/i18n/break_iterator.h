// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_I18N_BREAK_ITERATOR_H_
#define BASE_I18N_BREAK_ITERATOR_H_

#include <stddef.h>

#include "base/i18n/base_i18n_export.h"
#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"

// line breaks in a UTF-16 string.
//
// It provides several modes, BREAK_WORD, BREAK_LINE, BREAK_NEWLINE, and
// BREAK_SENTENCE which modify how characters are aggregated into the returned
// string.
//
// Under BREAK_WORD mode, once a word is encountered any non-word
// characters are not included in the returned string (e.g. in the
// UTF-16 equivalent of the string " foo bar! ", the word breaks are at
// the periods in ". .foo. .bar.!. .").
// Note that Chinese/Japanese/Thai do not use spaces between words so that
// boundaries can fall in the middle of a continuous run of non-space /
// non-punctuation characters.
//
// Under BREAK_LINE mode, once a line breaking opportunity is encountered,
// any non-word  characters are included in the returned string, breaking
// only when a space-equivalent character or a line breaking opportunity
// is encountered (e.g. in the UTF16-equivalent of the string " foo bar! ",
// the breaks are at the periods in ". .foo .bar! .").
//
// Note that lines can be broken at any character/syllable/grapheme cluster
// boundary in Chinese/Japanese/Korean and at word boundaries in Thai
// (Thai does not use spaces between words). Therefore, this is NOT the same
// as breaking only at space-equivalent characters where its former
// name (BREAK_SPACE) implied.
//
// Under BREAK_NEWLINE mode, all characters are included in the returned
// string, breaking only when a newline-equivalent character is encountered
// (eg. in the UTF-16 equivalent of the string "foo\nbar!\n\n", the line
// breaks are at the periods in ".foo\n.bar\n.\n.").
//
// Under BREAK_SENTENCE mode, all characters are included in the returned
// string, breaking only on sentence boundaries defined in "Unicode Standard
// Annex #29: Text Segmentation." Whitespace immediately following the sentence
// is also included. For example, in the UTF-16 equivalent of the string
// "foo bar! baz qux?" the breaks are at the periods in ".foo bar! .baz quz?."
//
// To extract the words from a string, move a BREAK_WORD BreakIterator
// through the string and test whether IsWord() is true. E.g.,
//   BreakIterator iter(str, BreakIterator::BREAK_WORD);
//   if (!iter.Init())
//     return false;
//   while (iter.Advance()) {
//     if (iter.IsWord()) {
//       // Region [iter.prev(), iter.pos()) contains a word.
//       VLOG(1) << "word: " << iter.GetString();
//     }
//   }

namespace base {
namespace i18n {

class BASE_I18N_EXPORT BreakIterator {
 public:
  enum BreakType {
    BREAK_WORD,
    BREAK_LINE,



    BREAK_SPACE = BREAK_LINE,
    BREAK_NEWLINE,
    BREAK_CHARACTER,

    RULE_BASED,
    BREAK_SENTENCE,
  };

  enum WordBreakStatus {


    IS_WORD_BREAK,



    IS_SKIPPABLE_WORD,


    IS_LINE_OR_CHAR_BREAK
  };

  BreakIterator(const StringPiece16& str, BreakType break_type);




  BreakIterator(const StringPiece16& str, const string16& rules);
  ~BreakIterator();


  bool Init();




  bool Advance();



  bool SetText(const base::char16* text, const size_t length);




  bool IsWord() const;












  BreakIterator::WordBreakStatus GetWordBreakStatus() const;



  bool IsEndOfWord(size_t position) const;
  bool IsStartOfWord(size_t position) const;



  bool IsSentenceBoundary(size_t position) const;


  bool IsGraphemeBoundary(size_t position) const;



  string16 GetString() const;

  StringPiece16 GetStringPiece() const;

  size_t prev() const { return prev_; }


  size_t pos() const { return pos_; }

 private:




  void* iter_;

  StringPiece16 string_;

  const string16 rules_;

  BreakType break_type_;

  size_t prev_, pos_;

  DISALLOW_COPY_AND_ASSIGN(BreakIterator);
};

}  // namespace i18n
}  // namespace base

#endif  // BASE_I18N_BREAK_ITERATOR_H_
