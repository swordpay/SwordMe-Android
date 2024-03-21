/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_VIDEO_CODING_FRAME_BUFFER2_H_
#define MODULES_VIDEO_CODING_FRAME_BUFFER2_H_

#include <array>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "absl/container/inlined_vector.h"
#include "api/field_trials_view.h"
#include "api/sequence_checker.h"
#include "api/task_queue/task_queue_base.h"
#include "api/video/encoded_frame.h"
#include "modules/video_coding/include/video_coding_defines.h"
#include "modules/video_coding/timing/inter_frame_delay.h"
#include "modules/video_coding/timing/jitter_estimator.h"
#include "modules/video_coding/utility/decoded_frames_history.h"
#include "rtc_base/event.h"
#include "rtc_base/experiments/field_trial_parser.h"
#include "rtc_base/experiments/rtt_mult_experiment.h"
#include "rtc_base/numerics/sequence_number_util.h"
#include "rtc_base/synchronization/mutex.h"
#include "rtc_base/system/no_unique_address.h"
#include "rtc_base/task_utils/repeating_task.h"
#include "rtc_base/thread_annotations.h"

namespace webrtc {

class Clock;
class VCMReceiveStatisticsCallback;
class JitterEstimator;
class VCMTiming;

namespace video_coding {

class FrameBuffer {
 public:
  FrameBuffer(Clock* clock,
              VCMTiming* timing,
              const FieldTrialsView& field_trials);

  FrameBuffer() = delete;
  FrameBuffer(const FrameBuffer&) = delete;
  FrameBuffer& operator=(const FrameBuffer&) = delete;

  virtual ~FrameBuffer();


  int64_t InsertFrame(std::unique_ptr<EncodedFrame> frame);

  using NextFrameCallback = std::function<void(std::unique_ptr<EncodedFrame>)>;


  void NextFrame(int64_t max_wait_time_ms,
                 bool keyframe_required,
                 TaskQueueBase* callback_queue,
                 NextFrameCallback handler);




  void SetProtectionMode(VCMVideoProtection mode);


  void Stop();

  void UpdateRtt(int64_t rtt_ms);

  void Clear();

  int Size();

 private:
  struct FrameInfo {
    FrameInfo();
    FrameInfo(FrameInfo&&);
    ~FrameInfo();


    absl::InlinedVector<int64_t, 8> dependent_frames;




    size_t num_missing_continuous = 0;



    size_t num_missing_decodable = 0;

    bool continuous = false;

    std::unique_ptr<EncodedFrame> frame;
  };

  using FrameMap = std::map<int64_t, FrameInfo>;

  bool ValidReferences(const EncodedFrame& frame) const;

  int64_t FindNextFrame(Timestamp now) RTC_EXCLUSIVE_LOCKS_REQUIRED(mutex_);
  std::unique_ptr<EncodedFrame> GetNextFrame()
      RTC_EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  void StartWaitForNextFrameOnQueue() RTC_EXCLUSIVE_LOCKS_REQUIRED(mutex_);
  void CancelCallback() RTC_EXCLUSIVE_LOCKS_REQUIRED(mutex_);


  void PropagateContinuity(FrameMap::iterator start)
      RTC_EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  void PropagateDecodability(const FrameInfo& info)
      RTC_EXCLUSIVE_LOCKS_REQUIRED(mutex_);



  bool UpdateFrameInfoWithIncomingFrame(const EncodedFrame& frame,
                                        FrameMap::iterator info)
      RTC_EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  void ClearFramesAndHistory() RTC_EXCLUSIVE_LOCKS_REQUIRED(mutex_);




  std::unique_ptr<EncodedFrame> CombineAndDeleteFrames(
      std::vector<std::unique_ptr<EncodedFrame>> frames) const;

  RTC_NO_UNIQUE_ADDRESS SequenceChecker construction_checker_;
  RTC_NO_UNIQUE_ADDRESS SequenceChecker callback_checker_;

  FrameMap frames_ RTC_GUARDED_BY(mutex_);
  DecodedFramesHistory decoded_frames_history_ RTC_GUARDED_BY(mutex_);

  Mutex mutex_;
  Clock* const clock_;

  TaskQueueBase* callback_queue_ RTC_GUARDED_BY(mutex_);
  RepeatingTaskHandle callback_task_ RTC_GUARDED_BY(mutex_);
  NextFrameCallback frame_handler_ RTC_GUARDED_BY(mutex_);
  int64_t latest_return_time_ms_ RTC_GUARDED_BY(mutex_);
  bool keyframe_required_ RTC_GUARDED_BY(mutex_);

  JitterEstimator jitter_estimator_ RTC_GUARDED_BY(mutex_);
  VCMTiming* const timing_ RTC_GUARDED_BY(mutex_);
  InterFrameDelay inter_frame_delay_ RTC_GUARDED_BY(mutex_);
  absl::optional<int64_t> last_continuous_frame_ RTC_GUARDED_BY(mutex_);
  std::vector<FrameMap::iterator> frames_to_decode_ RTC_GUARDED_BY(mutex_);
  bool stopped_ RTC_GUARDED_BY(mutex_);
  VCMVideoProtection protection_mode_ RTC_GUARDED_BY(mutex_);
  int64_t last_log_non_decoded_ms_ RTC_GUARDED_BY(mutex_);

  const absl::optional<RttMultExperiment::Settings> rtt_mult_settings_;





  FieldTrialParameter<unsigned> zero_playout_delay_max_decode_queue_size_;
};

}  // namespace video_coding
}  // namespace webrtc

#endif  // MODULES_VIDEO_CODING_FRAME_BUFFER2_H_
