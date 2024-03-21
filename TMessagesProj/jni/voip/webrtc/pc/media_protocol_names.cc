/*
 *  Copyright 2019 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "pc/media_protocol_names.h"

#include <ctype.h>
#include <stddef.h>

#include <string>

namespace cricket {

// http://www.iana.org/assignments/rtp-parameters/rtp-parameters.xml
// The UDP/DTLS and TCP/DTLS prefixes are not registered there.

// UDP/TLS/RTP/SAVPF (WebRTC default), RTP/AVP, RTP/AVPF, RTP/SAVPF,
// TCP/DTLS/RTP/SAVPF and so on. We accept anything that has RTP/
// embedded in it somewhere as being an RTP protocol.
const char kMediaProtocolRtpPrefix[] = "RTP/";

const char kMediaProtocolSctp[] = "SCTP";
const char kMediaProtocolUdpDtlsSctp[] = "UDP/DTLS/SCTP";
const char kMediaProtocolDtlsSctp[] = "DTLS/SCTP";
const char kMediaProtocolTcpDtlsSctp[] = "TCP/DTLS/SCTP";
// RFC5124
const char kMediaProtocolDtlsSavpf[] = "UDP/TLS/RTP/SAVPF";
const char kMediaProtocolSavpf[] = "RTP/SAVPF";
const char kMediaProtocolAvpf[] = "RTP/AVPF";

namespace {

// We always generate offers with "UDP/TLS/RTP/SAVPF" when using DTLS-SRTP,
// but we tolerate "RTP/SAVPF" and "RTP/SAVP" and the "UDP/TLS" and "TCP/TLS"
// prefixes in offers we receive, for compatibility.
// RFC4585
const char kMediaProtocolSavp[] = "RTP/SAVP";
const char kMediaProtocolAvp[] = "RTP/AVP";

const char kMediaProtocolTcpTlsSavpf[] = "TCP/TLS/RTP/SAVPF";
const char kMediaProtocolUdpTlsSavpf[] = "UDP/TLS/RTP/SAVPF";
const char kMediaProtocolTcpTlsSavp[] = "TCP/TLS/RTP/SAVP";
const char kMediaProtocolUdpTlsSavp[] = "UDP/TLS/RTP/SAVP";

}  // namespace

bool IsDtlsSctp(absl::string_view protocol) {
  return protocol == kMediaProtocolDtlsSctp ||
         protocol == kMediaProtocolUdpDtlsSctp ||
         protocol == kMediaProtocolTcpDtlsSctp;
}

bool IsPlainSctp(absl::string_view protocol) {
  return protocol == kMediaProtocolSctp;
}

bool IsSctpProtocol(absl::string_view protocol) {
  return IsPlainSctp(protocol) || IsDtlsSctp(protocol);
}

bool IsRtpProtocol(absl::string_view protocol) {
  if (protocol.empty()) {
    return true;
  }
  size_t pos = protocol.find(cricket::kMediaProtocolRtpPrefix);
  if (pos == std::string::npos) {
    return false;
  }

  if (pos == 0 || !isalpha(static_cast<unsigned char>(protocol[pos - 1]))) {
    return true;
  }
  return false;
}

// legacy compatibility, as required by JSEP in Section 5.1.2, Profile Names
// and Interoperability.

bool IsDtlsRtp(absl::string_view protocol) {

  return protocol == kMediaProtocolDtlsSavpf ||
         protocol == kMediaProtocolTcpTlsSavpf ||
         protocol == kMediaProtocolUdpTlsSavpf ||
         protocol == kMediaProtocolUdpTlsSavp ||
         protocol == kMediaProtocolTcpTlsSavp;
}

bool IsPlainRtp(absl::string_view protocol) {

  return protocol == kMediaProtocolSavpf || protocol == kMediaProtocolAvpf ||
         protocol == kMediaProtocolSavp || protocol == kMediaProtocolAvp;
}

}  // namespace cricket
