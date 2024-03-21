// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// implementation, and management of the Callback objects.

#ifndef BASE_CALLBACK_INTERNAL_H_
#define BASE_CALLBACK_INTERNAL_H_

#include "base/base_export.h"
#include "base/callback_forward.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"

namespace base {

struct FakeBindState;

namespace internal {

class BindStateBase;
class FinallyExecutorCommon;
class ThenAndCatchExecutorCommon;

template <typename ReturnType>
class PostTaskExecutor;

template <typename Functor, typename... BoundArgs>
struct BindState;

class CallbackBase;
class CallbackBaseCopyable;

struct BindStateBaseRefCountTraits {
  static void Destruct(const BindStateBase*);
};

template <typename T>
using PassingType = std::conditional_t<std::is_scalar<T>::value, T, T&&>;

// class can use to represent a function object with bound arguments.  It
// behaves as an existential type that is used by a corresponding
// DoInvoke function to perform the function execution.  This allows
// us to shield the Callback class from the types of the bound argument via
// "type erasure."
// At the base level, the only task is to add reference counting data. Avoid
// using or inheriting any virtual functions. Creating a vtable for every
// BindState template instantiation results in a lot of bloat. Its only task is
// to call the destructor which can be done with a function pointer.
class BASE_EXPORT BindStateBase
    : public RefCountedThreadSafe<BindStateBase, BindStateBaseRefCountTraits> {
 public:
  REQUIRE_ADOPTION_FOR_REFCOUNTED_TYPE();

  enum CancellationQueryMode {
    IS_CANCELLED,
    MAYBE_VALID,
  };

  using InvokeFuncStorage = void(*)();

 private:
  BindStateBase(InvokeFuncStorage polymorphic_invoke,
                void (*destructor)(const BindStateBase*));
  BindStateBase(InvokeFuncStorage polymorphic_invoke,
                void (*destructor)(const BindStateBase*),
                bool (*query_cancellation_traits)(const BindStateBase*,
                                                  CancellationQueryMode mode));

  ~BindStateBase() = default;

  friend struct BindStateBaseRefCountTraits;
  friend class RefCountedThreadSafe<BindStateBase, BindStateBaseRefCountTraits>;

  friend class CallbackBase;
  friend class CallbackBaseCopyable;

  template <typename Functor, typename... BoundArgs>
  friend struct BindState;
  friend struct ::base::FakeBindState;

  bool IsCancelled() const {
    return query_cancellation_traits_(this, IS_CANCELLED);
  }

  bool MaybeValid() const {
    return query_cancellation_traits_(this, MAYBE_VALID);
  }




  InvokeFuncStorage polymorphic_invoke_;

  void (*destructor_)(const BindStateBase*);
  bool (*query_cancellation_traits_)(const BindStateBase*,
                                     CancellationQueryMode mode);

  DISALLOW_COPY_AND_ASSIGN(BindStateBase);
};

// template bloat.
// CallbackBase<MoveOnly> is a direct base class of MoveOnly callbacks, and
// CallbackBase<Copyable> uses CallbackBase<MoveOnly> for its implementation.
class BASE_EXPORT CallbackBase {
 public:
  inline CallbackBase(CallbackBase&& c) noexcept;
  CallbackBase& operator=(CallbackBase&& c) noexcept;

  explicit CallbackBase(const CallbackBaseCopyable& c);
  CallbackBase& operator=(const CallbackBaseCopyable& c);

  explicit CallbackBase(CallbackBaseCopyable&& c) noexcept;
  CallbackBase& operator=(CallbackBaseCopyable&& c) noexcept;

  bool is_null() const { return !bind_state_; }
  explicit operator bool() const { return !is_null(); }




  bool IsCancelled() const;




  bool MaybeValid() const;

  void Reset();

 protected:
  friend class FinallyExecutorCommon;
  friend class ThenAndCatchExecutorCommon;

  template <typename ReturnType>
  friend class PostTaskExecutor;

  using InvokeFuncStorage = BindStateBase::InvokeFuncStorage;

  bool EqualsInternal(const CallbackBase& other) const;

  constexpr inline CallbackBase();


  explicit inline CallbackBase(BindStateBase* bind_state);

  InvokeFuncStorage polymorphic_invoke() const {
    return bind_state_->polymorphic_invoke_;
  }



  ~CallbackBase();

  scoped_refptr<BindStateBase> bind_state_;
};

constexpr CallbackBase::CallbackBase() = default;
CallbackBase::CallbackBase(CallbackBase&&) noexcept = default;
CallbackBase::CallbackBase(BindStateBase* bind_state)
    : bind_state_(AdoptRef(bind_state)) {}

class BASE_EXPORT CallbackBaseCopyable : public CallbackBase {
 public:
  CallbackBaseCopyable(const CallbackBaseCopyable& c);
  CallbackBaseCopyable(CallbackBaseCopyable&& c) noexcept = default;
  CallbackBaseCopyable& operator=(const CallbackBaseCopyable& c);
  CallbackBaseCopyable& operator=(CallbackBaseCopyable&& c) noexcept;

 protected:
  constexpr CallbackBaseCopyable() = default;
  explicit CallbackBaseCopyable(BindStateBase* bind_state)
      : CallbackBase(bind_state) {}
  ~CallbackBaseCopyable() = default;
};

}  // namespace internal
}  // namespace base

#endif  // BASE_CALLBACK_INTERNAL_H_
