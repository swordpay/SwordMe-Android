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

#ifndef OPENSSL_HEADER_CIPHER_H
#define OPENSSL_HEADER_CIPHER_H

#include <openssl/base.h>

#if defined(__cplusplus)
extern "C" {
#endif


//
// The following functions return |EVP_CIPHER| objects that implement the named
// cipher algorithm.

OPENSSL_EXPORT const EVP_CIPHER *EVP_rc4(void);

OPENSSL_EXPORT const EVP_CIPHER *EVP_des_cbc(void);
OPENSSL_EXPORT const EVP_CIPHER *EVP_des_ecb(void);
OPENSSL_EXPORT const EVP_CIPHER *EVP_des_ede(void);
OPENSSL_EXPORT const EVP_CIPHER *EVP_des_ede3(void);
OPENSSL_EXPORT const EVP_CIPHER *EVP_des_ede_cbc(void);
OPENSSL_EXPORT const EVP_CIPHER *EVP_des_ede3_cbc(void);

OPENSSL_EXPORT const EVP_CIPHER *EVP_aes_128_ecb(void);
OPENSSL_EXPORT const EVP_CIPHER *EVP_aes_128_cbc(void);
OPENSSL_EXPORT const EVP_CIPHER *EVP_aes_128_ctr(void);
OPENSSL_EXPORT const EVP_CIPHER *EVP_aes_128_ofb(void);

OPENSSL_EXPORT const EVP_CIPHER *EVP_aes_256_ecb(void);
OPENSSL_EXPORT const EVP_CIPHER *EVP_aes_256_cbc(void);
OPENSSL_EXPORT const EVP_CIPHER *EVP_aes_256_ctr(void);
OPENSSL_EXPORT const EVP_CIPHER *EVP_aes_256_ofb(void);
OPENSSL_EXPORT const EVP_CIPHER *EVP_aes_256_xts(void);

// ciphertext.
OPENSSL_EXPORT const EVP_CIPHER *EVP_enc_null(void);

OPENSSL_EXPORT const EVP_CIPHER *EVP_rc2_cbc(void);

// is obviously very, very weak and is included only in order to read PKCS#12
// files, which often encrypt the certificate chain using this cipher. It is
// deliberately not exported.
const EVP_CIPHER *EVP_rc2_40_cbc(void);

// NULL if no such cipher is known.
OPENSSL_EXPORT const EVP_CIPHER *EVP_get_cipherbynid(int nid);

//
// An |EVP_CIPHER_CTX| represents the state of an encryption or decryption in
// progress.

OPENSSL_EXPORT void EVP_CIPHER_CTX_init(EVP_CIPHER_CTX *ctx);

// |EVP_CIPHER_CTX_init| and returns it, or NULL on allocation failure.
OPENSSL_EXPORT EVP_CIPHER_CTX *EVP_CIPHER_CTX_new(void);

// one.
OPENSSL_EXPORT int EVP_CIPHER_CTX_cleanup(EVP_CIPHER_CTX *ctx);

// |ctx| itself.
OPENSSL_EXPORT void EVP_CIPHER_CTX_free(EVP_CIPHER_CTX *ctx);

// |in|. The |out| argument must have been previously initialised.
OPENSSL_EXPORT int EVP_CIPHER_CTX_copy(EVP_CIPHER_CTX *out,
                                       const EVP_CIPHER_CTX *in);

// |EVP_CIPHER_CTX_init| and returns one.
OPENSSL_EXPORT int EVP_CIPHER_CTX_reset(EVP_CIPHER_CTX *ctx);


// |enc| is zero) operation using |cipher|. If |ctx| has been previously
// configured with a cipher then |cipher|, |key| and |iv| may be |NULL| and
// |enc| may be -1 to reuse the previous values. The operation will use |key|
// as the key and |iv| as the IV (if any). These should have the correct
// lengths given by |EVP_CIPHER_key_length| and |EVP_CIPHER_iv_length|. It
// returns one on success and zero on error.
OPENSSL_EXPORT int EVP_CipherInit_ex(EVP_CIPHER_CTX *ctx,
                                     const EVP_CIPHER *cipher, ENGINE *engine,
                                     const uint8_t *key, const uint8_t *iv,
                                     int enc);

OPENSSL_EXPORT int EVP_EncryptInit_ex(EVP_CIPHER_CTX *ctx,
                                      const EVP_CIPHER *cipher, ENGINE *impl,
                                      const uint8_t *key, const uint8_t *iv);

OPENSSL_EXPORT int EVP_DecryptInit_ex(EVP_CIPHER_CTX *ctx,
                                      const EVP_CIPHER *cipher, ENGINE *impl,
                                      const uint8_t *key, const uint8_t *iv);


// of output bytes may be up to |in_len| plus the block length minus one and
// |out| must have sufficient space. The number of bytes actually output is
// written to |*out_len|. It returns one on success and zero otherwise.
OPENSSL_EXPORT int EVP_EncryptUpdate(EVP_CIPHER_CTX *ctx, uint8_t *out,
                                     int *out_len, const uint8_t *in,
                                     int in_len);

// |*out_len| to the number of bytes written. If padding is enabled (the
// default) then standard padding is applied to create the final block. If
// padding is disabled (with |EVP_CIPHER_CTX_set_padding|) then any partial
// block remaining will cause an error. The function returns one on success and
// zero otherwise.
OPENSSL_EXPORT int EVP_EncryptFinal_ex(EVP_CIPHER_CTX *ctx, uint8_t *out,
                                       int *out_len);

// output bytes may be up to |in_len| plus the block length minus one and |out|
// must have sufficient space. The number of bytes actually output is written
// to |*out_len|. It returns one on success and zero otherwise.
OPENSSL_EXPORT int EVP_DecryptUpdate(EVP_CIPHER_CTX *ctx, uint8_t *out,
                                     int *out_len, const uint8_t *in,
                                     int in_len);

// |*out_len| to the number of bytes written. If padding is enabled (the
// default) then padding is removed from the final block.
//
// WARNING: it is unsafe to call this function with unauthenticated
// ciphertext if padding is enabled.
OPENSSL_EXPORT int EVP_DecryptFinal_ex(EVP_CIPHER_CTX *ctx, unsigned char *out,
                                       int *out_len);

// blocks are maintained between calls. However, any internal cipher state is
// still updated. For CBC-mode ciphers, the IV is updated to the final
// ciphertext block. For stream ciphers, the stream is advanced past the bytes
// used. It returns one on success and zero otherwise, unless |EVP_CIPHER_flags|
// has |EVP_CIPH_FLAG_CUSTOM_CIPHER| set. Then it returns the number of bytes
// written or -1 on error.
//
// WARNING: this differs from the usual return value convention when using
// |EVP_CIPH_FLAG_CUSTOM_CIPHER|.
//
// TODO(davidben): The normal ciphers currently never fail, even if, e.g.,
// |in_len| is not a multiple of the block size for CBC-mode decryption. The
// input just gets rounded up while the output gets truncated. This should
// either be officially documented or fail.
OPENSSL_EXPORT int EVP_Cipher(EVP_CIPHER_CTX *ctx, uint8_t *out,
                              const uint8_t *in, size_t in_len);

// depending on how |ctx| has been setup.
OPENSSL_EXPORT int EVP_CipherUpdate(EVP_CIPHER_CTX *ctx, uint8_t *out,
                                    int *out_len, const uint8_t *in,
                                    int in_len);

// |EVP_DecryptFinal_ex| depending on how |ctx| has been setup.
OPENSSL_EXPORT int EVP_CipherFinal_ex(EVP_CIPHER_CTX *ctx, uint8_t *out,
                                      int *out_len);


// none has been set.
OPENSSL_EXPORT const EVP_CIPHER *EVP_CIPHER_CTX_cipher(
    const EVP_CIPHER_CTX *ctx);

// |ctx| (e.g. |NID_aes_128_gcm|). It will crash if no cipher has been
// configured.
OPENSSL_EXPORT int EVP_CIPHER_CTX_nid(const EVP_CIPHER_CTX *ctx);

// and zero otherwise.
OPENSSL_EXPORT int EVP_CIPHER_CTX_encrypting(const EVP_CIPHER_CTX *ctx);

// underlying |ctx|, or one if the cipher is a stream cipher. It will crash if
// no cipher has been configured.
OPENSSL_EXPORT unsigned EVP_CIPHER_CTX_block_size(const EVP_CIPHER_CTX *ctx);

// underlying |ctx| or zero if no cipher has been configured.
OPENSSL_EXPORT unsigned EVP_CIPHER_CTX_key_length(const EVP_CIPHER_CTX *ctx);

// underlying |ctx|. It will crash if no cipher has been configured.
OPENSSL_EXPORT unsigned EVP_CIPHER_CTX_iv_length(const EVP_CIPHER_CTX *ctx);

// |ctx|, or NULL if none has been set.
OPENSSL_EXPORT void *EVP_CIPHER_CTX_get_app_data(const EVP_CIPHER_CTX *ctx);

// |ctx| to |data|.
OPENSSL_EXPORT void EVP_CIPHER_CTX_set_app_data(EVP_CIPHER_CTX *ctx,
                                                void *data);

// |EVP_CIPH_*| flags. It will crash if no cipher has been configured.
OPENSSL_EXPORT uint32_t EVP_CIPHER_CTX_flags(const EVP_CIPHER_CTX *ctx);

// enumerated below. It will crash if no cipher has been configured.
OPENSSL_EXPORT uint32_t EVP_CIPHER_CTX_mode(const EVP_CIPHER_CTX *ctx);

// should be one of the |EVP_CTRL_*| values. The |arg| and |ptr| arguments are
// specific to the command in question.
OPENSSL_EXPORT int EVP_CIPHER_CTX_ctrl(EVP_CIPHER_CTX *ctx, int command,
                                       int arg, void *ptr);

// returns one. Pass a non-zero |pad| to enable padding (the default) or zero
// to disable.
OPENSSL_EXPORT int EVP_CIPHER_CTX_set_padding(EVP_CIPHER_CTX *ctx, int pad);

// valid for ciphers that can take a variable length key. It returns one on
// success and zero on error.
OPENSSL_EXPORT int EVP_CIPHER_CTX_set_key_length(EVP_CIPHER_CTX *ctx,
                                                 unsigned key_len);


// |NID_aes_128_gcm|.)
OPENSSL_EXPORT int EVP_CIPHER_nid(const EVP_CIPHER *cipher);

// if |cipher| is a stream cipher.
OPENSSL_EXPORT unsigned EVP_CIPHER_block_size(const EVP_CIPHER *cipher);

// |cipher| can take a variable key length then this function returns the
// default key length and |EVP_CIPHER_flags| will return a value with
// |EVP_CIPH_VARIABLE_LENGTH| set.
OPENSSL_EXPORT unsigned EVP_CIPHER_key_length(const EVP_CIPHER *cipher);

// |cipher| doesn't take an IV.
OPENSSL_EXPORT unsigned EVP_CIPHER_iv_length(const EVP_CIPHER *cipher);

// |EVP_CIPH_*| flags.
OPENSSL_EXPORT uint32_t EVP_CIPHER_flags(const EVP_CIPHER *cipher);

OPENSSL_EXPORT uint32_t EVP_CIPHER_mode(const EVP_CIPHER *cipher);


// |md| |count| times using |data| and |salt|. On entry, the |key| and |iv|
// buffers must have enough space to hold a key and IV for |type|. It returns
// the length of the key on success or zero on error.
OPENSSL_EXPORT int EVP_BytesToKey(const EVP_CIPHER *type, const EVP_MD *md,
                                  const uint8_t *salt, const uint8_t *data,
                                  size_t data_len, unsigned count, uint8_t *key,
                                  uint8_t *iv);


#define EVP_CIPH_STREAM_CIPHER 0x0
#define EVP_CIPH_ECB_MODE 0x1
#define EVP_CIPH_CBC_MODE 0x2
#define EVP_CIPH_CFB_MODE 0x3
#define EVP_CIPH_OFB_MODE 0x4
#define EVP_CIPH_CTR_MODE 0x5
#define EVP_CIPH_GCM_MODE 0x6
#define EVP_CIPH_XTS_MODE 0x7


// key.
#define EVP_CIPH_VARIABLE_LENGTH 0x40

// should always be called when initialising a new operation, even if the key
// is NULL to indicate that the same key is being used.
#define EVP_CIPH_ALWAYS_CALL_INIT 0x80

// than keeping it in the |iv| member of |EVP_CIPHER_CTX|.
#define EVP_CIPH_CUSTOM_IV 0x100

// initialising an |EVP_CIPHER_CTX|.
#define EVP_CIPH_CTRL_INIT 0x200

// itself. This causes EVP_(En|De)crypt_ex to be simple wrapper functions.
#define EVP_CIPH_FLAG_CUSTOM_CIPHER 0x400

// older version of the proper AEAD interface. See aead.h for the current
// one.
#define EVP_CIPH_FLAG_AEAD_CIPHER 0x800

// with |EVP_CTRL_COPY| at the end of normal |EVP_CIPHER_CTX_copy|
// processing.
#define EVP_CIPH_CUSTOM_COPY 0x1000


// is called on |cipher| first, if |cipher| is not NULL.
OPENSSL_EXPORT int EVP_CipherInit(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *cipher,
                                  const uint8_t *key, const uint8_t *iv,
                                  int enc);

OPENSSL_EXPORT int EVP_EncryptInit(EVP_CIPHER_CTX *ctx,
                                   const EVP_CIPHER *cipher, const uint8_t *key,
                                   const uint8_t *iv);

OPENSSL_EXPORT int EVP_DecryptInit(EVP_CIPHER_CTX *ctx,
                                   const EVP_CIPHER *cipher, const uint8_t *key,
                                   const uint8_t *iv);

OPENSSL_EXPORT int EVP_add_cipher_alias(const char *a, const char *b);

// |name|, or NULL if the name is unknown.
OPENSSL_EXPORT const EVP_CIPHER *EVP_get_cipherbyname(const char *name);

// |EVP_CIPH_FLAG_CUSTOM_CIPHER|. Use |EVP_aead_aes_128_gcm| and
// |EVP_aead_aes_256_gcm| instead.
OPENSSL_EXPORT const EVP_CIPHER *EVP_aes_128_gcm(void);
OPENSSL_EXPORT const EVP_CIPHER *EVP_aes_256_gcm(void);

OPENSSL_EXPORT const EVP_CIPHER *EVP_aes_192_ecb(void);
OPENSSL_EXPORT const EVP_CIPHER *EVP_aes_192_cbc(void);
OPENSSL_EXPORT const EVP_CIPHER *EVP_aes_192_ctr(void);
OPENSSL_EXPORT const EVP_CIPHER *EVP_aes_192_gcm(void);
OPENSSL_EXPORT const EVP_CIPHER *EVP_aes_192_ofb(void);

OPENSSL_EXPORT const EVP_CIPHER *EVP_des_ede3_ecb(void);

OPENSSL_EXPORT const EVP_CIPHER *EVP_aes_128_cfb128(void);

OPENSSL_EXPORT const EVP_CIPHER *EVP_aes_256_cfb128(void);

OPENSSL_EXPORT const EVP_CIPHER *EVP_bf_ecb(void);

OPENSSL_EXPORT const EVP_CIPHER *EVP_bf_cbc(void);

OPENSSL_EXPORT const EVP_CIPHER *EVP_bf_cfb(void);

OPENSSL_EXPORT const EVP_CIPHER *EVP_cast5_ecb(void);

OPENSSL_EXPORT const EVP_CIPHER *EVP_cast5_cbc(void);

// compile code with BoringSSL.
#define EVP_CIPH_CCM_MODE (-1)
#define EVP_CIPH_OCB_MODE (-2)
#define EVP_CIPH_WRAP_MODE (-3)
#define EVP_CIPHER_CTX_FLAG_WRAP_ALLOW 0

OPENSSL_EXPORT void EVP_CIPHER_CTX_set_flags(const EVP_CIPHER_CTX *ctx,
                                             uint32_t flags);


#define EVP_CIPH_NO_PADDING 0x800

#define EVP_CTRL_INIT 0x0
#define EVP_CTRL_SET_KEY_LENGTH 0x1
#define EVP_CTRL_GET_RC2_KEY_BITS 0x2
#define EVP_CTRL_SET_RC2_KEY_BITS 0x3
#define EVP_CTRL_GET_RC5_ROUNDS 0x4
#define EVP_CTRL_SET_RC5_ROUNDS 0x5
#define EVP_CTRL_RAND_KEY 0x6
#define EVP_CTRL_PBE_PRF_NID 0x7
#define EVP_CTRL_COPY 0x8
#define EVP_CTRL_AEAD_SET_IVLEN 0x9
#define EVP_CTRL_AEAD_GET_TAG 0x10
#define EVP_CTRL_AEAD_SET_TAG 0x11
#define EVP_CTRL_AEAD_SET_IV_FIXED 0x12
#define EVP_CTRL_GCM_IV_GEN 0x13
#define EVP_CTRL_AEAD_SET_MAC_KEY 0x17
// EVP_CTRL_GCM_SET_IV_INV sets the GCM invocation field, decrypt only
#define EVP_CTRL_GCM_SET_IV_INV 0x18

#define EVP_GCM_TLS_FIXED_IV_LEN 4
#define EVP_GCM_TLS_EXPLICIT_IV_LEN 8
#define EVP_GCM_TLS_TAG_LEN 16

#define EVP_CTRL_GCM_SET_IVLEN EVP_CTRL_AEAD_SET_IVLEN
#define EVP_CTRL_GCM_GET_TAG EVP_CTRL_AEAD_GET_TAG
#define EVP_CTRL_GCM_SET_TAG EVP_CTRL_AEAD_SET_TAG
#define EVP_CTRL_GCM_SET_IV_FIXED EVP_CTRL_AEAD_SET_IV_FIXED

#define EVP_MAX_KEY_LENGTH 64
#define EVP_MAX_IV_LENGTH 16
#define EVP_MAX_BLOCK_LENGTH 32

struct evp_cipher_ctx_st {

  const EVP_CIPHER *cipher;

  void *app_data;      // application stuff

  void *cipher_data;


  unsigned key_len;

  int encrypt;

  uint32_t flags;

  uint8_t oiv[EVP_MAX_IV_LENGTH];

  uint8_t iv[EVP_MAX_IV_LENGTH];


  uint8_t buf[EVP_MAX_BLOCK_LENGTH];


  int buf_len;


  unsigned num;

  int final_used;


  int block_mask;

  uint8_t final[EVP_MAX_BLOCK_LENGTH];  // possible final block
} /* EVP_CIPHER_CTX */;

typedef struct evp_cipher_info_st {
  const EVP_CIPHER *cipher;
  unsigned char iv[EVP_MAX_IV_LENGTH];
} EVP_CIPHER_INFO;

struct evp_cipher_st {

  int nid;


  unsigned block_size;


  unsigned key_len;

  unsigned iv_len;


  unsigned ctx_size;

  uint32_t flags;

  void *app_data;

  int (*init)(EVP_CIPHER_CTX *ctx, const uint8_t *key, const uint8_t *iv,
              int enc);

  int (*cipher)(EVP_CIPHER_CTX *ctx, uint8_t *out, const uint8_t *in,
                size_t inl);



  void (*cleanup)(EVP_CIPHER_CTX *);

  int (*ctrl)(EVP_CIPHER_CTX *, int type, int arg, void *ptr);
};


#if defined(__cplusplus)
}  // extern C

#if !defined(BORINGSSL_NO_CXX)
extern "C++" {

BSSL_NAMESPACE_BEGIN

BORINGSSL_MAKE_DELETER(EVP_CIPHER_CTX, EVP_CIPHER_CTX_free)

using ScopedEVP_CIPHER_CTX =
    internal::StackAllocated<EVP_CIPHER_CTX, int, EVP_CIPHER_CTX_init,
                             EVP_CIPHER_CTX_cleanup>;

BSSL_NAMESPACE_END

}  // extern C++
#endif

#endif

#define CIPHER_R_AES_KEY_SETUP_FAILED 100
#define CIPHER_R_BAD_DECRYPT 101
#define CIPHER_R_BAD_KEY_LENGTH 102
#define CIPHER_R_BUFFER_TOO_SMALL 103
#define CIPHER_R_CTRL_NOT_IMPLEMENTED 104
#define CIPHER_R_CTRL_OPERATION_NOT_IMPLEMENTED 105
#define CIPHER_R_DATA_NOT_MULTIPLE_OF_BLOCK_LENGTH 106
#define CIPHER_R_INITIALIZATION_ERROR 107
#define CIPHER_R_INPUT_NOT_INITIALIZED 108
#define CIPHER_R_INVALID_AD_SIZE 109
#define CIPHER_R_INVALID_KEY_LENGTH 110
#define CIPHER_R_INVALID_NONCE_SIZE 111
#define CIPHER_R_INVALID_OPERATION 112
#define CIPHER_R_IV_TOO_LARGE 113
#define CIPHER_R_NO_CIPHER_SET 114
#define CIPHER_R_OUTPUT_ALIASES_INPUT 115
#define CIPHER_R_TAG_TOO_LARGE 116
#define CIPHER_R_TOO_LARGE 117
#define CIPHER_R_UNSUPPORTED_AD_SIZE 118
#define CIPHER_R_UNSUPPORTED_INPUT_SIZE 119
#define CIPHER_R_UNSUPPORTED_KEY_SIZE 120
#define CIPHER_R_UNSUPPORTED_NONCE_SIZE 121
#define CIPHER_R_UNSUPPORTED_TAG_SIZE 122
#define CIPHER_R_WRONG_FINAL_BLOCK_LENGTH 123
#define CIPHER_R_NO_DIRECTION_SET 124
#define CIPHER_R_INVALID_NONCE 125

#endif  // OPENSSL_HEADER_CIPHER_H
