// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ANDROID_SCOPED_JAVA_REF_H_
#define BASE_ANDROID_SCOPED_JAVA_REF_H_

#include <jni.h>
#include <stddef.h>

#include <type_traits>
#include <utility>

#include "base/base_export.h"
#include "base/logging.h"
#include "base/macros.h"

namespace base {
namespace android {

// local references can be created. Note that local references already created
// in previous local frames are still valid in the current local frame.
class BASE_EXPORT ScopedJavaLocalFrame {
 public:
  explicit ScopedJavaLocalFrame(JNIEnv* env);
  ScopedJavaLocalFrame(JNIEnv* env, int capacity);
  ~ScopedJavaLocalFrame();

 private:


  JNIEnv* env_;

  DISALLOW_COPY_AND_ASSIGN(ScopedJavaLocalFrame);
};

template <typename T>
class JavaRef;

// other JavaRef<> template types. This allows you to e.g. pass
// ScopedJavaLocalRef<jstring> into a function taking const JavaRef<jobject>&
template <>
class BASE_EXPORT JavaRef<jobject> {
 public:

  constexpr JavaRef() {}



  constexpr JavaRef(std::nullptr_t) {}

  ~JavaRef() {}


  jobject obj() const { return obj_; }

  explicit operator bool() const { return obj_ != nullptr; }


  bool is_null() const { return obj_ == nullptr; }

 protected:
// Takes ownership of the |obj| reference passed; requires it to be a local
// reference type.
#if DCHECK_IS_ON()

  JavaRef(JNIEnv* env, jobject obj);
#else
  JavaRef(JNIEnv* env, jobject obj) : obj_(obj) {}
#endif

  void steal(JavaRef&& other) {
    obj_ = other.obj_;
    other.obj_ = nullptr;
  }


  JNIEnv* SetNewLocalRef(JNIEnv* env, jobject obj);
  void SetNewGlobalRef(JNIEnv* env, jobject obj);
  void ResetLocalRef(JNIEnv* env);
  void ResetGlobalRef();
  jobject ReleaseInternal();

 private:
  jobject obj_ = nullptr;

  DISALLOW_COPY_AND_ASSIGN(JavaRef);
};

template <typename T>
class JavaObjectArrayReader;

// for allowing functions to accept a reference without having to mandate
// whether it is a local or global type.
template <typename T>
class JavaRef : public JavaRef<jobject> {
 public:
  constexpr JavaRef() {}
  constexpr JavaRef(std::nullptr_t) {}
  ~JavaRef() {}

  T obj() const { return static_cast<T>(JavaRef<jobject>::obj()); }




  template <typename ElementType,
            typename T_ = T,
            typename = std::enable_if_t<std::is_same<T_, jobjectArray>::value>>
  JavaObjectArrayReader<ElementType> ReadElements() const {
    return JavaObjectArrayReader<ElementType>(*this);
  }

 protected:
  JavaRef(JNIEnv* env, T obj) : JavaRef<jobject>(env, obj) {}

 private:
  DISALLOW_COPY_AND_ASSIGN(JavaRef);
};

// Method parameters should not be deleted, and so this class exists purely to
// wrap them as a JavaRef<T> in the JNI binding generator. Do not create
// instances manually.
template <typename T>
class JavaParamRef : public JavaRef<T> {
 public:


  JavaParamRef(JNIEnv* env, T obj) : JavaRef<T>(env, obj) {}




  JavaParamRef(std::nullptr_t) {}

  ~JavaParamRef() {}


  operator T() const { return JavaRef<T>::obj(); }

 private:
  DISALLOW_COPY_AND_ASSIGN(JavaParamRef);
};

// to the lifetime of this object.
// Instances of this class may hold onto any JNIEnv passed into it until
// destroyed. Therefore, since a JNIEnv is only suitable for use on a single
// thread, objects of this class must be created, used, and destroyed, on a
// single thread.
// Therefore, this class should only be used as a stack-based object and from a
// single thread. If you wish to have the reference outlive the current
// callstack (e.g. as a class member) or you wish to pass it across threads,
// use a ScopedJavaGlobalRef instead.
template <typename T>
class ScopedJavaLocalRef : public JavaRef<T> {
 public:



  static ScopedJavaLocalRef Adopt(JNIEnv* env, T obj) {
    return ScopedJavaLocalRef(env, obj);
  }

  constexpr ScopedJavaLocalRef() {}
  constexpr ScopedJavaLocalRef(std::nullptr_t) {}


  ScopedJavaLocalRef(const ScopedJavaLocalRef& other) : env_(other.env_) {
    JavaRef<T>::SetNewLocalRef(env_, other.obj());
  }

  template <typename U,
            typename = std::enable_if_t<std::is_convertible<U, T>::value>>
  ScopedJavaLocalRef(const ScopedJavaLocalRef<U>& other) : env_(other.env_) {
    JavaRef<T>::SetNewLocalRef(env_, other.obj());
  }


  ScopedJavaLocalRef(ScopedJavaLocalRef&& other) : env_(other.env_) {
    JavaRef<T>::steal(std::move(other));
  }

  template <typename U,
            typename = std::enable_if_t<std::is_convertible<U, T>::value>>
  ScopedJavaLocalRef(ScopedJavaLocalRef<U>&& other) : env_(other.env_) {
    JavaRef<T>::steal(std::move(other));
  }

  explicit ScopedJavaLocalRef(const JavaRef<T>& other) { Reset(other); }




  ScopedJavaLocalRef(JNIEnv* env, T obj) : JavaRef<T>(env, obj), env_(env) {}

  ~ScopedJavaLocalRef() { Reset(); }

  ScopedJavaLocalRef& operator=(std::nullptr_t) {
    Reset();
    return *this;
  }

  ScopedJavaLocalRef& operator=(const ScopedJavaLocalRef& other) {
    Reset(other);
    return *this;
  }

  template <typename U,
            typename = std::enable_if_t<std::is_convertible<U, T>::value>>
  ScopedJavaLocalRef& operator=(const ScopedJavaLocalRef<U>& other) {
    Reset(other);
    return *this;
  }

  template <typename U,
            typename = std::enable_if_t<std::is_convertible<U, T>::value>>
  ScopedJavaLocalRef& operator=(ScopedJavaLocalRef<U>&& other) {
    env_ = other.env_;
    Reset();
    JavaRef<T>::steal(std::move(other));
    return *this;
  }

  ScopedJavaLocalRef& operator=(const JavaRef<T>& other) {
    Reset(other);
    return *this;
  }

  void Reset() { JavaRef<T>::ResetLocalRef(env_); }

  template <typename U,
            typename = std::enable_if_t<std::is_convertible<U, T>::value>>
  void Reset(const ScopedJavaLocalRef<U>& other) {



    Reset(other.env_, other.obj());
  }

  void Reset(const JavaRef<T>& other) {


    Reset(env_, other.obj());
  }




  void Reset(JNIEnv* env, T obj) {
    env_ = JavaRef<T>::SetNewLocalRef(env, obj);
  }



  T Release() { return static_cast<T>(JavaRef<T>::ReleaseInternal()); }

 private:


  JNIEnv* env_ = nullptr;





  ScopedJavaLocalRef(JNIEnv* env, const JavaParamRef<T>& other);

  template <typename U>
  friend class ScopedJavaLocalRef;

  template <typename U>
  friend class JavaObjectArrayReader;
};

// to the lifetime of this object. This class does not hold onto any JNIEnv*
// passed to it, hence it is safe to use across threads (within the constraints
// imposed by the underlying Java object that it references).
template <typename T>
class ScopedJavaGlobalRef : public JavaRef<T> {
 public:
  constexpr ScopedJavaGlobalRef() {}
  constexpr ScopedJavaGlobalRef(std::nullptr_t) {}


  ScopedJavaGlobalRef(const ScopedJavaGlobalRef& other) { Reset(other); }

  template <typename U,
            typename = std::enable_if_t<std::is_convertible<U, T>::value>>
  ScopedJavaGlobalRef(const ScopedJavaGlobalRef<U>& other) {
    Reset(other);
  }


  ScopedJavaGlobalRef(ScopedJavaGlobalRef&& other) {
    JavaRef<T>::steal(std::move(other));
  }

  template <typename U,
            typename = std::enable_if_t<std::is_convertible<U, T>::value>>
  ScopedJavaGlobalRef(ScopedJavaGlobalRef<U>&& other) {
    JavaRef<T>::steal(std::move(other));
  }

  explicit ScopedJavaGlobalRef(const JavaRef<T>& other) { Reset(other); }


  ScopedJavaGlobalRef(JNIEnv* env, T obj) { Reset(env, obj); }

  ~ScopedJavaGlobalRef() { Reset(); }

  ScopedJavaGlobalRef& operator=(std::nullptr_t) {
    Reset();
    return *this;
  }

  ScopedJavaGlobalRef& operator=(const ScopedJavaGlobalRef& other) {
    Reset(other);
    return *this;
  }

  template <typename U,
            typename = std::enable_if_t<std::is_convertible<U, T>::value>>
  ScopedJavaGlobalRef& operator=(const ScopedJavaGlobalRef<U>& other) {
    Reset(other);
    return *this;
  }

  template <typename U,
            typename = std::enable_if_t<std::is_convertible<U, T>::value>>
  ScopedJavaGlobalRef& operator=(ScopedJavaGlobalRef<U>&& other) {
    Reset();
    JavaRef<T>::steal(std::move(other));
    return *this;
  }

  ScopedJavaGlobalRef& operator=(const JavaRef<T>& other) {
    Reset(other);
    return *this;
  }

  void Reset() { JavaRef<T>::ResetGlobalRef(); }

  template <typename U,
            typename = std::enable_if_t<std::is_convertible<U, T>::value>>
  void Reset(const ScopedJavaGlobalRef<U>& other) {
    Reset(nullptr, other.obj());
  }

  void Reset(const JavaRef<T>& other) { Reset(nullptr, other.obj()); }

  void Reset(JNIEnv* env, const JavaParamRef<T>& other) {
    Reset(env, other.obj());
  }

  void Reset(JNIEnv* env, T obj) { JavaRef<T>::SetNewGlobalRef(env, obj); }



  T Release() { return static_cast<T>(JavaRef<T>::ReleaseInternal()); }
};

// arrays to be iterated over with a range-based for loop, or used with
// <algorithm> functions that accept input iterators.
//
// The iterator returns each object in the array in turn, wrapped in a
// ScopedJavaLocalRef<T>. T will usually be jobject, but if you know that the
// array contains a more specific type (such as jstring) you can use that
// instead. This does not check the type at runtime!
//
// The wrapper holds a local reference to the array and only queries the size of
// the array once, so must only be used as a stack-based object from the current
// thread.
//
// Note that this does *not* update the contents of the array if you mutate the
// returned ScopedJavaLocalRef.
template <typename T>
class JavaObjectArrayReader {
 public:
  class iterator {
   public:




    using iterator_category = std::input_iterator_tag;

    using difference_type = ptrdiff_t;
    using value_type = ScopedJavaLocalRef<T>;




    using reference = value_type;





    class pointer {
     public:
      explicit pointer(const reference& ref) : ref_(ref) {}
      pointer(const pointer& ptr) = default;
      pointer& operator=(const pointer& ptr) = delete;
      reference* operator->() { return &ref_; }

     private:
      reference ref_;
    };

    iterator(const iterator&) = default;
    ~iterator() = default;

    iterator& operator=(const iterator&) = default;

    bool operator==(const iterator& other) const {
      DCHECK(reader_ == other.reader_);
      return i_ == other.i_;
    }

    bool operator!=(const iterator& other) const {
      DCHECK(reader_ == other.reader_);
      return i_ != other.i_;
    }

    reference operator*() const {
      DCHECK(i_ < reader_->size_);


      return value_type::Adopt(
          reader_->array_.env_,
          static_cast<T>(reader_->array_.env_->GetObjectArrayElement(
              reader_->array_.obj(), i_)));
    }

    pointer operator->() const { return pointer(operator*()); }

    iterator& operator++() {
      DCHECK(i_ < reader_->size_);
      ++i_;
      return *this;
    }

    iterator operator++(int) {
      iterator old = *this;
      ++*this;
      return old;
    }

   private:
    iterator(const JavaObjectArrayReader* reader, jsize i)
        : reader_(reader), i_(i) {}
    const JavaObjectArrayReader* reader_;
    jsize i_;

    friend JavaObjectArrayReader;
  };

  JavaObjectArrayReader(const JavaRef<jobjectArray>& array) : array_(array) {
    size_ = array_.env_->GetArrayLength(array_.obj());
  }

  JavaObjectArrayReader(const JavaObjectArrayReader& other) = default;

  JavaObjectArrayReader& operator=(const JavaObjectArrayReader& other) =
      default;

  JavaObjectArrayReader(JavaObjectArrayReader&& other) = default;
  JavaObjectArrayReader& operator=(JavaObjectArrayReader&& other) = default;

  bool empty() const { return size_ == 0; }

  jsize size() const { return size_; }

  iterator begin() const { return iterator(this, 0); }

  iterator end() const { return iterator(this, size_); }

 private:
  ScopedJavaLocalRef<jobjectArray> array_;
  jsize size_;

  friend iterator;
};

}  // namespace android
}  // namespace base

#endif  // BASE_ANDROID_SCOPED_JAVA_REF_H_
