/*
 *  Copyright 2020 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef PC_CONNECTION_CONTEXT_H_
#define PC_CONNECTION_CONTEXT_H_

#include <memory>
#include <string>

#include "api/call/call_factory_interface.h"
#include "api/field_trials_view.h"
#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "api/ref_counted_base.h"
#include "api/scoped_refptr.h"
#include "api/sequence_checker.h"
#include "api/transport/sctp_transport_factory_interface.h"
#include "media/base/media_engine.h"
#include "p2p/base/basic_packet_socket_factory.h"
#include "rtc_base/checks.h"
#include "rtc_base/network.h"
#include "rtc_base/network_monitor_factory.h"
#include "rtc_base/rtc_certificate_generator.h"
#include "rtc_base/socket_factory.h"
#include "rtc_base/thread.h"
#include "rtc_base/thread_annotations.h"

namespace cricket {
class ChannelManager;
}

namespace rtc {
class BasicPacketSocketFactory;
class UniqueRandomIdGenerator;
}  // namespace rtc

namespace webrtc {

class RtcEventLog;

// objects. A reference to this object is passed to each PeerConnection. The
// methods on this object are assumed not to change the state in any way that
// interferes with the operation of other PeerConnections.
//
// This class must be created and destroyed on the signaling thread.
class ConnectionContext final
    : public rtc::RefCountedNonVirtual<ConnectionContext> {
 public:



  static rtc::scoped_refptr<ConnectionContext> Create(
      PeerConnectionFactoryDependencies* dependencies);

  ConnectionContext(const ConnectionContext&) = delete;
  ConnectionContext& operator=(const ConnectionContext&) = delete;

  SctpTransportFactoryInterface* sctp_transport_factory() const {
    return sctp_factory_.get();
  }

  cricket::MediaEngineInterface* media_engine() const {
    return media_engine_.get();
  }

  rtc::Thread* signaling_thread() { return signaling_thread_; }
  const rtc::Thread* signaling_thread() const { return signaling_thread_; }
  rtc::Thread* worker_thread() { return worker_thread_.get(); }
  const rtc::Thread* worker_thread() const { return worker_thread_.get(); }
  rtc::Thread* network_thread() { return network_thread_; }
  const rtc::Thread* network_thread() const { return network_thread_; }




  const FieldTrialsView& field_trials() const { return *trials_.get(); }

  rtc::NetworkManager* default_network_manager() {
    RTC_DCHECK_RUN_ON(signaling_thread_);
    return default_network_manager_.get();
  }
  rtc::PacketSocketFactory* default_socket_factory() {
    RTC_DCHECK_RUN_ON(signaling_thread_);
    return default_socket_factory_.get();
  }
  CallFactoryInterface* call_factory() {
    RTC_DCHECK_RUN_ON(worker_thread());
    return call_factory_.get();
  }
  rtc::UniqueRandomIdGenerator* ssrc_generator() { return &ssrc_generator_; }




  bool use_rtx() { return true; }

 protected:
  explicit ConnectionContext(PeerConnectionFactoryDependencies* dependencies);

  friend class rtc::RefCountedNonVirtual<ConnectionContext>;
  ~ConnectionContext();

 private:


  bool wraps_current_thread_;
  std::unique_ptr<rtc::SocketFactory> owned_socket_factory_;
  std::unique_ptr<rtc::Thread> owned_network_thread_
      RTC_GUARDED_BY(signaling_thread_);
  rtc::Thread* const network_thread_;
  AlwaysValidPointer<rtc::Thread> const worker_thread_;
  rtc::Thread* const signaling_thread_;

  std::unique_ptr<FieldTrialsView> const trials_;

  const std::unique_ptr<cricket::MediaEngineInterface> media_engine_;




  rtc::UniqueRandomIdGenerator ssrc_generator_;
  std::unique_ptr<rtc::NetworkMonitorFactory> const network_monitor_factory_
      RTC_GUARDED_BY(signaling_thread_);
  std::unique_ptr<rtc::NetworkManager> default_network_manager_
      RTC_GUARDED_BY(signaling_thread_);
  std::unique_ptr<webrtc::CallFactoryInterface> const call_factory_
      RTC_GUARDED_BY(worker_thread());

  std::unique_ptr<rtc::PacketSocketFactory> default_socket_factory_
      RTC_GUARDED_BY(signaling_thread_);
  std::unique_ptr<SctpTransportFactoryInterface> const sctp_factory_;
};

}  // namespace webrtc

#endif  // PC_CONNECTION_CONTEXT_H_
