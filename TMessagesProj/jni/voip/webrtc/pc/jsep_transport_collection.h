/*
 *  Copyright 2021 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef PC_JSEP_TRANSPORT_COLLECTION_H_
#define PC_JSEP_TRANSPORT_COLLECTION_H_

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "api/jsep.h"
#include "api/peer_connection_interface.h"
#include "api/sequence_checker.h"
#include "pc/jsep_transport.h"
#include "pc/session_description.h"
#include "rtc_base/checks.h"
#include "rtc_base/system/no_unique_address.h"
#include "rtc_base/thread_annotations.h"

namespace webrtc {

// in SDP descriptions.

// 1) Move all Bundle-related data structures from JsepTransport
//    into this class.
// 2) Move all Bundle-related functions into this class.
// 3) Move remaining Bundle-related logic into this class.
//    Make data members private.
// 4) Refine interface to have comprehensible semantics.
// 5) Add unit tests.
// 6) Change the logic to do what's right.
class BundleManager {
 public:
  explicit BundleManager(PeerConnectionInterface::BundlePolicy bundle_policy)
      : bundle_policy_(bundle_policy) {

    sequence_checker_.Detach();
  }
  const std::vector<std::unique_ptr<cricket::ContentGroup>>& bundle_groups()
      const {
    RTC_DCHECK_RUN_ON(&sequence_checker_);
    return bundle_groups_;
  }

  const cricket::ContentGroup* LookupGroupByMid(const std::string& mid) const;
  cricket::ContentGroup* LookupGroupByMid(const std::string& mid);


  bool IsFirstMidInGroup(const std::string& mid) const;


  void Update(const cricket::SessionDescription* description, SdpType type);

  void DeleteMid(const cricket::ContentGroup* bundle_group,
                 const std::string& mid);

  void DeleteGroup(const cricket::ContentGroup* bundle_group);

  void Rollback();

  void Commit();

 private:

  void RefreshEstablishedBundleGroupsByMid() RTC_RUN_ON(sequence_checker_);

  RTC_NO_UNIQUE_ADDRESS SequenceChecker sequence_checker_;
  PeerConnectionInterface::BundlePolicy bundle_policy_;
  std::vector<std::unique_ptr<cricket::ContentGroup>> bundle_groups_
      RTC_GUARDED_BY(sequence_checker_);
  std::vector<std::unique_ptr<cricket::ContentGroup>> stable_bundle_groups_
      RTC_GUARDED_BY(sequence_checker_);
  std::map<std::string, cricket::ContentGroup*>
      established_bundle_groups_by_mid_;
};

// It is pulled out here because a lot of the code that deals with
// bundles end up modifying this map, and the two need to be consistent;
// the managers may merge.
class JsepTransportCollection {
 public:
  JsepTransportCollection(std::function<bool(const std::string& mid,
                                             cricket::JsepTransport* transport)>
                              map_change_callback,
                          std::function<void()> state_change_callback)
      : map_change_callback_(map_change_callback),
        state_change_callback_(state_change_callback) {

    sequence_checker_.Detach();
  }

  void RegisterTransport(const std::string& mid,
                         std::unique_ptr<cricket::JsepTransport> transport);


  std::vector<cricket::JsepTransport*> Transports();

  std::vector<cricket::JsepTransport*> ActiveTransports();
  void DestroyAllTransports();

  cricket::JsepTransport* GetTransportByName(const std::string& mid);
  const cricket::JsepTransport* GetTransportByName(
      const std::string& mid) const;

  cricket::JsepTransport* GetTransportForMid(const std::string& mid);
  const cricket::JsepTransport* GetTransportForMid(
      const std::string& mid) const;
  cricket::JsepTransport* GetTransportForMid(absl::string_view mid);
  const cricket::JsepTransport* GetTransportForMid(absl::string_view mid) const;


  bool SetTransportForMid(const std::string& mid,
                          cricket::JsepTransport* jsep_transport);


  void RemoveTransportForMid(const std::string& mid);

  bool RollbackTransports();



  void CommitTransports();

 private:

  bool TransportInUse(cricket::JsepTransport* jsep_transport) const;


  bool TransportNeededForRollback(cricket::JsepTransport* jsep_transport) const;


  void MaybeDestroyJsepTransport(cricket::JsepTransport* transport);

  void DestroyUnusedTransports();

  bool IsConsistent();  // For testing only: Verify internal structure.

  RTC_NO_UNIQUE_ADDRESS SequenceChecker sequence_checker_;

  std::map<std::string, std::unique_ptr<cricket::JsepTransport>>
      jsep_transports_by_name_ RTC_GUARDED_BY(sequence_checker_);


  std::map<std::string, cricket::JsepTransport*> mid_to_transport_
      RTC_GUARDED_BY(sequence_checker_);


  std::map<std::string, cricket::JsepTransport*> stable_mid_to_transport_
      RTC_GUARDED_BY(sequence_checker_);

  const std::function<bool(const std::string& mid,
                           cricket::JsepTransport* transport)>
      map_change_callback_;

  const std::function<void()> state_change_callback_;
};

}  // namespace webrtc

#endif  // PC_JSEP_TRANSPORT_COLLECTION_H_
