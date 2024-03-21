/*
 *  Copyright (c) 2018 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef API_TEST_TEST_DEPENDENCY_FACTORY_H_
#define API_TEST_TEST_DEPENDENCY_FACTORY_H_

#include <memory>

#include "api/test/video_quality_test_fixture.h"

namespace webrtc {

// Not all WebRTC tests get their components from here, so you need to make
// sure the tests you want actually use this class.
//
// This class is not thread safe and you need to make call calls from the same
// (test main) thread.
class TestDependencyFactory {
 public:
  virtual ~TestDependencyFactory() = default;


  static const TestDependencyFactory& GetInstance();
  static void SetInstance(std::unique_ptr<TestDependencyFactory> instance);




  virtual std::unique_ptr<VideoQualityTestFixtureInterface::InjectionComponents>
  CreateComponents() const;

 private:
  static std::unique_ptr<TestDependencyFactory> instance_;
};

}  // namespace webrtc

#endif  // API_TEST_TEST_DEPENDENCY_FACTORY_H_
