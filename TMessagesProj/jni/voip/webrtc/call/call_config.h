/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef CALL_CALL_CONFIG_H_
#define CALL_CALL_CONFIG_H_

#include "api/fec_controller.h"
#include "api/field_trials_view.h"
#include "api/metronome/metronome.h"
#include "api/neteq/neteq_factory.h"
#include "api/network_state_predictor.h"
#include "api/rtc_error.h"
#include "api/task_queue/task_queue_factory.h"
#include "api/transport/bitrate_settings.h"
#include "api/transport/network_control.h"
#include "call/audio_state.h"
#include "call/rtp_transport_config.h"
#include "call/rtp_transport_controller_send_factory_interface.h"

namespace webrtc {

class AudioProcessing;
class RtcEventLog;

struct CallConfig {



  explicit CallConfig(RtcEventLog* event_log,
                      TaskQueueBase* network_task_queue = nullptr);
  CallConfig(const CallConfig&);
  RtpTransportConfig ExtractTransportConfig() const;
  ~CallConfig();


  BitrateConstraints bitrate_config;

  rtc::scoped_refptr<AudioState> audio_state;

  AudioProcessing* audio_processing = nullptr;


  RtcEventLog* const event_log = nullptr;

  FecControllerFactoryInterface* fec_controller_factory = nullptr;

  TaskQueueFactory* task_queue_factory = nullptr;

  NetworkStatePredictorFactoryInterface* network_state_predictor_factory =
      nullptr;

  NetworkControllerFactoryInterface* network_controller_factory = nullptr;

  NetEqFactory* neteq_factory = nullptr;


  const FieldTrialsView* trials = nullptr;

  TaskQueueBase* const network_task_queue_ = nullptr;

  RtpTransportControllerSendFactoryInterface*
      rtp_transport_controller_send_factory = nullptr;

  Metronome* metronome = nullptr;
};

}  // namespace webrtc

#endif  // CALL_CALL_CONFIG_H_
