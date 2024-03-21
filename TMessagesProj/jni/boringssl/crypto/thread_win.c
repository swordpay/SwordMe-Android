/* Copyright (c) 2015, Google Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

#include "internal.h"

#if defined(OPENSSL_WINDOWS_THREADS)

OPENSSL_MSVC_PRAGMA(warning(push, 3))
#include <windows.h>
OPENSSL_MSVC_PRAGMA(warning(pop))

#include <stdlib.h>
#include <string.h>

#include <openssl/mem.h>
#include <openssl/type_check.h>


OPENSSL_STATIC_ASSERT(sizeof(CRYPTO_MUTEX) >= sizeof(SRWLOCK),
                      "CRYPTO_MUTEX is too small");

static BOOL CALLBACK call_once_init(INIT_ONCE *once, void *arg, void **out) {
  void (**init)(void) = (void (**)(void))arg;
  (**init)();
  return TRUE;
}

void CRYPTO_once(CRYPTO_once_t *once, void (*init)(void)) {
  if (!InitOnceExecuteOnce(once, call_once_init, &init, NULL)) {
    abort();
  }
}

void CRYPTO_MUTEX_init(CRYPTO_MUTEX *lock) {
  InitializeSRWLock((SRWLOCK *) lock);
}

void CRYPTO_MUTEX_lock_read(CRYPTO_MUTEX *lock) {
  AcquireSRWLockShared((SRWLOCK *) lock);
}

void CRYPTO_MUTEX_lock_write(CRYPTO_MUTEX *lock) {
  AcquireSRWLockExclusive((SRWLOCK *) lock);
}

void CRYPTO_MUTEX_unlock_read(CRYPTO_MUTEX *lock) {
  ReleaseSRWLockShared((SRWLOCK *) lock);
}

void CRYPTO_MUTEX_unlock_write(CRYPTO_MUTEX *lock) {
  ReleaseSRWLockExclusive((SRWLOCK *) lock);
}

void CRYPTO_MUTEX_cleanup(CRYPTO_MUTEX *lock) {

}

void CRYPTO_STATIC_MUTEX_lock_read(struct CRYPTO_STATIC_MUTEX *lock) {
  AcquireSRWLockShared(&lock->lock);
}

void CRYPTO_STATIC_MUTEX_lock_write(struct CRYPTO_STATIC_MUTEX *lock) {
  AcquireSRWLockExclusive(&lock->lock);
}

void CRYPTO_STATIC_MUTEX_unlock_read(struct CRYPTO_STATIC_MUTEX *lock) {
  ReleaseSRWLockShared(&lock->lock);
}

void CRYPTO_STATIC_MUTEX_unlock_write(struct CRYPTO_STATIC_MUTEX *lock) {
  ReleaseSRWLockExclusive(&lock->lock);
}

static SRWLOCK g_destructors_lock = SRWLOCK_INIT;
static thread_local_destructor_t g_destructors[NUM_OPENSSL_THREAD_LOCALS];

static CRYPTO_once_t g_thread_local_init_once = CRYPTO_ONCE_INIT;
static DWORD g_thread_local_key;
static int g_thread_local_failed;

static void thread_local_init(void) {
  g_thread_local_key = TlsAlloc();
  g_thread_local_failed = (g_thread_local_key == TLS_OUT_OF_INDEXES);
}

static void NTAPI thread_local_destructor(PVOID module, DWORD reason,
                                          PVOID reserved) {





  if (reason != DLL_THREAD_DETACH) {
    return;
  }

  CRYPTO_once(&g_thread_local_init_once, thread_local_init);
  if (g_thread_local_failed) {
    return;
  }

  void **pointers = (void**) TlsGetValue(g_thread_local_key);
  if (pointers == NULL) {
    return;
  }

  thread_local_destructor_t destructors[NUM_OPENSSL_THREAD_LOCALS];

  AcquireSRWLockExclusive(&g_destructors_lock);
  OPENSSL_memcpy(destructors, g_destructors, sizeof(destructors));
  ReleaseSRWLockExclusive(&g_destructors_lock);

  for (unsigned i = 0; i < NUM_OPENSSL_THREAD_LOCALS; i++) {
    if (destructors[i] != NULL) {
      destructors[i](pointers[i]);
    }
  }

  OPENSSL_free(pointers);
}

//
// Windows doesn't support a per-thread destructor with its TLS primitives.
// So, we build it manually by inserting a function to be called on each
// thread's exit. This magic is from http://www.codeproject.com/threads/tls.asp
// and it works for VC++ 7.0 and later.
//
// Force a reference to _tls_used to make the linker create the TLS directory
// if it's not already there. (E.g. if __declspec(thread) is not used). Force
// a reference to p_thread_callback_boringssl to prevent whole program
// optimization from discarding the variable.
//
// Note, in the prefixed build, |p_thread_callback_boringssl| may be a macro.
#define STRINGIFY(x) #x
#define EXPAND_AND_STRINGIFY(x) STRINGIFY(x)
#ifdef _WIN64
__pragma(comment(linker, "/INCLUDE:_tls_used"))
__pragma(comment(
    linker, "/INCLUDE:" EXPAND_AND_STRINGIFY(p_thread_callback_boringssl)))
#else
__pragma(comment(linker, "/INCLUDE:__tls_used"))
__pragma(comment(
    linker, "/INCLUDE:_" EXPAND_AND_STRINGIFY(p_thread_callback_boringssl)))
#endif

// called automatically by the OS loader code (not the CRT) when the module is
// loaded and on thread creation. They are NOT called if the module has been
// loaded by a LoadLibrary() call. It must have implicitly been loaded at
// process startup.
//
// By implicitly loaded, I mean that it is directly referenced by the main EXE
// or by one of its dependent DLLs. Delay-loaded DLL doesn't count as being
// implicitly loaded.
//
// See VC\crt\src\tlssup.c for reference.

// reference to this variable with a linker /INCLUDE:symbol pragma to ensure
// that.) If this variable is discarded, the OnThreadExit function will never
// be called.
#ifdef _WIN64

#pragma const_seg(".CRT$XLC")
// When defining a const variable, it must have external linkage to be sure the
// linker doesn't discard it.
extern const PIMAGE_TLS_CALLBACK p_thread_callback_boringssl;
const PIMAGE_TLS_CALLBACK p_thread_callback_boringssl = thread_local_destructor;
// Reset the default section.
#pragma const_seg()

#else

#pragma data_seg(".CRT$XLC")
PIMAGE_TLS_CALLBACK p_thread_callback_boringssl = thread_local_destructor;
// Reset the default section.
#pragma data_seg()

#endif  // _WIN64

static void **get_thread_locals(void) {











  DWORD last_error = GetLastError();
  void **ret = TlsGetValue(g_thread_local_key);
  SetLastError(last_error);
  return ret;
}

void *CRYPTO_get_thread_local(thread_local_data_t index) {
  CRYPTO_once(&g_thread_local_init_once, thread_local_init);
  if (g_thread_local_failed) {
    return NULL;
  }

  void **pointers = get_thread_locals();
  if (pointers == NULL) {
    return NULL;
  }
  return pointers[index];
}

int CRYPTO_set_thread_local(thread_local_data_t index, void *value,
                            thread_local_destructor_t destructor) {
  CRYPTO_once(&g_thread_local_init_once, thread_local_init);
  if (g_thread_local_failed) {
    destructor(value);
    return 0;
  }

  void **pointers = get_thread_locals();
  if (pointers == NULL) {
    pointers = OPENSSL_malloc(sizeof(void *) * NUM_OPENSSL_THREAD_LOCALS);
    if (pointers == NULL) {
      destructor(value);
      return 0;
    }
    OPENSSL_memset(pointers, 0, sizeof(void *) * NUM_OPENSSL_THREAD_LOCALS);
    if (TlsSetValue(g_thread_local_key, pointers) == 0) {
      OPENSSL_free(pointers);
      destructor(value);
      return 0;
    }
  }

  AcquireSRWLockExclusive(&g_destructors_lock);
  g_destructors[index] = destructor;
  ReleaseSRWLockExclusive(&g_destructors_lock);

  pointers[index] = value;
  return 1;
}

#endif  // OPENSSL_WINDOWS_THREADS
