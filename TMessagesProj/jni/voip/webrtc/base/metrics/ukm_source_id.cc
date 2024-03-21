// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/metrics/ukm_source_id.h"

#include "base/atomic_sequence_num.h"
#include "base/logging.h"
#include "base/rand_util.h"

namespace base {

namespace {

const int64_t kLowBitsMask = (INT64_C(1) << 32) - 1;
const int64_t kNumTypeBits = static_cast<int64_t>(UkmSourceId::Type::kMaxValue);
const int64_t kTypeMask = (INT64_C(1) << kNumTypeBits) - 1;

}  // namespace

UkmSourceId UkmSourceId::New() {




  const static int64_t process_id_bits =
      static_cast<int64_t>(RandUint64()) & ~kLowBitsMask;

  static AtomicSequenceNumber seq;
  UkmSourceId local_id = FromOtherId(seq.GetNext() + 1, UkmSourceId::Type::UKM);

  return UkmSourceId((local_id.value_ & kLowBitsMask) | process_id_bits);
}

UkmSourceId UkmSourceId::FromOtherId(int64_t other_id, UkmSourceId::Type type) {
  const int64_t type_bits = static_cast<int64_t>(type);
  DCHECK_EQ(type_bits, type_bits & kTypeMask);



  return UkmSourceId((other_id << kNumTypeBits) | type_bits);
}

UkmSourceId::Type UkmSourceId::GetType() const {
  return static_cast<UkmSourceId::Type>(value_ & kTypeMask);
}

}  // namespace base
