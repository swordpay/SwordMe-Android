/*
 *  Copyright 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// https://chromium.googlesource.com/chromium/src.git/+/HEAD/tools/metrics/histograms/README.md#requirements
// for requirements when adding or changing metrics.

#ifndef API_UMA_METRICS_H_
#define API_UMA_METRICS_H_

namespace webrtc {

// numeric values should never be reused.
enum PeerConnectionAddressFamilyCounter {
  kPeerConnection_IPv4 = 0,
  kPeerConnection_IPv6 = 1,
  kBestConnections_IPv4 = 2,
  kBestConnections_IPv6 = 3,
  kPeerConnectionAddressFamilyCounter_Max
};

// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum PeerConnectionMetricsName {
  kNetworkInterfaces_IPv4 = 0,  // Number of IPv4 interfaces.
  kNetworkInterfaces_IPv6 = 1,  // Number of IPv6 interfaces.
  kTimeToConnect = 2,           // In milliseconds.
  kLocalCandidates_IPv4 = 3,    // Number of IPv4 local candidates.
  kLocalCandidates_IPv6 = 4,    // Number of IPv6 local candidates.
  kPeerConnectionMetricsName_Max
};

// <local_candidate_type>_<remote_candidate_type>. It is recorded based on the
// type of candidate pair used when the PeerConnection first goes to a completed
// state. When BUNDLE is enabled, only the first transport gets recorded.
// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum IceCandidatePairType {


  kIceCandidatePairHostHost = 0,
  kIceCandidatePairHostSrflx = 1,
  kIceCandidatePairHostRelay = 2,
  kIceCandidatePairHostPrflx = 3,
  kIceCandidatePairSrflxHost = 4,
  kIceCandidatePairSrflxSrflx = 5,
  kIceCandidatePairSrflxRelay = 6,
  kIceCandidatePairSrflxPrflx = 7,
  kIceCandidatePairRelayHost = 8,
  kIceCandidatePairRelaySrflx = 9,
  kIceCandidatePairRelayRelay = 10,
  kIceCandidatePairRelayPrflx = 11,
  kIceCandidatePairPrflxHost = 12,
  kIceCandidatePairPrflxSrflx = 13,
  kIceCandidatePairPrflxRelay = 14,


  kIceCandidatePairHostPrivateHostPrivate = 15,
  kIceCandidatePairHostPrivateHostPublic = 16,
  kIceCandidatePairHostPublicHostPrivate = 17,
  kIceCandidatePairHostPublicHostPublic = 18,
  kIceCandidatePairHostNameHostName = 19,
  kIceCandidatePairHostNameHostPrivate = 20,
  kIceCandidatePairHostNameHostPublic = 21,
  kIceCandidatePairHostPrivateHostName = 22,
  kIceCandidatePairHostPublicHostName = 23,
  kIceCandidatePairMax
};

// PeerConnectionMetricsName is that the "EnumCounter" is only counting the
// occurrences of events, while "Name" has a value associated with it which is
// used to form a histogram.

// numeric values should never be reused.
enum KeyExchangeProtocolMedia {
  kEnumCounterKeyProtocolMediaTypeDtlsAudio = 0,
  kEnumCounterKeyProtocolMediaTypeDtlsVideo = 1,
  kEnumCounterKeyProtocolMediaTypeDtlsData = 2,
  kEnumCounterKeyProtocolMediaTypeSdesAudio = 3,
  kEnumCounterKeyProtocolMediaTypeSdesVideo = 4,
  kEnumCounterKeyProtocolMediaTypeSdesData = 5,
  kEnumCounterKeyProtocolMediaTypeMax
};

// numeric values should never be reused.
enum SdpSemanticRequested {
  kSdpSemanticRequestDefault = 0,
  kSdpSemanticRequestPlanB = 1,
  kSdpSemanticRequestUnifiedPlan = 2,
  kSdpSemanticRequestMax
};

// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum AddIceCandidateResult {
  kAddIceCandidateSuccess = 0,
  kAddIceCandidateFailClosed = 1,
  kAddIceCandidateFailNoRemoteDescription = 2,
  kAddIceCandidateFailNullCandidate = 3,
  kAddIceCandidateFailNotValid = 4,
  kAddIceCandidateFailNotReady = 5,
  kAddIceCandidateFailInAddition = 6,
  kAddIceCandidateFailNotUsable = 7,
  kAddIceCandidateMax
};

// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum SimulcastApiVersion {
  kSimulcastApiVersionNone = 0,
  kSimulcastApiVersionLegacy = 1,
  kSimulcastApiVersionSpecCompliant = 2,
  kSimulcastApiVersionMax
};

// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum BundleUsage {

  kBundleUsageEmpty = 0,

  kBundleUsageNoBundleDatachannelOnly = 1,

  kBundleUsageNoBundleSimple = 2,

  kBundleUsageNoBundleComplex = 3,

  kBundleUsageBundleDatachannelOnly = 4,

  kBundleUsageBundleSimple = 5,

  kBundleUsageBundleComplex = 6,

  kBundleUsageNoBundlePlanB = 7,
  kBundleUsageBundlePlanB = 8,
  kBundleUsageMax
};

// https://w3c.github.io/webrtc-pc/#rtcbundlepolicy-enum
// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum BundlePolicyUsage {
  kBundlePolicyUsageBalanced = 0,
  kBundlePolicyUsageMaxBundle = 1,
  kBundlePolicyUsageMaxCompat = 2,
  kBundlePolicyUsageMax
};

// https://datatracker.ietf.org/doc/html/rfc8829#section-4.1.10.1
// These values are persisted to logs. Entries should not be renumbered and
// numeric values should never be reused.
enum ProvisionalAnswerUsage {
  kProvisionalAnswerNotUsed = 0,
  kProvisionalAnswerLocal = 1,
  kProvisionalAnswerRemote = 2,
  kProvisionalAnswerMax
};

// https://chromium.googlesource.com/chromium/src.git/+/HEAD/tools/metrics/histograms/README.md#usage
// instead of the legacy enums used above.

}  // namespace webrtc

#endif  // API_UMA_METRICS_H_
