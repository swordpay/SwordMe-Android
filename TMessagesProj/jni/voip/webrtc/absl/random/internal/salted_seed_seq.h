// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef ABSL_RANDOM_INTERNAL_SALTED_SEED_SEQ_H_
#define ABSL_RANDOM_INTERNAL_SALTED_SEED_SEQ_H_

#include <cstdint>
#include <cstdlib>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/container/inlined_vector.h"
#include "absl/meta/type_traits.h"
#include "absl/random/internal/seed_material.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace random_internal {

// [rand.req.seedseq].
//
// A `SaltedSeedSeq` is meant to wrap an existing seed sequence and modify
// generated sequence by mixing with extra entropy. This entropy may be
// build-dependent or process-dependent. The implementation may change to be
// have either or both kinds of entropy. If salt is not available sequence is
// not modified.
template <typename SSeq>
class SaltedSeedSeq {
 public:
  using inner_sequence_type = SSeq;
  using result_type = typename SSeq::result_type;

  SaltedSeedSeq() : seq_(absl::make_unique<SSeq>()) {}

  template <typename Iterator>
  SaltedSeedSeq(Iterator begin, Iterator end)
      : seq_(absl::make_unique<SSeq>(begin, end)) {}

  template <typename T>
  SaltedSeedSeq(std::initializer_list<T> il)
      : SaltedSeedSeq(il.begin(), il.end()) {}

  SaltedSeedSeq(const SaltedSeedSeq&) = delete;
  SaltedSeedSeq& operator=(const SaltedSeedSeq&) = delete;

  SaltedSeedSeq(SaltedSeedSeq&&) = default;
  SaltedSeedSeq& operator=(SaltedSeedSeq&&) = default;

  template <typename RandomAccessIterator>
  void generate(RandomAccessIterator begin, RandomAccessIterator end) {
    using U = typename std::iterator_traits<RandomAccessIterator>::value_type;



    using TagType = absl::conditional_t<
        (std::is_same<U, uint32_t>::value &&
         (std::is_pointer<RandomAccessIterator>::value ||
          std::is_same<RandomAccessIterator,
                       typename std::vector<U>::iterator>::value)),
        ContiguousAndUint32Tag, DefaultTag>;
    if (begin != end) {
      generate_impl(TagType{}, begin, end, std::distance(begin, end));
    }
  }

  template <typename OutIterator>
  void param(OutIterator out) const {
    seq_->param(out);
  }

  size_t size() const { return seq_->size(); }

 private:
  struct ContiguousAndUint32Tag {};
  struct DefaultTag {};



  template <typename Contiguous>
  void generate_impl(ContiguousAndUint32Tag, Contiguous begin, Contiguous end,
                     size_t n) {
    seq_->generate(begin, end);
    const uint32_t salt = absl::random_internal::GetSaltMaterial().value_or(0);
    auto span = absl::Span<uint32_t>(&*begin, n);
    MixIntoSeedMaterial(absl::MakeConstSpan(&salt, 1), span);
  }




  template <typename RandomAccessIterator>
  void generate_impl(DefaultTag, RandomAccessIterator begin,
                     RandomAccessIterator, size_t n) {


    absl::InlinedVector<uint32_t, 8> data(n, 0);
    generate_impl(ContiguousAndUint32Tag{}, data.begin(), data.end(), n);
    std::copy(data.begin(), data.end(), begin);
  }



  std::unique_ptr<SSeq> seq_;
};

template <typename T, typename = void>
struct is_salted_seed_seq : public std::false_type {};

template <typename T>
struct is_salted_seed_seq<
    T, typename std::enable_if<std::is_same<
           T, SaltedSeedSeq<typename T::inner_sequence_type>>::value>::type>
    : public std::true_type {};

// When provided with an existing SaltedSeedSeq, returns the input parameter,
// otherwise constructs a new SaltedSeedSeq which embodies the original
// non-salted seed parameters.
template <
    typename SSeq,  //
    typename EnableIf = absl::enable_if_t<is_salted_seed_seq<SSeq>::value>>
SSeq MakeSaltedSeedSeq(SSeq&& seq) {
  return SSeq(std::forward<SSeq>(seq));
}

template <
    typename SSeq,  //
    typename EnableIf = absl::enable_if_t<!is_salted_seed_seq<SSeq>::value>>
SaltedSeedSeq<typename std::decay<SSeq>::type> MakeSaltedSeedSeq(SSeq&& seq) {
  using sseq_type = typename std::decay<SSeq>::type;
  using result_type = typename sseq_type::result_type;

  absl::InlinedVector<result_type, 8> data;
  seq.param(std::back_inserter(data));
  return SaltedSeedSeq<sseq_type>(data.begin(), data.end());
}

}  // namespace random_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_RANDOM_INTERNAL_SALTED_SEED_SEQ_H_
