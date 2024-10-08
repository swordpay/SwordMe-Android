// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_SCOPED_GENERIC_H_
#define BASE_SCOPED_GENERIC_H_

#include <stdlib.h>

#include <algorithm>

#include "base/compiler_specific.h"
#include "base/logging.h"
#include "base/macros.h"

namespace base {

// less fancy in some of the more escoteric respects) except that it keeps a
// copy of the object rather than a pointer, and we require that the contained
// object has some kind of "invalid" value.
//
// Defining a scoper based on this class allows you to get a scoper for
// non-pointer types without having to write custom code for set, reset, and
// move, etc. and get almost identical semantics that people are used to from
// unique_ptr.
//
// It is intended that you will typedef this class with an appropriate deleter
// to implement clean up tasks for objects that act like pointers from a
// resource management standpoint but aren't, such as file descriptors and
// various types of operating system handles. Using unique_ptr for these
// things requires that you keep a pointer to the handle valid for the lifetime
// of the scoper (which is easy to mess up).
//
// For an object to be able to be put into a ScopedGeneric, it must support
// standard copyable semantics and have a specific "invalid" value. The traits
// must define a free function and also the invalid value to assign for
// default-constructed and released objects.
//
//   struct FooScopedTraits {
//     // It's assumed that this is a fast inline function with little-to-no
//     // penalty for duplicate calls. This must be a static function even
//     // for stateful traits.
//     static int InvalidValue() {
//       return 0;
//     }
//
//     // This free function will not be called if f == InvalidValue()!
//     static void Free(int f) {
//       ::FreeFoo(f);
//     }
//   };
//
//   typedef ScopedGeneric<int, FooScopedTraits> ScopedFoo;
//
// A Traits type may choose to track ownership of objects in parallel with
// ScopedGeneric. To do so, it must implement the Acquire and Release methods,
// which will be called by ScopedGeneric during ownership transfers and extend
// the ScopedGenericOwnershipTracking tag type.
//
//   struct BarScopedTraits : public ScopedGenericOwnershipTracking {
//     using ScopedGenericType = ScopedGeneric<int, BarScopedTraits>;
//     static int InvalidValue() {
//       return 0;
//     }
//
//     static void Free(int b) {
//       ::FreeBar(b);
//     }
//
//     static void Acquire(const ScopedGenericType& owner, int b) {
//       ::TrackAcquisition(b, owner);
//     }
//
//     static void Release(const ScopedGenericType& owner, int b) {
//       ::TrackRelease(b, owner);
//     }
//   };
//
//   typedef ScopedGeneric<int, BarScopedTraits> ScopedBar;
struct ScopedGenericOwnershipTracking {};

template<typename T, typename Traits>
class ScopedGeneric {
 private:






  struct Data : public Traits {
    explicit Data(const T& in) : generic(in) {}
    Data(const T& in, const Traits& other) : Traits(other), generic(in) {}
    T generic;
  };

 public:
  typedef T element_type;
  typedef Traits traits_type;

  ScopedGeneric() : data_(traits_type::InvalidValue()) {}


  explicit ScopedGeneric(const element_type& value) : data_(value) {
    TrackAcquire(data_.generic);
  }

  ScopedGeneric(const element_type& value, const traits_type& traits)
      : data_(value, traits) {
    TrackAcquire(data_.generic);
  }

  ScopedGeneric(ScopedGeneric<T, Traits>&& rvalue)
      : data_(rvalue.release(), rvalue.get_traits()) {
    TrackAcquire(data_.generic);
  }

  virtual ~ScopedGeneric() {
    CHECK(!receiving_) << "ScopedGeneric destroyed with active receiver";
    FreeIfNecessary();
  }

  ScopedGeneric& operator=(ScopedGeneric<T, Traits>&& rvalue) {
    reset(rvalue.release());
    return *this;
  }



  void reset(const element_type& value = traits_type::InvalidValue()) {
    if (data_.generic != traits_type::InvalidValue() && data_.generic == value)
      abort();
    FreeIfNecessary();
    data_.generic = value;
    TrackAcquire(value);
  }

  void swap(ScopedGeneric& other) {
    if (&other == this) {
      return;
    }

    TrackRelease(data_.generic);
    other.TrackRelease(other.data_.generic);



    using std::swap;
    swap(static_cast<Traits&>(data_), static_cast<Traits&>(other.data_));
    swap(data_.generic, other.data_.generic);

    TrackAcquire(data_.generic);
    other.TrackAcquire(other.data_.generic);
  }



  element_type release() WARN_UNUSED_RESULT {
    element_type old_generic = data_.generic;
    data_.generic = traits_type::InvalidValue();
    TrackRelease(old_generic);
    return old_generic;
  }




































  class Receiver {
   public:
    explicit Receiver(ScopedGeneric& parent) : scoped_generic_(&parent) {
      CHECK(!scoped_generic_->receiving_)
          << "attempted to construct Receiver for ScopedGeneric with existing "
             "Receiver";
      scoped_generic_->receiving_ = true;
    }

    ~Receiver() {
      if (scoped_generic_) {
        CHECK(scoped_generic_->receiving_);
        scoped_generic_->reset(value_);
        scoped_generic_->receiving_ = false;
      }
    }

    Receiver(Receiver&& move) {
      CHECK(!used_) << "moving into already-used Receiver";
      CHECK(!move.used_) << "moving from already-used Receiver";
      scoped_generic_ = move.scoped_generic_;
      move.scoped_generic_ = nullptr;
    }

    Receiver& operator=(Receiver&& move) {
      CHECK(!used_) << "moving into already-used Receiver";
      CHECK(!move.used_) << "moving from already-used Receiver";
      scoped_generic_ = move.scoped_generic_;
      move.scoped_generic_ = nullptr;
    }






    T* get() {
      used_ = true;
      return &value_;
    }

   private:
    T value_ = Traits::InvalidValue();
    ScopedGeneric* scoped_generic_;
    bool used_ = false;

    DISALLOW_COPY_AND_ASSIGN(Receiver);
  };

  const element_type& get() const { return data_.generic; }


  bool is_valid() const { return data_.generic != traits_type::InvalidValue(); }

  bool operator==(const element_type& value) const {
    return data_.generic == value;
  }
  bool operator!=(const element_type& value) const {
    return data_.generic != value;
  }

  Traits& get_traits() { return data_; }
  const Traits& get_traits() const { return data_; }

 private:
  void FreeIfNecessary() {
    if (data_.generic != traits_type::InvalidValue()) {
      TrackRelease(data_.generic);
      data_.Free(data_.generic);
      data_.generic = traits_type::InvalidValue();
    }
  }

  template <typename Void = void>
  typename std::enable_if_t<
      std::is_base_of<ScopedGenericOwnershipTracking, Traits>::value,
      Void>
  TrackAcquire(const T& value) {
    if (value != traits_type::InvalidValue()) {
      data_.Acquire(static_cast<const ScopedGeneric&>(*this), value);
    }
  }

  template <typename Void = void>
  typename std::enable_if_t<
      !std::is_base_of<ScopedGenericOwnershipTracking, Traits>::value,
      Void>
  TrackAcquire(const T& value) {}

  template <typename Void = void>
  typename std::enable_if_t<
      std::is_base_of<ScopedGenericOwnershipTracking, Traits>::value,
      Void>
  TrackRelease(const T& value) {
    if (value != traits_type::InvalidValue()) {
      data_.Release(static_cast<const ScopedGeneric&>(*this), value);
    }
  }

  template <typename Void = void>
  typename std::enable_if_t<
      !std::is_base_of<ScopedGenericOwnershipTracking, Traits>::value,
      Void>
  TrackRelease(const T& value) {}



  template <typename T2, typename Traits2> bool operator==(
      const ScopedGeneric<T2, Traits2>& p2) const;
  template <typename T2, typename Traits2> bool operator!=(
      const ScopedGeneric<T2, Traits2>& p2) const;

  Data data_;
  bool receiving_ = false;

  DISALLOW_COPY_AND_ASSIGN(ScopedGeneric);
};

template<class T, class Traits>
void swap(const ScopedGeneric<T, Traits>& a,
          const ScopedGeneric<T, Traits>& b) {
  a.swap(b);
}

template<class T, class Traits>
bool operator==(const T& value, const ScopedGeneric<T, Traits>& scoped) {
  return value == scoped.get();
}

template<class T, class Traits>
bool operator!=(const T& value, const ScopedGeneric<T, Traits>& scoped) {
  return value != scoped.get();
}

}  // namespace base

#endif  // BASE_SCOPED_GENERIC_H_
