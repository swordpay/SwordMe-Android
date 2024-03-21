//
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

#include "absl/flags/parse.h"

#include <stdlib.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include "absl/base/attributes.h"
#include "absl/base/config.h"
#include "absl/base/const_init.h"
#include "absl/base/thread_annotations.h"
#include "absl/flags/commandlineflag.h"
#include "absl/flags/config.h"
#include "absl/flags/flag.h"
#include "absl/flags/internal/commandlineflag.h"
#include "absl/flags/internal/flag.h"
#include "absl/flags/internal/parse.h"
#include "absl/flags/internal/private_handle_accessor.h"
#include "absl/flags/internal/program_name.h"
#include "absl/flags/internal/usage.h"
#include "absl/flags/reflection.h"
#include "absl/flags/usage.h"
#include "absl/flags/usage_config.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/strings/strip.h"
#include "absl/synchronization/mutex.h"


namespace absl {
ABSL_NAMESPACE_BEGIN
namespace flags_internal {
namespace {

ABSL_CONST_INIT absl::Mutex processing_checks_guard(absl::kConstInit);

ABSL_CONST_INIT bool flagfile_needs_processing
    ABSL_GUARDED_BY(processing_checks_guard) = false;
ABSL_CONST_INIT bool fromenv_needs_processing
    ABSL_GUARDED_BY(processing_checks_guard) = false;
ABSL_CONST_INIT bool tryfromenv_needs_processing
    ABSL_GUARDED_BY(processing_checks_guard) = false;

ABSL_CONST_INIT absl::Mutex specified_flags_guard(absl::kConstInit);
ABSL_CONST_INIT std::vector<const CommandLineFlag*>* specified_flags
    ABSL_GUARDED_BY(specified_flags_guard) = nullptr;

struct SpecifiedFlagsCompare {
  bool operator()(const CommandLineFlag* a, const CommandLineFlag* b) const {
    return a->Name() < b->Name();
  }
  bool operator()(const CommandLineFlag* a, absl::string_view b) const {
    return a->Name() < b;
  }
  bool operator()(absl::string_view a, const CommandLineFlag* b) const {
    return a < b->Name();
  }
};

}  // namespace
}  // namespace flags_internal
ABSL_NAMESPACE_END
}  // namespace absl

ABSL_FLAG(std::vector<std::string>, flagfile, {},
          "comma-separated list of files to load flags from")
    .OnUpdate([]() {
      if (absl::GetFlag(FLAGS_flagfile).empty()) return;

      absl::MutexLock l(&absl::flags_internal::processing_checks_guard);


      if (absl::flags_internal::flagfile_needs_processing) {
        ABSL_INTERNAL_LOG(WARNING, "flagfile set twice before it is handled");
      }

      absl::flags_internal::flagfile_needs_processing = true;
    });
ABSL_FLAG(std::vector<std::string>, fromenv, {},
          "comma-separated list of flags to set from the environment"
          " [use 'export FLAGS_flag1=value']")
    .OnUpdate([]() {
      if (absl::GetFlag(FLAGS_fromenv).empty()) return;

      absl::MutexLock l(&absl::flags_internal::processing_checks_guard);


      if (absl::flags_internal::fromenv_needs_processing) {
        ABSL_INTERNAL_LOG(WARNING, "fromenv set twice before it is handled.");
      }

      absl::flags_internal::fromenv_needs_processing = true;
    });
ABSL_FLAG(std::vector<std::string>, tryfromenv, {},
          "comma-separated list of flags to try to set from the environment if "
          "present")
    .OnUpdate([]() {
      if (absl::GetFlag(FLAGS_tryfromenv).empty()) return;

      absl::MutexLock l(&absl::flags_internal::processing_checks_guard);


      if (absl::flags_internal::tryfromenv_needs_processing) {
        ABSL_INTERNAL_LOG(WARNING,
                          "tryfromenv set twice before it is handled.");
      }

      absl::flags_internal::tryfromenv_needs_processing = true;
    });

ABSL_FLAG(std::vector<std::string>, undefok, {},
          "comma-separated list of flag names that it is okay to specify "
          "on the command line even if the program does not define a flag "
          "with that name");

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace flags_internal {

namespace {

class ArgsList {
 public:
  ArgsList() : next_arg_(0) {}
  ArgsList(int argc, char* argv[]) : args_(argv, argv + argc), next_arg_(0) {}
  explicit ArgsList(const std::vector<std::string>& args)
      : args_(args), next_arg_(0) {}

  bool ReadFromFlagfile(const std::string& flag_file_name);

  size_t Size() const { return args_.size() - next_arg_; }
  size_t FrontIndex() const { return next_arg_; }
  absl::string_view Front() const { return args_[next_arg_]; }
  void PopFront() { next_arg_++; }

 private:
  std::vector<std::string> args_;
  size_t next_arg_;
};

bool ArgsList::ReadFromFlagfile(const std::string& flag_file_name) {
  std::ifstream flag_file(flag_file_name);

  if (!flag_file) {
    flags_internal::ReportUsageError(
        absl::StrCat("Can't open flagfile ", flag_file_name), true);

    return false;
  }


  args_.push_back("");

  std::string line;
  bool success = true;

  while (std::getline(flag_file, line)) {
    absl::string_view stripped = absl::StripLeadingAsciiWhitespace(line);

    if (stripped.empty() || stripped[0] == '#') {

      continue;
    }

    if (stripped[0] == '-') {
      if (stripped == "--") {
        flags_internal::ReportUsageError(
            "Flagfile can't contain position arguments or --", true);

        success = false;
        break;
      }

      args_.push_back(std::string(stripped));
      continue;
    }

    flags_internal::ReportUsageError(
        absl::StrCat("Unexpected line in the flagfile ", flag_file_name, ": ",
                     line),
        true);

    success = false;
  }

  return success;
}


// `value`. If variable is not present in environment returns false, otherwise
// returns true.
bool GetEnvVar(const char* var_name, std::string& var_value) {
#ifdef _WIN32
  char buf[1024];
  auto get_res = GetEnvironmentVariableA(var_name, buf, sizeof(buf));
  if (get_res >= sizeof(buf)) {
    return false;
  }

  if (get_res == 0) {
    return false;
  }

  var_value = std::string(buf, get_res);
#else
  const char* val = ::getenv(var_name);
  if (val == nullptr) {
    return false;
  }

  var_value = val;
#endif

  return true;
}


//  Flag name or empty if arg= --
//  Flag value after = in --flag=value (empty if --foo)
//  "Is empty value" status. True if arg= --foo=, false otherwise. This is
//  required to separate --foo from --foo=.
// For example:
//      arg           return values
//   "--foo=bar" -> {"foo", "bar", false}.
//   "--foo"     -> {"foo", "", false}.
//   "--foo="    -> {"foo", "", true}.
std::tuple<absl::string_view, absl::string_view, bool> SplitNameAndValue(
    absl::string_view arg) {

  absl::ConsumePrefix(&arg, "-");

  if (arg.empty()) {
    return std::make_tuple("", "", false);
  }

  auto equal_sign_pos = arg.find("=");

  absl::string_view flag_name = arg.substr(0, equal_sign_pos);

  absl::string_view value;
  bool is_empty_value = false;

  if (equal_sign_pos != absl::string_view::npos) {
    value = arg.substr(equal_sign_pos + 1);
    is_empty_value = value.empty();
  }

  return std::make_tuple(flag_name, value, is_empty_value);
}


//  found flag or nullptr
//  is negative in case of --nofoo
std::tuple<CommandLineFlag*, bool> LocateFlag(absl::string_view flag_name) {
  CommandLineFlag* flag = absl::FindCommandLineFlag(flag_name);
  bool is_negative = false;

  if (!flag && absl::ConsumePrefix(&flag_name, "no")) {
    flag = absl::FindCommandLineFlag(flag_name);
    is_negative = true;
  }

  return std::make_tuple(flag, is_negative);
}


// back.
void CheckDefaultValuesParsingRoundtrip() {
#ifndef NDEBUG
  flags_internal::ForEachFlag([&](CommandLineFlag& flag) {
    if (flag.IsRetired()) return;

#define ABSL_FLAGS_INTERNAL_IGNORE_TYPE(T, _) \
  if (flag.IsOfType<T>()) return;

    ABSL_FLAGS_INTERNAL_SUPPORTED_TYPES(ABSL_FLAGS_INTERNAL_IGNORE_TYPE)
#undef ABSL_FLAGS_INTERNAL_IGNORE_TYPE

    flags_internal::PrivateHandleAccessor::CheckDefaultValueParsingRoundtrip(
        flag);
  });
#endif
}


// in which case new ArgLists are appended to the input_args in a reverse order
// of file names in the input flagfiles list. This order ensures that flags from
// the first flagfile in the input list are processed before the second flagfile
// etc.
bool ReadFlagfiles(const std::vector<std::string>& flagfiles,
                   std::vector<ArgsList>& input_args) {
  bool success = true;
  for (auto it = flagfiles.rbegin(); it != flagfiles.rend(); ++it) {
    ArgsList al;

    if (al.ReadFromFlagfile(*it)) {
      input_args.push_back(al);
    } else {
      success = false;
    }
  }

  return success;
}

// variables correctly or if fail_on_absent_in_env is false. The environment
// variable names are expected to be of the form `FLAGS_<flag_name>`, where
// `flag_name` is a string from the input flag_names list. If successful we
// append a single ArgList at the end of the input_args.
bool ReadFlagsFromEnv(const std::vector<std::string>& flag_names,
                      std::vector<ArgsList>& input_args,
                      bool fail_on_absent_in_env) {
  bool success = true;
  std::vector<std::string> args;


  args.push_back("");

  for (const auto& flag_name : flag_names) {

    if (flag_name == "fromenv" || flag_name == "tryfromenv") {
      flags_internal::ReportUsageError(
          absl::StrCat("Infinite recursion on flag ", flag_name), true);

      success = false;
      continue;
    }

    const std::string envname = absl::StrCat("FLAGS_", flag_name);
    std::string envval;
    if (!GetEnvVar(envname.c_str(), envval)) {
      if (fail_on_absent_in_env) {
        flags_internal::ReportUsageError(
            absl::StrCat(envname, " not found in environment"), true);

        success = false;
      }

      continue;
    }

    args.push_back(absl::StrCat("--", flag_name, "=", envval));
  }

  if (success) {
    input_args.emplace_back(args);
  }

  return success;
}


// flags (flagfile, fromenv, tryfromemv) successfully.
bool HandleGeneratorFlags(std::vector<ArgsList>& input_args,
                          std::vector<std::string>& flagfile_value) {
  bool success = true;

  absl::MutexLock l(&flags_internal::processing_checks_guard);

















  if (flags_internal::flagfile_needs_processing) {
    auto flagfiles = absl::GetFlag(FLAGS_flagfile);

    if (input_args.size() == 1) {
      flagfile_value.insert(flagfile_value.end(), flagfiles.begin(),
                            flagfiles.end());
    }

    success &= ReadFlagfiles(flagfiles, input_args);

    flags_internal::flagfile_needs_processing = false;
  }



  if (flags_internal::fromenv_needs_processing) {
    auto flags_list = absl::GetFlag(FLAGS_fromenv);

    success &= ReadFlagsFromEnv(flags_list, input_args, true);

    flags_internal::fromenv_needs_processing = false;
  }

  if (flags_internal::tryfromenv_needs_processing) {
    auto flags_list = absl::GetFlag(FLAGS_tryfromenv);

    success &= ReadFlagsFromEnv(flags_list, input_args, false);

    flags_internal::tryfromenv_needs_processing = false;
  }

  return success;
}


void ResetGeneratorFlags(const std::vector<std::string>& flagfile_value) {




  if (!flagfile_value.empty()) {
    absl::SetFlag(&FLAGS_flagfile, flagfile_value);
    absl::MutexLock l(&flags_internal::processing_checks_guard);
    flags_internal::flagfile_needs_processing = false;
  }

  if (!absl::GetFlag(FLAGS_fromenv).empty()) {
    absl::SetFlag(&FLAGS_fromenv, {});
  }
  if (!absl::GetFlag(FLAGS_tryfromenv).empty()) {
    absl::SetFlag(&FLAGS_tryfromenv, {});
  }

  absl::MutexLock l(&flags_internal::processing_checks_guard);
  flags_internal::fromenv_needs_processing = false;
  flags_internal::tryfromenv_needs_processing = false;
}


//  success status
//  deduced value
// We are also mutating curr_list in case if we need to get a hold of next
// argument in the input.
std::tuple<bool, absl::string_view> DeduceFlagValue(const CommandLineFlag& flag,
                                                    absl::string_view value,
                                                    bool is_negative,
                                                    bool is_empty_value,
                                                    ArgsList* curr_list) {














  if (flag.IsOfType<bool>()) {
    if (value.empty()) {
      if (is_empty_value) {

        flags_internal::ReportUsageError(
            absl::StrCat(
                "Missing the value after assignment for the boolean flag '",
                flag.Name(), "'"),
            true);
        return std::make_tuple(false, "");
      }

      value = is_negative ? "0" : "1";
    } else if (is_negative) {

      flags_internal::ReportUsageError(
          absl::StrCat("Negative form with assignment is not valid for the "
                       "boolean flag '",
                       flag.Name(), "'"),
          true);
      return std::make_tuple(false, "");
    }
  } else if (is_negative) {

    flags_internal::ReportUsageError(
        absl::StrCat("Negative form is not valid for the flag '", flag.Name(),
                     "'"),
        true);
    return std::make_tuple(false, "");
  } else if (value.empty() && (!is_empty_value)) {
    if (curr_list->Size() == 1) {

      flags_internal::ReportUsageError(
          absl::StrCat("Missing the value for the flag '", flag.Name(), "'"),
          true);
      return std::make_tuple(false, "");
    }

    curr_list->PopFront();
    value = curr_list->Front();





    if (!value.empty() && value[0] == '-' && flag.IsOfType<std::string>()) {
      auto maybe_flag_name = std::get<0>(SplitNameAndValue(value.substr(1)));

      if (maybe_flag_name.empty() ||
          std::get<0>(LocateFlag(maybe_flag_name)) != nullptr) {

        ABSL_INTERNAL_LOG(
            WARNING,
            absl::StrCat("Did you really mean to set flag '", flag.Name(),
                         "' to the value '", value, "'?"));
      }
    }
  }

  return std::make_tuple(true, value);
}


bool CanIgnoreUndefinedFlag(absl::string_view flag_name) {
  auto undefok = absl::GetFlag(FLAGS_undefok);
  if (std::find(undefok.begin(), undefok.end(), flag_name) != undefok.end()) {
    return true;
  }

  if (absl::ConsumePrefix(&flag_name, "no") &&
      std::find(undefok.begin(), undefok.end(), flag_name) != undefok.end()) {
    return true;
  }

  return false;
}

}  // namespace


bool WasPresentOnCommandLine(absl::string_view flag_name) {
  absl::MutexLock l(&specified_flags_guard);
  ABSL_INTERNAL_CHECK(specified_flags != nullptr,
                      "ParseCommandLine is not invoked yet");

  return std::binary_search(specified_flags->begin(), specified_flags->end(),
                            flag_name, SpecifiedFlagsCompare{});
}


std::vector<char*> ParseCommandLineImpl(int argc, char* argv[],
                                        ArgvListAction arg_list_act,
                                        UsageFlagsAction usage_flag_act,
                                        OnUndefinedFlag on_undef_flag) {
  ABSL_INTERNAL_CHECK(argc > 0, "Missing argv[0]");



  flags_internal::FinalizeRegistry();

  CheckDefaultValuesParsingRoundtrip();

  std::vector<std::string> flagfile_value;

  std::vector<ArgsList> input_args;
  input_args.push_back(ArgsList(argc, argv));

  std::vector<char*> output_args;
  std::vector<char*> positional_args;
  output_args.reserve(static_cast<size_t>(argc));




  std::vector<std::pair<bool, std::string>> undefined_flag_names;

  if (ProgramInvocationName() == "UNKNOWN") {
    flags_internal::SetProgramInvocationName(argv[0]);
  }
  output_args.push_back(argv[0]);

  absl::MutexLock l(&specified_flags_guard);
  if (specified_flags == nullptr) {
    specified_flags = new std::vector<const CommandLineFlag*>;
  } else {
    specified_flags->clear();
  }



  bool success = true;
  while (!input_args.empty()) {

    success &= HandleGeneratorFlags(input_args, flagfile_value);


    ArgsList& curr_list = input_args.back();

    curr_list.PopFront();

    if (curr_list.Size() == 0) {
      input_args.pop_back();
      continue;
    }



    absl::string_view arg(curr_list.Front());
    bool arg_from_argv = input_args.size() == 1;


    if (!absl::ConsumePrefix(&arg, "-") || arg.empty()) {
      ABSL_INTERNAL_CHECK(arg_from_argv,
                          "Flagfile cannot contain positional argument");

      positional_args.push_back(argv[curr_list.FrontIndex()]);
      continue;
    }

    if (arg_from_argv && (arg_list_act == ArgvListAction::kKeepParsedArgs)) {
      output_args.push_back(argv[curr_list.FrontIndex()]);
    }





    absl::string_view flag_name;
    absl::string_view value;
    bool is_empty_value = false;

    std::tie(flag_name, value, is_empty_value) = SplitNameAndValue(arg);


    if (flag_name.empty()) {
      ABSL_INTERNAL_CHECK(arg_from_argv,
                          "Flagfile cannot contain positional argument");

      curr_list.PopFront();
      break;
    }

    CommandLineFlag* flag = nullptr;
    bool is_negative = false;
    std::tie(flag, is_negative) = LocateFlag(flag_name);

    if (flag == nullptr) {

      if (flags_internal::DeduceUsageFlags(flag_name, value)) {
        continue;
      }

      if (on_undef_flag != OnUndefinedFlag::kIgnoreUndefined) {
        undefined_flag_names.emplace_back(arg_from_argv,
                                          std::string(flag_name));
      }
      continue;
    }

    auto curr_index = curr_list.FrontIndex();
    bool value_success = true;
    std::tie(value_success, value) =
        DeduceFlagValue(*flag, value, is_negative, is_empty_value, &curr_list);
    success &= value_success;

    if (arg_from_argv && (arg_list_act == ArgvListAction::kKeepParsedArgs) &&
        (curr_index != curr_list.FrontIndex())) {
      output_args.push_back(argv[curr_list.FrontIndex()]);
    }



    std::string error;
    if (!flags_internal::PrivateHandleAccessor::ParseFrom(
            *flag, value, SET_FLAGS_VALUE, kCommandLine, error)) {
      if (flag->IsRetired()) continue;

      flags_internal::ReportUsageError(error, true);
      success = false;
    } else {
      specified_flags->push_back(flag);
    }
  }

  for (const auto& flag_name : undefined_flag_names) {
    if (CanIgnoreUndefinedFlag(flag_name.second)) continue;

    flags_internal::ReportUsageError(
        absl::StrCat("Unknown command line flag '", flag_name.second, "'"),
        true);

    success = false;
  }

#if ABSL_FLAGS_STRIP_NAMES
  if (!success) {
    flags_internal::ReportUsageError(
        "NOTE: command line flags are disabled in this build", true);
  }
#endif

  if (!success) {
    flags_internal::HandleUsageFlags(std::cout,
                                     ProgramUsageMessage());
    std::exit(1);
  }

  if (usage_flag_act == UsageFlagsAction::kHandleUsage) {
    int exit_code = flags_internal::HandleUsageFlags(
        std::cout, ProgramUsageMessage());

    if (exit_code != -1) {
      std::exit(exit_code);
    }
  }

  ResetGeneratorFlags(flagfile_value);


  for (auto arg : positional_args) {
    output_args.push_back(arg);
  }

  if (!input_args.empty()) {
    for (size_t arg_index = input_args.back().FrontIndex();
         arg_index < static_cast<size_t>(argc); ++arg_index) {
      output_args.push_back(argv[arg_index]);
    }
  }

  specified_flags->shrink_to_fit();
  std::sort(specified_flags->begin(), specified_flags->end(),
            SpecifiedFlagsCompare{});
  return output_args;
}

}  // namespace flags_internal


std::vector<char*> ParseCommandLine(int argc, char* argv[]) {
  return flags_internal::ParseCommandLineImpl(
      argc, argv, flags_internal::ArgvListAction::kRemoveParsedArgs,
      flags_internal::UsageFlagsAction::kHandleUsage,
      flags_internal::OnUndefinedFlag::kAbortIfUndefined);
}

ABSL_NAMESPACE_END
}  // namespace absl
