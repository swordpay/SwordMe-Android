/* Copyright (c) 2017, Google Inc.
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

#include <openssl/ssl.h>

#include <assert.h>

#include <openssl/bytestring.h>
#include <openssl/err.h>

#include "internal.h"
#include "../crypto/internal.h"


BSSL_NAMESPACE_BEGIN

bool ssl_protocol_version_from_wire(uint16_t *out, uint16_t version) {
  switch (version) {
    case TLS1_VERSION:
    case TLS1_1_VERSION:
    case TLS1_2_VERSION:
    case TLS1_3_VERSION:
      *out = version;
      return true;

    case DTLS1_VERSION:

      *out = TLS1_1_VERSION;
      return true;

    case DTLS1_2_VERSION:
      *out = TLS1_2_VERSION;
      return true;

    default:
      return false;
  }
}

// decreasing preference.

static const uint16_t kTLSVersions[] = {
    TLS1_3_VERSION,
    TLS1_2_VERSION,
    TLS1_1_VERSION,
    TLS1_VERSION,
};

static const uint16_t kDTLSVersions[] = {
    DTLS1_2_VERSION,
    DTLS1_VERSION,
};

static Span<const uint16_t> get_method_versions(
    const SSL_PROTOCOL_METHOD *method) {
  return method->is_dtls ? Span<const uint16_t>(kDTLSVersions)
                         : Span<const uint16_t>(kTLSVersions);
}

bool ssl_method_supports_version(const SSL_PROTOCOL_METHOD *method,
                                 uint16_t version) {
  for (uint16_t supported : get_method_versions(method)) {
    if (supported == version) {
      return true;
    }
  }
  return false;
}

// public API works on wire versions.

static const char *ssl_version_to_string(uint16_t version) {
  switch (version) {
    case TLS1_3_VERSION:
      return "TLSv1.3";

    case TLS1_2_VERSION:
      return "TLSv1.2";

    case TLS1_1_VERSION:
      return "TLSv1.1";

    case TLS1_VERSION:
      return "TLSv1";

    case DTLS1_VERSION:
      return "DTLSv1";

    case DTLS1_2_VERSION:
      return "DTLSv1.2";

    default:
      return "unknown";
  }
}

static uint16_t wire_version_to_api(uint16_t version) {
  return version;
}

static bool api_version_to_wire(uint16_t *out, uint16_t version) {

  uint16_t unused;
  if (!ssl_protocol_version_from_wire(&unused, version)) {
    return false;
  }

  *out = version;
  return true;
}

static bool set_version_bound(const SSL_PROTOCOL_METHOD *method, uint16_t *out,
                              uint16_t version) {
  if (!api_version_to_wire(&version, version) ||
      !ssl_method_supports_version(method, version)) {
    OPENSSL_PUT_ERROR(SSL, SSL_R_UNKNOWN_SSL_VERSION);
    return false;
  }

  *out = version;
  return true;
}

static bool set_min_version(const SSL_PROTOCOL_METHOD *method, uint16_t *out,
                            uint16_t version) {

  if (version == 0) {
    *out = method->is_dtls ? DTLS1_VERSION : TLS1_VERSION;
    return true;
  }

  return set_version_bound(method, out, version);
}

static bool set_max_version(const SSL_PROTOCOL_METHOD *method, uint16_t *out,
                            uint16_t version) {

  if (version == 0) {
    *out = method->is_dtls ? DTLS1_2_VERSION : TLS1_2_VERSION;
    return true;
  }

  return set_version_bound(method, out, version);
}

const struct {
  uint16_t version;
  uint32_t flag;
} kProtocolVersions[] = {
    {TLS1_VERSION, SSL_OP_NO_TLSv1},
    {TLS1_1_VERSION, SSL_OP_NO_TLSv1_1},
    {TLS1_2_VERSION, SSL_OP_NO_TLSv1_2},
    {TLS1_3_VERSION, SSL_OP_NO_TLSv1_3},
};

bool ssl_get_version_range(const SSL_HANDSHAKE *hs, uint16_t *out_min_version,
                           uint16_t *out_max_version) {


  uint32_t options = hs->ssl->options;
  if (SSL_is_dtls(hs->ssl)) {
    options &= ~SSL_OP_NO_TLSv1_1;
    if (options & SSL_OP_NO_DTLSv1) {
      options |= SSL_OP_NO_TLSv1_1;
    }
  }

  uint16_t min_version, max_version;
  if (!ssl_protocol_version_from_wire(&min_version,
                                      hs->config->conf_min_version) ||
      !ssl_protocol_version_from_wire(&max_version,
                                      hs->config->conf_max_version)) {
    OPENSSL_PUT_ERROR(SSL, ERR_R_INTERNAL_ERROR);
    return false;
  }

  if (hs->ssl->quic_method && min_version < TLS1_3_VERSION) {
    min_version = TLS1_3_VERSION;
  }










  bool any_enabled = false;
  for (size_t i = 0; i < OPENSSL_ARRAY_SIZE(kProtocolVersions); i++) {

    if (min_version > kProtocolVersions[i].version) {
      continue;
    }
    if (max_version < kProtocolVersions[i].version) {
      break;
    }

    if (!(options & kProtocolVersions[i].flag)) {

      if (!any_enabled) {
        any_enabled = true;
        min_version = kProtocolVersions[i].version;
      }
      continue;
    }


    if (any_enabled) {
      max_version = kProtocolVersions[i-1].version;
      break;
    }
  }

  if (!any_enabled) {
    OPENSSL_PUT_ERROR(SSL, SSL_R_NO_SUPPORTED_VERSIONS_ENABLED);
    return false;
  }

  *out_min_version = min_version;
  *out_max_version = max_version;
  return true;
}

static uint16_t ssl_version(const SSL *ssl) {

  if (SSL_in_early_data(ssl) && !ssl->server) {
    return ssl->s3->hs->early_session->ssl_version;
  }
  return ssl->version;
}

uint16_t ssl_protocol_version(const SSL *ssl) {
  assert(ssl->s3->have_version);
  uint16_t version;
  if (!ssl_protocol_version_from_wire(&version, ssl->version)) {

    assert(0);
    return 0;
  }

  return version;
}

bool ssl_supports_version(SSL_HANDSHAKE *hs, uint16_t version) {
  SSL *const ssl = hs->ssl;
  uint16_t protocol_version;
  if (!ssl_method_supports_version(ssl->method, version) ||
      !ssl_protocol_version_from_wire(&protocol_version, version) ||
      hs->min_version > protocol_version ||
      protocol_version > hs->max_version) {
    return false;
  }

  return true;
}

bool ssl_add_supported_versions(SSL_HANDSHAKE *hs, CBB *cbb) {
  for (uint16_t version : get_method_versions(hs->ssl->method)) {
    if (ssl_supports_version(hs, version) &&
        !CBB_add_u16(cbb, version)) {
      return false;
    }
  }
  return true;
}

bool ssl_negotiate_version(SSL_HANDSHAKE *hs, uint8_t *out_alert,
                           uint16_t *out_version, const CBS *peer_versions) {
  for (uint16_t version : get_method_versions(hs->ssl->method)) {
    if (!ssl_supports_version(hs, version)) {
      continue;
    }








    if (version == TLS1_3_VERSION && hs->apply_jdk11_workaround) {
      continue;
    }

    CBS copy = *peer_versions;
    while (CBS_len(&copy) != 0) {
      uint16_t peer_version;
      if (!CBS_get_u16(&copy, &peer_version)) {
        OPENSSL_PUT_ERROR(SSL, SSL_R_DECODE_ERROR);
        *out_alert = SSL_AD_DECODE_ERROR;
        return false;
      }

      if (peer_version == version) {
        *out_version = version;
        return true;
      }
    }
  }

  OPENSSL_PUT_ERROR(SSL, SSL_R_UNSUPPORTED_PROTOCOL);
  *out_alert = SSL_AD_PROTOCOL_VERSION;
  return false;
}

BSSL_NAMESPACE_END

using namespace bssl;

int SSL_CTX_set_min_proto_version(SSL_CTX *ctx, uint16_t version) {
  return set_min_version(ctx->method, &ctx->conf_min_version, version);
}

int SSL_CTX_set_max_proto_version(SSL_CTX *ctx, uint16_t version) {
  return set_max_version(ctx->method, &ctx->conf_max_version, version);
}

uint16_t SSL_CTX_get_min_proto_version(SSL_CTX *ctx) {
  return ctx->conf_min_version;
}

uint16_t SSL_CTX_get_max_proto_version(SSL_CTX *ctx) {
  return ctx->conf_max_version;
}

int SSL_set_min_proto_version(SSL *ssl, uint16_t version) {
  if (!ssl->config) {
    return 0;
  }
  return set_min_version(ssl->method, &ssl->config->conf_min_version, version);
}

int SSL_set_max_proto_version(SSL *ssl, uint16_t version) {
  if (!ssl->config) {
    return 0;
  }
  return set_max_version(ssl->method, &ssl->config->conf_max_version, version);
}

int SSL_version(const SSL *ssl) {
  return wire_version_to_api(ssl_version(ssl));
}

const char *SSL_get_version(const SSL *ssl) {
  return ssl_version_to_string(ssl_version(ssl));
}

const char *SSL_SESSION_get_version(const SSL_SESSION *session) {
  return ssl_version_to_string(session->ssl_version);
}

uint16_t SSL_SESSION_get_protocol_version(const SSL_SESSION *session) {
  return wire_version_to_api(session->ssl_version);
}

int SSL_SESSION_set_protocol_version(SSL_SESSION *session, uint16_t version) {


  return api_version_to_wire(&session->ssl_version, version);
}
