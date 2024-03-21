// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_VERSION_H_
#define BASE_VERSION_H_

#include <stdint.h>

#include <iosfwd>
#include <string>
#include <vector>

#include "base/base_export.h"
#include "base/strings/string_piece.h"

namespace base {

// parsing and comparison.
class BASE_EXPORT Version {
 public:


  Version();

  Version(const Version& other);



  explicit Version(StringPiece version_str);


  explicit Version(std::vector<uint32_t> components);

  ~Version();

  bool IsValid() const;




  static bool IsValidWildcardString(StringPiece wildcard_string);

  int CompareTo(const Version& other) const;




  int CompareToWildcardString(StringPiece wildcard_string) const;

  std::string GetString() const;

  const std::vector<uint32_t>& components() const { return components_; }

 private:
  std::vector<uint32_t> components_;
};

BASE_EXPORT bool operator==(const Version& v1, const Version& v2);
BASE_EXPORT bool operator!=(const Version& v1, const Version& v2);
BASE_EXPORT bool operator<(const Version& v1, const Version& v2);
BASE_EXPORT bool operator<=(const Version& v1, const Version& v2);
BASE_EXPORT bool operator>(const Version& v1, const Version& v2);
BASE_EXPORT bool operator>=(const Version& v1, const Version& v2);
BASE_EXPORT std::ostream& operator<<(std::ostream& stream, const Version& v);

}  // namespace base

#endif  // BASE_VERSION_H_
