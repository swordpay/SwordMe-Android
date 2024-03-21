/*
 *  Copyright 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

// shouldn't be used externally.

#ifndef API_JSEP_ICE_CANDIDATE_H_
#define API_JSEP_ICE_CANDIDATE_H_

#include <stddef.h>

#include <memory>
#include <string>
#include <vector>

#include "api/candidate.h"
#include "api/jsep.h"
#include "rtc_base/system/rtc_export.h"

namespace webrtc {

class RTC_EXPORT JsepIceCandidate : public IceCandidateInterface {
 public:
  JsepIceCandidate(const std::string& sdp_mid, int sdp_mline_index);
  JsepIceCandidate(const std::string& sdp_mid,
                   int sdp_mline_index,
                   const cricket::Candidate& candidate);
  JsepIceCandidate(const JsepIceCandidate&) = delete;
  JsepIceCandidate& operator=(const JsepIceCandidate&) = delete;
  ~JsepIceCandidate() override;

  bool Initialize(const std::string& sdp, SdpParseError* err);
  void SetCandidate(const cricket::Candidate& candidate) {
    candidate_ = candidate;
  }

  std::string sdp_mid() const override;
  int sdp_mline_index() const override;
  const cricket::Candidate& candidate() const override;

  std::string server_url() const override;

  bool ToString(std::string* out) const override;

 private:
  std::string sdp_mid_;
  int sdp_mline_index_;
  cricket::Candidate candidate_;
};

class JsepCandidateCollection : public IceCandidateCollection {
 public:
  JsepCandidateCollection();


  JsepCandidateCollection(JsepCandidateCollection&& o);

  JsepCandidateCollection(const JsepCandidateCollection&) = delete;
  JsepCandidateCollection& operator=(const JsepCandidateCollection&) = delete;

  JsepCandidateCollection Clone() const;
  size_t count() const override;
  bool HasCandidate(const IceCandidateInterface* candidate) const override;



  virtual void add(JsepIceCandidate* candidate);
  const IceCandidateInterface* at(size_t index) const override;



  size_t remove(const cricket::Candidate& candidate);

 private:
  std::vector<std::unique_ptr<JsepIceCandidate>> candidates_;
};

}  // namespace webrtc

#endif  // API_JSEP_ICE_CANDIDATE_H_
