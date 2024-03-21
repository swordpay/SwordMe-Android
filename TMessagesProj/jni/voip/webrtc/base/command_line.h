// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Arguments with prefixes ('--', '-', and on Windows, '/') are switches.
// Switches will precede all other arguments without switch prefixes.
// Switches can optionally have values, delimited by '=', e.g., "-switch=value".
// If a switch is specified multiple times, only the last value is used.
// An argument of "--" will terminate switch parsing during initialization,
// interpreting subsequent tokens as non-switch arguments, regardless of prefix.

// that the current process was started with.  It must be initialized in main().

#ifndef BASE_COMMAND_LINE_H_
#define BASE_COMMAND_LINE_H_

#include <stddef.h>
#include <map>
#include <string>
#include <vector>

#include "base/base_export.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "build/build_config.h"

namespace base {

class FilePath;

class BASE_EXPORT CommandLine {
 public:
#if defined(OS_WIN)

  using StringType = std::wstring;
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
  using StringType = std::string;
#endif

  using StringPieceType = base::BasicStringPiece<StringType>;
  using CharType = StringType::value_type;
  using StringVector = std::vector<StringType>;
  using SwitchMap = std::map<std::string, StringType, std::less<>>;

  enum NoProgram { NO_PROGRAM };
  explicit CommandLine(NoProgram no_program);

  explicit CommandLine(const FilePath& program);

  CommandLine(int argc, const CharType* const* argv);
  explicit CommandLine(const StringVector& argv);

  CommandLine(const CommandLine& other);
  CommandLine& operator=(const CommandLine& other);

  ~CommandLine();

#if defined(OS_WIN)







  static void set_slash_is_not_a_switch();





  static void InitUsingArgvForTesting(int argc, const char* const* argv);
#endif






  static bool Init(int argc, const char* const* argv);





  static void Reset();



  static CommandLine* ForCurrentProcess();

  static bool InitializedForCurrentProcess();

#if defined(OS_WIN)
  static CommandLine FromString(StringPieceType command_line);
#endif

  void InitFromArgv(int argc, const CharType* const* argv);
  void InitFromArgv(const StringVector& argv);



  StringType GetCommandLineString() const {
    return GetCommandLineStringInternal(false);
  }

#if defined(OS_WIN)







  StringType GetCommandLineStringWithPlaceholders() const {
    return GetCommandLineStringInternal(true);
  }
#endif



  StringType GetArgumentsString() const {
    return GetArgumentsStringInternal(false);
  }

#if defined(OS_WIN)






  StringType GetArgumentsStringWithPlaceholders() const {
    return GetArgumentsStringInternal(true);
  }
#endif

  const StringVector& argv() const { return argv_; }

  FilePath GetProgram() const;
  void SetProgram(const FilePath& program);





  bool HasSwitch(const StringPiece& switch_string) const;
  bool HasSwitch(const char switch_constant[]) const;



  std::string GetSwitchValueASCII(const StringPiece& switch_string) const;
  FilePath GetSwitchValuePath(const StringPiece& switch_string) const;
  StringType GetSwitchValueNative(const StringPiece& switch_string) const;

  const SwitchMap& GetSwitches() const { return switches_; }


  void AppendSwitch(const std::string& switch_string);
  void AppendSwitchPath(const std::string& switch_string,
                        const FilePath& path);
  void AppendSwitchNative(const std::string& switch_string,
                          const StringType& value);
  void AppendSwitchASCII(const std::string& switch_string,
                         const std::string& value);


  void RemoveSwitch(const base::StringPiece switch_key_without_prefix);


  void CopySwitchesFrom(const CommandLine& source,
                        const char* const switches[],
                        size_t count);

  StringVector GetArgs() const;




  void AppendArg(const std::string& value);
  void AppendArgPath(const FilePath& value);
  void AppendArgNative(const StringType& value);


  void AppendArguments(const CommandLine& other, bool include_program);


  void PrependWrapper(const StringType& wrapper);

#if defined(OS_WIN)


  void ParseFromString(StringPieceType command_line);
#endif

 private:

  CommandLine() = delete;






  StringType GetCommandLineStringInternal(bool quote_placeholders) const;


  StringType GetArgumentsStringInternal(bool quote_placeholders) const;

  static CommandLine* current_process_commandline_;

  StringVector argv_;

  SwitchMap switches_;

  size_t begin_args_;
};

}  // namespace base

#endif  // BASE_COMMAND_LINE_H_
