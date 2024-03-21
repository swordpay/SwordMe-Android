// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_JSON_JSON_FILE_VALUE_SERIALIZER_H_
#define BASE_JSON_JSON_FILE_VALUE_SERIALIZER_H_

#include <stddef.h>

#include <string>

#include "base/base_export.h"
#include "base/files/file_path.h"
#include "base/macros.h"
#include "base/values.h"

class BASE_EXPORT JSONFileValueSerializer : public base::ValueSerializer {
 public:



  explicit JSONFileValueSerializer(const base::FilePath& json_file_path);

  ~JSONFileValueSerializer() override;








  bool Serialize(const base::Value& root) override;


  bool SerializeAndOmitBinaryValues(const base::Value& root);

 private:
  bool SerializeInternal(const base::Value& root, bool omit_binary_values);

  const base::FilePath json_file_path_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(JSONFileValueSerializer);
};

class BASE_EXPORT JSONFileValueDeserializer : public base::ValueDeserializer {
 public:


  explicit JSONFileValueDeserializer(const base::FilePath& json_file_path,
                                     int options = 0);

  ~JSONFileValueDeserializer() override;







  std::unique_ptr<base::Value> Deserialize(int* error_code,
                                           std::string* error_message) override;

  enum JsonFileError {
    JSON_NO_ERROR = 0,
    JSON_ACCESS_DENIED = 1000,
    JSON_CANNOT_READ_FILE,
    JSON_FILE_LOCKED,
    JSON_NO_SUCH_FILE
  };

  static const char kAccessDenied[];
  static const char kCannotReadFile[];
  static const char kFileLocked[];
  static const char kNoSuchFile[];


  static const char* GetErrorMessageForCode(int error_code);


  size_t get_last_read_size() const { return last_read_size_; }

 private:


  int ReadFileToString(std::string* json_string);

  const base::FilePath json_file_path_;
  const int options_;
  size_t last_read_size_;

  DISALLOW_IMPLICIT_CONSTRUCTORS(JSONFileValueDeserializer);
};

#endif  // BASE_JSON_JSON_FILE_VALUE_SERIALIZER_H_

