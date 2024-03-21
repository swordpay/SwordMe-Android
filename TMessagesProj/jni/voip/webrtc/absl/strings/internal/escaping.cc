// Copyright 2020 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "absl/strings/internal/escaping.h"

#include "absl/base/internal/endian.h"
#include "absl/base/internal/raw_logging.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace strings_internal {

ABSL_CONST_INIT const char kBase64Chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t CalculateBase64EscapedLenInternal(size_t input_len, bool do_padding) {













  size_t len = (input_len / 3) * 4;

  if (input_len % 3 == 0) {




  } else if (input_len % 3 == 1) {




    len += 2;
    if (do_padding) {
      len += 2;
    }
  } else {  // (input_len % 3 == 2)




    len += 3;
    if (do_padding) {
      len += 1;
    }
  }

  assert(len >= input_len);  // make sure we didn't overflow
  return len;
}

size_t Base64EscapeInternal(const unsigned char* src, size_t szsrc, char* dest,
                            size_t szdest, const char* base64,
                            bool do_padding) {
  static const char kPad64 = '=';

  if (szsrc * 4 > szdest * 3) return 0;

  char* cur_dest = dest;
  const unsigned char* cur_src = src;

  char* const limit_dest = dest + szdest;
  const unsigned char* const limit_src = src + szsrc;


  if (szsrc >= 3) {                    // "limit_src - 3" is UB if szsrc < 3.
    while (cur_src < limit_src - 3) {  // While we have >= 32 bits.
      uint32_t in = absl::big_endian::Load32(cur_src) >> 8;

      cur_dest[0] = base64[in >> 18];
      in &= 0x3FFFF;
      cur_dest[1] = base64[in >> 12];
      in &= 0xFFF;
      cur_dest[2] = base64[in >> 6];
      in &= 0x3F;
      cur_dest[3] = base64[in];

      cur_dest += 4;
      cur_src += 3;
    }
  }

  szdest = static_cast<size_t>(limit_dest - cur_dest);
  szsrc = static_cast<size_t>(limit_src - cur_src);

  /* now deal with the tail (<=3 bytes) */
  switch (szsrc) {
    case 0:

      break;
    case 1: {


      if (szdest < 2) return 0;
      uint32_t in = cur_src[0];
      cur_dest[0] = base64[in >> 2];
      in &= 0x3;
      cur_dest[1] = base64[in << 4];
      cur_dest += 2;
      szdest -= 2;
      if (do_padding) {
        if (szdest < 2) return 0;
        cur_dest[0] = kPad64;
        cur_dest[1] = kPad64;
        cur_dest += 2;
        szdest -= 2;
      }
      break;
    }
    case 2: {


      if (szdest < 3) return 0;
      uint32_t in = absl::big_endian::Load16(cur_src);
      cur_dest[0] = base64[in >> 10];
      in &= 0x3FF;
      cur_dest[1] = base64[in >> 4];
      in &= 0x00F;
      cur_dest[2] = base64[in << 2];
      cur_dest += 3;
      szdest -= 3;
      if (do_padding) {
        if (szdest < 1) return 0;
        cur_dest[0] = kPad64;
        cur_dest += 1;
        szdest -= 1;
      }
      break;
    }
    case 3: {



      if (szdest < 4) return 0;
      uint32_t in =
          (uint32_t{cur_src[0]} << 16) + absl::big_endian::Load16(cur_src + 1);
      cur_dest[0] = base64[in >> 18];
      in &= 0x3FFFF;
      cur_dest[1] = base64[in >> 12];
      in &= 0xFFF;
      cur_dest[2] = base64[in >> 6];
      in &= 0x3F;
      cur_dest[3] = base64[in];
      cur_dest += 4;
      szdest -= 4;
      break;
    }
    default:


      ABSL_RAW_LOG(FATAL, "Logic problem? szsrc = %zu", szsrc);
      break;
  }
  return static_cast<size_t>(cur_dest - dest);
}

}  // namespace strings_internal
ABSL_NAMESPACE_END
}  // namespace absl
