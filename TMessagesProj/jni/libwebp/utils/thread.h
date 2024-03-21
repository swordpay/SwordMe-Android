// Copyright 2011 Google Inc. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the COPYING file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS. All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
// -----------------------------------------------------------------------------
//
// Multi-threaded worker
//
// Author: Skal (pascal.massimino@gmail.com)

#ifndef WEBP_UTILS_THREAD_H_
#define WEBP_UTILS_THREAD_H_

#ifdef HAVE_CONFIG_H
#include "../webp/config.h"
#endif

#include "../webp/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  NOT_OK = 0,   // object is unusable
  OK,           // ready to work
  WORK          // busy finishing the current task
} WebPWorkerStatus;

// arguments (data1 and data2), and should return false in case of error.
typedef int (*WebPWorkerHook)(void*, void*);

typedef struct WebPWorkerImpl WebPWorkerImpl;

typedef struct {
  WebPWorkerImpl* impl_;
  WebPWorkerStatus status_;
  WebPWorkerHook hook;    // hook to call
  void* data1;            // first argument passed to 'hook'
  void* data2;            // second argument passed to 'hook'
  int had_error;          // return value of the last call to 'hook'
} WebPWorker;

// must be implemented.
typedef struct {

  void (*Init)(WebPWorker* const worker);


  int (*Reset)(WebPWorker* const worker);


  int (*Sync)(WebPWorker* const worker);



  void (*Launch)(WebPWorker* const worker);




  void (*Execute)(WebPWorker* const worker);


  void (*End)(WebPWorker* const worker);
} WebPWorkerInterface;

// should be done before any workers are started, i.e., before any encoding or
// decoding takes place. The contents of the interface struct are copied, it
// is safe to free the corresponding memory after this call. This function is
// not thread-safe. Return false in case of invalid pointer or methods.
WEBP_EXTERN(int) WebPSetWorkerInterface(
    const WebPWorkerInterface* const interface);

WEBP_EXTERN(const WebPWorkerInterface*) WebPGetWorkerInterface(void);


#ifdef __cplusplus
}    // extern "C"
#endif

#endif  /* WEBP_UTILS_THREAD_H_ */
