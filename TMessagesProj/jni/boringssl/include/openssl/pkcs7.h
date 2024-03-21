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

#ifndef OPENSSL_HEADER_PKCS7_H
#define OPENSSL_HEADER_PKCS7_H

#include <openssl/base.h>

#include <openssl/stack.h>

#if defined(__cplusplus)
extern "C" {
#endif

//
// This library contains functions for extracting information from PKCS#7
// structures (RFC 2315).

DECLARE_STACK_OF(CRYPTO_BUFFER)
DECLARE_STACK_OF(X509)
DECLARE_STACK_OF(X509_CRL)

// and appends the included certificates to |out_certs|. It returns one on
// success and zero on error. |cbs| is advanced passed the structure.
//
// Note that a SignedData structure may contain no certificates, in which case
// this function succeeds but does not append any certificates.
OPENSSL_EXPORT int PKCS7_get_raw_certificates(
    STACK_OF(CRYPTO_BUFFER) *out_certs, CBS *cbs, CRYPTO_BUFFER_POOL *pool);

// them into |X509| objects.
OPENSSL_EXPORT int PKCS7_get_certificates(STACK_OF(X509) *out_certs, CBS *cbs);

// |certs| to |out|. It returns one on success and zero on error.
OPENSSL_EXPORT int PKCS7_bundle_certificates(
    CBB *out, const STACK_OF(X509) *certs);

// the included CRLs to |out_crls|. It returns one on success and zero on error.
// |cbs| is advanced passed the structure.
//
// Note that a SignedData structure may contain no CRLs, in which case this
// function succeeds but does not append any CRLs.
OPENSSL_EXPORT int PKCS7_get_CRLs(STACK_OF(X509_CRL) *out_crls, CBS *cbs);

// |crls| to |out|. It returns one on success and zero on error.
OPENSSL_EXPORT int PKCS7_bundle_CRLs(CBB *out, const STACK_OF(X509_CRL) *crls);

// from |pem_bio| and appends the included certificates to |out_certs|. It
// returns one on success and zero on error.
//
// Note that a SignedData structure may contain no certificates, in which case
// this function succeeds but does not append any certificates.
OPENSSL_EXPORT int PKCS7_get_PEM_certificates(STACK_OF(X509) *out_certs,
                                              BIO *pem_bio);

// |pem_bio| and appends the included CRLs to |out_crls|. It returns one on
// success and zero on error.
//
// Note that a SignedData structure may contain no CRLs, in which case this
// function succeeds but does not append any CRLs.
OPENSSL_EXPORT int PKCS7_get_PEM_CRLs(STACK_OF(X509_CRL) *out_crls,
                                      BIO *pem_bio);

//
// These functions are a compatibility layer over a subset of OpenSSL's PKCS#7
// API. It intentionally does not implement the whole thing, only the minimum
// needed to build cryptography.io.

typedef struct {
  STACK_OF(X509) *cert;
  STACK_OF(X509_CRL) *crl;
} PKCS7_SIGNED;

typedef struct {
  STACK_OF(X509) *cert;
  STACK_OF(X509_CRL) *crl;
} PKCS7_SIGN_ENVELOPE;

typedef void PKCS7_ENVELOPE;
typedef void PKCS7_DIGEST;
typedef void PKCS7_ENCRYPT;

typedef struct {
  uint8_t *ber_bytes;
  size_t ber_len;


  ASN1_OBJECT *type;
  union {
    char *ptr;
    ASN1_OCTET_STRING *data;
    PKCS7_SIGNED *sign;
    PKCS7_ENVELOPE *enveloped;
    PKCS7_SIGN_ENVELOPE *signed_and_enveloped;
    PKCS7_DIGEST *digest;
    PKCS7_ENCRYPT *encrypted;
    ASN1_TYPE *other;
  } d;
} PKCS7;

// |len| bytes at |*inp|. If |out| is not NULL then, on exit, a pointer to the
// result is in |*out|. Note that, even if |*out| is already non-NULL on entry,
// it will not be written to. Rather, a fresh |PKCS7| is allocated and the
// previous one is freed. On successful exit, |*inp| is advanced past the BER
// structure.  It returns the result or NULL on error.
OPENSSL_EXPORT PKCS7 *d2i_PKCS7(PKCS7 **out, const uint8_t **inp,
                                size_t len);

// the length of the object is indefinite the full contents of |bio| are read.
//
// If the function fails then some unknown amount of data may have been read
// from |bio|.
OPENSSL_EXPORT PKCS7 *d2i_PKCS7_bio(BIO *bio, PKCS7 **out);

// not NULL then the result is written to |*out| and |*out| is advanced just
// past the output. It returns the number of bytes in the result, whether
// written or not, or a negative value on error.
OPENSSL_EXPORT int i2d_PKCS7(const PKCS7 *p7, uint8_t **out);

// error.
OPENSSL_EXPORT int i2d_PKCS7_bio(BIO *bio, const PKCS7 *p7);

OPENSSL_EXPORT void PKCS7_free(PKCS7 *p7);

OPENSSL_EXPORT int PKCS7_type_is_data(const PKCS7 *p7);

OPENSSL_EXPORT int PKCS7_type_is_digest(const PKCS7 *p7);

OPENSSL_EXPORT int PKCS7_type_is_encrypted(const PKCS7 *p7);

OPENSSL_EXPORT int PKCS7_type_is_enveloped(const PKCS7 *p7);

// ContentInfos.)
OPENSSL_EXPORT int PKCS7_type_is_signed(const PKCS7 *p7);

OPENSSL_EXPORT int PKCS7_type_is_signedAndEnveloped(const PKCS7 *p7);

#define PKCS7_DETACHED 0x40

#define PKCS7_TEXT 0x1
#define PKCS7_NOCERTS 0x2
#define PKCS7_NOSIGS 0x4
#define PKCS7_NOCHAIN 0x8
#define PKCS7_NOINTERN 0x10
#define PKCS7_NOVERIFY 0x20
#define PKCS7_BINARY 0x80
#define PKCS7_NOATTR 0x100
#define PKCS7_NOSMIMECAP 0x200
#define PKCS7_STREAM 0x1000

// external data and no signatures. It returns a newly-allocated |PKCS7| on
// success or NULL on error. |sign_cert| and |pkey| must be NULL. |data| is
// ignored. |flags| must be equal to |PKCS7_DETACHED|.
//
// Note this function only implements a subset of the corresponding OpenSSL
// function. It is provided for backwards compatibility only.
OPENSSL_EXPORT PKCS7 *PKCS7_sign(X509 *sign_cert, EVP_PKEY *pkey,
                                 STACK_OF(X509) *certs, BIO *data, int flags);


#if defined(__cplusplus)
}  // extern C

extern "C++" {
BSSL_NAMESPACE_BEGIN

BORINGSSL_MAKE_DELETER(PKCS7, PKCS7_free)

BSSL_NAMESPACE_END
}  // extern C++
#endif

#define PKCS7_R_BAD_PKCS7_VERSION 100
#define PKCS7_R_NOT_PKCS7_SIGNED_DATA 101
#define PKCS7_R_NO_CERTIFICATES_INCLUDED 102
#define PKCS7_R_NO_CRLS_INCLUDED 103

#endif  // OPENSSL_HEADER_PKCS7_H
