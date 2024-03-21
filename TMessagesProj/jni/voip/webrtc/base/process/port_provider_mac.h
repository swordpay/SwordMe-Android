// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_PROCESS_PORT_PROVIDER_MAC_H_
#define BASE_PROCESS_PORT_PROVIDER_MAC_H_

#include <mach/mach.h>

#include "base/base_export.h"
#include "base/macros.h"
#include "base/observer_list.h"
#include "base/process/process_handle.h"
#include "base/synchronization/lock.h"

namespace base {

// Mach task port. This replicates task_for_pid(), which requires root
// privileges.
class BASE_EXPORT PortProvider {
 public:
  PortProvider();
  virtual ~PortProvider();

  class Observer {
   public:
    virtual ~Observer() {}






    virtual void OnReceivedTaskPort(ProcessHandle process) = 0;
  };


  virtual mach_port_t TaskForPid(ProcessHandle process) const = 0;

  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

 protected:

  void NotifyObservers(ProcessHandle process);

 private:


  base::Lock lock_;
  base::ObserverList<Observer>::Unchecked observer_list_;

  DISALLOW_COPY_AND_ASSIGN(PortProvider);
};

}  // namespace base

#endif  // BASE_PROCESS_PORT_PROVIDER_MAC_H_
