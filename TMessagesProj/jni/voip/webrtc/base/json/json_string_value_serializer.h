// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_JSON_JSON_STRING_VALUE_SERIALIZER_H_
#define BASE_JSON_JSON_STRING_VALUE_SERIALIZER_H_

#include <string>

#include "base/base_export.h"
#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "base/values.h"

class BASE_EXPORT JSONStringValueSerializer : public base::ValueSerializer {
 public:



  explicit JSONStringValueSerializer(std::string* json_string);

  ~JSONStringValueSerializer() override;



  bool Serialize(const base::Value& root) override;


  bool SerializeAndOmitBinaryValues(const base::Value& root);

  void set_pretty_print(bool new_value) { pretty_print_ = new_value; }
  bool pretty_print() { return pretty_print_; }

 private:
  bool SerializeInternal(const base::Value& root, bool omit_binary_values);

  std::string* json_string_;
  bool pretty_print_;  // If true, serialization will span multiple lines.

  DISALLOW_COPY_AND_ASSIGN(JSONStringValueSerializer);
};

class BASE_EXPORT JSONStringValueDeserializer : public base::ValueDeserializer {
 public:



  explicit JSONStringValueDeserializer(const base::StringPiece& json_string,
                                       int options = 0);

  ~JSONStringValueDeserializer() override;







  std::unique_ptr<base::Value> Deserialize(int* error_code,
                                           std::string* error_message) override;

 private:

  base::StringPiece json_string_;
  const int options_;

  DISALLOW_COPY_AND_ASSIGN(JSONStringValueDeserializer);
};

#endif  // BASE_JSON_JSON_STRING_VALUE_SERIALIZER_H_
