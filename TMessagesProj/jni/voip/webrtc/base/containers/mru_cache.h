// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// constant-time access to items using a key, but easy identification of the
// least-recently-used items for removal.  Each key can only be associated with
// one payload item at a time.
//
// The key object will be stored twice, so it should support efficient copying.
//
// NOTE: While all operations are O(1), this code is written for
// legibility rather than optimality. If future profiling identifies this as
// a bottleneck, there is room for smaller values of 1 in the O(1). :]

#ifndef BASE_CONTAINERS_MRU_CACHE_H_
#define BASE_CONTAINERS_MRU_CACHE_H_

#include <stddef.h>

#include <algorithm>
#include <functional>
#include <list>
#include <map>
#include <unordered_map>
#include <utility>

#include "base/logging.h"
#include "base/macros.h"

namespace base {
namespace trace_event {
namespace internal {

template <class MruCacheType>
size_t DoEstimateMemoryUsageForMruCache(const MruCacheType&);

}  // namespace internal
}  // namespace trace_event


// by MRUCacheBase. This level of indirection is necessary because of the way
// that template template params and default template params interact.
template <class KeyType, class ValueType, class CompareType>
struct MRUCacheStandardMap {
  typedef std::map<KeyType, ValueType, CompareType> Type;
};

template <class KeyType,
          class PayloadType,
          class HashOrCompareType,
          template <typename, typename, typename> class MapType =
              MRUCacheStandardMap>
class MRUCacheBase {
 public:


  typedef std::pair<KeyType, PayloadType> value_type;

 private:
  typedef std::list<value_type> PayloadList;
  typedef typename MapType<KeyType,
                           typename PayloadList::iterator,
                           HashOrCompareType>::Type KeyIndex;

 public:
  typedef typename PayloadList::size_type size_type;

  typedef typename PayloadList::iterator iterator;
  typedef typename PayloadList::const_iterator const_iterator;
  typedef typename PayloadList::reverse_iterator reverse_iterator;
  typedef typename PayloadList::const_reverse_iterator const_reverse_iterator;

  enum { NO_AUTO_EVICT = 0 };




  explicit MRUCacheBase(size_type max_size) : max_size_(max_size) {}

  virtual ~MRUCacheBase() = default;

  size_type max_size() const { return max_size_; }





  template <typename Payload>
  iterator Put(const KeyType& key, Payload&& payload) {

    typename KeyIndex::iterator index_iter = index_.find(key);
    if (index_iter != index_.end()) {


      Erase(index_iter->second);
    } else if (max_size_ != NO_AUTO_EVICT) {


      ShrinkToSize(max_size_ - 1);
    }

    ordering_.emplace_front(key, std::forward<Payload>(payload));
    index_.emplace(key, ordering_.begin());
    return ordering_.begin();
  }



  iterator Get(const KeyType& key) {
    typename KeyIndex::iterator index_iter = index_.find(key);
    if (index_iter == index_.end())
      return end();
    typename PayloadList::iterator iter = index_iter->second;

    ordering_.splice(ordering_.begin(), ordering_, iter);
    return ordering_.begin();
  }


  iterator Peek(const KeyType& key) {
    typename KeyIndex::const_iterator index_iter = index_.find(key);
    if (index_iter == index_.end())
      return end();
    return index_iter->second;
  }

  const_iterator Peek(const KeyType& key) const {
    typename KeyIndex::const_iterator index_iter = index_.find(key);
    if (index_iter == index_.end())
      return end();
    return index_iter->second;
  }

  void Swap(MRUCacheBase& other) {
    ordering_.swap(other.ordering_);
    index_.swap(other.index_);
    std::swap(max_size_, other.max_size_);
  }


  iterator Erase(iterator pos) {
    index_.erase(pos->first);
    return ordering_.erase(pos);
  }


  reverse_iterator Erase(reverse_iterator pos) {



    return reverse_iterator(Erase((++pos).base()));
  }


  void ShrinkToSize(size_type new_size) {
    for (size_type i = size(); i > new_size; i--)
      Erase(rbegin());
  }

  void Clear() {
    index_.clear();
    ordering_.clear();
  }

  size_type size() const {


    DCHECK(index_.size() == ordering_.size());
    return index_.size();
  }






  iterator begin() { return ordering_.begin(); }
  const_iterator begin() const { return ordering_.begin(); }
  iterator end() { return ordering_.end(); }
  const_iterator end() const { return ordering_.end(); }

  reverse_iterator rbegin() { return ordering_.rbegin(); }
  const_reverse_iterator rbegin() const { return ordering_.rbegin(); }
  reverse_iterator rend() { return ordering_.rend(); }
  const_reverse_iterator rend() const { return ordering_.rend(); }

  bool empty() const { return ordering_.empty(); }

 private:
  template <class MruCacheType>
  friend size_t trace_event::internal::DoEstimateMemoryUsageForMruCache(
      const MruCacheType&);

  PayloadList ordering_;
  KeyIndex index_;

  size_type max_size_;

  DISALLOW_COPY_AND_ASSIGN(MRUCacheBase);
};


// value types (as opposed to pointers) in the list.
template <class KeyType,
          class PayloadType,
          class CompareType = std::less<KeyType>>
class MRUCache : public MRUCacheBase<KeyType, PayloadType, CompareType> {
 private:
  using ParentType = MRUCacheBase<KeyType, PayloadType, CompareType>;

 public:

  explicit MRUCache(typename ParentType::size_type max_size)
      : ParentType(max_size) {}
  virtual ~MRUCache() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(MRUCache);
};


template <class KeyType, class ValueType, class HashType>
struct MRUCacheHashMap {
  typedef std::unordered_map<KeyType, ValueType, HashType> Type;
};

// the map type instead of std::map. Note that your KeyType must be hashable to
// use this cache or you need to provide a hashing class.
template <class KeyType, class PayloadType, class HashType = std::hash<KeyType>>
class HashingMRUCache
    : public MRUCacheBase<KeyType, PayloadType, HashType, MRUCacheHashMap> {
 private:
  using ParentType =
      MRUCacheBase<KeyType, PayloadType, HashType, MRUCacheHashMap>;

 public:

  explicit HashingMRUCache(typename ParentType::size_type max_size)
      : ParentType(max_size) {}
  virtual ~HashingMRUCache() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(HashingMRUCache);
};

}  // namespace base

#endif  // BASE_CONTAINERS_MRU_CACHE_H_
