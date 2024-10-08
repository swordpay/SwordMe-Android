/*
 *  Copyright 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "sdk/android/src/jni/android_network_monitor.h"

#include <dlfcn.h>

#include "absl/strings/string_view.h"
#ifndef RTLD_NOLOAD
// This was added in Lollipop to dlfcn.h
#define RTLD_NOLOAD 4
#endif

#include "api/sequence_checker.h"
#include "rtc_base/checks.h"
#include "rtc_base/ip_address.h"
#include "rtc_base/logging.h"
#include "rtc_base/strings/string_builder.h"
#include "sdk/android/generated_base_jni/NetworkChangeDetector_jni.h"
#include "sdk/android/generated_base_jni/NetworkMonitor_jni.h"
#include "sdk/android/native_api/jni/java_types.h"
#include "sdk/android/src/jni/jni_helpers.h"

namespace webrtc {
namespace jni {

namespace {

const char* NetworkTypeToString(NetworkType type) {
  switch (type) {
    case NETWORK_UNKNOWN:
      return "UNKNOWN";
    case NETWORK_ETHERNET:
      return "ETHERNET";
    case NETWORK_WIFI:
      return "WIFI";
    case NETWORK_5G:
      return "5G";
    case NETWORK_4G:
      return "4G";
    case NETWORK_3G:
      return "3G";
    case NETWORK_2G:
      return "2G";
    case NETWORK_UNKNOWN_CELLULAR:
      return "UNKNOWN_CELLULAR";
    case NETWORK_BLUETOOTH:
      return "BLUETOOTH";
    case NETWORK_VPN:
      return "VPN";
    case NETWORK_NONE:
      return "NONE";
  }
}

}  // namespace

enum AndroidSdkVersion {
  SDK_VERSION_LOLLIPOP = 21,
  SDK_VERSION_MARSHMALLOW = 23
};

static NetworkType GetNetworkTypeFromJava(
    JNIEnv* jni,
    const JavaRef<jobject>& j_network_type) {
  std::string enum_name = GetJavaEnumName(jni, j_network_type);
  if (enum_name == "CONNECTION_UNKNOWN") {
    return NetworkType::NETWORK_UNKNOWN;
  }
  if (enum_name == "CONNECTION_ETHERNET") {
    return NetworkType::NETWORK_ETHERNET;
  }
  if (enum_name == "CONNECTION_WIFI") {
    return NetworkType::NETWORK_WIFI;
  }
  if (enum_name == "CONNECTION_5G") {
    return NetworkType::NETWORK_5G;
  }
  if (enum_name == "CONNECTION_4G") {
    return NetworkType::NETWORK_4G;
  }
  if (enum_name == "CONNECTION_3G") {
    return NetworkType::NETWORK_3G;
  }
  if (enum_name == "CONNECTION_2G") {
    return NetworkType::NETWORK_2G;
  }
  if (enum_name == "CONNECTION_UNKNOWN_CELLULAR") {
    return NetworkType::NETWORK_UNKNOWN_CELLULAR;
  }
  if (enum_name == "CONNECTION_BLUETOOTH") {
    return NetworkType::NETWORK_BLUETOOTH;
  }
  if (enum_name == "CONNECTION_VPN") {
    return NetworkType::NETWORK_VPN;
  }
  if (enum_name == "CONNECTION_NONE") {
    return NetworkType::NETWORK_NONE;
  }
  RTC_DCHECK_NOTREACHED();
  return NetworkType::NETWORK_UNKNOWN;
}

static rtc::AdapterType AdapterTypeFromNetworkType(
    NetworkType network_type,
    bool surface_cellular_types) {
  switch (network_type) {
    case NETWORK_UNKNOWN:
      return rtc::ADAPTER_TYPE_UNKNOWN;
    case NETWORK_ETHERNET:
      return rtc::ADAPTER_TYPE_ETHERNET;
    case NETWORK_WIFI:
      return rtc::ADAPTER_TYPE_WIFI;
    case NETWORK_5G:
      return surface_cellular_types ? rtc::ADAPTER_TYPE_CELLULAR_5G
                                    : rtc::ADAPTER_TYPE_CELLULAR;
    case NETWORK_4G:
      return surface_cellular_types ? rtc::ADAPTER_TYPE_CELLULAR_4G
                                    : rtc::ADAPTER_TYPE_CELLULAR;
    case NETWORK_3G:
      return surface_cellular_types ? rtc::ADAPTER_TYPE_CELLULAR_3G
                                    : rtc::ADAPTER_TYPE_CELLULAR;
    case NETWORK_2G:
      return surface_cellular_types ? rtc::ADAPTER_TYPE_CELLULAR_2G
                                    : rtc::ADAPTER_TYPE_CELLULAR;
    case NETWORK_UNKNOWN_CELLULAR:
      return rtc::ADAPTER_TYPE_CELLULAR;
    case NETWORK_VPN:
      return rtc::ADAPTER_TYPE_VPN;
    case NETWORK_BLUETOOTH:


      return rtc::ADAPTER_TYPE_UNKNOWN;
    case NETWORK_NONE:
      return rtc::ADAPTER_TYPE_UNKNOWN;
  }

  RTC_DCHECK_NOTREACHED() << "Invalid network type " << network_type;
  return rtc::ADAPTER_TYPE_UNKNOWN;
}

static rtc::IPAddress JavaToNativeIpAddress(
    JNIEnv* jni,
    const JavaRef<jobject>& j_ip_address) {
  std::vector<int8_t> address =
      JavaToNativeByteArray(jni, Java_IPAddress_getAddress(jni, j_ip_address));
  size_t address_length = address.size();
  if (address_length == 4) {

    struct in_addr ip4_addr;
    memcpy(&ip4_addr.s_addr, address.data(), 4);
    return rtc::IPAddress(ip4_addr);
  }

  RTC_CHECK(address_length == 16);
  struct in6_addr ip6_addr;
  memcpy(ip6_addr.s6_addr, address.data(), address_length);
  return rtc::IPAddress(ip6_addr);
}

static NetworkInformation GetNetworkInformationFromJava(
    JNIEnv* jni,
    const JavaRef<jobject>& j_network_info) {
  NetworkInformation network_info;
  network_info.interface_name = JavaToStdString(
      jni, Java_NetworkInformation_getName(jni, j_network_info));
  network_info.handle = static_cast<NetworkHandle>(
      Java_NetworkInformation_getHandle(jni, j_network_info));
  network_info.type = GetNetworkTypeFromJava(
      jni, Java_NetworkInformation_getConnectionType(jni, j_network_info));
  network_info.underlying_type_for_vpn = GetNetworkTypeFromJava(
      jni, Java_NetworkInformation_getUnderlyingConnectionTypeForVpn(
               jni, j_network_info));
  ScopedJavaLocalRef<jobjectArray> j_ip_addresses =
      Java_NetworkInformation_getIpAddresses(jni, j_network_info);
  network_info.ip_addresses = JavaToNativeVector<rtc::IPAddress>(
      jni, j_ip_addresses, &JavaToNativeIpAddress);
  return network_info;
}

static bool AddressMatch(const rtc::IPAddress& ip1, const rtc::IPAddress& ip2) {
  if (ip1.family() != ip2.family()) {
    return false;
  }
  if (ip1.family() == AF_INET) {
    return ip1.ipv4_address().s_addr == ip2.ipv4_address().s_addr;
  }
  if (ip1.family() == AF_INET6) {


    return memcmp(ip1.ipv6_address().s6_addr, ip2.ipv6_address().s6_addr,
                  sizeof(in6_addr) / 2) == 0;
  }
  return false;
}

NetworkInformation::NetworkInformation() = default;

NetworkInformation::NetworkInformation(const NetworkInformation&) = default;

NetworkInformation::NetworkInformation(NetworkInformation&&) = default;

NetworkInformation::~NetworkInformation() = default;

NetworkInformation& NetworkInformation::operator=(const NetworkInformation&) =
    default;

NetworkInformation& NetworkInformation::operator=(NetworkInformation&&) =
    default;

std::string NetworkInformation::ToString() const {
  rtc::StringBuilder ss;
  ss << "NetInfo[name " << interface_name << "; handle " << handle << "; type "
     << type;
  if (type == NETWORK_VPN) {
    ss << "; underlying_type_for_vpn " << underlying_type_for_vpn;
  }
  ss << "]";
  return ss.Release();
}

AndroidNetworkMonitor::AndroidNetworkMonitor(
    JNIEnv* env,
    const JavaRef<jobject>& j_application_context,
    const FieldTrialsView& field_trials)
    : android_sdk_int_(Java_NetworkMonitor_androidSdkInt(env)),
      j_application_context_(env, j_application_context),
      j_network_monitor_(env, Java_NetworkMonitor_getInstance(env)),
      network_thread_(rtc::Thread::Current()),
      field_trials_(field_trials) {}

AndroidNetworkMonitor::~AndroidNetworkMonitor() {
  RTC_DCHECK(!started_);
}

void AndroidNetworkMonitor::Start() {
  RTC_DCHECK_RUN_ON(network_thread_);
  if (started_) {
    return;
  }
  reset();
  started_ = true;
  surface_cellular_types_ =
      field_trials_.IsEnabled("WebRTC-SurfaceCellularTypes");
  find_network_handle_without_ipv6_temporary_part_ = field_trials_.IsEnabled(
      "WebRTC-FindNetworkHandleWithoutIpv6TemporaryPart");
  bind_using_ifname_ =
      !field_trials_.IsDisabled("WebRTC-BindUsingInterfaceName");
  disable_is_adapter_available_ = field_trials_.IsDisabled(
      "WebRTC-AndroidNetworkMonitor-IsAdapterAvailable");



  safety_flag_ = PendingTaskSafetyFlag::Create();

  JNIEnv* env = AttachCurrentThreadIfNeeded();
  Java_NetworkMonitor_startMonitoring(
      env, j_network_monitor_, j_application_context_, jlongFromPointer(this));
}

void AndroidNetworkMonitor::reset() {
  RTC_DCHECK_RUN_ON(network_thread_);
  network_handle_by_address_.clear();
  network_handle_by_if_name_.clear();
  network_info_by_handle_.clear();
  network_preference_by_adapter_type_.clear();
}

void AndroidNetworkMonitor::Stop() {
  RTC_DCHECK_RUN_ON(network_thread_);
  if (!started_) {
    return;
  }
  started_ = false;
  find_network_handle_without_ipv6_temporary_part_ = false;


  safety_flag_->SetNotAlive();

  JNIEnv* env = AttachCurrentThreadIfNeeded();
  Java_NetworkMonitor_stopMonitoring(env, j_network_monitor_,
                                     jlongFromPointer(this));

  reset();
}

// https://cs.chromium.org/chromium/src/net/udp/udp_socket_posix.cc
rtc::NetworkBindingResult AndroidNetworkMonitor::BindSocketToNetwork(
    int socket_fd,
    const rtc::IPAddress& address,
    absl::string_view if_name) {
  RTC_DCHECK_RUN_ON(network_thread_);



  JNIEnv* env = AttachCurrentThreadIfNeeded();
  const bool network_binding_supported =
      Java_NetworkMonitor_networkBindingSupported(env, j_network_monitor_);
  if (!network_binding_supported) {
    RTC_LOG(LS_WARNING)
        << "BindSocketToNetwork is not supported on this platform "
           "(Android SDK: "
        << android_sdk_int_ << ")";
    return rtc::NetworkBindingResult::NOT_IMPLEMENTED;
  }

  absl::optional<NetworkHandle> network_handle =
      FindNetworkHandleFromAddressOrName(address, if_name);
  if (!network_handle) {
    RTC_LOG(LS_WARNING)
        << "BindSocketToNetwork unable to find network handle for"
        << " addr: " << address.ToSensitiveString() << " ifname: " << if_name;
    return rtc::NetworkBindingResult::ADDRESS_NOT_FOUND;
  }

  if (*network_handle == 0 /* NETWORK_UNSPECIFIED */) {
    RTC_LOG(LS_WARNING) << "BindSocketToNetwork 0 network handle for"
                        << " addr: " << address.ToSensitiveString()
                        << " ifname: " << if_name;
    return rtc::NetworkBindingResult::NOT_IMPLEMENTED;
  }

  int rv = 0;
  if (android_sdk_int_ >= SDK_VERSION_MARSHMALLOW) {




    typedef int (*MarshmallowSetNetworkForSocket)(NetworkHandle net,
                                                  int socket);
    static MarshmallowSetNetworkForSocket marshmallowSetNetworkForSocket;


    if (!marshmallowSetNetworkForSocket) {
      const std::string android_native_lib_path = "libandroid.so";
      void* lib = dlopen(android_native_lib_path.c_str(), RTLD_NOW);
      if (lib == nullptr) {
        RTC_LOG(LS_ERROR) << "Library " << android_native_lib_path
                          << " not found!";
        return rtc::NetworkBindingResult::NOT_IMPLEMENTED;
      }
      marshmallowSetNetworkForSocket =
          reinterpret_cast<MarshmallowSetNetworkForSocket>(
              dlsym(lib, "android_setsocknetwork"));
    }
    if (!marshmallowSetNetworkForSocket) {
      RTC_LOG(LS_ERROR) << "Symbol marshmallowSetNetworkForSocket is not found";
      return rtc::NetworkBindingResult::NOT_IMPLEMENTED;
    }
    rv = marshmallowSetNetworkForSocket(*network_handle, socket_fd);
  } else {


    typedef int (*LollipopSetNetworkForSocket)(unsigned net, int socket);
    static LollipopSetNetworkForSocket lollipopSetNetworkForSocket;


    if (!lollipopSetNetworkForSocket) {


      const std::string net_library_path = "libnetd_client.so";




      void* lib = dlopen(net_library_path.c_str(), RTLD_NOW | RTLD_NOLOAD);
      if (lib == nullptr) {
        RTC_LOG(LS_ERROR) << "Library " << net_library_path << " not found!";
        return rtc::NetworkBindingResult::NOT_IMPLEMENTED;
      }
      lollipopSetNetworkForSocket =
          reinterpret_cast<LollipopSetNetworkForSocket>(
              dlsym(lib, "setNetworkForSocket"));
    }
    if (!lollipopSetNetworkForSocket) {
      RTC_LOG(LS_ERROR) << "Symbol lollipopSetNetworkForSocket is not found ";
      return rtc::NetworkBindingResult::NOT_IMPLEMENTED;
    }
    rv = lollipopSetNetworkForSocket(*network_handle, socket_fd);
  }



  if (rv == 0) {
    RTC_LOG(LS_VERBOSE) << "BindSocketToNetwork bound network handle for"
                        << " addr: " << address.ToSensitiveString()
                        << " ifname: " << if_name;
    return rtc::NetworkBindingResult::SUCCESS;
  }

  RTC_LOG(LS_WARNING) << "BindSocketToNetwork got error: " << rv
                      << " addr: " << address.ToSensitiveString()
                      << " ifname: " << if_name;
  if (rv == ENONET) {
    return rtc::NetworkBindingResult::NETWORK_CHANGED;
  }

  return rtc::NetworkBindingResult::FAILURE;
}

void AndroidNetworkMonitor::OnNetworkConnected_n(
    const NetworkInformation& network_info) {
  RTC_DCHECK_RUN_ON(network_thread_);
  RTC_LOG(LS_INFO) << "Network connected: " << network_info.ToString();
  network_info_by_handle_[network_info.handle] = network_info;
  for (const rtc::IPAddress& address : network_info.ip_addresses) {
    network_handle_by_address_[address] = network_info.handle;
  }
  network_handle_by_if_name_[network_info.interface_name] = network_info.handle;
  RTC_CHECK(network_info_by_handle_.size() >=
            network_handle_by_if_name_.size());
  InvokeNetworksChangedCallback();
}

absl::optional<NetworkHandle>
AndroidNetworkMonitor::FindNetworkHandleFromAddressOrName(
    const rtc::IPAddress& ip_address,
    absl::string_view if_name) const {
  RTC_DCHECK_RUN_ON(network_thread_);
  RTC_LOG(LS_INFO) << "Find network handle.";
  if (find_network_handle_without_ipv6_temporary_part_) {
    for (auto const& iter : network_info_by_handle_) {
      const std::vector<rtc::IPAddress>& addresses = iter.second.ip_addresses;
      auto address_it = std::find_if(addresses.begin(), addresses.end(),
                                     [ip_address](rtc::IPAddress address) {
                                       return AddressMatch(ip_address, address);
                                     });
      if (address_it != addresses.end()) {
        return absl::make_optional(iter.first);
      }
    }
  } else {
    auto iter = network_handle_by_address_.find(ip_address);
    if (iter != network_handle_by_address_.end()) {
      return absl::make_optional(iter->second);
    }
  }

  return FindNetworkHandleFromIfname(if_name);
}

absl::optional<NetworkHandle>
AndroidNetworkMonitor::FindNetworkHandleFromIfname(
    absl::string_view if_name) const {
  RTC_DCHECK_RUN_ON(network_thread_);

  auto iter = network_handle_by_if_name_.find(if_name);
  if (iter != network_handle_by_if_name_.end()) {
    return iter->second;
  }

  if (bind_using_ifname_) {
    for (auto const& iter : network_handle_by_if_name_) {


      if (if_name.find(iter.first) != absl::string_view::npos) {
        return absl::make_optional(iter.second);
      }
    }
  }

  return absl::nullopt;
}

void AndroidNetworkMonitor::OnNetworkDisconnected_n(NetworkHandle handle) {
  RTC_DCHECK_RUN_ON(network_thread_);
  RTC_LOG(LS_INFO) << "Network disconnected for handle " << handle;
  auto iter = network_info_by_handle_.find(handle);
  if (iter == network_info_by_handle_.end()) {
    return;
  }

  for (const rtc::IPAddress& address : iter->second.ip_addresses) {
    network_handle_by_address_.erase(address);
  }












  const auto& if_name = iter->second.interface_name;
  auto iter2 = network_handle_by_if_name_.find(if_name);
  RTC_DCHECK(iter2 != network_handle_by_if_name_.end());
  if (iter2 != network_handle_by_if_name_.end() && iter2->second == handle) {


    bool found = false;
    for (const auto& info : network_info_by_handle_) {
      if (info.first == handle) {
        continue;
      }
      if (info.second.interface_name == if_name) {
        found = true;
        network_handle_by_if_name_[if_name] = info.first;
        break;
      }
    }
    if (!found) {

      network_handle_by_if_name_.erase(iter2);
    }
  } else {

#if RTC_DCHECK_IS_ON
    auto owner_handle = FindNetworkHandleFromIfname(if_name);
    RTC_DCHECK(owner_handle && *owner_handle != handle);
#endif
  }

  network_info_by_handle_.erase(iter);
}

void AndroidNetworkMonitor::OnNetworkPreference_n(
    NetworkType type,
    rtc::NetworkPreference preference) {
  RTC_DCHECK_RUN_ON(network_thread_);
  RTC_LOG(LS_INFO) << "Android network monitor preference for "
                   << NetworkTypeToString(type) << " changed to "
                   << rtc::NetworkPreferenceToString(preference);
  auto adapter_type = AdapterTypeFromNetworkType(type, surface_cellular_types_);
  network_preference_by_adapter_type_[adapter_type] = preference;
  InvokeNetworksChangedCallback();
}

void AndroidNetworkMonitor::SetNetworkInfos(
    const std::vector<NetworkInformation>& network_infos) {
  RTC_DCHECK_RUN_ON(network_thread_);


  RTC_DCHECK(network_handle_by_if_name_.empty());
  RTC_DCHECK(network_handle_by_address_.empty());
  RTC_DCHECK(network_info_by_handle_.empty());
  RTC_DCHECK(network_preference_by_adapter_type_.empty());

  reset();
  RTC_LOG(LS_INFO) << "Android network monitor found " << network_infos.size()
                   << " networks";
  for (const NetworkInformation& network : network_infos) {
    OnNetworkConnected_n(network);
  }
}

rtc::NetworkMonitorInterface::InterfaceInfo
AndroidNetworkMonitor::GetInterfaceInfo(absl::string_view if_name) {
  RTC_DCHECK_RUN_ON(network_thread_);
  auto handle = FindNetworkHandleFromIfname(if_name);
  if (!handle) {
    return {
        .adapter_type = rtc::ADAPTER_TYPE_UNKNOWN,
        .available = (disable_is_adapter_available_ ? true : false),
    };
  }
  auto iter = network_info_by_handle_.find(*handle);
  RTC_DCHECK(iter != network_info_by_handle_.end());
  if (iter == network_info_by_handle_.end()) {
    return {
        .adapter_type = rtc::ADAPTER_TYPE_UNKNOWN,
        .available = (disable_is_adapter_available_ ? true : false),
    };
  }

  auto type =
      AdapterTypeFromNetworkType(iter->second.type, surface_cellular_types_);
  auto vpn_type =
      (type == rtc::ADAPTER_TYPE_VPN)
          ? AdapterTypeFromNetworkType(iter->second.underlying_type_for_vpn,
                                       surface_cellular_types_)
          : rtc::ADAPTER_TYPE_UNKNOWN;
  return {
      .adapter_type = type,
      .underlying_type_for_vpn = vpn_type,
      .network_preference = GetNetworkPreference(type),
      .available = true,
  };
}

rtc::NetworkPreference AndroidNetworkMonitor::GetNetworkPreference(
    rtc::AdapterType adapter_type) const {
  RTC_DCHECK_RUN_ON(network_thread_);
  auto preference_iter = network_preference_by_adapter_type_.find(adapter_type);
  if (preference_iter == network_preference_by_adapter_type_.end()) {
    return rtc::NetworkPreference::NEUTRAL;
  }

  return preference_iter->second;
}

AndroidNetworkMonitorFactory::AndroidNetworkMonitorFactory()
    : j_application_context_(nullptr) {}

AndroidNetworkMonitorFactory::AndroidNetworkMonitorFactory(
    JNIEnv* env,
    const JavaRef<jobject>& j_application_context)
    : j_application_context_(env, j_application_context) {}

AndroidNetworkMonitorFactory::~AndroidNetworkMonitorFactory() = default;

rtc::NetworkMonitorInterface*
AndroidNetworkMonitorFactory::CreateNetworkMonitor(
    const FieldTrialsView& field_trials) {
  return new AndroidNetworkMonitor(AttachCurrentThreadIfNeeded(),
                                   j_application_context_, field_trials);
}

void AndroidNetworkMonitor::NotifyConnectionTypeChanged(
    JNIEnv* env,
    const JavaRef<jobject>& j_caller) {
  network_thread_->PostTask(SafeTask(safety_flag_, [this] {
    RTC_LOG(LS_INFO)
        << "Android network monitor detected connection type change.";
    InvokeNetworksChangedCallback();
  }));
}

void AndroidNetworkMonitor::NotifyOfActiveNetworkList(
    JNIEnv* env,
    const JavaRef<jobject>& j_caller,
    const JavaRef<jobjectArray>& j_network_infos) {
  std::vector<NetworkInformation> network_infos =
      JavaToNativeVector<NetworkInformation>(env, j_network_infos,
                                             &GetNetworkInformationFromJava);
  SetNetworkInfos(network_infos);
}

void AndroidNetworkMonitor::NotifyOfNetworkConnect(
    JNIEnv* env,
    const JavaRef<jobject>& j_caller,
    const JavaRef<jobject>& j_network_info) {
  NetworkInformation network_info =
      GetNetworkInformationFromJava(env, j_network_info);
  network_thread_->PostTask(
      SafeTask(safety_flag_, [this, network_info = std::move(network_info)] {
        OnNetworkConnected_n(network_info);
      }));
}

void AndroidNetworkMonitor::NotifyOfNetworkDisconnect(
    JNIEnv* env,
    const JavaRef<jobject>& j_caller,
    jlong network_handle) {
  network_thread_->PostTask(SafeTask(safety_flag_, [this, network_handle] {
    OnNetworkDisconnected_n(static_cast<NetworkHandle>(network_handle));
  }));
}

void AndroidNetworkMonitor::NotifyOfNetworkPreference(
    JNIEnv* env,
    const JavaRef<jobject>& j_caller,
    const JavaRef<jobject>& j_connection_type,
    jint jpreference) {
  NetworkType type = GetNetworkTypeFromJava(env, j_connection_type);
  rtc::NetworkPreference preference =
      static_cast<rtc::NetworkPreference>(jpreference);

  network_thread_->PostTask(SafeTask(safety_flag_, [this, type, preference] {
    OnNetworkPreference_n(type, preference);
  }));
}

}  // namespace jni
}  // namespace webrtc
