// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/json/json_writer.h"

#include <stdint.h>

#include <cmath>
#include <limits>

#include "base/json/string_escape.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"
#include "base/values.h"
#include "build/build_config.h"

namespace base {

#if defined(OS_WIN)
const char kPrettyPrintLineEnding[] = "\r\n";
#else
const char kPrettyPrintLineEnding[] = "\n";
#endif

bool JSONWriter::Write(const Value& node, std::string* json, size_t max_depth) {
  return WriteWithOptions(node, 0, json, max_depth);
}

bool JSONWriter::WriteWithOptions(const Value& node,
                                  int options,
                                  std::string* json,
                                  size_t max_depth) {
  json->clear();

  json->reserve(1024);

  JSONWriter writer(options, json, max_depth);
  bool result = writer.BuildJSONString(node, 0U);

  if (options & OPTIONS_PRETTY_PRINT)
    json->append(kPrettyPrintLineEnding);

  return result;
}

JSONWriter::JSONWriter(int options, std::string* json, size_t max_depth)
    : omit_binary_values_((options & OPTIONS_OMIT_BINARY_VALUES) != 0),
      omit_double_type_preservation_(
          (options & OPTIONS_OMIT_DOUBLE_TYPE_PRESERVATION) != 0),
      pretty_print_((options & OPTIONS_PRETTY_PRINT) != 0),
      json_string_(json),
      max_depth_(max_depth),
      stack_depth_(0) {
  DCHECK(json);
  CHECK_LE(max_depth, internal::kAbsoluteMaxDepth);
}

bool JSONWriter::BuildJSONString(const Value& node, size_t depth) {
  internal::StackMarker depth_check(max_depth_, &stack_depth_);
  if (depth_check.IsTooDeep())
    return false;

  switch (node.type()) {
    case Value::Type::NONE:
      json_string_->append("null");
      return true;

    case Value::Type::BOOLEAN:


      json_string_->append(node.GetBool() ? std::string("true")
                                          : std::string("false"));
      return true;

    case Value::Type::INTEGER:
      json_string_->append(NumberToString(node.GetInt()));
      return true;

    case Value::Type::DOUBLE: {
      double value = node.GetDouble();
      if (omit_double_type_preservation_ &&
          value <= std::numeric_limits<int64_t>::max() &&
          value >= std::numeric_limits<int64_t>::min() &&
          std::floor(value) == value) {
        json_string_->append(NumberToString(static_cast<int64_t>(value)));
        return true;
      }
      std::string real = NumberToString(value);



      if (real.find_first_of(".eE") == std::string::npos)
        real.append(".0");


      if (real[0] == '.') {
        real.insert(static_cast<size_t>(0), static_cast<size_t>(1), '0');
      } else if (real.length() > 1 && real[0] == '-' && real[1] == '.') {

        real.insert(static_cast<size_t>(1), static_cast<size_t>(1), '0');
      }
      json_string_->append(real);
      return true;
    }

    case Value::Type::STRING:
      EscapeJSONString(node.GetString(), true, json_string_);
      return true;

    case Value::Type::LIST: {
      json_string_->push_back('[');
      if (pretty_print_)
        json_string_->push_back(' ');

      bool first_value_has_been_output = false;
      bool result = true;
      for (const auto& value : node.GetList()) {
        if (omit_binary_values_ && value.type() == Value::Type::BINARY)
          continue;

        if (first_value_has_been_output) {
          json_string_->push_back(',');
          if (pretty_print_)
            json_string_->push_back(' ');
        }

        if (!BuildJSONString(value, depth))
          result = false;

        first_value_has_been_output = true;
      }

      if (pretty_print_)
        json_string_->push_back(' ');
      json_string_->push_back(']');
      return result;
    }

    case Value::Type::DICTIONARY: {
      json_string_->push_back('{');
      if (pretty_print_)
        json_string_->append(kPrettyPrintLineEnding);

      bool first_value_has_been_output = false;
      bool result = true;
      for (const auto& pair : node.DictItems()) {
        const auto& key = pair.first;
        const auto& value = pair.second;
        if (omit_binary_values_ && value.type() == Value::Type::BINARY)
          continue;

        if (first_value_has_been_output) {
          json_string_->push_back(',');
          if (pretty_print_)
            json_string_->append(kPrettyPrintLineEnding);
        }

        if (pretty_print_)
          IndentLine(depth + 1U);

        EscapeJSONString(key, true, json_string_);
        json_string_->push_back(':');
        if (pretty_print_)
          json_string_->push_back(' ');

        if (!BuildJSONString(value, depth + 1U))
          result = false;

        first_value_has_been_output = true;
      }

      if (pretty_print_) {
        json_string_->append(kPrettyPrintLineEnding);
        IndentLine(depth);
      }

      json_string_->push_back('}');
      return result;
    }

    case Value::Type::BINARY:

      DLOG_IF(ERROR, !omit_binary_values_) << "Cannot serialize binary value.";
      return omit_binary_values_;

    case Value::Type::DEAD:
      CHECK(false);
      return false;
  }

  CHECK(false);
  return false;
}

void JSONWriter::IndentLine(size_t depth) {
  json_string_->append(depth * 3U, ' ');
}

}  // namespace base
