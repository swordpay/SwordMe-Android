/*
 *  Copyright 2017 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef PC_SDP_UTILS_H_
#define PC_SDP_UTILS_H_

#include <functional>
#include <memory>
#include <string>

#include "api/jsep.h"
#include "p2p/base/transport_info.h"
#include "pc/session_description.h"
#include "rtc_base/system/rtc_export.h"

namespace webrtc {

RTC_EXPORT std::unique_ptr<SessionDescriptionInterface> CloneSessionDescription(
    const SessionDescriptionInterface* sdesc);

RTC_EXPORT std::unique_ptr<SessionDescriptionInterface>
CloneSessionDescriptionAsType(const SessionDescriptionInterface* sdesc,
                              SdpType type);

// corresponding transport and produces a boolean.
typedef std::function<bool(const cricket::ContentInfo*,
                           const cricket::TransportInfo*)>
    SdpContentPredicate;

// session description.
bool SdpContentsAll(SdpContentPredicate pred,
                    const cricket::SessionDescription* desc);

// given session description.
bool SdpContentsNone(SdpContentPredicate pred,
                     const cricket::SessionDescription* desc);

// corresponding transport and can mutate the content and/or the transport.
typedef std::function<void(cricket::ContentInfo*, cricket::TransportInfo*)>
    SdpContentMutator;

// description.
void SdpContentsForEach(SdpContentMutator fn,
                        cricket::SessionDescription* desc);

}  // namespace webrtc

#endif  // PC_SDP_UTILS_H_
