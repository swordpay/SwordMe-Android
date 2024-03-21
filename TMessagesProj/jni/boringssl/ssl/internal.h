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
 * [including the GNU Public Licence.]
 */
/* ====================================================================
 * Copyright (c) 1998-2007 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */
/* ====================================================================
 * Copyright 2002 Sun Microsystems, Inc. ALL RIGHTS RESERVED.
 * ECC cipher suite support in OpenSSL originally developed by
 * SUN MICROSYSTEMS, INC., and contributed to the OpenSSL project.
 */
/* ====================================================================
 * Copyright 2005 Nokia. All rights reserved.
 *
 * The portions of the attached software ("Contribution") is developed by
 * Nokia Corporation and is licensed pursuant to the OpenSSL open source
 * license.
 *
 * The Contribution, originally written by Mika Kousa and Pasi Eronen of
 * Nokia Corporation, consists of the "PSK" (Pre-Shared Key) ciphersuites
 * support (see RFC 4279) to OpenSSL.
 *
 * No patent licenses or other rights except those expressly stated in
 * the OpenSSL open source license shall be deemed granted or received
 * expressly, by implication, estoppel, or otherwise.
 *
 * No assurances are provided by Nokia that the Contribution does not
 * infringe the patent or other intellectual property rights of any third
 * party or that the license provides you with all the necessary rights
 * to make use of the Contribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. IN
 * ADDITION TO THE DISCLAIMERS INCLUDED IN THE LICENSE, NOKIA
 * SPECIFICALLY DISCLAIMS ANY LIABILITY FOR CLAIMS BROUGHT BY YOU OR ANY
 * OTHER ENTITY BASED ON INFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS OR
 * OTHERWISE.
 */

#ifndef OPENSSL_HEADER_SSL_INTERNAL_H
#define OPENSSL_HEADER_SSL_INTERNAL_H

#include <openssl/base.h>

#include <stdlib.h>

#include <limits>
#include <new>
#include <type_traits>
#include <utility>

#include <openssl/aead.h>
#include <openssl/err.h>
#include <openssl/lhash.h>
#include <openssl/mem.h>
#include <openssl/span.h>
#include <openssl/ssl.h>
#include <openssl/stack.h>

#include "../crypto/err/internal.h"
#include "../crypto/internal.h"


#if defined(OPENSSL_WINDOWS)
// Windows defines struct timeval in winsock2.h.
OPENSSL_MSVC_PRAGMA(warning(push, 3))
#include <winsock2.h>
OPENSSL_MSVC_PRAGMA(warning(pop))
#else
#include <sys/time.h>
#endif


BSSL_NAMESPACE_BEGIN

struct SSL_CONFIG;
struct SSL_HANDSHAKE;
struct SSL_PROTOCOL_METHOD;
struct SSL_X509_METHOD;


// returns nullptr on allocation error. It only implements single-object
// allocation and not new T[n].
//
// Note: unlike |new|, this does not support non-public constructors.
template <typename T, typename... Args>
T *New(Args &&... args) {
  void *t = OPENSSL_malloc(sizeof(T));
  if (t == nullptr) {
    OPENSSL_PUT_ERROR(SSL, ERR_R_MALLOC_FAILURE);
    return nullptr;
  }
  return new (t) T(std::forward<Args>(args)...);
}

//
// Note: unlike |delete| this does not support non-public destructors.
template <typename T>
void Delete(T *t) {
  if (t != nullptr) {
    t->~T();
    OPENSSL_free(t);
  }
}

// may be C structs which require a |BORINGSSL_MAKE_DELETER| registration.
namespace internal {
template <typename T>
struct DeleterImpl<T, typename std::enable_if<T::kAllowUniquePtr>::type> {
  static void Free(T *t) { Delete(t); }
};
}  // namespace internal

// error.
template <typename T, typename... Args>
UniquePtr<T> MakeUnique(Args &&... args) {
  return UniquePtr<T>(New<T>(std::forward<Args>(args)...));
}

#if defined(BORINGSSL_ALLOW_CXX_RUNTIME)
#define HAS_VIRTUAL_DESTRUCTOR
#define PURE_VIRTUAL = 0
#else
// HAS_VIRTUAL_DESTRUCTOR should be declared in any base class which defines a
// virtual destructor. This avoids a dependency on |_ZdlPv| and prevents the
// class from being used with |delete|.
#define HAS_VIRTUAL_DESTRUCTOR \
  void operator delete(void *) { abort(); }

// functions. This avoids a dependency on |__cxa_pure_virtual| but loses
// compile-time checking.
#define PURE_VIRTUAL \
  { abort(); }
#endif

// on constexpr arrays.
#if defined(_MSC_VER) && !defined(__clang__) && _MSC_VER < 1910
#define CONSTEXPR_ARRAY const
#else
#define CONSTEXPR_ARRAY constexpr
#endif

template <typename T>
class Array {
 public:

  Array() {}
  Array(const Array &) = delete;
  Array(Array &&other) { *this = std::move(other); }

  ~Array() { Reset(); }

  Array &operator=(const Array &) = delete;
  Array &operator=(Array &&other) {
    Reset();
    other.Release(&data_, &size_);
    return *this;
  }

  const T *data() const { return data_; }
  T *data() { return data_; }
  size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }

  const T &operator[](size_t i) const { return data_[i]; }
  T &operator[](size_t i) { return data_[i]; }

  T *begin() { return data_; }
  const T *cbegin() const { return data_; }
  T *end() { return data_ + size_; }
  const T *cend() const { return data_ + size_; }

  void Reset() { Reset(nullptr, 0); }


  void Reset(T *new_data, size_t new_size) {
    for (size_t i = 0; i < size_; i++) {
      data_[i].~T();
    }
    OPENSSL_free(data_);
    data_ = new_data;
    size_ = new_size;
  }


  void Release(T **out, size_t *out_size) {
    *out = data_;
    *out_size = size_;
    data_ = nullptr;
    size_ = 0;
  }





  bool Init(size_t new_size) {
    Reset();
    if (new_size == 0) {
      return true;
    }

    if (new_size > std::numeric_limits<size_t>::max() / sizeof(T)) {
      OPENSSL_PUT_ERROR(SSL, ERR_R_OVERFLOW);
      return false;
    }
    data_ = reinterpret_cast<T *>(OPENSSL_malloc(new_size * sizeof(T)));
    if (data_ == nullptr) {
      OPENSSL_PUT_ERROR(SSL, ERR_R_MALLOC_FAILURE);
      return false;
    }
    size_ = new_size;
    for (size_t i = 0; i < size_; i++) {
      new (&data_[i]) T;
    }
    return true;
  }


  bool CopyFrom(Span<const T> in) {
    if (!Init(in.size())) {
      return false;
    }
    OPENSSL_memcpy(data_, in.data(), sizeof(T) * in.size());
    return true;
  }


  void Shrink(size_t new_size) {
    if (new_size > size_) {
      abort();
    }
    size_ = new_size;
  }

 private:
  T *data_ = nullptr;
  size_t size_ = 0;
};

OPENSSL_EXPORT bool CBBFinishArray(CBB *cbb, Array<uint8_t> *out);

//
// Due to DTLS's historical wire version differences, we maintain two notions of
// version.
//
// The "version" or "wire version" is the actual 16-bit value that appears on
// the wire. It uniquely identifies a version and is also used at API
// boundaries. The set of supported versions differs between TLS and DTLS. Wire
// versions are opaque values and may not be compared numerically.
//
// The "protocol version" identifies the high-level handshake variant being
// used. DTLS versions map to the corresponding TLS versions. Protocol versions
// are sequential and may be compared numerically.

// corresponding to wire version |version| and returns true. If |version| is not
// a valid TLS or DTLS version, it returns false.
//
// Note this simultaneously handles both DTLS and TLS. Use one of the
// higher-level functions below for most operations.
bool ssl_protocol_version_from_wire(uint16_t *out, uint16_t version);

// minimum and maximum enabled protocol versions, respectively.
bool ssl_get_version_range(const SSL_HANDSHAKE *hs, uint16_t *out_min_version,
                           uint16_t *out_max_version);

bool ssl_supports_version(SSL_HANDSHAKE *hs, uint16_t version);

bool ssl_method_supports_version(const SSL_PROTOCOL_METHOD *method,
                                 uint16_t version);

// decreasing preference order.
bool ssl_add_supported_versions(SSL_HANDSHAKE *hs, CBB *cbb);

// and the peer preference list in |peer_versions|. On success, it returns true
// and sets |*out_version| to the selected version. Otherwise, it returns false
// and sets |*out_alert| to an alert to send.
bool ssl_negotiate_version(SSL_HANDSHAKE *hs, uint8_t *out_alert,
                           uint16_t *out_version, const CBS *peer_versions);

// call this function before the version is determined.
uint16_t ssl_protocol_version(const SSL *ssl);


BSSL_NAMESPACE_END

struct ssl_cipher_st {

  const char *name;

  const char *standard_name;

  uint32_t id;

  uint32_t algorithm_mkey;
  uint32_t algorithm_auth;
  uint32_t algorithm_enc;
  uint32_t algorithm_mac;
  uint32_t algorithm_prf;
};

BSSL_NAMESPACE_BEGIN

#define SSL_kRSA 0x00000001u
#define SSL_kECDHE 0x00000002u
// SSL_kPSK is only set for plain PSK, not ECDHE_PSK.
#define SSL_kPSK 0x00000004u
#define SSL_kGENERIC 0x00000008u

#define SSL_aRSA 0x00000001u
#define SSL_aECDSA 0x00000002u
// SSL_aPSK is set for both PSK and ECDHE_PSK.
#define SSL_aPSK 0x00000004u
#define SSL_aGENERIC 0x00000008u

#define SSL_aCERT (SSL_aRSA | SSL_aECDSA)

#define SSL_3DES 0x00000001u
#define SSL_AES128 0x00000002u
#define SSL_AES256 0x00000004u
#define SSL_AES128GCM 0x00000008u
#define SSL_AES256GCM 0x00000010u
#define SSL_eNULL 0x00000020u
#define SSL_CHACHA20POLY1305 0x00000040u

#define SSL_AES (SSL_AES128 | SSL_AES256 | SSL_AES128GCM | SSL_AES256GCM)

#define SSL_SHA1 0x00000001u
// SSL_AEAD is set for all AEADs.
#define SSL_AEAD 0x00000002u

#define SSL_HANDSHAKE_MAC_DEFAULT 0x1
#define SSL_HANDSHAKE_MAC_SHA256 0x2
#define SSL_HANDSHAKE_MAC_SHA384 0x4

#define SSL_MAX_MD_SIZE 48

// preference groups. For TLS clients, the groups are moot because the server
// picks the cipher and groups cannot be expressed on the wire. However, for
// servers, the equal-preference groups allow the client's preferences to be
// partially respected. (This only has an effect with
// SSL_OP_CIPHER_SERVER_PREFERENCE).
//
// The equal-preference groups are expressed by grouping SSL_CIPHERs together.
// All elements of a group have the same priority: no ordering is expressed
// within a group.
//
// The values in |ciphers| are in one-to-one correspondence with
// |in_group_flags|. (That is, sk_SSL_CIPHER_num(ciphers) is the number of
// bytes in |in_group_flags|.) The bytes in |in_group_flags| are either 1, to
// indicate that the corresponding SSL_CIPHER is not the last element of a
// group, or 0 to indicate that it is.
//
// For example, if |in_group_flags| contains all zeros then that indicates a
// traditional, fully-ordered preference. Every SSL_CIPHER is the last element
// of the group (i.e. they are all in a one-element group).
//
// For a more complex example, consider:
//   ciphers:        A  B  C  D  E  F
//   in_group_flags: 1  1  0  0  1  0
//
// That would express the following, order:
//
//    A         E
//    B -> D -> F
//    C
struct SSLCipherPreferenceList {
  static constexpr bool kAllowUniquePtr = true;

  SSLCipherPreferenceList() = default;
  ~SSLCipherPreferenceList();

  bool Init(UniquePtr<STACK_OF(SSL_CIPHER)> ciphers,
            Span<const bool> in_group_flags);
  bool Init(const SSLCipherPreferenceList &);

  void Remove(const SSL_CIPHER *cipher);

  UniquePtr<STACK_OF(SSL_CIPHER)> ciphers;
  bool *in_group_flags = nullptr;
};

Span<const SSL_CIPHER> AllCiphers();

// object for |cipher| protocol version |version|. It sets |*out_mac_secret_len|
// and |*out_fixed_iv_len| to the MAC key length and fixed IV length,
// respectively. The MAC key length is zero except for legacy block and stream
// ciphers. It returns true on success and false on error.
bool ssl_cipher_get_evp_aead(const EVP_AEAD **out_aead,
                             size_t *out_mac_secret_len,
                             size_t *out_fixed_iv_len, const SSL_CIPHER *cipher,
                             uint16_t version, bool is_dtls);

// |cipher|.
const EVP_MD *ssl_get_handshake_digest(uint16_t version,
                                       const SSL_CIPHER *cipher);

// newly-allocated |SSLCipherPreferenceList| containing the result. It returns
// true on success and false on failure. If |strict| is true, nonsense will be
// rejected. If false, nonsense will be silently ignored. An empty result is
// considered an error regardless of |strict|.
bool ssl_create_cipher_list(UniquePtr<SSLCipherPreferenceList> *out_cipher_list,
                            const char *rule_str, bool strict);

uint16_t ssl_cipher_get_value(const SSL_CIPHER *cipher);

// values suitable for use with |key| in TLS 1.2 and below.
uint32_t ssl_cipher_auth_mask_for_key(const EVP_PKEY *key);

// server and, optionally, the client with a certificate.
bool ssl_cipher_uses_certificate_auth(const SSL_CIPHER *cipher);

// ServerKeyExchange message.
//
// This function may return false while still allowing |cipher| an optional
// ServerKeyExchange. This is the case for plain PSK ciphers.
bool ssl_cipher_requires_server_key_exchange(const SSL_CIPHER *cipher);

// length of an encrypted 1-byte record, for use in record-splitting. Otherwise
// it returns zero.
size_t ssl_cipher_get_record_split_len(const SSL_CIPHER *cipher);

// available from |cipher_suites| compatible with |version| and |group_id|. It
// returns NULL if there isn't a compatible cipher.
const SSL_CIPHER *ssl_choose_tls13_cipher(CBS cipher_suites, uint16_t version,
                                          uint16_t group_id);


// buffer and running hash.
class SSLTranscript {
 public:
  SSLTranscript();
  ~SSLTranscript();



  bool Init();




  bool InitHash(uint16_t version, const SSL_CIPHER *cipher);




  bool UpdateForHelloRetryRequest();




  bool CopyToHashContext(EVP_MD_CTX *ctx, const EVP_MD *digest);

  Span<const uint8_t> buffer() {
    return MakeConstSpan(reinterpret_cast<const uint8_t *>(buffer_->data),
                         buffer_->length);
  }


  void FreeBuffer();

  size_t DigestLen() const;


  const EVP_MD *Digest() const;


  bool Update(Span<const uint8_t> in);



  bool GetHash(uint8_t *out, size_t *out_len);




  bool GetFinishedMAC(uint8_t *out, size_t *out_len, const SSL_SESSION *session,
                      bool from_server);

 private:

  UniquePtr<BUF_MEM> buffer_;

  ScopedEVP_MD_CTX hash_;
};

// as the secret and |label| as the label. |seed1| and |seed2| are concatenated
// to form the seed parameter. It returns true on success and false on failure.
bool tls1_prf(const EVP_MD *digest, Span<uint8_t> out,
              Span<const uint8_t> secret, Span<const char> label,
              Span<const uint8_t> seed1, Span<const uint8_t> seed2);


// encrypt an SSL connection.
class SSLAEADContext {
 public:
  SSLAEADContext(uint16_t version, bool is_dtls, const SSL_CIPHER *cipher);
  ~SSLAEADContext();
  static constexpr bool kAllowUniquePtr = true;

  SSLAEADContext(const SSLAEADContext &&) = delete;
  SSLAEADContext &operator=(const SSLAEADContext &&) = delete;

  static UniquePtr<SSLAEADContext> CreateNullCipher(bool is_dtls);




  static UniquePtr<SSLAEADContext> Create(enum evp_aead_direction_t direction,
                                          uint16_t version, bool is_dtls,
                                          const SSL_CIPHER *cipher,
                                          Span<const uint8_t> enc_key,
                                          Span<const uint8_t> mac_key,
                                          Span<const uint8_t> fixed_iv);



  static UniquePtr<SSLAEADContext> CreatePlaceholderForQUIC(
      uint16_t version, const SSL_CIPHER *cipher);



  void SetVersionIfNullCipher(uint16_t version);



  uint16_t ProtocolVersion() const;


  uint16_t RecordVersion() const;

  const SSL_CIPHER *cipher() const { return cipher_; }

  bool is_null_cipher() const { return !cipher_; }

  size_t ExplicitNonceLen() const;

  size_t MaxOverhead() const;




  bool SuffixLen(size_t *out_suffix_len, size_t in_len,
                 size_t extra_in_len) const;




  bool CiphertextLen(size_t *out_len, size_t in_len, size_t extra_in_len) const;



  bool Open(Span<uint8_t> *out, uint8_t type, uint16_t record_version,
            const uint8_t seqnum[8], Span<const uint8_t> header,
            Span<uint8_t> in);




  bool Seal(uint8_t *out, size_t *out_len, size_t max_out, uint8_t type,
            uint16_t record_version, const uint8_t seqnum[8],
            Span<const uint8_t> header, const uint8_t *in, size_t in_len);















  bool SealScatter(uint8_t *out_prefix, uint8_t *out, uint8_t *out_suffix,
                   uint8_t type, uint16_t record_version,
                   const uint8_t seqnum[8], Span<const uint8_t> header,
                   const uint8_t *in, size_t in_len, const uint8_t *extra_in,
                   size_t extra_in_len);

  bool GetIV(const uint8_t **out_iv, size_t *out_iv_len) const;

 private:


  Span<const uint8_t> GetAdditionalData(uint8_t storage[13], uint8_t type,
                                        uint16_t record_version,
                                        const uint8_t seqnum[8],
                                        size_t plaintext_len,
                                        Span<const uint8_t> header);

  const SSL_CIPHER *cipher_;
  ScopedEVP_AEAD_CTX ctx_;


  uint8_t fixed_nonce_[12];
  uint8_t fixed_nonce_len_ = 0, variable_nonce_len_ = 0;

  uint16_t version_;

  bool is_dtls_;


  bool variable_nonce_included_in_record_ : 1;



  bool random_variable_nonce_ : 1;


  bool xor_fixed_nonce_ : 1;


  bool omit_length_in_ad_ : 1;

  bool ad_is_header_ : 1;
};


// replayed packets. It should be initialized by zeroing every field.
struct DTLS1_BITMAP {


  uint64_t map = 0;


  uint64_t max_seq_num = 0;
};


// returns true on success and false on wraparound.
bool ssl_record_sequence_update(uint8_t *seq, size_t seq_len);

// of a record for |ssl|.
//
// TODO(davidben): Expose this as part of public API once the high-level
// buffer-free APIs are available.
size_t ssl_record_prefix_len(const SSL *ssl);

enum ssl_open_record_t {
  ssl_open_record_success,
  ssl_open_record_discard,
  ssl_open_record_partial,
  ssl_open_record_close_notify,
  ssl_open_record_error,
};

//
// If the input did not contain a complete record, it returns
// |ssl_open_record_partial|. It sets |*out_consumed| to the total number of
// bytes necessary. It is guaranteed that a successful call to |tls_open_record|
// will consume at least that many bytes.
//
// Otherwise, it sets |*out_consumed| to the number of bytes of input
// consumed. Note that input may be consumed on all return codes if a record was
// decrypted.
//
// On success, it returns |ssl_open_record_success|. It sets |*out_type| to the
// record type and |*out| to the record body in |in|. Note that |*out| may be
// empty.
//
// If a record was successfully processed but should be discarded, it returns
// |ssl_open_record_discard|.
//
// If a record was successfully processed but is a close_notify, it returns
// |ssl_open_record_close_notify|.
//
// On failure or fatal alert, it returns |ssl_open_record_error| and sets
// |*out_alert| to an alert to emit, or zero if no alert should be emitted.
enum ssl_open_record_t tls_open_record(SSL *ssl, uint8_t *out_type,
                                       Span<uint8_t> *out, size_t *out_consumed,
                                       uint8_t *out_alert, Span<uint8_t> in);

// |ssl_open_record_partial| if |in| was empty and sets |*out_consumed| to
// zero. The caller should read one packet and try again.
enum ssl_open_record_t dtls_open_record(SSL *ssl, uint8_t *out_type,
                                        Span<uint8_t> *out,
                                        size_t *out_consumed,
                                        uint8_t *out_alert, Span<uint8_t> in);

// of the bulk of the ciphertext when sealing a record with |ssl|. Callers may
// use this to align buffers.
//
// Note when TLS 1.0 CBC record-splitting is enabled, this includes the one byte
// record and is the offset into second record's ciphertext. Thus sealing a
// small record may result in a smaller output than this value.
//
// TODO(davidben): Is this alignment valuable? Record-splitting makes this a
// mess.
size_t ssl_seal_align_prefix_len(const SSL *ssl);

// to |out|. At most |max_out| bytes will be written. It returns true on success
// and false on error. If enabled, |tls_seal_record| implements TLS 1.0 CBC
// 1/n-1 record splitting and may write two records concatenated.
//
// For a large record, the bulk of the ciphertext will begin
// |ssl_seal_align_prefix_len| bytes into out. Aligning |out| appropriately may
// improve performance. It writes at most |in_len| + |SSL_max_seal_overhead|
// bytes to |out|.
//
// |in| and |out| may not alias.
bool tls_seal_record(SSL *ssl, uint8_t *out, size_t *out_len, size_t max_out,
                     uint8_t type, const uint8_t *in, size_t in_len);

enum dtls1_use_epoch_t {
  dtls1_use_previous_epoch,
  dtls1_use_current_epoch,
};

// record.
size_t dtls_max_seal_overhead(const SSL *ssl, enum dtls1_use_epoch_t use_epoch);

// front of the plaintext when sealing a record in-place.
size_t dtls_seal_prefix_len(const SSL *ssl, enum dtls1_use_epoch_t use_epoch);

// which epoch's cipher state to use. Unlike |tls_seal_record|, |in| and |out|
// may alias but, if they do, |in| must be exactly |dtls_seal_prefix_len| bytes
// ahead of |out|.
bool dtls_seal_record(SSL *ssl, uint8_t *out, size_t *out_len, size_t max_out,
                      uint8_t type, const uint8_t *in, size_t in_len,
                      enum dtls1_use_epoch_t use_epoch);

// state. It returns one of |ssl_open_record_discard|, |ssl_open_record_error|,
// |ssl_open_record_close_notify|, or |ssl_open_record_fatal_alert| as
// appropriate.
enum ssl_open_record_t ssl_process_alert(SSL *ssl, uint8_t *out_alert,
                                         Span<const uint8_t> in);


bool ssl_has_private_key(const SSL_HANDSHAKE *hs);

// |SSL_PRIVATE_KEY_METHOD|. If there is a custom private key configured, they
// call the corresponding function or |complete| depending on whether there is a
// pending operation. Otherwise, they implement the operation with
// |EVP_PKEY|.

enum ssl_private_key_result_t ssl_private_key_sign(
    SSL_HANDSHAKE *hs, uint8_t *out, size_t *out_len, size_t max_out,
    uint16_t sigalg, Span<const uint8_t> in);

enum ssl_private_key_result_t ssl_private_key_decrypt(SSL_HANDSHAKE *hs,
                                                      uint8_t *out,
                                                      size_t *out_len,
                                                      size_t max_out,
                                                      Span<const uint8_t> in);

// key supports |sigalg|.
bool ssl_private_key_supports_signature_algorithm(SSL_HANDSHAKE *hs,
                                                  uint16_t sigalg);

// key |pkey| and input |in|, using the signature algorithm |sigalg|.
bool ssl_public_key_verify(SSL *ssl, Span<const uint8_t> signature,
                           uint16_t sigalg, EVP_PKEY *pkey,
                           Span<const uint8_t> in);


class SSLKeyShare {
 public:
  virtual ~SSLKeyShare() {}
  static constexpr bool kAllowUniquePtr = true;
  HAS_VIRTUAL_DESTRUCTOR


  static UniquePtr<SSLKeyShare> Create(uint16_t group_id);


  static UniquePtr<SSLKeyShare> Create(CBS *in);

  virtual uint16_t GroupID() const PURE_VIRTUAL;


  virtual bool Offer(CBB *out_public_key) PURE_VIRTUAL;







  virtual bool Accept(CBB *out_public_key, Array<uint8_t> *out_secret,
                      uint8_t *out_alert, Span<const uint8_t> peer_key);




  virtual bool Finish(Array<uint8_t> *out_secret, uint8_t *out_alert,
                      Span<const uint8_t> peer_key) PURE_VIRTUAL;


  virtual bool Serialize(CBB *out) { return false; }


  virtual bool Deserialize(CBS *in) { return false; }
};

struct NamedGroup {
  int nid;
  uint16_t group_id;
  const char name[8], alias[11];
};

Span<const NamedGroup> NamedGroups();

// sets |*out_group_id| to the group ID and returns true. Otherwise, it returns
// false.
bool ssl_nid_to_group_id(uint16_t *out_group_id, int nid);

// length |len|. On success, it sets |*out_group_id| to the group ID and returns
// true. Otherwise, it returns false.
bool ssl_name_to_group_id(uint16_t *out_group_id, const char *name, size_t len);


struct SSLMessage {
  bool is_v2_hello;
  uint8_t type;
  CBS body;


  CBS raw;
};

// ChangeCipherSpec, in the longest handshake flight. Currently this is the
// client's second leg in a full handshake when client certificates, NPN, and
// Channel ID, are all enabled.
#define SSL_MAX_HANDSHAKE_FLIGHT 7

extern const uint8_t kHelloRetryRequest[SSL3_RANDOM_SIZE];
extern const uint8_t kTLS12DowngradeRandom[8];
extern const uint8_t kTLS13DowngradeRandom[8];
extern const uint8_t kJDK11DowngradeRandom[8];

// in a handshake message for |ssl|.
size_t ssl_max_handshake_message_len(const SSL *ssl);

// data into handshake buffer.
bool tls_can_accept_handshake_data(const SSL *ssl, uint8_t *out_alert);

// handshake data that has not been consumed by |get_message|.
bool tls_has_unprocessed_handshake_data(const SSL *ssl);

// true on success and false on allocation failure.
bool tls_append_handshake_data(SSL *ssl, Span<const uint8_t> data);

// |tls_has_unprocessed_handshake_data| for DTLS.
bool dtls_has_unprocessed_handshake_data(const SSL *ssl);

bool tls_flush_pending_hs_data(SSL *ssl);

struct DTLS_OUTGOING_MESSAGE {
  DTLS_OUTGOING_MESSAGE() {}
  DTLS_OUTGOING_MESSAGE(const DTLS_OUTGOING_MESSAGE &) = delete;
  DTLS_OUTGOING_MESSAGE &operator=(const DTLS_OUTGOING_MESSAGE &) = delete;
  ~DTLS_OUTGOING_MESSAGE() { Clear(); }

  void Clear();

  uint8_t *data = nullptr;
  uint32_t len = 0;
  uint16_t epoch = 0;
  bool is_ccs = false;
};

void dtls_clear_outgoing_messages(SSL *ssl);


void ssl_do_info_callback(const SSL *ssl, int type, int value);

void ssl_do_msg_callback(const SSL *ssl, int is_write, int content_type,
                         Span<const uint8_t> in);


class SSLBuffer {
 public:
  SSLBuffer() {}
  ~SSLBuffer() { Clear(); }

  SSLBuffer(const SSLBuffer &) = delete;
  SSLBuffer &operator=(const SSLBuffer &) = delete;

  uint8_t *data() { return buf_ + offset_; }
  size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }
  size_t cap() const { return cap_; }

  Span<uint8_t> span() { return MakeSpan(data(), size()); }

  Span<uint8_t> remaining() {
    return MakeSpan(data() + size(), cap() - size());
  }

  void Clear();




  bool EnsureCap(size_t header_len, size_t new_cap);


  void DidWrite(size_t len);



  void Consume(size_t len);


  void DiscardConsumed();

 private:

  uint8_t *buf_ = nullptr;

  uint16_t offset_ = 0;

  uint16_t size_ = 0;

  uint16_t cap_ = 0;
};

// TLS, it reads to the end of the buffer until the buffer is |len| bytes
// long. For DTLS, it reads a new packet and ignores |len|. It returns one on
// success, zero on EOF, and a negative number on error.
//
// It is an error to call |ssl_read_buffer_extend_to| in DTLS when the buffer is
// non-empty.
int ssl_read_buffer_extend_to(SSL *ssl, size_t len);

// to a record-processing function. If |ret| is a success or if the caller
// should retry, it returns one and sets |*out_retry|. Otherwise, it returns <=
// 0.
int ssl_handle_open_record(SSL *ssl, bool *out_retry, ssl_open_record_t ret,
                           size_t consumed, uint8_t alert);

// one on success and <= 0 on error. For DTLS, whether or not the write
// succeeds, the write buffer will be cleared.
int ssl_write_buffer_flush(SSL *ssl);


// configured.
bool ssl_has_certificate(const SSL_HANDSHAKE *hs);

// by a TLS Certificate message. On success, it advances |cbs| and returns
// true. Otherwise, it returns false and sets |*out_alert| to an alert to send
// to the peer.
//
// If the list is non-empty then |*out_chain| and |*out_pubkey| will be set to
// the certificate chain and the leaf certificate's public key
// respectively. Otherwise, both will be set to nullptr.
//
// If the list is non-empty and |out_leaf_sha256| is non-NULL, it writes the
// SHA-256 hash of the leaf to |out_leaf_sha256|.
bool ssl_parse_cert_chain(uint8_t *out_alert,
                          UniquePtr<STACK_OF(CRYPTO_BUFFER)> *out_chain,
                          UniquePtr<EVP_PKEY> *out_pubkey,
                          uint8_t *out_leaf_sha256, CBS *cbs,
                          CRYPTO_BUFFER_POOL *pool);

// used by a TLS Certificate message. If there is no certificate chain, it emits
// an empty certificate list. It returns true on success and false on error.
bool ssl_add_cert_chain(SSL_HANDSHAKE *hs, CBB *cbb);

enum ssl_key_usage_t {
  key_usage_digital_signature = 0,
  key_usage_encipherment = 2,
};

// and returns true if doesn't specify a key usage or, if it does, if it
// includes |bit|. Otherwise it pushes to the error queue and returns false.
bool ssl_cert_check_key_usage(const CBS *in, enum ssl_key_usage_t bit);

// certificate in |in|. It returns an allocated |EVP_PKEY| or else returns
// nullptr and pushes to the error queue.
UniquePtr<EVP_PKEY> ssl_cert_parse_pubkey(const CBS *in);

// TLS CertificateRequest message. On success, it returns a newly-allocated
// |CRYPTO_BUFFER| list and advances |cbs|. Otherwise, it returns nullptr and
// sets |*out_alert| to an alert to send to the peer.
UniquePtr<STACK_OF(CRYPTO_BUFFER)> ssl_parse_client_CA_list(SSL *ssl,
                                                            uint8_t *out_alert,
                                                            CBS *cbs);

bool ssl_has_client_CAs(const SSL_CONFIG *cfg);

// used by a TLS CertificateRequest message. It returns true on success and
// false on error.
bool ssl_add_client_CA_list(SSL_HANDSHAKE *hs, CBB *cbb);

// a server's leaf certificate for |hs|. Otherwise, it returns zero and pushes
// an error on the error queue.
bool ssl_check_leaf_certificate(SSL_HANDSHAKE *hs, EVP_PKEY *pkey,
                               const CRYPTO_BUFFER *leaf);

// It finalizes the certificate and initializes |hs->local_pubkey|. It returns
// true on success and false on error.
bool ssl_on_certificate_selected(SSL_HANDSHAKE *hs);


// state, and incorporates the PSK. The cipher suite and PRF hash must have been
// selected at this point. It returns true on success and false on error.
bool tls13_init_key_schedule(SSL_HANDSHAKE *hs, Span<const uint8_t> psk);

// derivation state from the resumption secret and incorporates the PSK to
// derive the early secrets. It returns one on success and zero on error.
bool tls13_init_early_key_schedule(SSL_HANDSHAKE *hs, Span<const uint8_t> psk);

// HKDF-Extract. It returns true on success and false on error.
bool tls13_advance_key_schedule(SSL_HANDSHAKE *hs, Span<const uint8_t> in);

// |traffic_secret|. It returns true on success and false on error.
bool tls13_set_traffic_key(SSL *ssl, enum ssl_encryption_level_t level,
                           enum evp_aead_direction_t direction,
                           Span<const uint8_t> traffic_secret);

// on success and false on error. Unlike with other traffic secrets, this
// function does not pass the keys to QUIC. Call
// |tls13_set_early_secret_for_quic| to do so. This is done to due to an
// ordering complication around resolving HelloRetryRequest on the server.
bool tls13_derive_early_secret(SSL_HANDSHAKE *hs);

// derived by |tls13_derive_early_secret|, to QUIC. It returns true on success
// and false on error.
bool tls13_set_early_secret_for_quic(SSL_HANDSHAKE *hs);

// returns true on success and false on error.
bool tls13_derive_handshake_secrets(SSL_HANDSHAKE *hs);

// returns true on success and false on error.
bool tls13_rotate_traffic_key(SSL *ssl, enum evp_aead_direction_t direction);

// and exporter secrets based on the handshake transcripts and |master_secret|.
// It returns true on success and false on error.
bool tls13_derive_application_secrets(SSL_HANDSHAKE *hs);

bool tls13_derive_resumption_secret(SSL_HANDSHAKE *hs);

// |exporter_secret|.
bool tls13_export_keying_material(SSL *ssl, Span<uint8_t> out,
                                  Span<const uint8_t> secret,
                                  Span<const char> label,
                                  Span<const uint8_t> context);

// the integrity of the Finished message, and stores the result in |out| and
// length in |out_len|. |is_server| is true if this is for the Server Finished
// and false for the Client Finished.
bool tls13_finished_mac(SSL_HANDSHAKE *hs, uint8_t *out, size_t *out_len,
                        bool is_server);

// resumption master secret and |nonce|. It returns true on success, and false
// on failure.
bool tls13_derive_session_psk(SSL_SESSION *session, Span<const uint8_t> nonce);

// bytes of |msg| with the resulting value. It returns true on success, and
// false on failure.
bool tls13_write_psk_binder(SSL_HANDSHAKE *hs, Span<uint8_t> msg);

// to the binders has a valid signature using the value of |session|'s
// resumption secret. It returns true on success, and false on failure.
bool tls13_verify_psk_binder(SSL_HANDSHAKE *hs, SSL_SESSION *session,
                             const SSLMessage &msg, CBS *binders);


enum ssl_hs_wait_t {
  ssl_hs_error,
  ssl_hs_ok,
  ssl_hs_read_server_hello,
  ssl_hs_read_message,
  ssl_hs_flush,
  ssl_hs_certificate_selection_pending,
  ssl_hs_handoff,
  ssl_hs_handback,
  ssl_hs_x509_lookup,
  ssl_hs_channel_id_lookup,
  ssl_hs_private_key_operation,
  ssl_hs_pending_session,
  ssl_hs_pending_ticket,
  ssl_hs_early_return,
  ssl_hs_early_data_rejected,
  ssl_hs_read_end_of_early_data,
  ssl_hs_read_change_cipher_spec,
  ssl_hs_certificate_verify,
};

enum ssl_grease_index_t {
  ssl_grease_cipher = 0,
  ssl_grease_group,
  ssl_grease_extension1,
  ssl_grease_extension2,
  ssl_grease_version,
  ssl_grease_ticket_extension,
  ssl_grease_last_index = ssl_grease_ticket_extension,
};

enum tls12_server_hs_state_t {
  state12_start_accept = 0,
  state12_read_client_hello,
  state12_select_certificate,
  state12_tls13,
  state12_select_parameters,
  state12_send_server_hello,
  state12_send_server_certificate,
  state12_send_server_key_exchange,
  state12_send_server_hello_done,
  state12_read_client_certificate,
  state12_verify_client_certificate,
  state12_read_client_key_exchange,
  state12_read_client_certificate_verify,
  state12_read_change_cipher_spec,
  state12_process_change_cipher_spec,
  state12_read_next_proto,
  state12_read_channel_id,
  state12_read_client_finished,
  state12_send_server_finished,
  state12_finish_server_handshake,
  state12_done,
};

// These are the different points at which key material is no longer needed.
enum handback_t {
  handback_after_session_resumption,
  handback_after_ecdhe,
  handback_after_handshake,
};


// draft-ietf-tls-subcerts-03.
struct DC {
  static constexpr bool kAllowUniquePtr = true;
  ~DC();

  UniquePtr<DC> Dup();



  static UniquePtr<DC> Parse(CRYPTO_BUFFER *in, uint8_t *out_alert);


  UniquePtr<CRYPTO_BUFFER> raw;


  uint16_t expected_cert_verify_algorithm = 0;

  UniquePtr<EVP_PKEY> pkey;

 private:
  friend DC* New<DC>();
  DC();
};

// delegated credentials and this host has sent a delegated credential in
// response. If this is true then we've committed to using the DC in the
// handshake.
bool ssl_signing_with_dc(const SSL_HANDSHAKE *hs);


struct SSL_HANDSHAKE {
  explicit SSL_HANDSHAKE(SSL *ssl);
  ~SSL_HANDSHAKE();
  static constexpr bool kAllowUniquePtr = true;

  SSL *ssl;

  SSL_CONFIG *config;


  enum ssl_hs_wait_t wait = ssl_hs_ok;


  int state = 0;


  int tls13_state = 0;


  uint16_t min_version = 0;


  uint16_t max_version = 0;

 private:
  size_t hash_len_ = 0;
  uint8_t secret_[SSL_MAX_MD_SIZE] = {0};
  uint8_t early_traffic_secret_[SSL_MAX_MD_SIZE] = {0};
  uint8_t client_handshake_secret_[SSL_MAX_MD_SIZE] = {0};
  uint8_t server_handshake_secret_[SSL_MAX_MD_SIZE] = {0};
  uint8_t client_traffic_secret_0_[SSL_MAX_MD_SIZE] = {0};
  uint8_t server_traffic_secret_0_[SSL_MAX_MD_SIZE] = {0};
  uint8_t expected_client_finished_[SSL_MAX_MD_SIZE] = {0};

 public:
  void ResizeSecrets(size_t hash_len);

  Span<uint8_t> secret() { return MakeSpan(secret_, hash_len_); }
  Span<uint8_t> early_traffic_secret() {
    return MakeSpan(early_traffic_secret_, hash_len_);
  }
  Span<uint8_t> client_handshake_secret() {
    return MakeSpan(client_handshake_secret_, hash_len_);
  }
  Span<uint8_t> server_handshake_secret() {
    return MakeSpan(server_handshake_secret_, hash_len_);
  }
  Span<uint8_t> client_traffic_secret_0() {
    return MakeSpan(client_traffic_secret_0_, hash_len_);
  }
  Span<uint8_t> server_traffic_secret_0() {
    return MakeSpan(server_traffic_secret_0_, hash_len_);
  }
  Span<uint8_t> expected_client_finished() {
    return MakeSpan(expected_client_finished_, hash_len_);
  }

  union {



    uint32_t sent = 0;


    uint32_t received;
  } extensions;


  uint16_t retry_group = 0;

  UniquePtr<ERR_SAVE_STATE> error;



  UniquePtr<SSLKeyShare> key_shares[2];

  SSLTranscript transcript;

  Array<uint8_t> cookie;


  Array<uint8_t> key_share_bytes;


  Array<uint8_t> ecdh_public_key;



  Array<uint16_t> peer_sigalgs;



  Array<uint16_t> peer_supported_group_list;

  Array<uint8_t> peer_key;




  uint16_t negotiated_token_binding_version;



  uint16_t cert_compression_alg_id;



  Array<uint8_t> server_params;


  UniquePtr<char> peer_psk_identity_hint;


  UniquePtr<STACK_OF(CRYPTO_BUFFER)> ca_names;



  STACK_OF(X509_NAME) *cached_x509_ca_names = nullptr;


  Array<uint8_t> certificate_types;

  UniquePtr<EVP_PKEY> local_pubkey;

  UniquePtr<EVP_PKEY> peer_pubkey;


  UniquePtr<SSL_SESSION> new_session;


  UniquePtr<SSL_SESSION> early_session;

  const SSL_CIPHER *new_cipher = nullptr;

  Array<uint8_t> key_block;

  bool scts_requested : 1;


  bool needs_psk_binder : 1;

  bool received_hello_retry_request : 1;
  bool sent_hello_retry_request : 1;


  bool handshake_finalized : 1;


  bool accept_psk_mode : 1;

  bool cert_request : 1;



  bool certificate_status_expected : 1;

  bool ocsp_stapling_requested : 1;


  bool delegated_credential_requested : 1;


  bool should_ack_sni : 1;


  bool in_false_start : 1;


  bool in_early_data : 1;

  bool early_data_offered : 1;


  bool can_early_read : 1;


  bool can_early_write : 1;

  bool next_proto_neg_seen : 1;


  bool ticket_expected : 1;


  bool extended_master_secret : 1;


  bool pending_private_key_op : 1;

  bool grease_seeded : 1;




  bool handback : 1;

  bool cert_compression_negotiated : 1;


  bool apply_jdk11_workaround : 1;

  uint16_t client_version = 0;


  uint16_t early_data_read = 0;


  uint16_t early_data_written = 0;

  uint8_t session_id[SSL_MAX_SSL_SESSION_ID_LENGTH] = {0};
  uint8_t session_id_len = 0;


  uint8_t grease_seed[ssl_grease_last_index + 1] = {0};
};

UniquePtr<SSL_HANDSHAKE> ssl_handshake_new(SSL *ssl);

// one. Otherwise, it sends an alert and returns zero.
bool ssl_check_message_type(SSL *ssl, const SSLMessage &msg, int type);

// on error. It sets |out_early_return| to one if we've completed the handshake
// early.
int ssl_run_handshake(SSL_HANDSHAKE *hs, bool *out_early_return);

// server.
enum ssl_hs_wait_t ssl_client_handshake(SSL_HANDSHAKE *hs);
enum ssl_hs_wait_t ssl_server_handshake(SSL_HANDSHAKE *hs);
enum ssl_hs_wait_t tls13_client_handshake(SSL_HANDSHAKE *hs);
enum ssl_hs_wait_t tls13_server_handshake(SSL_HANDSHAKE *hs);

// handshake states for debugging.
const char *ssl_client_handshake_state(SSL_HANDSHAKE *hs);
const char *ssl_server_handshake_state(SSL_HANDSHAKE *hs);
const char *tls13_client_handshake_state(SSL_HANDSHAKE *hs);
const char *tls13_server_handshake_state(SSL_HANDSHAKE *hs);

// |update_requested| argument must be one of |SSL_KEY_UPDATE_REQUESTED| or
// |SSL_KEY_UPDATE_NOT_REQUESTED|.
bool tls13_add_key_update(SSL *ssl, int update_requested);

// success and false on failure.
bool tls13_post_handshake(SSL *ssl, const SSLMessage &msg);

bool tls13_process_certificate(SSL_HANDSHAKE *hs, const SSLMessage &msg,
                               bool allow_anonymous);
bool tls13_process_certificate_verify(SSL_HANDSHAKE *hs, const SSLMessage &msg);

// peer. If |use_saved_value| is true, the verify_data is compared against
// |hs->expected_client_finished| rather than computed fresh.
bool tls13_process_finished(SSL_HANDSHAKE *hs, const SSLMessage &msg,
                            bool use_saved_value);

bool tls13_add_certificate(SSL_HANDSHAKE *hs);

// handshake. If it returns |ssl_private_key_retry|, it should be called again
// to retry when the signing operation is completed.
enum ssl_private_key_result_t tls13_add_certificate_verify(SSL_HANDSHAKE *hs);

bool tls13_add_finished(SSL_HANDSHAKE *hs);
bool tls13_process_new_session_ticket(SSL *ssl, const SSLMessage &msg);

bool ssl_ext_key_share_parse_serverhello(SSL_HANDSHAKE *hs,
                                         Array<uint8_t> *out_secret,
                                         uint8_t *out_alert, CBS *contents);
bool ssl_ext_key_share_parse_clienthello(SSL_HANDSHAKE *hs, bool *out_found,
                                         Array<uint8_t> *out_secret,
                                         uint8_t *out_alert, CBS *contents);
bool ssl_ext_key_share_add_serverhello(SSL_HANDSHAKE *hs, CBB *out);

bool ssl_ext_pre_shared_key_parse_serverhello(SSL_HANDSHAKE *hs,
                                              uint8_t *out_alert,
                                              CBS *contents);
bool ssl_ext_pre_shared_key_parse_clienthello(
    SSL_HANDSHAKE *hs, CBS *out_ticket, CBS *out_binders,
    uint32_t *out_obfuscated_ticket_age, uint8_t *out_alert,
    const SSL_CLIENT_HELLO *client_hello, CBS *contents);
bool ssl_ext_pre_shared_key_add_serverhello(SSL_HANDSHAKE *hs, CBB *out);

// returns whether it's valid.
bool ssl_is_sct_list_valid(const CBS *contents);

bool ssl_write_client_hello(SSL_HANDSHAKE *hs);

enum ssl_cert_verify_context_t {
  ssl_cert_verify_server,
  ssl_cert_verify_client,
  ssl_cert_verify_channel_id,
};

// TLS 1.3's CertificateVerify message. |cert_verify_context| determines the
// type of signature. It sets |*out| to a newly allocated buffer containing the
// result. This function returns true on success and false on failure.
bool tls13_get_cert_verify_signature_input(
    SSL_HANDSHAKE *hs, Array<uint8_t> *out,
    enum ssl_cert_verify_context_t cert_verify_context);

// selection for |hs->ssl|'s client preferences.
bool ssl_is_alpn_protocol_allowed(const SSL_HANDSHAKE *hs,
                                  Span<const uint8_t> protocol);

// true on successful negotiation or if nothing was negotiated. It returns false
// and sets |*out_alert| to an alert on error.
bool ssl_negotiate_alpn(SSL_HANDSHAKE *hs, uint8_t *out_alert,
                        const SSL_CLIENT_HELLO *client_hello);

struct SSL_EXTENSION_TYPE {
  uint16_t type;
  bool *out_present;
  CBS *out_data;
};

// it. It writes the parsed extensions to pointers denoted by |ext_types|. On
// success, it fills in the |out_present| and |out_data| fields and returns one.
// Otherwise, it sets |*out_alert| to an alert to send and returns zero. Unknown
// extensions are rejected unless |ignore_unknown| is 1.
int ssl_parse_extensions(const CBS *cbs, uint8_t *out_alert,
                         const SSL_EXTENSION_TYPE *ext_types,
                         size_t num_ext_types, int ignore_unknown);

enum ssl_verify_result_t ssl_verify_peer_cert(SSL_HANDSHAKE *hs);
// ssl_reverify_peer_cert verifies the peer certificate for |hs| when resuming a
// session.
enum ssl_verify_result_t ssl_reverify_peer_cert(SSL_HANDSHAKE *hs);

enum ssl_hs_wait_t ssl_get_finished(SSL_HANDSHAKE *hs);
bool ssl_send_finished(SSL_HANDSHAKE *hs);
bool ssl_output_cert_chain(SSL_HANDSHAKE *hs);


// |ssl|. It returns true on success and false on failure.
bool ssl_log_secret(const SSL *ssl, const char *label,
                    Span<const uint8_t> secret);


bool ssl_client_hello_init(const SSL *ssl, SSL_CLIENT_HELLO *out,
                           const SSLMessage &msg);

bool ssl_client_hello_get_extension(const SSL_CLIENT_HELLO *client_hello,
                                    CBS *out, uint16_t extension_type);

bool ssl_client_cipher_list_contains_cipher(
    const SSL_CLIENT_HELLO *client_hello, uint16_t id);


// connection, the values for each index will be deterministic. This allows the
// same ClientHello be sent twice for a HelloRetryRequest or the same group be
// advertised in both supported_groups and key_shares.
uint16_t ssl_get_grease_value(SSL_HANDSHAKE *hs, enum ssl_grease_index_t index);


// algorithms and saves them on |hs|. It returns true on success and false on
// error.
bool tls1_parse_peer_sigalgs(SSL_HANDSHAKE *hs, const CBS *sigalgs);

// that should be used with |pkey| in TLS 1.1 and earlier. It returns true on
// success and false if |pkey| may not be used at those versions.
bool tls1_get_legacy_signature_algorithm(uint16_t *out, const EVP_PKEY *pkey);

// with |hs|'s private key based on the peer's preferences and the algorithms
// supported. It returns true on success and false on error.
bool tls1_choose_signature_algorithm(SSL_HANDSHAKE *hs, uint16_t *out);

// peer indicated support.
//
// NOTE: The related function |SSL_get0_peer_verify_algorithms| only has
// well-defined behavior during the callbacks set by |SSL_CTX_set_cert_cb| and
// |SSL_CTX_set_client_cert_cb|, or when the handshake is paused because of
// them.
Span<const uint16_t> tls1_get_peer_verify_algorithms(const SSL_HANDSHAKE *hs);

// peer signature to |out|. It returns true on success and false on error. If
// |for_certs| is true, the potentially more restrictive list of algorithms for
// certificates is used. Otherwise, the online signature one is used.
bool tls12_add_verify_sigalgs(const SSL *ssl, CBB *out, bool for_certs);

// signature. It returns true on success and false on error, setting
// |*out_alert| to an alert to send.
bool tls12_check_peer_sigalg(const SSL *ssl, uint8_t *out_alert,
                             uint16_t sigalg);

// different, more restrictive, list of signature algorithms acceptable for the
// certificate than the online signature.
bool tls12_has_different_verify_sigalgs_for_certs(const SSL *ssl);

//
// Functions below here haven't been touched up and may be underdocumented.

#define TLSEXT_CHANNEL_ID_SIZE 128

#define NAMED_CURVE_TYPE 3

struct CERT {
  static constexpr bool kAllowUniquePtr = true;

  explicit CERT(const SSL_X509_METHOD *x509_method);
  ~CERT();

  UniquePtr<EVP_PKEY> privatekey;






  UniquePtr<STACK_OF(CRYPTO_BUFFER)> chain;



  STACK_OF(X509) *x509_chain = nullptr;



  X509 *x509_leaf = nullptr;



  X509 *x509_stash = nullptr;


  const SSL_PRIVATE_KEY_METHOD *key_method = nullptr;


  const SSL_X509_METHOD *x509_method = nullptr;


  Array<uint16_t> sigalgs;






  int (*cert_cb)(SSL *ssl, void *arg) = nullptr;
  void *cert_cb_arg = nullptr;


  X509_STORE *verify_store = nullptr;

  UniquePtr<CRYPTO_BUFFER> signed_cert_timestamp_list;

  UniquePtr<CRYPTO_BUFFER> ocsp_response;


  uint8_t sid_ctx_length = 0;
  uint8_t sid_ctx[SSL_MAX_SID_CTX_LENGTH] = {0};


  UniquePtr<DC> dc = nullptr;


  UniquePtr<EVP_PKEY> dc_privatekey = nullptr;


  const SSL_PRIVATE_KEY_METHOD *dc_key_method = nullptr;
};

struct SSL_PROTOCOL_METHOD {
  bool is_dtls;
  bool (*ssl_new)(SSL *ssl);
  void (*ssl_free)(SSL *ssl);


  bool (*get_message)(const SSL *ssl, SSLMessage *out);

  void (*next_message)(SSL *ssl);

  ssl_open_record_t (*open_handshake)(SSL *ssl, size_t *out_consumed,
                                      uint8_t *out_alert, Span<uint8_t> in);

  ssl_open_record_t (*open_change_cipher_spec)(SSL *ssl, size_t *out_consumed,
                                               uint8_t *out_alert,
                                               Span<uint8_t> in);

  ssl_open_record_t (*open_app_data)(SSL *ssl, Span<uint8_t> *out,
                                     size_t *out_consumed, uint8_t *out_alert,
                                     Span<uint8_t> in);
  int (*write_app_data)(SSL *ssl, bool *out_needs_handshake, const uint8_t *buf,
                        int len);
  int (*dispatch_alert)(SSL *ssl);



  bool (*init_message)(SSL *ssl, CBB *cbb, CBB *body, uint8_t type);


  bool (*finish_message)(SSL *ssl, CBB *cbb, bssl::Array<uint8_t> *out_msg);


  bool (*add_message)(SSL *ssl, bssl::Array<uint8_t> msg);


  bool (*add_change_cipher_spec)(SSL *ssl);


  int (*flush_flight)(SSL *ssl);

  void (*on_handshake_complete)(SSL *ssl);



  bool (*set_read_state)(SSL *ssl, UniquePtr<SSLAEADContext> aead_ctx);



  bool (*set_write_state)(SSL *ssl, UniquePtr<SSLAEADContext> aead_ctx);
};


// message.
ssl_open_record_t ssl_open_handshake(SSL *ssl, size_t *out_consumed,
                                     uint8_t *out_alert, Span<uint8_t> in);

// ChangeCipherSpec.
ssl_open_record_t ssl_open_change_cipher_spec(SSL *ssl, size_t *out_consumed,
                                              uint8_t *out_alert,
                                              Span<uint8_t> in);

// On success, it returns |ssl_open_record_success| and sets |*out| to the
// input. If it encounters a post-handshake message, it returns
// |ssl_open_record_discard|. The caller should then retry, after processing any
// messages received with |get_message|.
ssl_open_record_t ssl_open_app_data(SSL *ssl, Span<uint8_t> *out,
                                    size_t *out_consumed, uint8_t *out_alert,
                                    Span<uint8_t> in);

struct SSL_X509_METHOD {



  bool (*check_client_CA_list)(STACK_OF(CRYPTO_BUFFER) *names);

  void (*cert_clear)(CERT *cert);

  void (*cert_free)(CERT *cert);



  void (*cert_dup)(CERT *new_cert, const CERT *cert);
  void (*cert_flush_cached_chain)(CERT *cert);


  void (*cert_flush_cached_leaf)(CERT *cert);



  bool (*session_cache_objects)(SSL_SESSION *session);


  bool (*session_dup)(SSL_SESSION *new_session, const SSL_SESSION *session);

  void (*session_clear)(SSL_SESSION *session);



  bool (*session_verify_cert_chain)(SSL_SESSION *session, SSL_HANDSHAKE *ssl,
                                    uint8_t *out_alert);

  void (*hs_flush_cached_ca_names)(SSL_HANDSHAKE *hs);


  bool (*ssl_new)(SSL_HANDSHAKE *hs);

  void (*ssl_config_free)(SSL_CONFIG *cfg);

  void (*ssl_flush_cached_client_CA)(SSL_CONFIG *cfg);



  bool (*ssl_auto_chain_if_needed)(SSL_HANDSHAKE *hs);


  bool (*ssl_ctx_new)(SSL_CTX *ctx);

  void (*ssl_ctx_free)(SSL_CTX *ctx);

  void (*ssl_ctx_flush_cached_client_CA)(SSL_CTX *ssl);
};

// crypto/x509.
extern const SSL_X509_METHOD ssl_crypto_x509_method;

// crypto/x509.
extern const SSL_X509_METHOD ssl_noop_x509_method;

struct TicketKey {
  static constexpr bool kAllowUniquePtr = true;

  uint8_t name[SSL_TICKET_KEY_NAME_LEN] = {0};
  uint8_t hmac_key[16] = {0};
  uint8_t aes_key[16] = {0};




  uint64_t next_rotation_tv_sec = 0;
};

struct CertCompressionAlg {
  static constexpr bool kAllowUniquePtr = true;

  ssl_cert_compression_func_t compress = nullptr;
  ssl_cert_decompression_func_t decompress = nullptr;
  uint16_t alg_id = 0;
};

BSSL_NAMESPACE_END

DEFINE_LHASH_OF(SSL_SESSION)

DEFINE_NAMED_STACK_OF(CertCompressionAlg, bssl::CertCompressionAlg)

BSSL_NAMESPACE_BEGIN

// whether it is alive or has been shutdown via close_notify or fatal alert.
enum ssl_shutdown_t {
  ssl_shutdown_none = 0,
  ssl_shutdown_close_notify = 1,
  ssl_shutdown_error = 2,
};

struct SSL3_STATE {
  static constexpr bool kAllowUniquePtr = true;

  SSL3_STATE();
  ~SSL3_STATE();

  uint8_t read_sequence[8] = {0};
  uint8_t write_sequence[8] = {0};

  uint8_t server_random[SSL3_RANDOM_SIZE] = {0};
  uint8_t client_random[SSL3_RANDOM_SIZE] = {0};

  SSLBuffer read_buffer;

  SSLBuffer write_buffer;


  Span<uint8_t> pending_app_data;

  unsigned int wnum = 0;  // number of bytes sent so far
  int wpend_tot = 0;      // number bytes written
  int wpend_type = 0;
  int wpend_ret = 0;  // number of bytes submitted
  const uint8_t *wpend_buf = nullptr;

  enum ssl_shutdown_t read_shutdown = ssl_shutdown_none;

  enum ssl_shutdown_t write_shutdown = ssl_shutdown_none;


  UniquePtr<ERR_SAVE_STATE> read_error;

  int total_renegotiations = 0;



  int rwstate = SSL_ERROR_NONE;

  enum ssl_encryption_level_t read_level = ssl_encryption_initial;
  enum ssl_encryption_level_t write_level = ssl_encryption_initial;


  uint16_t early_data_skipped = 0;

  uint8_t empty_record_count = 0;


  uint8_t warning_alert_count = 0;

  uint8_t key_update_count = 0;


  uint8_t negotiated_token_binding_param = 0;


  bool skip_early_data : 1;


  bool have_version : 1;


  bool v2_hello_done : 1;


  bool is_v2_hello : 1;


  bool has_message : 1;


  bool initial_handshake_complete : 1;

  bool session_reused : 1;


  bool delegated_credential_used : 1;

  bool send_connection_binding : 1;



  bool channel_id_valid : 1;


  bool key_update_pending : 1;

  bool wpend_pending : 1;

  bool early_data_accepted : 1;

  bool tls13_downgrade : 1;

  bool token_binding_negotiated : 1;


  bool pq_experiment_signal_seen : 1;

  bool alert_dispatch : 1;

  UniquePtr<BUF_MEM> hs_buf;



  UniquePtr<BUF_MEM> pending_hs_data;



  UniquePtr<BUF_MEM> pending_flight;


  uint32_t pending_flight_offset = 0;



  int32_t ticket_age_skew = 0;

  enum ssl_early_data_reason_t early_data_reason = ssl_early_data_unknown;

  UniquePtr<SSLAEADContext> aead_read_ctx;

  UniquePtr<SSLAEADContext> aead_write_ctx;


  UniquePtr<SSL_HANDSHAKE> hs;

  uint8_t write_traffic_secret[SSL_MAX_MD_SIZE] = {0};
  uint8_t read_traffic_secret[SSL_MAX_MD_SIZE] = {0};
  uint8_t exporter_secret[SSL_MAX_MD_SIZE] = {0};
  uint8_t write_traffic_secret_len = 0;
  uint8_t read_traffic_secret_len = 0;
  uint8_t exporter_secret_len = 0;

  uint8_t previous_client_finished[12] = {0};
  uint8_t previous_client_finished_len = 0;
  uint8_t previous_server_finished_len = 0;
  uint8_t previous_server_finished[12] = {0};

  uint8_t send_alert[2] = {0};



  UniquePtr<SSL_SESSION> established_session;






  Array<uint8_t> next_proto_negotiated;





  Array<uint8_t> alpn_selected;

  UniquePtr<char> hostname;




  uint8_t channel_id[64] = {0};

  Array<uint8_t> peer_quic_transport_params;


  const SRTP_PROTECTION_PROFILE *srtp_profile = nullptr;
};

#define DTLS1_COOKIE_LENGTH 256

#define DTLS1_RT_HEADER_LENGTH 13

#define DTLS1_HM_HEADER_LENGTH 12

#define DTLS1_CCS_HEADER_LENGTH 1

#define DTLS1_AL_HEADER_LENGTH 2

struct hm_header_st {
  uint8_t type;
  uint32_t msg_len;
  uint16_t seq;
  uint32_t frag_off;
  uint32_t frag_len;
};

struct hm_fragment {
  static constexpr bool kAllowUniquePtr = true;

  hm_fragment() {}
  hm_fragment(const hm_fragment &) = delete;
  hm_fragment &operator=(const hm_fragment &) = delete;

  ~hm_fragment();

  uint8_t type = 0;

  uint16_t seq = 0;

  uint32_t msg_len = 0;


  uint8_t *data = nullptr;


  uint8_t *reassembly = nullptr;
};

struct OPENSSL_timeval {
  uint64_t tv_sec;
  uint32_t tv_usec;
};

struct DTLS1_STATE {
  static constexpr bool kAllowUniquePtr = true;

  DTLS1_STATE();
  ~DTLS1_STATE();


  bool has_change_cipher_spec : 1;



  bool outgoing_messages_complete : 1;



  bool flight_has_reply : 1;

  uint8_t cookie[DTLS1_COOKIE_LENGTH] = {0};
  size_t cookie_len = 0;


  uint16_t r_epoch = 0;
  uint16_t w_epoch = 0;

  DTLS1_BITMAP bitmap;

  uint16_t handshake_write_seq = 0;
  uint16_t handshake_read_seq = 0;

  uint8_t last_write_sequence[8] = {0};
  UniquePtr<SSLAEADContext> last_aead_write_ctx;




  UniquePtr<hm_fragment> incoming_messages[SSL_MAX_HANDSHAKE_FLIGHT];


  DTLS_OUTGOING_MESSAGE outgoing_messages[SSL_MAX_HANDSHAKE_FLIGHT];
  uint8_t outgoing_messages_len = 0;


  uint8_t outgoing_written = 0;


  uint32_t outgoing_offset = 0;

  unsigned mtu = 0;  // max DTLS packet size


  unsigned num_timeouts = 0;


  struct OPENSSL_timeval next_timeout = {0, 0};

  unsigned timeout_duration_ms = 0;
};

// completes.  Objects of this type are not shared; they are unique to a
// particular |SSL|.
//
// See SSL_shed_handshake_config() for more about the conditions under which
// configuration can be shed.
struct SSL_CONFIG {
  static constexpr bool kAllowUniquePtr = true;

  explicit SSL_CONFIG(SSL *ssl_arg);
  ~SSL_CONFIG();

  SSL *const ssl = nullptr;



  uint16_t conf_max_version = 0;



  uint16_t conf_min_version = 0;

  X509_VERIFY_PARAM *param = nullptr;

  UniquePtr<SSLCipherPreferenceList> cipher_list;


  UniquePtr<CERT> cert;

  int (*verify_callback)(int ok,
                         X509_STORE_CTX *ctx) =
      nullptr;  // fail if callback returns 0

  enum ssl_verify_result_t (*custom_verify_callback)(
      SSL *ssl, uint8_t *out_alert) = nullptr;


  UniquePtr<char> psk_identity_hint;

  unsigned (*psk_client_callback)(SSL *ssl, const char *hint, char *identity,
                                  unsigned max_identity_len, uint8_t *psk,
                                  unsigned max_psk_len) = nullptr;
  unsigned (*psk_server_callback)(SSL *ssl, const char *identity, uint8_t *psk,
                                  unsigned max_psk_len) = nullptr;

  UniquePtr<STACK_OF(CRYPTO_BUFFER)> client_CA;


  STACK_OF(X509_NAME) *cached_x509_client_CA = nullptr;

  Array<uint16_t> supported_group_list;  // our list

  UniquePtr<EVP_PKEY> channel_id_private;


  Array<uint8_t> alpn_client_proto_list;

  Array<uint8_t> token_binding_params;

  Array<uint8_t> quic_transport_params;


  Array<uint16_t> verify_sigalgs;


  UniquePtr<STACK_OF(SRTP_PROTECTION_PROFILE)> srtp_profiles;

  uint8_t verify_mode = SSL_VERIFY_NONE;

  bool signed_cert_timestamps_enabled : 1;


  bool ocsp_stapling_enabled : 1;



  bool channel_id_enabled : 1;



  bool enforce_rsa_key_usage : 1;



  bool retain_only_sha256_of_client_certs : 1;




  bool handoff : 1;


  bool shed_handshake_config : 1;


  bool ignore_tls13_downgrade : 1;


  bool jdk11_workaround : 1;
};

#define SSL_PSK_DHE_KE 0x1

// data that will be accepted. This value should be slightly below
// kMaxEarlyDataSkipped in tls_record.c, which is measured in ciphertext.
static const size_t kMaxEarlyDataAccepted = 14336;

UniquePtr<CERT> ssl_cert_dup(CERT *cert);
void ssl_cert_clear_certs(CERT *cert);
bool ssl_set_cert(CERT *cert, UniquePtr<CRYPTO_BUFFER> buffer);
bool ssl_is_key_type_supported(int key_type);
// ssl_compare_public_and_private_key returns true if |pubkey| is the public
// counterpart to |privkey|. Otherwise it returns false and pushes a helpful
// message on the error queue.
bool ssl_compare_public_and_private_key(const EVP_PKEY *pubkey,
                                       const EVP_PKEY *privkey);
bool ssl_cert_check_private_key(const CERT *cert, const EVP_PKEY *privkey);
int ssl_get_new_session(SSL_HANDSHAKE *hs, int is_server);
int ssl_encrypt_ticket(SSL_HANDSHAKE *hs, CBB *out, const SSL_SESSION *session);
int ssl_ctx_rotate_ticket_encryption_key(SSL_CTX *ctx);

// error.
UniquePtr<SSL_SESSION> ssl_session_new(const SSL_X509_METHOD *x509_method);

// keyed on session IDs.
uint32_t ssl_hash_session_id(Span<const uint8_t> session_id);

// the parsed data.
OPENSSL_EXPORT UniquePtr<SSL_SESSION> SSL_SESSION_parse(
    CBS *cbs, const SSL_X509_METHOD *x509_method, CRYPTO_BUFFER_POOL *pool);

// session for Session-ID resumption. It returns one on success and zero on
// error.
OPENSSL_EXPORT int ssl_session_serialize(const SSL_SESSION *in, CBB *cbb);

// matches the one set on |hs| and zero otherwise.
int ssl_session_is_context_valid(const SSL_HANDSHAKE *hs,
                                 const SSL_SESSION *session);

// it has expired.
int ssl_session_is_time_valid(const SSL *ssl, const SSL_SESSION *session);

// zero otherwise.
int ssl_session_is_resumable(const SSL_HANDSHAKE *hs,
                             const SSL_SESSION *session);

// |session|. Note that despite the name, this is not the same as
// |SSL_SESSION_get_protocol_version|. The latter is based on upstream's name.
uint16_t ssl_session_protocol_version(const SSL_SESSION *session);

const EVP_MD *ssl_session_get_digest(const SSL_SESSION *session);

void ssl_set_session(SSL *ssl, SSL_SESSION *session);

// On success, it sets |*out_session| to the session or nullptr if none was
// found. If the session could not be looked up synchronously, it returns
// |ssl_hs_pending_session| and should be called again. If a ticket could not be
// decrypted immediately it returns |ssl_hs_pending_ticket| and should also
// be called again. Otherwise, it returns |ssl_hs_error|.
enum ssl_hs_wait_t ssl_get_prev_session(SSL_HANDSHAKE *hs,
                                        UniquePtr<SSL_SESSION> *out_session,
                                        bool *out_tickets_supported,
                                        bool *out_renew_ticket,
                                        const SSL_CLIENT_HELLO *client_hello);

#define SSL_SESSION_DUP_AUTH_ONLY 0x0
#define SSL_SESSION_INCLUDE_TICKET 0x1
#define SSL_SESSION_INCLUDE_NONAUTH 0x2
#define SSL_SESSION_DUP_ALL \
  (SSL_SESSION_INCLUDE_TICKET | SSL_SESSION_INCLUDE_NONAUTH)

// fields in |session| or nullptr on error. The new session is non-resumable and
// must be explicitly marked resumable once it has been filled in.
OPENSSL_EXPORT UniquePtr<SSL_SESSION> SSL_SESSION_dup(SSL_SESSION *session,
                                                      int dup_flags);

// adjusting the timeout so the expiration time is unchanged.
void ssl_session_rebase_time(SSL *ssl, SSL_SESSION *session);

// |session|'s timeout to |timeout| (measured from the current time). The
// renewal is clamped to the session's auth_timeout.
void ssl_session_renew_timeout(SSL *ssl, SSL_SESSION *session,
                               uint32_t timeout);

void ssl_update_cache(SSL_HANDSHAKE *hs, int mode);

void ssl_send_alert(SSL *ssl, int level, int desc);
int ssl_send_alert_impl(SSL *ssl, int level, int desc);
bool ssl3_get_message(const SSL *ssl, SSLMessage *out);
ssl_open_record_t ssl3_open_handshake(SSL *ssl, size_t *out_consumed,
                                      uint8_t *out_alert, Span<uint8_t> in);
void ssl3_next_message(SSL *ssl);

int ssl3_dispatch_alert(SSL *ssl);
ssl_open_record_t ssl3_open_app_data(SSL *ssl, Span<uint8_t> *out,
                                     size_t *out_consumed, uint8_t *out_alert,
                                     Span<uint8_t> in);
ssl_open_record_t ssl3_open_change_cipher_spec(SSL *ssl, size_t *out_consumed,
                                               uint8_t *out_alert,
                                               Span<uint8_t> in);
int ssl3_write_app_data(SSL *ssl, bool *out_needs_handshake, const uint8_t *buf,
                        int len);

bool ssl3_new(SSL *ssl);
void ssl3_free(SSL *ssl);

bool ssl3_init_message(SSL *ssl, CBB *cbb, CBB *body, uint8_t type);
bool ssl3_finish_message(SSL *ssl, CBB *cbb, Array<uint8_t> *out_msg);
bool ssl3_add_message(SSL *ssl, Array<uint8_t> msg);
bool ssl3_add_change_cipher_spec(SSL *ssl);
int ssl3_flush_flight(SSL *ssl);

bool dtls1_init_message(SSL *ssl, CBB *cbb, CBB *body, uint8_t type);
bool dtls1_finish_message(SSL *ssl, CBB *cbb, Array<uint8_t> *out_msg);
bool dtls1_add_message(SSL *ssl, Array<uint8_t> msg);
bool dtls1_add_change_cipher_spec(SSL *ssl);
int dtls1_flush_flight(SSL *ssl);

// the pending flight. It returns true on success and false on error.
bool ssl_add_message_cbb(SSL *ssl, CBB *cbb);

// on success and false on allocation failure.
bool ssl_hash_message(SSL_HANDSHAKE *hs, const SSLMessage &msg);

ssl_open_record_t dtls1_open_app_data(SSL *ssl, Span<uint8_t> *out,
                                      size_t *out_consumed, uint8_t *out_alert,
                                      Span<uint8_t> in);
ssl_open_record_t dtls1_open_change_cipher_spec(SSL *ssl, size_t *out_consumed,
                                                uint8_t *out_alert,
                                                Span<uint8_t> in);

int dtls1_write_app_data(SSL *ssl, bool *out_needs_handshake,
                         const uint8_t *buf, int len);

// error.
int dtls1_write_record(SSL *ssl, int type, const uint8_t *buf, size_t len,
                       enum dtls1_use_epoch_t use_epoch);

int dtls1_retransmit_outgoing_messages(SSL *ssl);
bool dtls1_parse_fragment(CBS *cbs, struct hm_header_st *out_hdr,
                          CBS *out_body);
bool dtls1_check_timeout_num(SSL *ssl);

void dtls1_start_timer(SSL *ssl);
void dtls1_stop_timer(SSL *ssl);
bool dtls1_is_timer_expired(SSL *ssl);
unsigned int dtls1_min_mtu(void);

bool dtls1_new(SSL *ssl);
void dtls1_free(SSL *ssl);

bool dtls1_get_message(const SSL *ssl, SSLMessage *out);
ssl_open_record_t dtls1_open_handshake(SSL *ssl, size_t *out_consumed,
                                       uint8_t *out_alert, Span<uint8_t> in);
void dtls1_next_message(SSL *ssl);
int dtls1_dispatch_alert(SSL *ssl);

// determined by |direction|) using the keys generated by the TLS KDF. The
// |key_block_cache| argument is used to store the generated key block, if
// empty. Otherwise it's assumed that the key block is already contained within
// it. Returns one on success or zero on error.
int tls1_configure_aead(SSL *ssl, evp_aead_direction_t direction,
                        Array<uint8_t> *key_block_cache,
                        const SSL_CIPHER *cipher,
                        Span<const uint8_t> iv_override);

int tls1_change_cipher_state(SSL_HANDSHAKE *hs, evp_aead_direction_t direction);
int tls1_generate_master_secret(SSL_HANDSHAKE *hs, uint8_t *out,
                                Span<const uint8_t> premaster);

Span<const uint16_t> tls1_get_grouplist(const SSL_HANDSHAKE *ssl);

// configured group preferences.
bool tls1_check_group_id(const SSL_HANDSHAKE *ssl, uint16_t group_id);

// group between client and server preferences and returns true. If none may be
// found, it returns false.
bool tls1_get_shared_group(SSL_HANDSHAKE *hs, uint16_t *out_group_id);

// array of TLS group IDs. On success, the function returns true and writes the
// array to |*out_group_ids|. Otherwise, it returns false.
bool tls1_set_curves(Array<uint16_t> *out_group_ids, Span<const int> curves);

// into a newly allocated array of TLS group IDs. On success, the function
// returns true and writes the array to |*out_group_ids|. Otherwise, it returns
// false.
bool tls1_set_curves_list(Array<uint16_t> *out_group_ids, const char *curves);

// true on success and false on failure. The |header_len| argument is the length
// of the ClientHello written so far and is used to compute the padding length.
// (It does not include the record header.)
bool ssl_add_clienthello_tlsext(SSL_HANDSHAKE *hs, CBB *out, size_t header_len);

bool ssl_add_serverhello_tlsext(SSL_HANDSHAKE *hs, CBB *out);
bool ssl_parse_clienthello_tlsext(SSL_HANDSHAKE *hs,
                                  const SSL_CLIENT_HELLO *client_hello);
bool ssl_parse_serverhello_tlsext(SSL_HANDSHAKE *hs, CBS *cbs);

#define tlsext_tick_md EVP_sha256

// one of:
//   |ssl_ticket_aead_success|: |*out_session| is set to the parsed session and
//       |*out_renew_ticket| is set to whether the ticket should be renewed.
//   |ssl_ticket_aead_ignore_ticket|: |*out_renew_ticket| is set to whether a
//       fresh ticket should be sent, but the given ticket cannot be used.
//   |ssl_ticket_aead_retry|: the ticket could not be immediately decrypted.
//       Retry later.
//   |ssl_ticket_aead_error|: an error occured that is fatal to the connection.
enum ssl_ticket_aead_result_t ssl_process_ticket(
    SSL_HANDSHAKE *hs, UniquePtr<SSL_SESSION> *out_session,
    bool *out_renew_ticket, Span<const uint8_t> ticket,
    Span<const uint8_t> session_id);

// the signature. If the key is valid, it saves the Channel ID and returns true.
// Otherwise, it returns false.
bool tls1_verify_channel_id(SSL_HANDSHAKE *hs, const SSLMessage &msg);

// |cbb|. |ssl->channel_id_private| must already be set before calling.  This
// function returns true on success and false on error.
bool tls1_write_channel_id(SSL_HANDSHAKE *hs, CBB *cbb);

// it to |out|, which must contain at least |EVP_MAX_MD_SIZE| bytes. It returns
// true on success and false on failure.
bool tls1_channel_id_hash(SSL_HANDSHAKE *hs, uint8_t *out, size_t *out_len);

// hashes in |hs->new_session| so that Channel ID resumptions can sign that
// data.
bool tls1_record_handshake_hashes_for_channel_id(SSL_HANDSHAKE *hs);

// necessary. It returns true on success and false on fatal error. Note that, on
// success, |hs->ssl->channel_id_private| may be unset, in which case the
// operation should be retried later.
bool ssl_do_channel_id_callback(SSL_HANDSHAKE *hs);

bool ssl_can_write(const SSL *ssl);

bool ssl_can_read(const SSL *ssl);

void ssl_get_current_time(const SSL *ssl, struct OPENSSL_timeval *out_clock);
void ssl_ctx_get_current_time(const SSL_CTX *ctx,
                              struct OPENSSL_timeval *out_clock);

void ssl_reset_error_state(SSL *ssl);

// current state of the error queue.
void ssl_set_read_error(SSL *ssl);

BSSL_NAMESPACE_END

//
// The following types are exported to C code as public typedefs, so they must
// be defined outside of the namespace.

// structure to support the legacy version-locked methods.
struct ssl_method_st {


  uint16_t version;


  const bssl::SSL_PROTOCOL_METHOD *method;


  const bssl::SSL_X509_METHOD *x509_method;
};

struct ssl_ctx_st {
  explicit ssl_ctx_st(const SSL_METHOD *ssl_method);
  ssl_ctx_st(const ssl_ctx_st &) = delete;
  ssl_ctx_st &operator=(const ssl_ctx_st &) = delete;

  const bssl::SSL_PROTOCOL_METHOD *method = nullptr;
  const bssl::SSL_X509_METHOD *x509_method = nullptr;

  CRYPTO_MUTEX lock;



  uint16_t conf_max_version = 0;



  uint16_t conf_min_version = 0;

  const SSL_QUIC_METHOD *quic_method = nullptr;

  bssl::UniquePtr<bssl::SSLCipherPreferenceList> cipher_list;

  X509_STORE *cert_store = nullptr;
  LHASH_OF(SSL_SESSION) *sessions = nullptr;


  unsigned long session_cache_size = SSL_SESSION_CACHE_MAX_SIZE_DEFAULT;
  SSL_SESSION *session_cache_head = nullptr;
  SSL_SESSION *session_cache_tail = nullptr;


  int handshakes_since_cache_flush = 0;





  int session_cache_mode = SSL_SESS_CACHE_SERVER;


  uint32_t session_timeout = SSL_DEFAULT_SESSION_TIMEOUT;


  uint32_t session_psk_dhe_timeout = SSL_DEFAULT_SESSION_PSK_DHE_TIMEOUT;







  int (*new_session_cb)(SSL *ssl, SSL_SESSION *sess) = nullptr;
  void (*remove_session_cb)(SSL_CTX *ctx, SSL_SESSION *sess) = nullptr;
  SSL_SESSION *(*get_session_cb)(SSL *ssl, const uint8_t *data, int len,
                                 int *copy) = nullptr;

  CRYPTO_refcount_t references = 1;

  int (*app_verify_callback)(X509_STORE_CTX *store_ctx, void *arg) = nullptr;
  void *app_verify_arg = nullptr;

  ssl_verify_result_t (*custom_verify_callback)(SSL *ssl,
                                                uint8_t *out_alert) = nullptr;

  pem_password_cb *default_passwd_callback = nullptr;

  void *default_passwd_callback_userdata = nullptr;

  int (*client_cert_cb)(SSL *ssl, X509 **out_x509,
                        EVP_PKEY **out_pkey) = nullptr;

  void (*channel_id_cb)(SSL *ssl, EVP_PKEY **out_pkey) = nullptr;

  CRYPTO_EX_DATA ex_data;


  void (*info_callback)(const SSL *ssl, int type, int value) = nullptr;

  bssl::UniquePtr<STACK_OF(CRYPTO_BUFFER)> client_CA;


  STACK_OF(X509_NAME) *cached_x509_client_CA = nullptr;



  uint32_t options = 0;


  uint32_t mode = SSL_MODE_NO_AUTO_CHAIN;
  uint32_t max_cert_list = SSL_MAX_CERT_LIST_DEFAULT;

  bssl::UniquePtr<bssl::CERT> cert;

  void (*msg_callback)(int write_p, int version, int content_type,
                       const void *buf, size_t len, SSL *ssl,
                       void *arg) = nullptr;
  void *msg_callback_arg = nullptr;

  int verify_mode = SSL_VERIFY_NONE;
  int (*default_verify_callback)(int ok, X509_STORE_CTX *ctx) =
      nullptr;  // called 'verify_callback' in the SSL

  X509_VERIFY_PARAM *param = nullptr;



  ssl_select_cert_result_t (*select_certificate_cb)(const SSL_CLIENT_HELLO *) =
      nullptr;



  int (*dos_protection_cb)(const SSL_CLIENT_HELLO *) = nullptr;



  bool reverify_on_resume = false;


  uint16_t max_send_fragment = SSL3_RT_MAX_PLAIN_LENGTH;

  int (*servername_callback)(SSL *, int *, void *) = nullptr;
  void *servername_arg = nullptr;




  bssl::UniquePtr<bssl::TicketKey> ticket_key_current;
  bssl::UniquePtr<bssl::TicketKey> ticket_key_prev;

  int (*ticket_key_cb)(SSL *ssl, uint8_t *name, uint8_t *iv,
                       EVP_CIPHER_CTX *ectx, HMAC_CTX *hctx, int enc) = nullptr;


  bssl::UniquePtr<char> psk_identity_hint;

  unsigned (*psk_client_callback)(SSL *ssl, const char *hint, char *identity,
                                  unsigned max_identity_len, uint8_t *psk,
                                  unsigned max_psk_len) = nullptr;
  unsigned (*psk_server_callback)(SSL *ssl, const char *identity, uint8_t *psk,
                                  unsigned max_psk_len) = nullptr;




  int (*next_protos_advertised_cb)(SSL *ssl, const uint8_t **out,
                                   unsigned *out_len, void *arg) = nullptr;
  void *next_protos_advertised_cb_arg = nullptr;


  int (*next_proto_select_cb)(SSL *ssl, uint8_t **out, uint8_t *out_len,
                              const uint8_t *in, unsigned in_len,
                              void *arg) = nullptr;
  void *next_proto_select_cb_arg = nullptr;










  int (*alpn_select_cb)(SSL *ssl, const uint8_t **out, uint8_t *out_len,
                        const uint8_t *in, unsigned in_len,
                        void *arg) = nullptr;
  void *alpn_select_cb_arg = nullptr;


  bssl::Array<uint8_t> alpn_client_proto_list;

  bssl::UniquePtr<STACK_OF(SRTP_PROTECTION_PROFILE)> srtp_profiles;

  bssl::UniquePtr<STACK_OF(CertCompressionAlg)> cert_compression_algs;

  bssl::Array<uint16_t> supported_group_list;

  bssl::UniquePtr<EVP_PKEY> channel_id_private;


  void (*keylog_callback)(const SSL *ssl, const char *line) = nullptr;



  void (*current_time_cb)(const SSL *ssl, struct timeval *out_clock) = nullptr;


  CRYPTO_BUFFER_POOL *pool = nullptr;


  const SSL_TICKET_AEAD_METHOD *ticket_aead_method = nullptr;


  int (*legacy_ocsp_callback)(SSL *ssl, void *arg) = nullptr;
  void *legacy_ocsp_callback_arg = nullptr;


  bssl::Array<uint16_t> verify_sigalgs;



  bool retain_only_sha256_of_client_certs : 1;


  bool quiet_shutdown : 1;


  bool ocsp_stapling_enabled : 1;

  bool signed_cert_timestamps_enabled : 1;



  bool channel_id_enabled : 1;

  bool grease_enabled : 1;


  bool allow_unknown_alpn_protos : 1;

  bool ed25519_enabled : 1;


  bool rsa_pss_rsae_certs_enabled : 1;


  bool false_start_allowed_without_alpn : 1;


  bool ignore_tls13_downgrade:1;



  bool handoff : 1;

  bool enable_early_data : 1;



  bool pq_experiment_signal : 1;

 private:
  ~ssl_ctx_st();
  friend void SSL_CTX_free(SSL_CTX *);
};

struct ssl_st {
  explicit ssl_st(SSL_CTX *ctx_arg);
  ssl_st(const ssl_st &) = delete;
  ssl_st &operator=(const ssl_st &) = delete;
  ~ssl_st();


  const bssl::SSL_PROTOCOL_METHOD *method = nullptr;




  bssl::UniquePtr<bssl::SSL_CONFIG> config;

  uint16_t version = 0;

  uint16_t max_send_fragment = 0;



  bssl::UniquePtr<BIO> rbio;  // used by SSL_read
  bssl::UniquePtr<BIO> wbio;  // used by SSL_write



  bssl::ssl_hs_wait_t (*do_handshake)(bssl::SSL_HANDSHAKE *hs) = nullptr;

  bssl::SSL3_STATE *s3 = nullptr;   // TLS variables
  bssl::DTLS1_STATE *d1 = nullptr;  // DTLS variables

  void (*msg_callback)(int write_p, int version, int content_type,
                       const void *buf, size_t len, SSL *ssl,
                       void *arg) = nullptr;
  void *msg_callback_arg = nullptr;






  unsigned initial_timeout_duration_ms = 1000;


  bssl::UniquePtr<SSL_SESSION> session;

  void (*info_callback)(const SSL *ssl, int type, int value) = nullptr;

  bssl::UniquePtr<SSL_CTX> ctx;


  bssl::UniquePtr<SSL_CTX> session_ctx;

  CRYPTO_EX_DATA ex_data;

  uint32_t options = 0;  // protocol behaviour
  uint32_t mode = 0;     // API behaviour
  uint32_t max_cert_list = 0;
  bssl::UniquePtr<char> hostname;

  const SSL_QUIC_METHOD *quic_method = nullptr;

  ssl_renegotiate_mode_t renegotiate_mode = ssl_renegotiate_never;



  bool server : 1;


  bool quiet_shutdown : 1;

  bool enable_early_data : 1;
};

struct ssl_session_st {
  explicit ssl_session_st(const bssl::SSL_X509_METHOD *method);
  ssl_session_st(const ssl_session_st &) = delete;
  ssl_session_st &operator=(const ssl_session_st &) = delete;

  CRYPTO_refcount_t references = 1;

  uint16_t ssl_version = 0;


  uint16_t group_id = 0;


  uint16_t peer_signature_algorithm = 0;


  int master_key_length = 0;
  uint8_t master_key[SSL_MAX_MASTER_KEY_LENGTH] = {0};

  unsigned session_id_length = 0;
  uint8_t session_id[SSL_MAX_SSL_SESSION_ID_LENGTH] = {0};



  uint8_t sid_ctx_length = 0;
  uint8_t sid_ctx[SSL_MAX_SID_CTX_LENGTH] = {0};

  bssl::UniquePtr<char> psk_identity;


  bssl::UniquePtr<STACK_OF(CRYPTO_BUFFER)> certs;

  const bssl::SSL_X509_METHOD *x509_method = nullptr;

  X509 *x509_peer = nullptr;



  STACK_OF(X509) *x509_chain = nullptr;





  STACK_OF(X509) *x509_chain_without_leaf = nullptr;


  long verify_result = X509_V_ERR_INVALID_CALL;


  uint32_t timeout = SSL_DEFAULT_SESSION_TIMEOUT;


  uint32_t auth_timeout = SSL_DEFAULT_SESSION_TIMEOUT;


  uint64_t time = 0;

  const SSL_CIPHER *cipher = nullptr;

  CRYPTO_EX_DATA ex_data;  // application specific data


  SSL_SESSION *prev = nullptr, *next = nullptr;

  bssl::Array<uint8_t> ticket;

  bssl::UniquePtr<CRYPTO_BUFFER> signed_cert_timestamp_list;

  bssl::UniquePtr<CRYPTO_BUFFER> ocsp_response;


  uint8_t peer_sha256[SHA256_DIGEST_LENGTH] = {0};



  uint8_t original_handshake_hash[EVP_MAX_MD_SIZE] = {0};
  uint8_t original_handshake_hash_len = 0;

  uint32_t ticket_lifetime_hint = 0;  // Session lifetime hint in seconds

  uint32_t ticket_age_add = 0;


  uint32_t ticket_max_early_data = 0;



  bssl::Array<uint8_t> early_alpn;



  bool extended_master_secret : 1;

  bool peer_sha256_valid : 1;  // Non-zero if peer_sha256 is valid

  bool not_resumable : 1;

  bool ticket_age_add_valid : 1;

  bool is_server : 1;

 private:
  ~ssl_session_st();
  friend void SSL_SESSION_free(SSL_SESSION *);
};


#endif  // OPENSSL_HEADER_SSL_INTERNAL_H
