// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_SUPPORTS_USER_DATA_H_
#define BASE_SUPPORTS_USER_DATA_H_

#include <map>
#include <memory>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/sequence_checker.h"

namespace base {

// key. At destruction all the objects will be destructed.
class BASE_EXPORT SupportsUserData {
 public:
  SupportsUserData();
  SupportsUserData(SupportsUserData&&);
  SupportsUserData& operator=(SupportsUserData&&);



  class BASE_EXPORT Data {
   public:
    virtual ~Data() = default;

    virtual std::unique_ptr<Data> Clone();
  };




  Data* GetUserData(const void* key) const;
  void SetUserData(const void* key, std::unique_ptr<Data> data);
  void RemoveUserData(const void* key);



  void CloneDataFrom(const SupportsUserData& other);




  void DetachFromSequence();

 protected:
  virtual ~SupportsUserData();


  void ClearAllUserData();

 private:
  using DataMap = std::map<const void*, std::unique_ptr<Data>>;

  DataMap user_data_;

  SequenceChecker sequence_checker_;

  DISALLOW_COPY_AND_ASSIGN(SupportsUserData);
};

// SupportsUserData::Data object is deleted.
template <typename T>
class UserDataAdapter : public SupportsUserData::Data {
 public:
  static T* Get(const SupportsUserData* supports_user_data, const void* key) {
    UserDataAdapter* data =
      static_cast<UserDataAdapter*>(supports_user_data->GetUserData(key));
    return data ? static_cast<T*>(data->object_.get()) : nullptr;
  }

  explicit UserDataAdapter(T* object) : object_(object) {}
  ~UserDataAdapter() override = default;

  T* release() { return object_.release(); }

 private:
  scoped_refptr<T> const object_;

  DISALLOW_COPY_AND_ASSIGN(UserDataAdapter);
};

}  // namespace base

#endif  // BASE_SUPPORTS_USER_DATA_H_
