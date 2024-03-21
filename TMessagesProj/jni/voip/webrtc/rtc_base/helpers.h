/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_HELPERS_H_
#define RTC_BASE_HELPERS_H_

#include <stddef.h>
#include <stdint.h>

#include <string>

#include "absl/strings/string_view.h"
#include "rtc_base/system/rtc_export.h"

namespace rtc {

void SetRandomTestMode(bool test);

bool InitRandom(int seed);
bool InitRandom(const char* seed, size_t len);

// We generate base64 values so that they will be printable.
RTC_EXPORT std::string CreateRandomString(size_t length);

// We generate base64 values so that they will be printable.
// Return false if the random number generator failed.
RTC_EXPORT bool CreateRandomString(size_t length, std::string* str);

// with characters from the given table. Return false if the random
// number generator failed.
// For ease of implementation, the function requires that the table
// size evenly divide 256; otherwise, it returns false.
RTC_EXPORT bool CreateRandomString(size_t length,
                                   absl::string_view table,
                                   std::string* str);

// Return false if the random number generator failed.
bool CreateRandomData(size_t length, std::string* data);

std::string CreateRandomUuid();

uint32_t CreateRandomId();

RTC_EXPORT uint64_t CreateRandomId64();

uint32_t CreateRandomNonZeroId();

double CreateRandomDouble();

// value and the current value.
double GetNextMovingAverage(double prev_average, double cur, double ratio);

}  // namespace rtc

#endif  // RTC_BASE_HELPERS_H_
