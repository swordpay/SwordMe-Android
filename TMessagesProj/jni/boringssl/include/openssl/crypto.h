/* Copyright (c) 2014, Google Inc.
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

#ifndef OPENSSL_HEADER_CRYPTO_H
#define OPENSSL_HEADER_CRYPTO_H

#include <openssl/base.h>
#include <openssl/sha.h>

// mem.h.
#include <openssl/mem.h>

// thread.h.
#include <openssl/thread.h>


#if defined(__cplusplus)
extern "C" {
#endif


// library is built with BORINGSSL_NO_STATIC_INITIALIZER. Otherwise, it does
// nothing and a static initializer is used instead. It is safe to call this
// function multiple times and concurrently from multiple threads.
//
// On some ARM configurations, this function may require filesystem access and
// should be called before entering a sandbox.
OPENSSL_EXPORT void CRYPTO_library_init(void);

// has been built with the BORINGSSL_CONFIDENTIAL define and zero otherwise.
//
// This is used by some consumers to identify whether they are using an
// internal version of BoringSSL.
OPENSSL_EXPORT int CRYPTO_is_confidential_build(void);

// in which case it returns zero.
OPENSSL_EXPORT int CRYPTO_has_asm(void);

// which case it returns one.
OPENSSL_EXPORT int FIPS_mode(void);

// success and zero on error. The argument is the integrity hash of the FIPS
// module and may be used to check and write flag files to suppress duplicate
// self-tests. If it is all zeros, no flag file will be checked nor written and
// tests will always be run.
OPENSSL_EXPORT int BORINGSSL_self_test(
    const uint8_t module_sha512_hash[SHA512_DIGEST_LENGTH]);


// “OpenSSL”. node.js requires a version number in this text.
#define OPENSSL_VERSION_TEXT "OpenSSL 1.1.0 (compatible; BoringSSL)"

#define OPENSSL_VERSION 0
#define OPENSSL_CFLAGS 1
#define OPENSSL_BUILT_ON 2
#define OPENSSL_PLATFORM 3
#define OPENSSL_DIR 4

// "BoringSSL" if |which| is |OPENSSL_VERSION| and placeholder strings
// otherwise.
OPENSSL_EXPORT const char *OpenSSL_version(int which);

#define SSLEAY_VERSION OPENSSL_VERSION
#define SSLEAY_CFLAGS OPENSSL_CFLAGS
#define SSLEAY_BUILT_ON OPENSSL_BUILT_ON
#define SSLEAY_PLATFORM OPENSSL_PLATFORM
#define SSLEAY_DIR OPENSSL_DIR

OPENSSL_EXPORT const char *SSLeay_version(int which);

// base.h.
OPENSSL_EXPORT unsigned long SSLeay(void);

// OPENSSL_VERSION_NUMBER from base.h.
OPENSSL_EXPORT unsigned long OpenSSL_version_num(void);

OPENSSL_EXPORT int CRYPTO_malloc_init(void);

OPENSSL_EXPORT int OPENSSL_malloc_init(void);

OPENSSL_EXPORT void ENGINE_load_builtin_engines(void);

OPENSSL_EXPORT int ENGINE_register_all_complete(void);

OPENSSL_EXPORT void OPENSSL_load_builtin_modules(void);

#define OPENSSL_INIT_NO_LOAD_CRYPTO_STRINGS 0
#define OPENSSL_INIT_LOAD_CRYPTO_STRINGS 0
#define OPENSSL_INIT_ADD_ALL_CIPHERS 0
#define OPENSSL_INIT_ADD_ALL_DIGESTS 0
#define OPENSSL_INIT_NO_ADD_ALL_CIPHERS 0
#define OPENSSL_INIT_NO_ADD_ALL_DIGESTS 0
#define OPENSSL_INIT_LOAD_CONFIG 0
#define OPENSSL_INIT_NO_LOAD_CONFIG 0

OPENSSL_EXPORT int OPENSSL_init_crypto(uint64_t opts,
                                       const OPENSSL_INIT_SETTINGS *settings);

OPENSSL_EXPORT void OPENSSL_cleanup(void);

// |BORINGSSL_FIPS| and zero otherwise.
OPENSSL_EXPORT int FIPS_mode_set(int on);


#if defined(__cplusplus)
}  // extern C
#endif

#endif  // OPENSSL_HEADER_CRYPTO_H
