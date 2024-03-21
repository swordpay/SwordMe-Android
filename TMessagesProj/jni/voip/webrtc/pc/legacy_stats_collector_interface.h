/*
 *  Copyright 2020 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// is used by compilation units that do not wish to depend on the StatsCollector
// implementation.

#ifndef PC_LEGACY_STATS_COLLECTOR_INTERFACE_H_
#define PC_LEGACY_STATS_COLLECTOR_INTERFACE_H_

#include <stdint.h>

#include "api/media_stream_interface.h"
#include "api/stats_types.h"

namespace webrtc {

class LegacyStatsCollectorInterface {
 public:
  virtual ~LegacyStatsCollectorInterface() {}

  virtual void AddLocalAudioTrack(AudioTrackInterface* audio_track,
                                  uint32_t ssrc) = 0;


  virtual void RemoveLocalAudioTrack(AudioTrackInterface* audio_track,
                                     uint32_t ssrc) = 0;
  virtual void GetStats(MediaStreamTrackInterface* track,
                        StatsReports* reports) = 0;
};

}  // namespace webrtc

#endif  // PC_LEGACY_STATS_COLLECTOR_INTERFACE_H_
