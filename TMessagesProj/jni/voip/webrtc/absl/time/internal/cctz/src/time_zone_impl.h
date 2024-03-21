// Copyright 2016 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   https://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#ifndef ABSL_TIME_INTERNAL_CCTZ_TIME_ZONE_IMPL_H_
#define ABSL_TIME_INTERNAL_CCTZ_TIME_ZONE_IMPL_H_

#include <memory>
#include <string>

#include "absl/base/config.h"
#include "absl/time/internal/cctz/include/cctz/civil_time.h"
#include "absl/time/internal/cctz/include/cctz/time_zone.h"
#include "time_zone_if.h"
#include "time_zone_info.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace time_internal {
namespace cctz {

class time_zone::Impl {
 public:

  static time_zone UTC();


  static bool LoadTimeZone(const std::string& name, time_zone* tz);


  static void ClearTimeZoneMapTestOnly();

  const std::string& Name() const {

    return name_;
  }

  time_zone::absolute_lookup BreakTime(const time_point<seconds>& tp) const {
    return zone_->BreakTime(tp);
  }



  time_zone::civil_lookup MakeTime(const civil_second& cs) const {
    return zone_->MakeTime(cs);
  }

  bool NextTransition(const time_point<seconds>& tp,
                      time_zone::civil_transition* trans) const {
    return zone_->NextTransition(tp, trans);
  }
  bool PrevTransition(const time_point<seconds>& tp,
                      time_zone::civil_transition* trans) const {
    return zone_->PrevTransition(tp, trans);
  }

  std::string Version() const { return zone_->Version(); }

  std::string Description() const { return zone_->Description(); }

 private:
  explicit Impl(const std::string& name);
  static const Impl* UTCImpl();

  const std::string name_;
  std::unique_ptr<TimeZoneIf> zone_;
};

}  // namespace cctz
}  // namespace time_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_TIME_INTERNAL_CCTZ_TIME_ZONE_IMPL_H_
