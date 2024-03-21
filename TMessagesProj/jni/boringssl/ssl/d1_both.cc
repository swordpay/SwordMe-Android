/*
 * DTLS implementation written by Nagendra Modadugu
 * (nagendra@cs.stanford.edu) for the OpenSSL project 2005.
 */
/* ====================================================================
 * Copyright (c) 1998-2005 The OpenSSL Project.  All rights reserved.
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

#include <openssl/ssl.h>

#include <assert.h>
#include <limits.h>
#include <string.h>

#include <openssl/buf.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/mem.h>
#include <openssl/rand.h>

#include "../crypto/internal.h"
#include "internal.h"


BSSL_NAMESPACE_BEGIN

// for these values? Notably, why is kMinMTU a function of the transport
// protocol's overhead rather than, say, what's needed to hold a minimally-sized
// handshake fragment plus protocol overhead.

static const unsigned int kMinMTU = 256 - 28;

// the underlying BIO supplies one.
static const unsigned int kDefaultMTU = 1500 - 28;


hm_fragment::~hm_fragment() {
  OPENSSL_free(data);
  OPENSSL_free(reassembly);
}

static UniquePtr<hm_fragment> dtls1_hm_fragment_new(
    const struct hm_header_st *msg_hdr) {
  ScopedCBB cbb;
  UniquePtr<hm_fragment> frag = MakeUnique<hm_fragment>();
  if (!frag) {
    return nullptr;
  }
  frag->type = msg_hdr->type;
  frag->seq = msg_hdr->seq;
  frag->msg_len = msg_hdr->msg_len;

  frag->data =
      (uint8_t *)OPENSSL_malloc(DTLS1_HM_HEADER_LENGTH + msg_hdr->msg_len);
  if (frag->data == NULL) {
    OPENSSL_PUT_ERROR(SSL, ERR_R_MALLOC_FAILURE);
    return nullptr;
  }

  if (!CBB_init_fixed(cbb.get(), frag->data, DTLS1_HM_HEADER_LENGTH) ||
      !CBB_add_u8(cbb.get(), msg_hdr->type) ||
      !CBB_add_u24(cbb.get(), msg_hdr->msg_len) ||
      !CBB_add_u16(cbb.get(), msg_hdr->seq) ||
      !CBB_add_u24(cbb.get(), 0 /* frag_off */) ||
      !CBB_add_u24(cbb.get(), msg_hdr->msg_len) ||
      !CBB_finish(cbb.get(), NULL, NULL)) {
    OPENSSL_PUT_ERROR(SSL, ERR_R_MALLOC_FAILURE);
    return nullptr;
  }

  if (msg_hdr->msg_len > 0) {

    if (msg_hdr->msg_len + 7 < msg_hdr->msg_len) {
      OPENSSL_PUT_ERROR(SSL, ERR_R_OVERFLOW);
      return nullptr;
    }
    size_t bitmask_len = (msg_hdr->msg_len + 7) / 8;
    frag->reassembly = (uint8_t *)OPENSSL_malloc(bitmask_len);
    if (frag->reassembly == NULL) {
      OPENSSL_PUT_ERROR(SSL, ERR_R_MALLOC_FAILURE);
      return nullptr;
    }
    OPENSSL_memset(frag->reassembly, 0, bitmask_len);
  }

  return frag;
}

// exclusive, set.
static uint8_t bit_range(size_t start, size_t end) {
  return (uint8_t)(~((1u << start) - 1) & ((1u << end) - 1));
}

// as received in |frag|. If |frag| becomes complete, it clears
// |frag->reassembly|. The range must be within the bounds of |frag|'s message
// and |frag->reassembly| must not be NULL.
static void dtls1_hm_fragment_mark(hm_fragment *frag, size_t start,
                                   size_t end) {
  size_t msg_len = frag->msg_len;

  if (frag->reassembly == NULL || start > end || end > msg_len) {
    assert(0);
    return;
  }

  assert(msg_len > 0);

  if (start == end) {
    return;
  }

  if ((start >> 3) == (end >> 3)) {
    frag->reassembly[start >> 3] |= bit_range(start & 7, end & 7);
  } else {
    frag->reassembly[start >> 3] |= bit_range(start & 7, 8);
    for (size_t i = (start >> 3) + 1; i < (end >> 3); i++) {
      frag->reassembly[i] = 0xff;
    }
    if ((end & 7) != 0) {
      frag->reassembly[end >> 3] |= bit_range(0, end & 7);
    }
  }

  for (size_t i = 0; i < (msg_len >> 3); i++) {
    if (frag->reassembly[i] != 0xff) {
      return;
    }
  }
  if ((msg_len & 7) != 0 &&
      frag->reassembly[msg_len >> 3] != bit_range(0, msg_len & 7)) {
    return;
  }

  OPENSSL_free(frag->reassembly);
  frag->reassembly = NULL;
}

// message is complete.
static bool dtls1_is_current_message_complete(const SSL *ssl) {
  size_t idx = ssl->d1->handshake_read_seq % SSL_MAX_HANDSHAKE_FLIGHT;
  hm_fragment *frag = ssl->d1->incoming_messages[idx].get();
  return frag != NULL && frag->reassembly == NULL;
}

// |msg_hdr|. If none exists, it creates a new one and inserts it in the
// queue. Otherwise, it checks |msg_hdr| is consistent with the existing one. It
// returns NULL on failure. The caller does not take ownership of the result.
static hm_fragment *dtls1_get_incoming_message(
    SSL *ssl, uint8_t *out_alert, const struct hm_header_st *msg_hdr) {
  if (msg_hdr->seq < ssl->d1->handshake_read_seq ||
      msg_hdr->seq - ssl->d1->handshake_read_seq >= SSL_MAX_HANDSHAKE_FLIGHT) {
    *out_alert = SSL_AD_INTERNAL_ERROR;
    return NULL;
  }

  size_t idx = msg_hdr->seq % SSL_MAX_HANDSHAKE_FLIGHT;
  hm_fragment *frag = ssl->d1->incoming_messages[idx].get();
  if (frag != NULL) {
    assert(frag->seq == msg_hdr->seq);


    if (frag->type != msg_hdr->type ||
        frag->msg_len != msg_hdr->msg_len) {
      OPENSSL_PUT_ERROR(SSL, SSL_R_FRAGMENT_MISMATCH);
      *out_alert = SSL_AD_ILLEGAL_PARAMETER;
      return NULL;
    }
    return frag;
  }

  ssl->d1->incoming_messages[idx] = dtls1_hm_fragment_new(msg_hdr);
  if (!ssl->d1->incoming_messages[idx]) {
    *out_alert = SSL_AD_INTERNAL_ERROR;
    return NULL;
  }
  return ssl->d1->incoming_messages[idx].get();
}

ssl_open_record_t dtls1_open_handshake(SSL *ssl, size_t *out_consumed,
                                       uint8_t *out_alert, Span<uint8_t> in) {
  uint8_t type;
  Span<uint8_t> record;
  auto ret = dtls_open_record(ssl, &type, &record, out_consumed, out_alert, in);
  if (ret != ssl_open_record_success) {
    return ret;
  }

  switch (type) {
    case SSL3_RT_APPLICATION_DATA:

      if (ssl->s3->aead_read_ctx->is_null_cipher()) {
        OPENSSL_PUT_ERROR(SSL, SSL_R_UNEXPECTED_RECORD);
        *out_alert = SSL_AD_UNEXPECTED_MESSAGE;
        return ssl_open_record_error;
      }


      return ssl_open_record_discard;

    case SSL3_RT_CHANGE_CIPHER_SPEC:


      if (!ssl->s3->aead_read_ctx->is_null_cipher()) {
        OPENSSL_PUT_ERROR(SSL, SSL_R_UNEXPECTED_RECORD);
        *out_alert = SSL_AD_UNEXPECTED_MESSAGE;
        return ssl_open_record_error;
      }

      if (record.size() != 1u || record[0] != SSL3_MT_CCS) {
        OPENSSL_PUT_ERROR(SSL, SSL_R_BAD_CHANGE_CIPHER_SPEC);
        *out_alert = SSL_AD_ILLEGAL_PARAMETER;
        return ssl_open_record_error;
      }

      ssl->d1->has_change_cipher_spec = true;
      ssl_do_msg_callback(ssl, 0 /* read */, SSL3_RT_CHANGE_CIPHER_SPEC,
                          record);
      return ssl_open_record_success;

    case SSL3_RT_HANDSHAKE:

      break;

    default:
      OPENSSL_PUT_ERROR(SSL, SSL_R_UNEXPECTED_RECORD);
      *out_alert = SSL_AD_UNEXPECTED_MESSAGE;
      return ssl_open_record_error;
  }

  CBS cbs;
  CBS_init(&cbs, record.data(), record.size());
  while (CBS_len(&cbs) > 0) {

    struct hm_header_st msg_hdr;
    CBS body;
    if (!dtls1_parse_fragment(&cbs, &msg_hdr, &body)) {
      OPENSSL_PUT_ERROR(SSL, SSL_R_BAD_HANDSHAKE_RECORD);
      *out_alert = SSL_AD_DECODE_ERROR;
      return ssl_open_record_error;
    }

    const size_t frag_off = msg_hdr.frag_off;
    const size_t frag_len = msg_hdr.frag_len;
    const size_t msg_len = msg_hdr.msg_len;
    if (frag_off > msg_len || frag_off + frag_len < frag_off ||
        frag_off + frag_len > msg_len ||
        msg_len > ssl_max_handshake_message_len(ssl)) {
      OPENSSL_PUT_ERROR(SSL, SSL_R_EXCESSIVE_MESSAGE_SIZE);
      *out_alert = SSL_AD_ILLEGAL_PARAMETER;
      return ssl_open_record_error;
    }

    if (ssl->d1->r_epoch == 1 && msg_hdr.seq != ssl->d1->handshake_read_seq) {
      OPENSSL_PUT_ERROR(SSL, SSL_R_UNEXPECTED_RECORD);
      *out_alert = SSL_AD_UNEXPECTED_MESSAGE;
      return ssl_open_record_error;
    }

    if (msg_hdr.seq < ssl->d1->handshake_read_seq ||
        msg_hdr.seq >
            (unsigned)ssl->d1->handshake_read_seq + SSL_MAX_HANDSHAKE_FLIGHT) {

      continue;
    }

    hm_fragment *frag = dtls1_get_incoming_message(ssl, out_alert, &msg_hdr);
    if (frag == NULL) {
      return ssl_open_record_error;
    }
    assert(frag->msg_len == msg_len);

    if (frag->reassembly == NULL) {

      continue;
    }
    assert(msg_len > 0);

    OPENSSL_memcpy(frag->data + DTLS1_HM_HEADER_LENGTH + frag_off,
                   CBS_data(&body), CBS_len(&body));
    dtls1_hm_fragment_mark(frag, frag_off, frag_off + frag_len);
  }

  return ssl_open_record_success;
}

bool dtls1_get_message(const SSL *ssl, SSLMessage *out) {
  if (!dtls1_is_current_message_complete(ssl)) {
    return false;
  }

  size_t idx = ssl->d1->handshake_read_seq % SSL_MAX_HANDSHAKE_FLIGHT;
  hm_fragment *frag = ssl->d1->incoming_messages[idx].get();
  out->type = frag->type;
  CBS_init(&out->body, frag->data + DTLS1_HM_HEADER_LENGTH, frag->msg_len);
  CBS_init(&out->raw, frag->data, DTLS1_HM_HEADER_LENGTH + frag->msg_len);
  out->is_v2_hello = false;
  if (!ssl->s3->has_message) {
    ssl_do_msg_callback(ssl, 0 /* read */, SSL3_RT_HANDSHAKE, out->raw);
    ssl->s3->has_message = true;
  }
  return true;
}

void dtls1_next_message(SSL *ssl) {
  assert(ssl->s3->has_message);
  assert(dtls1_is_current_message_complete(ssl));
  size_t index = ssl->d1->handshake_read_seq % SSL_MAX_HANDSHAKE_FLIGHT;
  ssl->d1->incoming_messages[index].reset();
  ssl->d1->handshake_read_seq++;
  ssl->s3->has_message = false;


  if (ssl->d1->outgoing_messages_complete) {
    ssl->d1->flight_has_reply = true;
  }
}

bool dtls_has_unprocessed_handshake_data(const SSL *ssl) {
  if (ssl->d1->has_change_cipher_spec) {
    return true;
  }

  size_t current = ssl->d1->handshake_read_seq % SSL_MAX_HANDSHAKE_FLIGHT;
  for (size_t i = 0; i < SSL_MAX_HANDSHAKE_FLIGHT; i++) {

    if (ssl->s3->has_message && i == current) {
      assert(dtls1_is_current_message_complete(ssl));
      continue;
    }
    if (ssl->d1->incoming_messages[i] != nullptr) {
      return true;
    }
  }
  return false;
}

bool dtls1_parse_fragment(CBS *cbs, struct hm_header_st *out_hdr,
                          CBS *out_body) {
  OPENSSL_memset(out_hdr, 0x00, sizeof(struct hm_header_st));

  if (!CBS_get_u8(cbs, &out_hdr->type) ||
      !CBS_get_u24(cbs, &out_hdr->msg_len) ||
      !CBS_get_u16(cbs, &out_hdr->seq) ||
      !CBS_get_u24(cbs, &out_hdr->frag_off) ||
      !CBS_get_u24(cbs, &out_hdr->frag_len) ||
      !CBS_get_bytes(cbs, out_body, out_hdr->frag_len)) {
    return false;
  }

  return true;
}

ssl_open_record_t dtls1_open_change_cipher_spec(SSL *ssl, size_t *out_consumed,
                                                uint8_t *out_alert,
                                                Span<uint8_t> in) {
  if (!ssl->d1->has_change_cipher_spec) {

    auto ret = dtls1_open_handshake(ssl, out_consumed, out_alert, in);
    if (ret != ssl_open_record_success) {
      return ret;
    }
  }
  if (ssl->d1->has_change_cipher_spec) {
    ssl->d1->has_change_cipher_spec = false;
    return ssl_open_record_success;
  }
  return ssl_open_record_discard;
}


void DTLS_OUTGOING_MESSAGE::Clear() {
  OPENSSL_free(data);
  data = nullptr;
}

void dtls_clear_outgoing_messages(SSL *ssl) {
  for (size_t i = 0; i < ssl->d1->outgoing_messages_len; i++) {
    ssl->d1->outgoing_messages[i].Clear();
  }
  ssl->d1->outgoing_messages_len = 0;
  ssl->d1->outgoing_written = 0;
  ssl->d1->outgoing_offset = 0;
  ssl->d1->outgoing_messages_complete = false;
  ssl->d1->flight_has_reply = false;
}

bool dtls1_init_message(SSL *ssl, CBB *cbb, CBB *body, uint8_t type) {

  if (!CBB_init(cbb, 64) ||
      !CBB_add_u8(cbb, type) ||
      !CBB_add_u24(cbb, 0 /* length (filled in later) */) ||
      !CBB_add_u16(cbb, ssl->d1->handshake_write_seq) ||
      !CBB_add_u24(cbb, 0 /* offset */) ||
      !CBB_add_u24_length_prefixed(cbb, body)) {
    return false;
  }

  return true;
}

bool dtls1_finish_message(SSL *ssl, CBB *cbb, Array<uint8_t> *out_msg) {
  if (!CBBFinishArray(cbb, out_msg) ||
      out_msg->size() < DTLS1_HM_HEADER_LENGTH) {
    OPENSSL_PUT_ERROR(SSL, ERR_R_INTERNAL_ERROR);
    return false;
  }


  OPENSSL_memcpy(out_msg->data() + 1,
                 out_msg->data() + DTLS1_HM_HEADER_LENGTH - 3, 3);
  return true;
}

// 32-bit value. The obvious thing doesn't work because, in some 32-bit build
// configurations, the compiler warns that the test is always false and breaks
// the build.
static bool ssl_size_t_greater_than_32_bits(size_t v) {
#if defined(OPENSSL_64_BIT)
  return v > 0xffffffff;
#elif defined(OPENSSL_32_BIT)
  return false;
#else
#error "Building for neither 32- nor 64-bits."
#endif
}

// outgoing flight. It returns true on success and false on error.
static bool add_outgoing(SSL *ssl, bool is_ccs, Array<uint8_t> data) {
  if (ssl->d1->outgoing_messages_complete) {


    dtls1_stop_timer(ssl);
    dtls_clear_outgoing_messages(ssl);
  }

  static_assert(SSL_MAX_HANDSHAKE_FLIGHT <
                    (1 << 8 * sizeof(ssl->d1->outgoing_messages_len)),
                "outgoing_messages_len is too small");
  if (ssl->d1->outgoing_messages_len >= SSL_MAX_HANDSHAKE_FLIGHT ||
      ssl_size_t_greater_than_32_bits(data.size())) {
    assert(false);
    OPENSSL_PUT_ERROR(SSL, ERR_R_INTERNAL_ERROR);
    return false;
  }

  if (!is_ccs) {


    if (ssl->s3->hs != NULL &&
        !ssl->s3->hs->transcript.Update(data)) {
      OPENSSL_PUT_ERROR(SSL, ERR_R_INTERNAL_ERROR);
      return false;
    }
    ssl->d1->handshake_write_seq++;
  }

  DTLS_OUTGOING_MESSAGE *msg =
      &ssl->d1->outgoing_messages[ssl->d1->outgoing_messages_len];
  size_t len;
  data.Release(&msg->data, &len);
  msg->len = len;
  msg->epoch = ssl->d1->w_epoch;
  msg->is_ccs = is_ccs;

  ssl->d1->outgoing_messages_len++;
  return true;
}

bool dtls1_add_message(SSL *ssl, Array<uint8_t> data) {
  return add_outgoing(ssl, false /* handshake */, std::move(data));
}

bool dtls1_add_change_cipher_spec(SSL *ssl) {
  return add_outgoing(ssl, true /* ChangeCipherSpec */, Array<uint8_t>());
}

// the minimum.
static void dtls1_update_mtu(SSL *ssl) {



  if (ssl->d1->mtu < dtls1_min_mtu() &&
      !(SSL_get_options(ssl) & SSL_OP_NO_QUERY_MTU)) {
    long mtu = BIO_ctrl(ssl->wbio.get(), BIO_CTRL_DGRAM_QUERY_MTU, 0, NULL);
    if (mtu >= 0 && mtu <= (1 << 30) && (unsigned)mtu >= dtls1_min_mtu()) {
      ssl->d1->mtu = (unsigned)mtu;
    } else {
      ssl->d1->mtu = kDefaultMTU;
      BIO_ctrl(ssl->wbio.get(), BIO_CTRL_DGRAM_SET_MTU, ssl->d1->mtu, NULL);
    }
  }

  assert(ssl->d1->mtu >= dtls1_min_mtu());
}

enum seal_result_t {
  seal_error,
  seal_no_progress,
  seal_partial,
  seal_success,
};

// progress was made, it returns |seal_partial| or |seal_success| and sets
// |*out_len| to the number of bytes written.
static enum seal_result_t seal_next_message(SSL *ssl, uint8_t *out,
                                            size_t *out_len, size_t max_out,
                                            const DTLS_OUTGOING_MESSAGE *msg) {
  assert(ssl->d1->outgoing_written < ssl->d1->outgoing_messages_len);
  assert(msg == &ssl->d1->outgoing_messages[ssl->d1->outgoing_written]);

  enum dtls1_use_epoch_t use_epoch = dtls1_use_current_epoch;
  if (ssl->d1->w_epoch >= 1 && msg->epoch == ssl->d1->w_epoch - 1) {
    use_epoch = dtls1_use_previous_epoch;
  } else if (msg->epoch != ssl->d1->w_epoch) {
    OPENSSL_PUT_ERROR(SSL, ERR_R_INTERNAL_ERROR);
    return seal_error;
  }

  size_t overhead = dtls_max_seal_overhead(ssl, use_epoch);
  size_t prefix = dtls_seal_prefix_len(ssl, use_epoch);

  if (msg->is_ccs) {

    static const uint8_t kChangeCipherSpec[1] = {SSL3_MT_CCS};
    if (max_out < sizeof(kChangeCipherSpec) + overhead) {
      return seal_no_progress;
    }

    if (!dtls_seal_record(ssl, out, out_len, max_out,
                          SSL3_RT_CHANGE_CIPHER_SPEC, kChangeCipherSpec,
                          sizeof(kChangeCipherSpec), use_epoch)) {
      return seal_error;
    }

    ssl_do_msg_callback(ssl, 1 /* write */, SSL3_RT_CHANGE_CIPHER_SPEC,
                        kChangeCipherSpec);
    return seal_success;
  }

  CBS cbs, body;
  struct hm_header_st hdr;
  CBS_init(&cbs, msg->data, msg->len);
  if (!dtls1_parse_fragment(&cbs, &hdr, &body) ||
      hdr.frag_off != 0 ||
      hdr.frag_len != CBS_len(&body) ||
      hdr.msg_len != CBS_len(&body) ||
      !CBS_skip(&body, ssl->d1->outgoing_offset) ||
      CBS_len(&cbs) != 0) {
    OPENSSL_PUT_ERROR(SSL, ERR_R_INTERNAL_ERROR);
    return seal_error;
  }

  if (max_out < DTLS1_HM_HEADER_LENGTH + 1 + overhead || max_out < prefix) {
    return seal_no_progress;
  }
  size_t todo = CBS_len(&body);
  if (todo > max_out - DTLS1_HM_HEADER_LENGTH - overhead) {
    todo = max_out - DTLS1_HM_HEADER_LENGTH - overhead;
  }

  ScopedCBB cbb;
  uint8_t *frag = out + prefix;
  size_t max_frag = max_out - prefix, frag_len;
  if (!CBB_init_fixed(cbb.get(), frag, max_frag) ||
      !CBB_add_u8(cbb.get(), hdr.type) ||
      !CBB_add_u24(cbb.get(), hdr.msg_len) ||
      !CBB_add_u16(cbb.get(), hdr.seq) ||
      !CBB_add_u24(cbb.get(), ssl->d1->outgoing_offset) ||
      !CBB_add_u24(cbb.get(), todo) ||
      !CBB_add_bytes(cbb.get(), CBS_data(&body), todo) ||
      !CBB_finish(cbb.get(), NULL, &frag_len)) {
    OPENSSL_PUT_ERROR(SSL, ERR_R_INTERNAL_ERROR);
    return seal_error;
  }

  ssl_do_msg_callback(ssl, 1 /* write */, SSL3_RT_HANDSHAKE,
                      MakeSpan(frag, frag_len));

  if (!dtls_seal_record(ssl, out, out_len, max_out, SSL3_RT_HANDSHAKE,
                        out + prefix, frag_len, use_epoch)) {
    return seal_error;
  }

  if (todo == CBS_len(&body)) {

    ssl->d1->outgoing_offset = 0;
    return seal_success;
  }

  ssl->d1->outgoing_offset += todo;
  return seal_partial;
}

// advances |ssl->d1->outgoing_written| and |ssl->d1->outgoing_offset| as
// appropriate.
static bool seal_next_packet(SSL *ssl, uint8_t *out, size_t *out_len,
                             size_t max_out) {
  bool made_progress = false;
  size_t total = 0;
  assert(ssl->d1->outgoing_written < ssl->d1->outgoing_messages_len);
  for (; ssl->d1->outgoing_written < ssl->d1->outgoing_messages_len;
       ssl->d1->outgoing_written++) {
    const DTLS_OUTGOING_MESSAGE *msg =
        &ssl->d1->outgoing_messages[ssl->d1->outgoing_written];
    size_t len;
    enum seal_result_t ret = seal_next_message(ssl, out, &len, max_out, msg);
    switch (ret) {
      case seal_error:
        return false;

      case seal_no_progress:
        goto packet_full;

      case seal_partial:
      case seal_success:
        out += len;
        max_out -= len;
        total += len;
        made_progress = true;

        if (ret == seal_partial) {
          goto packet_full;
        }
        break;
    }
  }

packet_full:

  if (!made_progress) {
    OPENSSL_PUT_ERROR(SSL, SSL_R_MTU_TOO_SMALL);
    return false;
  }

  *out_len = total;
  return true;
}

static int send_flight(SSL *ssl) {
  if (ssl->s3->write_shutdown != ssl_shutdown_none) {
    OPENSSL_PUT_ERROR(SSL, SSL_R_PROTOCOL_IS_SHUTDOWN);
    return -1;
  }

  dtls1_update_mtu(ssl);

  int ret = -1;
  uint8_t *packet = (uint8_t *)OPENSSL_malloc(ssl->d1->mtu);
  if (packet == NULL) {
    OPENSSL_PUT_ERROR(SSL, ERR_R_MALLOC_FAILURE);
    goto err;
  }

  while (ssl->d1->outgoing_written < ssl->d1->outgoing_messages_len) {
    uint8_t old_written = ssl->d1->outgoing_written;
    uint32_t old_offset = ssl->d1->outgoing_offset;

    size_t packet_len;
    if (!seal_next_packet(ssl, packet, &packet_len, ssl->d1->mtu)) {
      goto err;
    }

    int bio_ret = BIO_write(ssl->wbio.get(), packet, packet_len);
    if (bio_ret <= 0) {

      ssl->d1->outgoing_written = old_written;
      ssl->d1->outgoing_offset = old_offset;
      ssl->s3->rwstate = SSL_ERROR_WANT_WRITE;
      ret = bio_ret;
      goto err;
    }
  }

  if (BIO_flush(ssl->wbio.get()) <= 0) {
    ssl->s3->rwstate = SSL_ERROR_WANT_WRITE;
    goto err;
  }

  ret = 1;

err:
  OPENSSL_free(packet);
  return ret;
}

int dtls1_flush_flight(SSL *ssl) {
  ssl->d1->outgoing_messages_complete = true;

  dtls1_start_timer(ssl);
  return send_flight(ssl);
}

int dtls1_retransmit_outgoing_messages(SSL *ssl) {




  ssl->d1->outgoing_written = 0;
  ssl->d1->outgoing_offset = 0;

  return send_flight(ssl);
}

unsigned int dtls1_min_mtu(void) {
  return kMinMTU;
}

BSSL_NAMESPACE_END
