// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_UNGUESSABLE_TOKEN_H_
#define BASE_UNGUESSABLE_TOKEN_H_

#include <stdint.h>
#include <string.h>
#include <iosfwd>
#include <tuple>

#include "base/base_export.h"
#include "base/hash/hash.h"
#include "base/logging.h"
#include "base/token.h"

namespace base {

struct UnguessableTokenHash;

// Token, a new UnguessableToken is always generated at runtime from a
// cryptographically strong random source (or copied or serialized and
// deserialized from another such UnguessableToken). It can be used as part of a
// larger aggregate type, or as an ID in and of itself.
//
// An UnguessableToken is a strong *bearer token*. Bearer tokens are like HTTP
// cookies: if a caller has the token, the callee thereby considers the caller
// authorized to request the operation the callee performs.
//
// UnguessableToken can be used when the resource associated with the ID needs
// to be protected against manipulation by other untrusted agents in the system,
// and there is no other convenient way to verify the authority of the agent to
// do so (because the resource is part of a table shared across processes, for
// instance). In such a scheme, knowledge of the token value in and of itself is
// sufficient proof of authority to carry out an operation on the associated
// resource.
//
// Use Create() for creating new UnguessableTokens.
//
// NOTE: It is illegal to send empty UnguessableTokens across processes, and
// sending/receiving empty tokens should be treated as a security issue. If
// there is a valid scenario for sending "no token" across processes, use
// base::Optional instead of an empty token.

class BASE_EXPORT UnguessableToken {
 public:

  static UnguessableToken Create();




  static const UnguessableToken& Null();





  static UnguessableToken Deserialize(uint64_t high, uint64_t low);


  constexpr UnguessableToken() = default;

  uint64_t GetHighForSerialization() const {
    DCHECK(!is_empty());
    return token_.high();
  }

  uint64_t GetLowForSerialization() const {
    DCHECK(!is_empty());
    return token_.low();
  }

  bool is_empty() const { return token_.is_zero(); }

  std::string ToString() const { return token_.ToString(); }

  explicit operator bool() const { return !is_empty(); }

  bool operator<(const UnguessableToken& other) const {
    return token_ < other.token_;
  }

  bool operator==(const UnguessableToken& other) const {
    return token_ == other.token_;
  }

  bool operator!=(const UnguessableToken& other) const {
    return !(*this == other);
  }

 private:
  friend struct UnguessableTokenHash;
  explicit UnguessableToken(const Token& token);

  base::Token token_;
};

BASE_EXPORT std::ostream& operator<<(std::ostream& out,
                                     const UnguessableToken& token);

struct UnguessableTokenHash {
  size_t operator()(const base::UnguessableToken& token) const {
    DCHECK(token);
    return TokenHash()(token.token_);
  }
};

}  // namespace base

#endif  // BASE_UNGUESSABLE_TOKEN_H_
