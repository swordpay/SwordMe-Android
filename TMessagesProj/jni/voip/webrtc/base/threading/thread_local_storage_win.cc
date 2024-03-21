// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/threading/thread_local_storage.h"

#include <windows.h>

#include "base/logging.h"

namespace base {

namespace internal {

bool PlatformThreadLocalStorage::AllocTLS(TLSKey* key) {
  TLSKey value = TlsAlloc();
  if (value != TLS_OUT_OF_INDEXES) {
    *key = value;
    return true;
  }
  return false;
}

void PlatformThreadLocalStorage::FreeTLS(TLSKey key) {
  BOOL ret = TlsFree(key);
  DCHECK(ret);
}

void PlatformThreadLocalStorage::SetTLSValue(TLSKey key, void* value) {
  BOOL ret = TlsSetValue(key, value);
  DCHECK(ret);
}

}  // namespace internal

}  // namespace base

// Windows doesn't support a per-thread destructor with its
// TLS primitives.  So, we build it manually by inserting a
// function to be called on each thread's exit.
// This magic is from http://www.codeproject.com/threads/tls.asp
// and it works for VC++ 7.0 and later.

// if it's not already there.  (e.g. if __declspec(thread) is not used).
// Force a reference to p_thread_callback_base to prevent whole program
// optimization from discarding the variable.
#ifdef _WIN64

#pragma comment(linker, "/INCLUDE:_tls_used")
#pragma comment(linker, "/INCLUDE:p_thread_callback_base")

#else  // _WIN64

#pragma comment(linker, "/INCLUDE:__tls_used")
#pragma comment(linker, "/INCLUDE:_p_thread_callback_base")

#endif  // _WIN64

void NTAPI OnThreadExit(PVOID module, DWORD reason, PVOID reserved) {


  if (DLL_THREAD_DETACH == reason || DLL_PROCESS_DETACH == reason)
    base::internal::PlatformThreadLocalStorage::OnThreadExit();
}

// called automatically by the OS loader code (not the CRT) when the module is
// loaded and on thread creation. They are NOT called if the module has been
// loaded by a LoadLibrary() call. It must have implicitly been loaded at
// process startup.
// By implicitly loaded, I mean that it is directly referenced by the main EXE
// or by one of its dependent DLLs. Delay-loaded DLL doesn't count as being
// implicitly loaded.
//
// See VC\crt\src\tlssup.c for reference.

// linker /INCLUDE:symbol pragma above.
extern "C" {
// The linker must not discard p_thread_callback_base.  (We force a reference
// to this variable with a linker /INCLUDE:symbol pragma to ensure that.) If
// this variable is discarded, the OnThreadExit function will never be called.
#ifdef _WIN64

#pragma const_seg(".CRT$XLB")
// When defining a const variable, it must have external linkage to be sure the
// linker doesn't discard it.
extern const PIMAGE_TLS_CALLBACK p_thread_callback_base;
const PIMAGE_TLS_CALLBACK p_thread_callback_base = OnThreadExit;

#pragma const_seg()

#else  // _WIN64

#pragma data_seg(".CRT$XLB")
PIMAGE_TLS_CALLBACK p_thread_callback_base = OnThreadExit;

#pragma data_seg()

#endif  // _WIN64
}  // extern "C"
