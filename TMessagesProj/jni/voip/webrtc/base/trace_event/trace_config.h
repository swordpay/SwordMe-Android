// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_TRACE_EVENT_TRACE_CONFIG_H_
#define BASE_TRACE_EVENT_TRACE_CONFIG_H_

#include <stdint.h>

#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#include "base/base_export.h"
#include "base/gtest_prod_util.h"
#include "base/strings/string_piece.h"
#include "base/trace_event/memory_dump_request_args.h"
#include "base/trace_event/trace_config_category_filter.h"
#include "base/values.h"

namespace base {
namespace trace_event {

class ConvertableToTraceFormat;

// A Java counterpart will be generated for this enum.
// GENERATED_JAVA_ENUM_PACKAGE: org.chromium.base
enum TraceRecordMode {

  RECORD_UNTIL_FULL,


  RECORD_CONTINUOUSLY,

  RECORD_AS_MUCH_AS_POSSIBLE,

  ECHO_TO_CONSOLE,
};

class BASE_EXPORT TraceConfig {
 public:
  using StringList = std::vector<std::string>;


  struct BASE_EXPORT MemoryDumpConfig {
    MemoryDumpConfig();
    MemoryDumpConfig(const MemoryDumpConfig& other);
    ~MemoryDumpConfig();

    struct Trigger {
      uint32_t min_time_between_dumps_ms;
      MemoryDumpLevelOfDetail level_of_detail;
      MemoryDumpType trigger_type;
    };

    struct HeapProfiler {

      enum { kDefaultBreakdownThresholdBytes = 1024 };

      HeapProfiler();

      void Clear();

      uint32_t breakdown_threshold_bytes;
    };

    void Clear();

    void Merge(const MemoryDumpConfig& config);



    std::set<MemoryDumpLevelOfDetail> allowed_dump_modes;

    std::vector<Trigger> triggers;
    HeapProfiler heap_profiler_options;
  };

  class BASE_EXPORT ProcessFilterConfig {
   public:
    ProcessFilterConfig();
    explicit ProcessFilterConfig(
        const std::unordered_set<base::ProcessId>& included_process_ids);
    ProcessFilterConfig(const ProcessFilterConfig&);
    ~ProcessFilterConfig();

    bool empty() const { return included_process_ids_.empty(); }

    void Clear();
    void Merge(const ProcessFilterConfig&);

    void InitializeFromConfigDict(const Value&);
    void ToDict(Value*) const;

    bool IsEnabled(base::ProcessId) const;
    const std::unordered_set<base::ProcessId>& included_process_ids() const {
      return included_process_ids_;
    }

    bool operator==(const ProcessFilterConfig& other) const {
      return included_process_ids_ == other.included_process_ids_;
    }

   private:
    std::unordered_set<base::ProcessId> included_process_ids_;
  };

  class BASE_EXPORT EventFilterConfig {
   public:
    explicit EventFilterConfig(const std::string& predicate_name);
    EventFilterConfig(const EventFilterConfig& tc);

    ~EventFilterConfig();

    EventFilterConfig& operator=(const EventFilterConfig& rhs);

    void InitializeFromConfigDict(const Value& event_filter);

    void SetCategoryFilter(const TraceConfigCategoryFilter& category_filter);

    void ToDict(Value* filter_dict) const;

    bool GetArgAsSet(const char* key, std::unordered_set<std::string>*) const;

    bool IsCategoryGroupEnabled(const StringPiece& category_group_name) const;

    const std::string& predicate_name() const { return predicate_name_; }
    const Value& filter_args() const { return args_; }
    const TraceConfigCategoryFilter& category_filter() const {
      return category_filter_;
    }

   private:
    std::string predicate_name_;
    TraceConfigCategoryFilter category_filter_;
    Value args_;
  };
  typedef std::vector<EventFilterConfig> EventFilters;

  static std::string TraceRecordModeToStr(TraceRecordMode record_mode);

  TraceConfig();
































  TraceConfig(StringPiece category_filter_string,
              StringPiece trace_options_string);

  TraceConfig(StringPiece category_filter_string, TraceRecordMode record_mode);


























  explicit TraceConfig(StringPiece config_string);


  explicit TraceConfig(const Value& config);

  TraceConfig(const TraceConfig& tc);

  ~TraceConfig();

  TraceConfig& operator=(const TraceConfig& rhs);

  TraceRecordMode GetTraceRecordMode() const { return record_mode_; }
  size_t GetTraceBufferSizeInEvents() const {
    return trace_buffer_size_in_events_;
  }
  size_t GetTraceBufferSizeInKb() const { return trace_buffer_size_in_kb_; }
  bool IsSystraceEnabled() const { return enable_systrace_; }
  bool IsArgumentFilterEnabled() const { return enable_argument_filter_; }

  void SetTraceRecordMode(TraceRecordMode mode) { record_mode_ = mode; }
  void SetTraceBufferSizeInEvents(size_t size) {
    trace_buffer_size_in_events_ = size;
  }
  void SetTraceBufferSizeInKb(size_t size) { trace_buffer_size_in_kb_ = size; }
  void EnableSystrace() { enable_systrace_ = true; }
  void EnableSystraceEvent(const std::string& systrace_event);
  void EnableArgumentFilter() { enable_argument_filter_ = true; }
  void EnableHistogram(const std::string& histogram_name);


  std::string ToString() const;

  std::unique_ptr<ConvertableToTraceFormat> AsConvertableToTraceFormat() const;

  std::string ToCategoryFilterString() const;



  std::string ToTraceOptionsString() const;



  bool IsCategoryGroupEnabled(const StringPiece& category_group_name) const;

  void Merge(const TraceConfig& config);

  void Clear();

  void ResetMemoryDumpConfig(const MemoryDumpConfig& memory_dump_config);

  const TraceConfigCategoryFilter& category_filter() const {
    return category_filter_;
  }

  const MemoryDumpConfig& memory_dump_config() const {
    return memory_dump_config_;
  }

  const ProcessFilterConfig& process_filter_config() const {
    return process_filter_config_;
  }
  void SetProcessFilterConfig(const ProcessFilterConfig&);

  const EventFilters& event_filters() const { return event_filters_; }
  void SetEventFilters(const EventFilters& filter_configs) {
    event_filters_ = filter_configs;
  }

  const std::unordered_set<std::string>& systrace_events() const {
    return systrace_events_;
  }

  const std::unordered_set<std::string>& histogram_names() const {
    return histogram_names_;
  }

 private:
  FRIEND_TEST_ALL_PREFIXES(TraceConfigTest, TraceConfigFromValidLegacyFormat);
  FRIEND_TEST_ALL_PREFIXES(TraceConfigTest,
                           TraceConfigFromInvalidLegacyStrings);
  FRIEND_TEST_ALL_PREFIXES(TraceConfigTest, SystraceEventsSerialization);



  void InitializeDefault();

  void InitializeFromConfigDict(const Value& dict);

  void InitializeFromConfigString(StringPiece config_string);

  void InitializeFromStrings(StringPiece category_filter_string,
                             StringPiece trace_options_string);

  void SetMemoryDumpConfigFromConfigDict(const Value& memory_dump_config);
  void SetDefaultMemoryDumpConfig();

  void SetHistogramNamesFromConfigList(const Value& histogram_names);
  void SetEventFiltersFromConfigList(const Value& event_filters);
  Value ToValue() const;

  TraceRecordMode record_mode_;
  size_t trace_buffer_size_in_events_ = 0;  // 0 specifies default size
  size_t trace_buffer_size_in_kb_ = 0;      // 0 specifies default size
  bool enable_systrace_ : 1;
  bool enable_argument_filter_ : 1;

  TraceConfigCategoryFilter category_filter_;

  MemoryDumpConfig memory_dump_config_;
  ProcessFilterConfig process_filter_config_;

  EventFilters event_filters_;
  std::unordered_set<std::string> histogram_names_;
  std::unordered_set<std::string> systrace_events_;
};

}  // namespace trace_event
}  // namespace base

#endif  // BASE_TRACE_EVENT_TRACE_CONFIG_H_
