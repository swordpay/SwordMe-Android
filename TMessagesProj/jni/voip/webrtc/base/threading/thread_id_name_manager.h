// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_THREADING_THREAD_ID_NAME_MANAGER_H_
#define BASE_THREADING_THREAD_ID_NAME_MANAGER_H_

#include <map>
#include <string>
#include <vector>

#include "base/base_export.h"
#include "base/callback.h"
#include "base/macros.h"
#include "base/observer_list.h"
#include "base/synchronization/lock.h"
#include "base/threading/platform_thread.h"

namespace base {

template <typename T>
struct DefaultSingletonTraits;

class BASE_EXPORT ThreadIdNameManager {
 public:
  static ThreadIdNameManager* GetInstance();

  static const char* GetDefaultInternedString();

  class BASE_EXPORT Observer {
   public:
    virtual ~Observer();






    virtual void OnThreadNameChanged(const char* name) = 0;
  };

  void RegisterThread(PlatformThreadHandle::Handle handle, PlatformThreadId id);

  void AddObserver(Observer*);
  void RemoveObserver(Observer*);

  void SetName(const std::string& name);

  const char* GetName(PlatformThreadId id);

  const char* GetNameForCurrentThread();

  void RemoveName(PlatformThreadHandle::Handle handle, PlatformThreadId id);

 private:
  friend struct DefaultSingletonTraits<ThreadIdNameManager>;

  typedef std::map<PlatformThreadId, PlatformThreadHandle::Handle>
      ThreadIdToHandleMap;
  typedef std::map<PlatformThreadHandle::Handle, std::string*>
      ThreadHandleToInternedNameMap;
  typedef std::map<std::string, std::string*> NameToInternedNameMap;

  ThreadIdNameManager();
  ~ThreadIdNameManager();


  Lock lock_;

  NameToInternedNameMap name_to_interned_name_;
  ThreadIdToHandleMap thread_id_to_handle_;
  ThreadHandleToInternedNameMap thread_handle_to_interned_name_;

  std::string* main_process_name_;
  PlatformThreadId main_process_id_;


  std::vector<Observer*> observers_;

  DISALLOW_COPY_AND_ASSIGN(ThreadIdNameManager);
};

}  // namespace base

#endif  // BASE_THREADING_THREAD_ID_NAME_MANAGER_H_
