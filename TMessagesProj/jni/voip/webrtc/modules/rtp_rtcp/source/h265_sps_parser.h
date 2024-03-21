/*
 *  Intel License
 */

#ifndef WEBRTC_MODULES_RTP_RTCP_SOURCE_H265_SPS_PARSER_H_
#define WEBRTC_MODULES_RTP_RTCP_SOURCE_H265_SPS_PARSER_H_

#include "webrtc/base/common.h"

namespace webrtc {

// Currently, only resolution is read without being ignored.
class H265SpsParser {
 public:
  H265SpsParser(const uint8_t* sps, size_t byte_length);

  bool Parse();
  uint16_t width() { return width_; }
  uint16_t height() { return height_; }

 private:
  const uint8_t* const sps_;
  const size_t byte_length_;

  uint16_t width_;
  uint16_t height_;
};

}  // namespace webrtc
#endif  // WEBRTC_MODULES_RTP_RTCP_SOURCE_H265_SPS_PARSER_H_
