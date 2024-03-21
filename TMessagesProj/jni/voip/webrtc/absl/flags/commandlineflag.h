//
// Copyright 2020 The Abseil Authors.
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
//
// -----------------------------------------------------------------------------
// File: commandlineflag.h
// -----------------------------------------------------------------------------
//
// This header file defines the `CommandLineFlag`, which acts as a type-erased
// handle for accessing metadata about the Abseil Flag in question.
//
// Because an actual Abseil flag is of an unspecified type, you should not
// manipulate or interact directly with objects of that type. Instead, use the
// CommandLineFlag type as an intermediary.
#ifndef ABSL_FLAGS_COMMANDLINEFLAG_H_
#define ABSL_FLAGS_COMMANDLINEFLAG_H_

#include <memory>
#include <string>

#include "absl/base/config.h"
#include "absl/base/internal/fast_type_id.h"
#include "absl/flags/internal/commandlineflag.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace flags_internal {
class PrivateHandleAccessor;
}  // namespace flags_internal

//
// This type acts as a type-erased handle for an instance of an Abseil Flag and
// holds reflection information pertaining to that flag. Use CommandLineFlag to
// access a flag's name, location, help string etc.
//
// To obtain an absl::CommandLineFlag, invoke `absl::FindCommandLineFlag()`
// passing it the flag name string.
//
// Example:
//
//   // Obtain reflection handle for a flag named "flagname".
//   const absl::CommandLineFlag* my_flag_data =
//        absl::FindCommandLineFlag("flagname");
//
//   // Now you can get flag info from that reflection handle.
//   std::string flag_location = my_flag_data->Filename();
//   ...
class CommandLineFlag {
 public:
  constexpr CommandLineFlag() = default;

  CommandLineFlag(const CommandLineFlag&) = delete;
  CommandLineFlag& operator=(const CommandLineFlag&) = delete;



  template <typename T>
  inline bool IsOfType() const {
    return TypeId() == base_internal::FastTypeId<T>();
  }




  template <typename T>
  absl::optional<T> TryGet() const {
    if (IsRetired() || !IsOfType<T>()) {
      return absl::nullopt;
    }















    union U {
      T value;
      U() {}
      ~U() { value.~T(); }
    };
    U u;

    Read(&u.value);

    if (IsRetired()) {
      return absl::nullopt;
    }
    return std::move(u.value);
  }



  virtual absl::string_view Name() const = 0;



  virtual std::string Filename() const = 0;



  virtual std::string Help() const = 0;



  virtual bool IsRetired() const;



  virtual std::string DefaultValue() const = 0;



  virtual std::string CurrentValue() const = 0;





  bool ParseFrom(absl::string_view value, std::string* error);

 protected:
  ~CommandLineFlag() = default;

 private:
  friend class flags_internal::PrivateHandleAccessor;








  virtual bool ParseFrom(absl::string_view value,
                         flags_internal::FlagSettingMode set_mode,
                         flags_internal::ValueSource source,
                         std::string& error) = 0;

  virtual flags_internal::FlagFastTypeId TypeId() const = 0;


  virtual std::unique_ptr<flags_internal::FlagStateInterface> SaveState() = 0;


  virtual void Read(void* dst) const = 0;


  virtual bool IsSpecifiedOnCommandLine() const = 0;

  virtual bool ValidateInputValue(absl::string_view value) const = 0;


  virtual void CheckDefaultValueParsingRoundtrip() const = 0;
};

ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_FLAGS_COMMANDLINEFLAG_H_
