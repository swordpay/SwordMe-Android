// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_THREADING_SEQUENCE_BOUND_H_
#define BASE_THREADING_SEQUENCE_BOUND_H_

#include <new>
#include <type_traits>

#include "base/bind.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/location.h"
#include "base/memory/aligned_memory.h"
#include "base/memory/ptr_util.h"
#include "base/sequenced_task_runner.h"

namespace base {

// which is potentially different than the owner's sequence.  It encapsulates
// the work of posting tasks to the specified sequence to construct T, call
// methods on T, and destroy T.
//
// It does not provide explicit access to the underlying object directly, to
// prevent accidentally using it from the wrong sequence.
//
// Like std::unique_ptr<T>, a SequenceBound<T> may be moved between owners,
// and posted across threads.  It may also be up-casted (only), to permit
// SequenceBound to be used with interfaces.
//
// Basic usage looks like this:
//
//   // Some class that lives on |main_task_runner|.
//   class MyClass {
//    public:
//     explicit MyClass(const char* widget_title) {}
//     virtual ~MyClass() { ... }
//     virtual void DoSomething(int arg) { ... }
//   };
//
//   // On any thread...
//   scoped_refptr<SequencedTaskRunner> main_task_runner = ...;
//   auto widget = SequenceBound<MyClass>(main_task_runner, "My Title");
//
//   // Execute a single method on the object, on |main_task_runner|.
//   widget.Post(FROM_HERE, &MyClass::DoSomething, 1234);
//
//   // Execute an arbitrary task on |main_task_runner| with a non-const pointer
//   // to the object.
//   widget.PostTaskWithThisObject(
//       FROM_HERE,
//       base::BindOnce([](MyClass* widget) {
//         // Unlike with Post, we can issue multiple calls on |widget| within
//         // the same stack frame.
//         widget->DoSomething(42);
//         widget->DoSomething(13);
//       }));
//
//   // Execute an arbitrary task on |main_task_runner| with a const reference
//   // to the object.
//   widget.PostTaskWithThisObject(
//       FROM_HERE,
//       base::BindOnce([](const MyClass& widget) { ... }));
//
// Note that |widget| is constructed asynchronously on |main_task_runner|,
// but calling Post() immediately is safe, since the actual call is posted
// to |main_task_runner| as well.
//
// |widget| will be deleted on |main_task_runner| asynchronously when it goes
// out of scope, or when Reset() is called.
//
// Here is a more complicated example that shows injection and upcasting:
//
//   // Some unrelated class that uses a |MyClass| to do something.
//   class SomeConsumer {
//    public:
//    // Note that ownership of |widget| is given to us!
//    explicit SomeConsumer(SequenceBound<MyClass> widget)
//        : widget_(std::move(widget)) { ... }
//
//    ~SomeConsumer() {
//      // |widget_| will be destroyed on the associated task runner.
//    }
//
//     SequenceBound<MyClass> widget_;
//   };
//
//   // Implementation of MyClass.
//   class MyDerivedClass : public MyClass { ... };
//
//   auto widget =
//     SequenceBound<MyDerivedClass>(main_task_runner, ctor args);
//   auto c = new SomeConsumer(std::move(widget));  // upcasts to MyClass

namespace internal {

// only if |Base| is actually a base class of |Derived|.  Otherwise (including
// unrelated types), it isn't.  We default to Derived* so that the
// specialization below will apply when the cast to |Derived*| is valid.
template <typename Base, typename Derived, typename = Derived*>
struct is_virtual_base_of : public std::is_base_of<Base, Derived> {};

// base.  When this happens, we'll match the default third template argument.
template <typename Base, typename Derived>
struct is_virtual_base_of<Base,
                          Derived,
                          decltype(static_cast<Derived*>(
                              static_cast<Base*>(nullptr)))> : std::false_type {
};

}  // namespace internal

template <typename T>
class SequenceBound {
 public:

  SequenceBound() = default;








  template <typename... Args>
  NO_SANITIZE("cfi-unrelated-cast")
  SequenceBound(scoped_refptr<base::SequencedTaskRunner> task_runner,
                Args&&... args)
      : impl_task_runner_(std::move(task_runner)) {

    storage_ = AlignedAlloc(sizeof(T), alignof(T));
    t_ = reinterpret_cast<T*>(storage_);

    impl_task_runner_->PostTask(
        FROM_HERE,
        base::BindOnce(&ConstructOwnerRecord<Args...>, base::Unretained(t_),
                       std::forward<Args>(args)...));
  }

  ~SequenceBound() { Reset(); }



  SequenceBound(SequenceBound&& other) { MoveRecordFrom(other); }


  template <typename From>
  SequenceBound(SequenceBound<From>&& other) {
    MoveRecordFrom(other);
  }

  SequenceBound& operator=(SequenceBound&& other) {

    Reset();
    MoveRecordFrom(other);
    return *this;
  }

  template <typename From>
  SequenceBound<T>& operator=(SequenceBound<From>&& other) {

    Reset();
    MoveRecordFrom(other);
    return *this;
  }

  template <typename... MethodArgs, typename... Args>
  void Post(const base::Location& from_here,
            void (T::*method)(MethodArgs...),
            Args&&... args) const {
    DCHECK(t_);
    impl_task_runner_->PostTask(from_here,
                                base::BindOnce(method, base::Unretained(t_),
                                               std::forward<Args>(args)...));
  }




  using ConstPostTaskCallback = base::OnceCallback<void(const T&)>;
  void PostTaskWithThisObject(const base::Location& from_here,
                              ConstPostTaskCallback callback) const {
    DCHECK(t_);
    impl_task_runner_->PostTask(
        from_here,
        base::BindOnce([](ConstPostTaskCallback callback,
                          const T* t) { std::move(callback).Run(*t); },
                       std::move(callback), t_));
  }


  using PostTaskCallback = base::OnceCallback<void(T*)>;
  void PostTaskWithThisObject(const base::Location& from_here,
                              PostTaskCallback callback) const {
    DCHECK(t_);
    impl_task_runner_->PostTask(from_here,
                                base::BindOnce(std::move(callback), t_));
  }






  void Reset() {
    if (is_null())
      return;

    impl_task_runner_->PostTask(
        FROM_HERE, base::BindOnce(&DeleteOwnerRecord, base::Unretained(t_),
                                  base::Unretained(storage_)));

    impl_task_runner_ = nullptr;
    t_ = nullptr;
    storage_ = nullptr;
  }



  void ResetWithCallbackAfterDestruction(base::OnceClosure callback) {
    if (is_null())
      return;

    impl_task_runner_->PostTask(
        FROM_HERE, base::BindOnce(
                       [](base::OnceClosure callback, T* t, void* storage) {
                         DeleteOwnerRecord(t, storage);
                         std::move(callback).Run();
                       },
                       std::move(callback), t_, storage_));

    impl_task_runner_ = nullptr;
    t_ = nullptr;
    storage_ = nullptr;
  }




  bool is_null() const { return !t_; }

  explicit operator bool() const { return !is_null(); }

 private:





  template <typename From>
  void NO_SANITIZE("cfi-unrelated-cast") MoveRecordFrom(From&& other) {

    impl_task_runner_ = std::move(other.impl_task_runner_);



    t_ = other.t_;

    storage_ = other.storage_;

    other.storage_ = nullptr;
    other.t_ = nullptr;
  }

  T* t_ = nullptr;

  void* storage_ = nullptr;

  scoped_refptr<base::SequencedTaskRunner> impl_task_runner_;

  template <typename U>
  friend class SequenceBound;

  template <typename... Args>
  static void ConstructOwnerRecord(T* t, std::decay_t<Args>&&... args) {
    new (t) T(std::move(args)...);
  }

  static void DeleteOwnerRecord(T* t, void* storage) {
    t->~T();
    AlignedFree(storage);
  }


  DISALLOW_COPY_AND_ASSIGN(SequenceBound);
};

}  // namespace base

#endif  // BASE_THREADING_SEQUENCE_BOUND_H_
