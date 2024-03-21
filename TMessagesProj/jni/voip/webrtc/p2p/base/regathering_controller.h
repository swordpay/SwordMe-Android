/*
 *  Copyright 2018 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef P2P_BASE_REGATHERING_CONTROLLER_H_
#define P2P_BASE_REGATHERING_CONTROLLER_H_

#include <memory>

#include "api/task_queue/pending_task_safety_flag.h"
#include "p2p/base/ice_transport_internal.h"
#include "p2p/base/port_allocator.h"
#include "rtc_base/thread.h"

namespace webrtc {

// reacting to signals like SignalWritableState, SignalNetworkRouteChange, etc.,
// using methods like GetStats to get additional information, and calling
// methods like RegatherOnFailedNetworks on the PortAllocatorSession when
// regathering is desired.
//
// "Regathering" is defined as gathering additional candidates within a single
// ICE generation (or in other words, PortAllocatorSession), and is possible
// when "continual gathering" is enabled. This may allow connectivity to be
// maintained and/or restored without a full ICE restart.
//
// Regathering will only begin after PortAllocationSession is set via
// set_allocator_session. This should be called any time the "active"
// PortAllocatorSession is changed (in other words, when an ICE restart occurs),
// so that candidates are gathered for the "current" ICE generation.
//
// All methods of BasicRegatheringController should be called on the same
// thread as the one passed to the constructor, and this thread should be the
// same one where PortAllocatorSession runs, which is also identical to the
// network thread of the ICE transport, as given by
// P2PTransportChannel::thread().
class BasicRegatheringController : public sigslot::has_slots<> {
 public:
  struct Config {
    int regather_on_failed_networks_interval =
        cricket::REGATHER_ON_FAILED_NETWORKS_INTERVAL;
  };

  BasicRegatheringController() = delete;
  BasicRegatheringController(const Config& config,
                             cricket::IceTransportInternal* ice_transport,
                             rtc::Thread* thread);
  ~BasicRegatheringController() override;



  void Start();
  void set_allocator_session(cricket::PortAllocatorSession* allocator_session) {
    allocator_session_ = allocator_session;
  }





  void SetConfig(const Config& config);

 private:



  void OnIceTransportStateChanged(cricket::IceTransportInternal*) {}
  void OnIceTransportWritableState(rtc::PacketTransportInternal*) {}
  void OnIceTransportReceivingState(rtc::PacketTransportInternal*) {}
  void OnIceTransportNetworkRouteChanged(absl::optional<rtc::NetworkRoute>) {}




  void ScheduleRecurringRegatheringOnFailedNetworks();

  void CancelScheduledRecurringRegatheringOnAllNetworks();


  std::unique_ptr<ScopedTaskSafety> pending_regathering_;
  Config config_;
  cricket::IceTransportInternal* ice_transport_;
  cricket::PortAllocatorSession* allocator_session_ = nullptr;
  rtc::Thread* const thread_;
};

}  // namespace webrtc

#endif  // P2P_BASE_REGATHERING_CONTROLLER_H_
