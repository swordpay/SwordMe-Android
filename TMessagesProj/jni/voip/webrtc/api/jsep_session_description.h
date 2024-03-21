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

#ifndef API_JSEP_SESSION_DESCRIPTION_H_
#define API_JSEP_SESSION_DESCRIPTION_H_

#include <memory>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "api/candidate.h"
#include "api/jsep.h"
#include "api/jsep_ice_candidate.h"

namespace cricket {
class SessionDescription;
}

namespace webrtc {

class JsepSessionDescription : public SessionDescriptionInterface {
 public:
  explicit JsepSessionDescription(SdpType type);

  explicit JsepSessionDescription(const std::string& type);
  JsepSessionDescription(
      SdpType type,
      std::unique_ptr<cricket::SessionDescription> description,
      absl::string_view session_id,
      absl::string_view session_version);
  virtual ~JsepSessionDescription();

  JsepSessionDescription(const JsepSessionDescription&) = delete;
  JsepSessionDescription& operator=(const JsepSessionDescription&) = delete;

  bool Initialize(std::unique_ptr<cricket::SessionDescription> description,
                  const std::string& session_id,
                  const std::string& session_version);

  virtual std::unique_ptr<SessionDescriptionInterface> Clone() const;

  virtual cricket::SessionDescription* description() {
    return description_.get();
  }
  virtual const cricket::SessionDescription* description() const {
    return description_.get();
  }
  virtual std::string session_id() const { return session_id_; }
  virtual std::string session_version() const { return session_version_; }
  virtual SdpType GetType() const { return type_; }
  virtual std::string type() const { return SdpTypeToString(type_); }

  virtual bool AddCandidate(const IceCandidateInterface* candidate);
  virtual size_t RemoveCandidates(
      const std::vector<cricket::Candidate>& candidates);
  virtual size_t number_of_mediasections() const;
  virtual const IceCandidateCollection* candidates(
      size_t mediasection_index) const;
  virtual bool ToString(std::string* out) const;

  static const int kDefaultVideoCodecId;
  static const char kDefaultVideoCodecName[];

 private:
  std::unique_ptr<cricket::SessionDescription> description_;
  std::string session_id_;
  std::string session_version_;
  SdpType type_;
  std::vector<JsepCandidateCollection> candidate_collection_;

  bool GetMediasectionIndex(const IceCandidateInterface* candidate,
                            size_t* index);
  int GetMediasectionIndex(const cricket::Candidate& candidate);
};

}  // namespace webrtc

#endif  // API_JSEP_SESSION_DESCRIPTION_H_
