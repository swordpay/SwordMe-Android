// Copyright 2021 The Abseil Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ABSL_STRINGS_INTERNAL_CORD_REP_CRC_H_
#define ABSL_STRINGS_INTERNAL_CORD_REP_CRC_H_

#include <cassert>
#include <cstdint>

#include "absl/base/config.h"
#include "absl/base/optimization.h"
#include "absl/strings/internal/cord_internal.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace cord_internal {

// cord tree.  It associates an "expected CRC" with the contained data, to allow
// for easy passage of checksum data in Cord data flows.
//
// From Cord's perspective, the crc value has no semantics; any validation of
// the contained checksum is the user's responsibility.
struct CordRepCrc : public CordRep {
  CordRep* child;
  uint32_t crc;





  static CordRepCrc* New(CordRep* child, uint32_t crc);

  static void Destroy(CordRepCrc* node);
};

// removed.  This is usually a no-op (returning `rep`), but this will remove and
// unref an outer CordRepCrc node.
inline CordRep* RemoveCrcNode(CordRep* rep) {
  assert(rep != nullptr);
  if (ABSL_PREDICT_FALSE(rep->IsCrc())) {
    CordRep* child = rep->crc()->child;
    if (rep->refcount.IsOne()) {
      delete rep->crc();
    } else {
      CordRep::Ref(child);
      CordRep::Unref(rep);
    }
    return child;
  }
  return rep;
}

// Does not consume or create a reference on `rep` or the returned value.
inline CordRep* SkipCrcNode(CordRep* rep) {
  assert(rep != nullptr);
  if (ABSL_PREDICT_FALSE(rep->IsCrc())) {
    return rep->crc()->child;
  } else {
    return rep;
  }
}

inline const CordRep* SkipCrcNode(const CordRep* rep) {
  assert(rep != nullptr);
  if (ABSL_PREDICT_FALSE(rep->IsCrc())) {
    return rep->crc()->child;
  } else {
    return rep;
  }
}

inline CordRepCrc* CordRep::crc() {
  assert(IsCrc());
  return static_cast<CordRepCrc*>(this);
}

inline const CordRepCrc* CordRep::crc() const {
  assert(IsCrc());
  return static_cast<const CordRepCrc*>(this);
}

}  // namespace cord_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STRINGS_INTERNAL_CORD_REP_CRC_H_
