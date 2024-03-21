/*
 *  Copyright (c) 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_TEST_NETWORK_EMULATION_MANAGER_H_
#define API_TEST_NETWORK_EMULATION_MANAGER_H_

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "api/array_view.h"
#include "api/packet_socket_factory.h"
#include "api/test/network_emulation/cross_traffic.h"
#include "api/test/network_emulation/network_emulation_interfaces.h"
#include "api/test/peer_network_dependencies.h"
#include "api/test/simulated_network.h"
#include "api/test/time_controller.h"
#include "api/units/timestamp.h"
#include "rtc_base/network.h"
#include "rtc_base/network_constants.h"
#include "rtc_base/thread.h"

namespace webrtc {


// make it possible for client code to operate with these abstractions and build
// required network configuration. With forward declaration here implementation
// is more readable, than with interfaces approach and cause user needn't any
// API methods on these abstractions it is acceptable here.

// like 3G network between peers, or Wi-Fi for one peer and LTE for another.
// Multiple networks can be joined into chain emulating a network path from
// one peer to another.
class EmulatedNetworkNode;

// peer device to another network interface on another peer device.
class EmulatedRoute;

struct EmulatedEndpointConfig {
  enum class IpAddressFamily { kIpv4, kIpv6 };
  enum class StatsGatheringMode {

    kDefault,


    kDebug
  };

  absl::optional<std::string> name = absl::nullopt;
  IpAddressFamily generated_ip_family = IpAddressFamily::kIpv4;


  absl::optional<rtc::IPAddress> ip;


  bool start_as_enabled = true;

  rtc::AdapterType type = rtc::AdapterType::ADAPTER_TYPE_UNKNOWN;
  StatsGatheringMode stats_gathering_mode = StatsGatheringMode::kDefault;



  bool allow_send_packet_with_different_source_ip = false;



  bool allow_receive_packets_with_different_dest_ip = false;
};

struct EmulatedTURNServerConfig {
  EmulatedEndpointConfig client_config;
  EmulatedEndpointConfig peer_config;
};

class EmulatedTURNServerInterface {
 public:
  struct IceServerConfig {
    std::string username;
    std::string password;
    std::string url;
  };

  virtual ~EmulatedTURNServerInterface() {}

  virtual IceServerConfig GetIceServerConfig() const = 0;


  virtual EmulatedEndpoint* GetClientEndpoint() const = 0;


  virtual rtc::SocketAddress GetClientEndpointAddress() const = 0;


  virtual EmulatedEndpoint* GetPeerEndpoint() const = 0;
};

// layer into PeerConnection. Also contains information about network interfaces
// accessible by PeerConnection.
class EmulatedNetworkManagerInterface {
 public:
  virtual ~EmulatedNetworkManagerInterface() = default;



  virtual rtc::Thread* network_thread() = 0;



  virtual rtc::NetworkManager* network_manager() = 0;



  virtual rtc::PacketSocketFactory* packet_socket_factory() = 0;
  webrtc::webrtc_pc_e2e::PeerNetworkDependencies network_dependencies() {
    return {network_thread(), network_manager(), packet_socket_factory()};
  }


  virtual std::vector<EmulatedEndpoint*> endpoints() const = 0;



  virtual void GetStats(
      std::function<void(std::unique_ptr<EmulatedNetworkStats>)> stats_callback)
      const = 0;
};

enum class TimeMode { kRealTime, kSimulated };

// from the command line flag value `text`.
// Returns `true` and sets `*mode` on success;
// returns `false` and sets `*error` on failure.
bool AbslParseFlag(absl::string_view text, TimeMode* mode, std::string* error);

// `mode`.
std::string AbslUnparseFlag(TimeMode mode);

// All objects returned by this API are owned by NetworkEmulationManager itself
// and will be deleted when manager will be deleted.
class NetworkEmulationManager {
 public:


  struct SimulatedNetworkNode {
    SimulatedNetworkInterface* simulation;
    EmulatedNetworkNode* node;

    class Builder {
     public:
      explicit Builder(NetworkEmulationManager* net) : net_(net) {}
      Builder() : net_(nullptr) {}
      Builder(const Builder&) = default;


      Builder& config(BuiltInNetworkBehaviorConfig config);
      Builder& delay_ms(int queue_delay_ms);
      Builder& capacity_kbps(int link_capacity_kbps);
      Builder& capacity_Mbps(int link_capacity_Mbps);
      Builder& loss(double loss_rate);
      Builder& packet_queue_length(int max_queue_length_in_packets);
      SimulatedNetworkNode Build(uint64_t random_seed = 1) const;
      SimulatedNetworkNode Build(NetworkEmulationManager* net,
                                 uint64_t random_seed = 1) const;

     private:
      NetworkEmulationManager* const net_;
      BuiltInNetworkBehaviorConfig config_;
    };
  };
  virtual ~NetworkEmulationManager() = default;

  virtual TimeController* time_controller() = 0;

  virtual TimeMode time_mode() const = 0;


  EmulatedNetworkNode* CreateUnconstrainedEmulatedNode() {
    return CreateEmulatedNode(BuiltInNetworkBehaviorConfig());
  }




  virtual EmulatedNetworkNode* CreateEmulatedNode(
      BuiltInNetworkBehaviorConfig config,
      uint64_t random_seed = 1) = 0;



  virtual EmulatedNetworkNode* CreateEmulatedNode(
      std::unique_ptr<NetworkBehaviorInterface> network_behavior) = 0;

  virtual SimulatedNetworkNode::Builder NodeBuilder() = 0;


  virtual EmulatedEndpoint* CreateEndpoint(EmulatedEndpointConfig config) = 0;


  virtual void EnableEndpoint(EmulatedEndpoint* endpoint) = 0;


  virtual void DisableEndpoint(EmulatedEndpoint* endpoint) = 0;




















  virtual EmulatedRoute* CreateRoute(
      EmulatedEndpoint* from,
      const std::vector<EmulatedNetworkNode*>& via_nodes,
      EmulatedEndpoint* to) = 0;



  virtual EmulatedRoute* CreateRoute(
      const std::vector<EmulatedNetworkNode*>& via_nodes) = 0;
























  virtual EmulatedRoute* CreateDefaultRoute(
      EmulatedEndpoint* from,
      const std::vector<EmulatedNetworkNode*>& via_nodes,
      EmulatedEndpoint* to) = 0;




  virtual void ClearRoute(EmulatedRoute* route) = 0;




  virtual TcpMessageRoute* CreateTcpRoute(EmulatedRoute* send_route,
                                          EmulatedRoute* ret_route) = 0;


  virtual CrossTrafficRoute* CreateCrossTrafficRoute(
      const std::vector<EmulatedNetworkNode*>& via_nodes) = 0;


  virtual CrossTrafficGenerator* StartCrossTraffic(
      std::unique_ptr<CrossTrafficGenerator> generator) = 0;


  virtual void StopCrossTraffic(CrossTrafficGenerator* generator) = 0;





  virtual EmulatedNetworkManagerInterface*
  CreateEmulatedNetworkManagerInterface(
      const std::vector<EmulatedEndpoint*>& endpoints) = 0;



  virtual void GetStats(
      rtc::ArrayView<EmulatedEndpoint* const> endpoints,
      std::function<void(std::unique_ptr<EmulatedNetworkStats>)>
          stats_callback) = 0;




  virtual EmulatedTURNServerInterface* CreateTURNServer(
      EmulatedTURNServerConfig config) = 0;

  std::pair<EmulatedNetworkManagerInterface*, EmulatedNetworkManagerInterface*>
  CreateEndpointPairWithTwoWayRoutes(
      const BuiltInNetworkBehaviorConfig& config);
};

}  // namespace webrtc

#endif  // API_TEST_NETWORK_EMULATION_MANAGER_H_
