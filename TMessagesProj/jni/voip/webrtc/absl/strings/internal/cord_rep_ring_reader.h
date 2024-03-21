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

#ifndef ABSL_STRINGS_INTERNAL_CORD_REP_RING_READER_H_
#define ABSL_STRINGS_INTERNAL_CORD_REP_RING_READER_H_

#include <cassert>
#include <cstddef>
#include <cstdint>

#include "absl/strings/internal/cord_internal.h"
#include "absl/strings/internal/cord_rep_ring.h"
#include "absl/strings/string_view.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace cord_internal {

class CordRepRingReader {
 public:

  explicit operator bool() const { return ring_ != nullptr; }

  CordRepRing* ring() const { return ring_; }


  CordRepRing::index_type index() const { return index_; }


  CordRep* node() const { return ring_->entry_child(index_); }


  size_t length() const {
    assert(ring_);
    return ring_->length;
  }





  size_t consumed() const {
    assert(ring_);
    return ring_->entry_end_offset(index_);
  }


  size_t remaining() const {
    assert(ring_);
    return length() - consumed();
  }

  void Reset() { ring_ = nullptr; }


  absl::string_view Reset(CordRepRing* ring) {
    assert(ring);
    ring_ = ring;
    index_ = ring_->head();
    return ring_->entry_data(index_);
  }



  absl::string_view Next() {
    assert(remaining());
    index_ = ring_->advance(index_);
    return ring_->entry_data(index_);
  }






  absl::string_view Seek(size_t offset) {
    assert(offset < length());
    size_t current = ring_->entry_end_offset(index_);
    CordRepRing::index_type hint = (offset >= current) ? index_ : ring_->head();
    const CordRepRing::Position head = ring_->Find(hint, offset);
    index_ = head.index;
    auto data = ring_->entry_data(head.index);
    data.remove_prefix(head.offset);
    return data;
  }

 private:
  CordRepRing* ring_ = nullptr;
  CordRepRing::index_type index_;
};

}  // namespace cord_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_STRINGS_INTERNAL_CORD_REP_RING_READER_H_
