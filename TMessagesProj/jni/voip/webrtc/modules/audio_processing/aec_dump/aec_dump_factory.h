/*
 *  Copyright (c) 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_PROCESSING_AEC_DUMP_AEC_DUMP_FACTORY_H_
#define MODULES_AUDIO_PROCESSING_AEC_DUMP_AEC_DUMP_FACTORY_H_

#include <memory>

#include "absl/strings/string_view.h"
#include "modules/audio_processing/include/aec_dump.h"
#include "rtc_base/system/file_wrapper.h"
#include "rtc_base/system/rtc_export.h"

namespace rtc {
class TaskQueue;
}  // namespace rtc

namespace webrtc {

class RTC_EXPORT AecDumpFactory {
 public:






  static std::unique_ptr<AecDump> Create(webrtc::FileWrapper file,
                                         int64_t max_log_size_bytes,
                                         rtc::TaskQueue* worker_queue);
  static std::unique_ptr<AecDump> Create(absl::string_view file_name,
                                         int64_t max_log_size_bytes,
                                         rtc::TaskQueue* worker_queue);
  static std::unique_ptr<AecDump> Create(FILE* handle,
                                         int64_t max_log_size_bytes,
                                         rtc::TaskQueue* worker_queue);
};

}  // namespace webrtc

#endif  // MODULES_AUDIO_PROCESSING_AEC_DUMP_AEC_DUMP_FACTORY_H_
