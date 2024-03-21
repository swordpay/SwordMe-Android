/*
 *  Copyright (c) 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef API_TEST_NETWORK_EMULATION_NETWORK_EMULATION_INTERFACES_H_
#define API_TEST_NETWORK_EMULATION_NETWORK_EMULATION_INTERFACES_H_

#include <map>
#include <memory>
#include <vector>

#include "absl/types/optional.h"
#include "api/array_view.h"
#include "api/numerics/samples_stats_counter.h"
#include "api/units/data_rate.h"
#include "api/units/data_size.h"
#include "api/units/timestamp.h"
#include "rtc_base/copy_on_write_buffer.h"
#include "rtc_base/ip_address.h"
#include "rtc_base/socket_address.h"

namespace webrtc {

struct EmulatedIpPacket {
 public:
  EmulatedIpPacket(const rtc::SocketAddress& from,
                   const rtc::SocketAddress& to,
                   rtc::CopyOnWriteBuffer data,
                   Timestamp arrival_time,
                   uint16_t application_overhead = 0);
  ~EmulatedIpPacket() = default;

  EmulatedIpPacket(const EmulatedIpPacket&) = delete;
  EmulatedIpPacket& operator=(const EmulatedIpPacket&) = delete;

  EmulatedIpPacket(EmulatedIpPacket&&) = default;
  EmulatedIpPacket& operator=(EmulatedIpPacket&&) = default;

  size_t size() const { return data.size(); }
  const uint8_t* cdata() const { return data.cdata(); }

  size_t ip_packet_size() const { return size() + headers_size; }
  rtc::SocketAddress from;
  rtc::SocketAddress to;

  rtc::CopyOnWriteBuffer data;
  uint16_t headers_size;
  Timestamp arrival_time;
};

// EmulatedEndpoint to receive packets on a specific port.
class EmulatedNetworkReceiverInterface {
 public:
  virtual ~EmulatedNetworkReceiverInterface() = default;

  virtual void OnPacketReceived(EmulatedIpPacket packet) = 0;
};

class EmulatedNetworkOutgoingStats {
 public:
  virtual ~EmulatedNetworkOutgoingStats() = default;

  virtual int64_t PacketsSent() const = 0;

  virtual DataSize BytesSent() const = 0;




  virtual const SamplesStatsCounter& SentPacketsSizeCounter() const = 0;

  virtual DataSize FirstSentPacketSize() const = 0;


  virtual Timestamp FirstPacketSentTime() const = 0;


  virtual Timestamp LastPacketSentTime() const = 0;

  virtual DataRate AverageSendRate() const = 0;
};

class EmulatedNetworkIncomingStats {
 public:
  virtual ~EmulatedNetworkIncomingStats() = default;

  virtual int64_t PacketsReceived() const = 0;

  virtual DataSize BytesReceived() const = 0;




  virtual const SamplesStatsCounter& ReceivedPacketsSizeCounter() const = 0;

  virtual int64_t PacketsDropped() const = 0;

  virtual DataSize BytesDropped() const = 0;





  virtual const SamplesStatsCounter& DroppedPacketsSizeCounter() const = 0;

  virtual DataSize FirstReceivedPacketSize() const = 0;


  virtual Timestamp FirstPacketReceivedTime() const = 0;


  virtual Timestamp LastPacketReceivedTime() const = 0;

  virtual DataRate AverageReceiveRate() const = 0;
};

class EmulatedNetworkStats {
 public:
  virtual ~EmulatedNetworkStats() = default;


  virtual std::vector<rtc::IPAddress> LocalAddresses() const = 0;

  virtual int64_t PacketsSent() const = 0;

  virtual DataSize BytesSent() const = 0;




  virtual const SamplesStatsCounter& SentPacketsSizeCounter() const = 0;





  virtual const SamplesStatsCounter& SentPacketsQueueWaitTimeUs() const = 0;

  virtual DataSize FirstSentPacketSize() const = 0;


  virtual Timestamp FirstPacketSentTime() const = 0;


  virtual Timestamp LastPacketSentTime() const = 0;

  virtual DataRate AverageSendRate() const = 0;

  virtual int64_t PacketsReceived() const = 0;

  virtual DataSize BytesReceived() const = 0;




  virtual const SamplesStatsCounter& ReceivedPacketsSizeCounter() const = 0;

  virtual int64_t PacketsDropped() const = 0;

  virtual DataSize BytesDropped() const = 0;





  virtual const SamplesStatsCounter& DroppedPacketsSizeCounter() const = 0;

  virtual DataSize FirstReceivedPacketSize() const = 0;


  virtual Timestamp FirstPacketReceivedTime() const = 0;


  virtual Timestamp LastPacketReceivedTime() const = 0;

  virtual DataRate AverageReceiveRate() const = 0;

  virtual std::map<rtc::IPAddress,
                   std::unique_ptr<EmulatedNetworkOutgoingStats>>
  OutgoingStatsPerDestination() const = 0;

  virtual std::map<rtc::IPAddress,
                   std::unique_ptr<EmulatedNetworkIncomingStats>>
  IncomingStatsPerSource() const = 0;
};

// of this are created by NetworkEmulationManager::CreateEndpoint and
// thread safe.
class EmulatedEndpoint : public EmulatedNetworkReceiverInterface {
 public:





  virtual void SendPacket(const rtc::SocketAddress& from,
                          const rtc::SocketAddress& to,
                          rtc::CopyOnWriteBuffer packet_data,
                          uint16_t application_overhead = 0) = 0;












  virtual absl::optional<uint16_t> BindReceiver(
      uint16_t desired_port,
      EmulatedNetworkReceiverInterface* receiver) = 0;



  virtual void UnbindReceiver(uint16_t port) = 0;


  virtual void BindDefaultReceiver(
      EmulatedNetworkReceiverInterface* receiver) = 0;


  virtual void UnbindDefaultReceiver() = 0;
  virtual rtc::IPAddress GetPeerLocalAddress() const = 0;

 private:



  friend class EmulatedEndpointImpl;
  EmulatedEndpoint() = default;
};

// difference from TCP this only support sending messages with a fixed length,
// no streaming. This is useful to simulate signaling and cross traffic using
// message based protocols such as HTTP. It differs from UDP messages in that
// they are guranteed to be delivered eventually, even on lossy networks.
class TcpMessageRoute {
 public:



  virtual void SendMessage(size_t size, std::function<void()> on_received) = 0;

 protected:
  ~TcpMessageRoute() = default;
};
}  // namespace webrtc

#endif  // API_TEST_NETWORK_EMULATION_NETWORK_EMULATION_INTERFACES_H_
