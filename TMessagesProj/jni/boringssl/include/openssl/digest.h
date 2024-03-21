/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.] */

#ifndef OPENSSL_HEADER_DIGEST_H
#define OPENSSL_HEADER_DIGEST_H

#include <openssl/base.h>

#if defined(__cplusplus)
extern "C" {
#endif

//
// An EVP_MD abstracts the details of a specific hash function allowing code to
// deal with the concept of a "hash function" without needing to know exactly
// which hash function it is.

//
// The following functions return |EVP_MD| objects that implement the named hash
// function.

OPENSSL_EXPORT const EVP_MD *EVP_md4(void);
OPENSSL_EXPORT const EVP_MD *EVP_md5(void);
OPENSSL_EXPORT const EVP_MD *EVP_sha1(void);
OPENSSL_EXPORT const EVP_MD *EVP_sha224(void);
OPENSSL_EXPORT const EVP_MD *EVP_sha256(void);
OPENSSL_EXPORT const EVP_MD *EVP_sha384(void);
OPENSSL_EXPORT const EVP_MD *EVP_sha512(void);

// MD5 and SHA-1, as used in TLS 1.1 and below.
OPENSSL_EXPORT const EVP_MD *EVP_md5_sha1(void);

// such digest is known.
OPENSSL_EXPORT const EVP_MD *EVP_get_digestbynid(int nid);

// if no such digest is known.
OPENSSL_EXPORT const EVP_MD *EVP_get_digestbyobj(const ASN1_OBJECT *obj);

//
// An EVP_MD_CTX represents the state of a specific digest operation in
// progress.

// same as setting the structure to zero.
OPENSSL_EXPORT void EVP_MD_CTX_init(EVP_MD_CTX *ctx);

// it, or NULL on allocation failure. The caller must use |EVP_MD_CTX_free| to
// release the resulting object.
OPENSSL_EXPORT EVP_MD_CTX *EVP_MD_CTX_new(void);

// freshly initialised state. It does not free |ctx| itself. It returns one.
OPENSSL_EXPORT int EVP_MD_CTX_cleanup(EVP_MD_CTX *ctx);

OPENSSL_EXPORT void EVP_MD_CTX_free(EVP_MD_CTX *ctx);

// copy of |in|. It returns one on success and zero on allocation failure.
OPENSSL_EXPORT int EVP_MD_CTX_copy_ex(EVP_MD_CTX *out, const EVP_MD_CTX *in);

// returns one.
OPENSSL_EXPORT int EVP_MD_CTX_reset(EVP_MD_CTX *ctx);


// initialised, for a fresh hashing operation using |type|. It returns one on
// success and zero on allocation failure.
OPENSSL_EXPORT int EVP_DigestInit_ex(EVP_MD_CTX *ctx, const EVP_MD *type,
                                     ENGINE *engine);

// initialised before use.
OPENSSL_EXPORT int EVP_DigestInit(EVP_MD_CTX *ctx, const EVP_MD *type);

// in |ctx|. It returns one.
OPENSSL_EXPORT int EVP_DigestUpdate(EVP_MD_CTX *ctx, const void *data,
                                    size_t len);

// Functions that output a digest generally require the buffer have
// at least this much space.
#define EVP_MAX_MD_SIZE 64  // SHA-512 is the longest so far.

// bytes.
#define EVP_MAX_MD_BLOCK_SIZE 128  // SHA-512 is the longest so far.

// |md_out|. |EVP_MD_CTX_size| bytes are written, which is at most
// |EVP_MAX_MD_SIZE|. If |out_size| is not NULL then |*out_size| is set to the
// number of bytes written. It returns one. After this call, the hash cannot be
// updated or finished again until |EVP_DigestInit_ex| is called to start
// another hashing operation.
OPENSSL_EXPORT int EVP_DigestFinal_ex(EVP_MD_CTX *ctx, uint8_t *md_out,
                                      unsigned int *out_size);

// |EVP_MD_CTX_cleanup| is called on |ctx| before returning.
OPENSSL_EXPORT int EVP_DigestFinal(EVP_MD_CTX *ctx, uint8_t *md_out,
                                   unsigned int *out_size);

// bytes from |data| and writes the digest to |md_out|. |EVP_MD_CTX_size| bytes
// are written, which is at most |EVP_MAX_MD_SIZE|. If |out_size| is not NULL
// then |*out_size| is set to the number of bytes written. It returns one on
// success and zero otherwise.
OPENSSL_EXPORT int EVP_Digest(const void *data, size_t len, uint8_t *md_out,
                              unsigned int *md_out_size, const EVP_MD *type,
                              ENGINE *impl);

//
// These functions allow code to learn details about an abstract hash
// function.

OPENSSL_EXPORT int EVP_MD_type(const EVP_MD *md);

// values, ORed together.
OPENSSL_EXPORT uint32_t EVP_MD_flags(const EVP_MD *md);

OPENSSL_EXPORT size_t EVP_MD_size(const EVP_MD *md);

OPENSSL_EXPORT size_t EVP_MD_block_size(const EVP_MD *md);

// specific public key in order to verify signatures. (For example,
// EVP_dss1.)
#define EVP_MD_FLAG_PKEY_DIGEST 1

// DigestAlgorithmIdentifier representing this digest function should be
// undefined rather than NULL.
#define EVP_MD_FLAG_DIGALGID_ABSENT 2

// (XOF). This flag is defined for compatibility and will never be set in any
// |EVP_MD| in BoringSSL.
#define EVP_MD_FLAG_XOF 4


// been set.
OPENSSL_EXPORT const EVP_MD *EVP_MD_CTX_md(const EVP_MD_CTX *ctx);

// will crash if a digest hasn't been set on |ctx|.
OPENSSL_EXPORT size_t EVP_MD_CTX_size(const EVP_MD_CTX *ctx);

// |ctx|, in bytes. It will crash if a digest hasn't been set on |ctx|.
OPENSSL_EXPORT size_t EVP_MD_CTX_block_size(const EVP_MD_CTX *ctx);

// (For example, |NID_sha256|.) It will crash if a digest hasn't been set on
// |ctx|.
OPENSSL_EXPORT int EVP_MD_CTX_type(const EVP_MD_CTX *ctx);

//
// These functions allow code to parse and serialize AlgorithmIdentifiers for
// hash functions.

// a hash function OID (for example, 2.16.840.1.101.3.4.2.1 is SHA-256) and
// advances |cbs|. The parameters field may either be omitted or a NULL. It
// returns the digest function or NULL on error.
OPENSSL_EXPORT const EVP_MD *EVP_parse_digest_algorithm(CBS *cbs);

// structure and appends the result to |cbb|. It returns one on success and zero
// on error.
OPENSSL_EXPORT int EVP_marshal_digest_algorithm(CBB *cbb, const EVP_MD *md);


// |in|. It returns one on success and zero on error.
OPENSSL_EXPORT int EVP_MD_CTX_copy(EVP_MD_CTX *out, const EVP_MD_CTX *in);

// compatibility with OpenSSL.
OPENSSL_EXPORT int EVP_add_digest(const EVP_MD *digest);

// |name|, or NULL if the name is unknown.
OPENSSL_EXPORT const EVP_MD *EVP_get_digestbyname(const char *);

// specifiy the original DSA signatures, which were fixed to use SHA-1. Note,
// however, that attempting to sign or verify DSA signatures with the EVP
// interface will always fail.
OPENSSL_EXPORT const EVP_MD *EVP_dss1(void);

OPENSSL_EXPORT EVP_MD_CTX *EVP_MD_CTX_create(void);

OPENSSL_EXPORT void EVP_MD_CTX_destroy(EVP_MD_CTX *ctx);

// BoringSSL does not support any XOF digests.
OPENSSL_EXPORT int EVP_DigestFinalXOF(EVP_MD_CTX *ctx, uint8_t *out,
                                      size_t len);

OPENSSL_EXPORT uint32_t EVP_MD_meth_get_flags(const EVP_MD *md);


struct evp_md_pctx_ops;

struct env_md_ctx_st {

  const EVP_MD *digest;


  void *md_data;


  EVP_PKEY_CTX *pctx;


  const struct evp_md_pctx_ops *pctx_ops;
} /* EVP_MD_CTX */;


#if defined(__cplusplus)
}  // extern C

#if !defined(BORINGSSL_NO_CXX)
extern "C++" {

BSSL_NAMESPACE_BEGIN

BORINGSSL_MAKE_DELETER(EVP_MD_CTX, EVP_MD_CTX_free)

using ScopedEVP_MD_CTX =
    internal::StackAllocated<EVP_MD_CTX, int, EVP_MD_CTX_init,
                             EVP_MD_CTX_cleanup>;

BSSL_NAMESPACE_END

}  // extern C++
#endif

#endif

#define DIGEST_R_INPUT_NOT_INITIALIZED 100
#define DIGEST_R_DECODE_ERROR 101
#define DIGEST_R_UNKNOWN_HASH 102

#endif  // OPENSSL_HEADER_DIGEST_H
