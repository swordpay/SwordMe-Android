// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_JSON_JSON_WRITER_H_
#define BASE_JSON_JSON_WRITER_H_

#include <stddef.h>

#include <string>

#include "base/base_export.h"
#include "base/json/json_common.h"
#include "base/macros.h"

namespace base {

class Value;

class BASE_EXPORT JSONWriter {
 public:
  enum Options {




    OPTIONS_OMIT_BINARY_VALUES = 1 << 0,




    OPTIONS_OMIT_DOUBLE_TYPE_PRESERVATION = 1 << 1,


    OPTIONS_PRETTY_PRINT = 1 << 2,
  };






  static bool Write(const Value& node,
                    std::string* json,
                    size_t max_depth = internal::kAbsoluteMaxDepth);


  static bool WriteWithOptions(const Value& node,
                               int options,
                               std::string* json,
                               size_t max_depth = internal::kAbsoluteMaxDepth);

 private:
  JSONWriter(int options,
             std::string* json,
             size_t max_depth = internal::kAbsoluteMaxDepth);


  bool BuildJSONString(const Value& node, size_t depth);

  void IndentLine(size_t depth);

  bool omit_binary_values_;
  bool omit_double_type_preservation_;
  bool pretty_print_;

  std::string* json_string_;

  const size_t max_depth_;

  size_t stack_depth_;

  DISALLOW_COPY_AND_ASSIGN(JSONWriter);
};

}  // namespace base

#endif  // BASE_JSON_JSON_WRITER_H_
