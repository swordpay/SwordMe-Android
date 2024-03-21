/*
 *  Copyright 2020 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef CALL_ADAPTATION_BROADCAST_RESOURCE_LISTENER_H_
#define CALL_ADAPTATION_BROADCAST_RESOURCE_LISTENER_H_

#include <vector>

#include "api/adaptation/resource.h"
#include "api/scoped_refptr.h"
#include "rtc_base/synchronization/mutex.h"

namespace webrtc {

// creating N "adapter" resources.
//
// Example:
// If we have ResourceA, ResourceListenerX and ResourceListenerY we can create a
// BroadcastResourceListener that listens to ResourceA, use CreateAdapter() to
// spawn adapter resources ResourceX and ResourceY and let ResourceListenerX
// listen to ResourceX and ResourceListenerY listen to ResourceY. When ResourceA
// makes a measurement it will be echoed by both ResourceX and ResourceY.
//
// TODO(https://crbug.com/webrtc/11565): When the ResourceAdaptationProcessor is
// moved to call there will only be one ResourceAdaptationProcessor that needs
// to listen to the injected resources. When this is the case, delete this class
// and DCHECK that a Resource's listener is never overwritten.
class BroadcastResourceListener : public ResourceListener {
 public:
  explicit BroadcastResourceListener(
      rtc::scoped_refptr<Resource> source_resource);
  ~BroadcastResourceListener() override;

  rtc::scoped_refptr<Resource> SourceResource() const;
  void StartListening();
  void StopListening();


  rtc::scoped_refptr<Resource> CreateAdapterResource();



  void RemoveAdapterResource(rtc::scoped_refptr<Resource> resource);
  std::vector<rtc::scoped_refptr<Resource>> GetAdapterResources();

  void OnResourceUsageStateMeasured(rtc::scoped_refptr<Resource> resource,
                                    ResourceUsageState usage_state) override;

 private:
  class AdapterResource;
  friend class AdapterResource;

  const rtc::scoped_refptr<Resource> source_resource_;
  Mutex lock_;
  bool is_listening_ RTC_GUARDED_BY(lock_);


  std::vector<rtc::scoped_refptr<AdapterResource>> adapters_
      RTC_GUARDED_BY(lock_);
};

}  // namespace webrtc

#endif  // CALL_ADAPTATION_BROADCAST_RESOURCE_LISTENER_H_
