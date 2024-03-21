/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MODULES_AUDIO_CODING_NETEQ_RED_PAYLOAD_SPLITTER_H_
#define MODULES_AUDIO_CODING_NETEQ_RED_PAYLOAD_SPLITTER_H_

#include "modules/audio_coding/neteq/packet.h"

namespace webrtc {

class DecoderDatabase;

static const size_t kRedHeaderLength = 4;  // 4 bytes RED header.
static const size_t kRedLastHeaderLength =
    1;  // reduced size for last RED header.
// This class handles splitting of RED payloads into smaller parts.
// Codec-specific packet splitting can be performed by
// AudioDecoder::ParsePayload.
class RedPayloadSplitter {
 public:
  RedPayloadSplitter() {}

  virtual ~RedPayloadSplitter() {}

  RedPayloadSplitter(const RedPayloadSplitter&) = delete;
  RedPayloadSplitter& operator=(const RedPayloadSplitter&) = delete;






  virtual bool SplitRed(PacketList* packet_list);



  virtual void CheckRedPayloads(PacketList* packet_list,
                                const DecoderDatabase& decoder_database);
};

}  // namespace webrtc
#endif  // MODULES_AUDIO_CODING_NETEQ_RED_PAYLOAD_SPLITTER_H_
