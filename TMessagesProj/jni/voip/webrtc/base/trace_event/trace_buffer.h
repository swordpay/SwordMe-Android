// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_TRACE_BUFFER_H_
#define BASE_TRACE_EVENT_TRACE_BUFFER_H_

#include <stddef.h>
#include <stdint.h>

#include "base/base_export.h"
#include "base/trace_event/trace_event.h"
#include "base/trace_event/trace_event_impl.h"

namespace base {

namespace trace_event {

class BASE_EXPORT TraceBufferChunk {
 public:
  explicit TraceBufferChunk(uint32_t seq);
  ~TraceBufferChunk();

  void Reset(uint32_t new_seq);
  TraceEvent* AddTraceEvent(size_t* event_index);
  bool IsFull() const { return next_free_ == kTraceBufferChunkSize; }

  uint32_t seq() const { return seq_; }
  size_t capacity() const { return kTraceBufferChunkSize; }
  size_t size() const { return next_free_; }

  TraceEvent* GetEventAt(size_t index) {
    DCHECK(index < size());
    return &chunk_[index];
  }
  const TraceEvent* GetEventAt(size_t index) const {
    DCHECK(index < size());
    return &chunk_[index];
  }

  void EstimateTraceMemoryOverhead(TraceEventMemoryOverhead* overhead);



  static const size_t kMaxChunkIndex = (1u << 26) - 1;
  static const size_t kTraceBufferChunkSize = 64;

 private:
  size_t next_free_;
  std::unique_ptr<TraceEventMemoryOverhead> cached_overhead_estimate_;
  TraceEvent chunk_[kTraceBufferChunkSize];
  uint32_t seq_;
};

class BASE_EXPORT TraceBuffer {
 public:
  virtual ~TraceBuffer() = default;

  virtual std::unique_ptr<TraceBufferChunk> GetChunk(size_t* index) = 0;
  virtual void ReturnChunk(size_t index,
                           std::unique_ptr<TraceBufferChunk> chunk) = 0;

  virtual bool IsFull() const = 0;
  virtual size_t Size() const = 0;
  virtual size_t Capacity() const = 0;
  virtual TraceEvent* GetEventByHandle(TraceEventHandle handle) = 0;

  virtual const TraceBufferChunk* NextChunk() = 0;


  virtual void EstimateTraceMemoryOverhead(
      TraceEventMemoryOverhead* overhead) = 0;

  static TraceBuffer* CreateTraceBufferRingBuffer(size_t max_chunks);
  static TraceBuffer* CreateTraceBufferVectorOfSize(size_t max_chunks);
};

// to JSON output.
class BASE_EXPORT TraceResultBuffer {
 public:
  using OutputCallback = base::RepeatingCallback<void(const std::string&)>;



  struct BASE_EXPORT SimpleOutput {
    OutputCallback GetCallback();
    void Append(const std::string& json_string);


    std::string json_output;
  };

  TraceResultBuffer();
  ~TraceResultBuffer();




  void SetOutputCallback(OutputCallback json_chunk_callback);


  void Start();

  void AddFragment(const std::string& trace_fragment);


  void Finish();

 private:
  OutputCallback output_callback_;
  bool append_comma_;
};

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_TRACE_BUFFER_H_
