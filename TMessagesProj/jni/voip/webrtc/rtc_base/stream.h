/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef RTC_BASE_STREAM_H_
#define RTC_BASE_STREAM_H_

#include <memory>

#include "rtc_base/buffer.h"
#include "rtc_base/system/rtc_export.h"
#include "rtc_base/third_party/sigslot/sigslot.h"
#include "rtc_base/thread.h"

namespace rtc {

// StreamInterface is a generic asynchronous stream interface, supporting read,
// write, and close operations, and asynchronous signalling of state changes.
// The interface is designed with file, memory, and socket implementations in
// mind.  Some implementations offer extended operations, such as seeking.
///////////////////////////////////////////////////////////////////////////////

// class for brevity in use.

// in the future.
enum StreamState { SS_CLOSED, SS_OPENING, SS_OPEN };

// and failure conditions described below.
enum StreamResult { SR_ERROR, SR_SUCCESS, SR_BLOCK, SR_EOS };

// may be combined.
//  SE_OPEN: The stream has transitioned to the SS_OPEN state
//  SE_CLOSE: The stream has transitioned to the SS_CLOSED state
//  SE_READ: Data is available, so Read is likely to not return SR_BLOCK
//  SE_WRITE: Data can be written, so Write is likely to not return SR_BLOCK
enum StreamEvent { SE_OPEN = 1, SE_READ = 2, SE_WRITE = 4, SE_CLOSE = 8 };

class RTC_EXPORT StreamInterface {
 public:
  virtual ~StreamInterface() {}

  StreamInterface(const StreamInterface&) = delete;
  StreamInterface& operator=(const StreamInterface&) = delete;

  virtual StreamState GetState() const = 0;













  virtual StreamResult Read(void* buffer,
                            size_t buffer_len,
                            size_t* read,
                            int* error) = 0;
  virtual StreamResult Write(const void* data,
                             size_t data_len,
                             size_t* written,
                             int* error) = 0;


  virtual void Close() = 0;








  sigslot::signal3<StreamInterface*, int, int> SignalEvent;

  virtual bool Flush();










  StreamResult WriteAll(const void* data,
                        size_t data_len,
                        size_t* written,
                        int* error);

 protected:
  StreamInterface();
};

}  // namespace rtc

#endif  // RTC_BASE_STREAM_H_
