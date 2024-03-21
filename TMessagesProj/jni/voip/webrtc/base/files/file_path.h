// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// type, providing containers for manipulation in according with the
// platform's conventions for pathnames.  It supports the following path
// types:
//
//                   POSIX            Windows
//                   ---------------  ----------------------------------
// Fundamental type  char[]           wchar_t[]
// Encoding          unspecified*     UTF-16
// Separator         /                \, tolerant of /
// Drive letters     no               case-insensitive A-Z followed by :
// Alternate root    // (surprise!)   \\, for UNC paths
//
// * The encoding need not be specified on POSIX systems, although some
//   POSIX-compliant systems do specify an encoding.  Mac OS X uses UTF-8.
//   Chrome OS also uses UTF-8.
//   Linux does not specify an encoding, but in practice, the locale's
//   character set may be used.
//
// For more arcane bits of path trivia, see below.
//
// FilePath objects are intended to be used anywhere paths are.  An
// application may pass FilePath objects around internally, masking the
// underlying differences between systems, only differing in implementation
// where interfacing directly with the system.  For example, a single
// OpenFile(const FilePath &) function may be made available, allowing all
// callers to operate without regard to the underlying implementation.  On
// POSIX-like platforms, OpenFile might wrap fopen, and on Windows, it might
// wrap _wfopen_s, perhaps both by calling file_path.value().c_str().  This
// allows each platform to pass pathnames around without requiring conversions
// between encodings, which has an impact on performance, but more imporantly,
// has an impact on correctness on platforms that do not have well-defined
// encodings for pathnames.
//
// Several methods are available to perform common operations on a FilePath
// object, such as determining the parent directory (DirName), isolating the
// final path component (BaseName), and appending a relative pathname string
// to an existing FilePath object (Append).  These methods are highly
// recommended over attempting to split and concatenate strings directly.
// These methods are based purely on string manipulation and knowledge of
// platform-specific pathname conventions, and do not consult the filesystem
// at all, making them safe to use without fear of blocking on I/O operations.
// These methods do not function as mutators but instead return distinct
// instances of FilePath objects, and are therefore safe to use on const
// objects.  The objects themselves are safe to share between threads.
//
// To aid in initialization of FilePath objects from string literals, a
// FILE_PATH_LITERAL macro is provided, which accounts for the difference
// between char[]-based pathnames on POSIX systems and wchar_t[]-based
// pathnames on Windows.
//
// As a precaution against premature truncation, paths can't contain NULs.
//
// Because a FilePath object should not be instantiated at the global scope,
// instead, use a FilePath::CharType[] and initialize it with
// FILE_PATH_LITERAL.  At runtime, a FilePath object can be created from the
// character array.  Example:
//
// | const FilePath::CharType kLogFileName[] = FILE_PATH_LITERAL("log.txt");
// |
// | void Function() {
// |   FilePath log_file_path(kLogFileName);
// |   [...]
// | }
//
// WARNING: FilePaths should ALWAYS be displayed with LTR directionality, even
// when the UI language is RTL. This means you always need to pass filepaths
// through base::i18n::WrapPathWithLTRFormatting() before displaying it in the
// RTL UI.
//
// This is a very common source of bugs, please try to keep this in mind.
//
// ARCANE BITS OF PATH TRIVIA
//
//  - A double leading slash is actually part of the POSIX standard.  Systems
//    are allowed to treat // as an alternate root, as Windows does for UNC
//    (network share) paths.  Most POSIX systems don't do anything special
//    with two leading slashes, but FilePath handles this case properly
//    in case it ever comes across such a system.  FilePath needs this support
//    for Windows UNC paths, anyway.
//    References:
//    The Open Group Base Specifications Issue 7, sections 3.267 ("Pathname")
//    and 4.12 ("Pathname Resolution"), available at:
//    http://www.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap03.html#tag_03_267
//    http://www.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap04.html#tag_04_12
//
//  - Windows treats c:\\ the same way it treats \\.  This was intended to
//    allow older applications that require drive letters to support UNC paths
//    like \\server\share\path, by permitting c:\\server\share\path as an
//    equivalent.  Since the OS treats these paths specially, FilePath needs
//    to do the same.  Since Windows can use either / or \ as the separator,
//    FilePath treats c://, c:\\, //, and \\ all equivalently.
//    Reference:
//    The Old New Thing, "Why is a drive letter permitted in front of UNC
//    paths (sometimes)?", available at:
//    http://blogs.msdn.com/oldnewthing/archive/2005/11/22/495740.aspx

#ifndef BASE_FILES_FILE_PATH_H_
#define BASE_FILES_FILE_PATH_H_

#include <stddef.h>

#include <functional>
#include <iosfwd>
#include <string>
#include <vector>

#include "base/base_export.h"
#include "base/compiler_specific.h"
#include "base/stl_util.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "build/build_config.h"

// enabled and disabled independently, to aid testing.  These #defines are
// here so that the same setting can be used in both the implementation and
// in the unit test.
#if defined(OS_WIN)
#define FILE_PATH_USES_DRIVE_LETTERS
#define FILE_PATH_USES_WIN_SEPARATORS
#endif  // OS_WIN

// C99 and format_macros.h) like this:
// base::StringPrintf("Path is %" PRFilePath ".\n", path.value().c_str());
#if defined(OS_WIN)
#define PRFilePath "ls"
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
#define PRFilePath "s"
#endif  // OS_WIN

#if defined(OS_WIN)
#define FILE_PATH_LITERAL(x) L##x
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
#define FILE_PATH_LITERAL(x) x
#endif  // OS_WIN

namespace base {

class Pickle;
class PickleIterator;

// pathnames on different platforms.
class BASE_EXPORT FilePath {
 public:
#if defined(OS_WIN)


  typedef std::wstring StringType;
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)



  typedef std::string StringType;
#endif  // OS_WIN

  typedef BasicStringPiece<StringType> StringPieceType;
  typedef StringType::value_type CharType;




  static const CharType kSeparators[];

  static const size_t kSeparatorsLength;

  static const CharType kCurrentDirectory[];

  static const CharType kParentDirectory[];

  static const CharType kExtensionSeparator;

  FilePath();
  FilePath(const FilePath& that);
  explicit FilePath(StringPieceType path);
  ~FilePath();
  FilePath& operator=(const FilePath& that);


  FilePath(FilePath&& that) noexcept;


  FilePath& operator=(FilePath&& that);

  bool operator==(const FilePath& that) const;

  bool operator!=(const FilePath& that) const;

  bool operator<(const FilePath& that) const {
    return path_ < that.path_;
  }

  const StringType& value() const { return path_; }

  bool empty() const { return path_.empty(); }

  void clear() { path_.clear(); }

  static bool IsSeparator(CharType character);










  void GetComponents(std::vector<FilePath::StringType>* components) const;






  bool IsParent(const FilePath& child) const;








  bool AppendRelativePath(const FilePath& child, FilePath* path) const;






  FilePath DirName() const WARN_UNUSED_RESULT;




  FilePath BaseName() const WARN_UNUSED_RESULT;










  StringType Extension() const WARN_UNUSED_RESULT;







  StringType FinalExtension() const WARN_UNUSED_RESULT;



  FilePath RemoveExtension() const WARN_UNUSED_RESULT;


  FilePath RemoveFinalExtension() const WARN_UNUSED_RESULT;







  FilePath InsertBeforeExtension(
      StringPieceType suffix) const WARN_UNUSED_RESULT;
  FilePath InsertBeforeExtensionASCII(
      StringPiece suffix) const WARN_UNUSED_RESULT;


  FilePath AddExtension(StringPieceType extension) const WARN_UNUSED_RESULT;


  FilePath AddExtensionASCII(StringPiece extension) const WARN_UNUSED_RESULT;




  FilePath ReplaceExtension(StringPieceType extension) const WARN_UNUSED_RESULT;


  bool MatchesExtension(StringPieceType extension) const;






  FilePath Append(StringPieceType component) const WARN_UNUSED_RESULT;
  FilePath Append(const FilePath& component) const WARN_UNUSED_RESULT;






  FilePath AppendASCII(StringPiece component) const WARN_UNUSED_RESULT;




  bool IsAbsolute() const;

  bool EndsWithSeparator() const WARN_UNUSED_RESULT;


  FilePath AsEndingWithSeparator() const WARN_UNUSED_RESULT;


  FilePath StripTrailingSeparators() const WARN_UNUSED_RESULT;


  bool ReferencesParent() const;




  string16 LossyDisplayName() const;



  std::string MaybeAsASCII() const;













  std::string AsUTF8Unsafe() const;

  string16 AsUTF16Unsafe() const;








  static FilePath FromUTF8Unsafe(StringPiece utf8);

  static FilePath FromUTF16Unsafe(StringPiece16 utf16);

  void WriteToPickle(Pickle* pickle) const;
  bool ReadFromPickle(PickleIterator* iter);


  FilePath NormalizePathSeparators() const;


  FilePath NormalizePathSeparatorsTo(CharType separator) const;








  static int CompareIgnoreCase(StringPieceType string1,
                               StringPieceType string2);
  static bool CompareEqualIgnoreCase(StringPieceType string1,
                                     StringPieceType string2) {
    return CompareIgnoreCase(string1, string2) == 0;
  }
  static bool CompareLessIgnoreCase(StringPieceType string1,
                                    StringPieceType string2) {
    return CompareIgnoreCase(string1, string2) < 0;
  }

#if defined(OS_MACOSX)





  static StringType GetHFSDecomposedForm(StringPieceType string);




  static int HFSFastUnicodeCompare(StringPieceType string1,
                                   StringPieceType string2);
#endif

#if defined(OS_ANDROID)





  bool IsContentUri() const;
#endif

 private:





  void StripTrailingSeparatorsInternal();

  StringType path_;
};

BASE_EXPORT std::ostream& operator<<(std::ostream& out,
                                     const FilePath& file_path);

}  // namespace base

namespace std {

template <>
struct hash<base::FilePath> {
  typedef base::FilePath argument_type;
  typedef std::size_t result_type;
  result_type operator()(argument_type const& f) const {
    return hash<base::FilePath::StringType>()(f.value());
  }
};

}  // namespace std

#endif  // BASE_FILES_FILE_PATH_H_
