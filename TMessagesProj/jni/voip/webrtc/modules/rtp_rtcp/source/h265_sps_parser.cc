/*
 * Intel License
 */

#include "webrtc/modules/rtp_rtcp/source/h265_sps_parser.h"

#include "webrtc/base/bitbuffer.h"
#include "webrtc/base/bytebuffer.h"
#include "webrtc/base/logging.h"

#include <vector>

#define RETURN_FALSE_ON_FAIL(x) \
  if (!(x)) {                   \
    return false;               \
  }

namespace webrtc {

H265SpsParser::H265SpsParser(const uint8_t* sps, size_t byte_length)
    : sps_(sps), byte_length_(byte_length), width_(), height_() {
}

bool H265SpsParser::Parse() {




  const char* sps_bytes = reinterpret_cast<const char*>(sps_);



  rtc::ByteBufferWriter rbsp_buffer;
  for (size_t i = 0; i < byte_length_;) {




    if (byte_length_ - i >= 3 && sps_[i] == 0 && sps_[i + 1] == 0 &&
        sps_[i + 2] == 3) {

      rbsp_buffer.WriteBytes(sps_bytes + i, 2);
      i += 3;
    } else {

      rbsp_buffer.WriteBytes(sps_bytes + i, 1);
      i++;
    }
  }










  rtc::BitBuffer parser(reinterpret_cast<const uint8_t*>(rbsp_buffer.Data()),
                        rbsp_buffer.Length());

  uint32_t golomb_ignored;


  uint32_t separate_colour_plane_flag = 0;


  uint32_t chroma_format_idc = 1;

  RETURN_FALSE_ON_FAIL(parser.ConsumeBits(4));

  uint32_t sps_max_sub_layers_minus1 = 0;
  RETURN_FALSE_ON_FAIL(parser.ReadBits(3, sps_max_sub_layers_minus1));

  RETURN_FALSE_ON_FAIL(parser.ConsumeBits(1));



  RETURN_FALSE_ON_FAIL(parser.ConsumeBytes(1));

  RETURN_FALSE_ON_FAIL(parser.ConsumeBytes(4));


  RETURN_FALSE_ON_FAIL(parser.ConsumeBits(4));

  RETURN_FALSE_ON_FAIL(parser.ConsumeBits(43));

  RETURN_FALSE_ON_FAIL(parser.ConsumeBits(1));

  RETURN_FALSE_ON_FAIL(parser.ConsumeBytes(1));

  std::vector<uint32_t> sub_layer_profile_present_flags;
  std::vector<uint32_t> sub_layer_level_present_flags;
  uint32_t sub_layer_profile_present = 0;
  uint32_t sub_layer_level_present = 0;
  for (uint32_t i = 0; i < sps_max_sub_layers_minus1; i++) {

      RETURN_FALSE_ON_FAIL(parser.ReadBits(1, sub_layer_profile_present));
      RETURN_FALSE_ON_FAIL(parser.ReadBits(1, sub_layer_level_present));
      sub_layer_profile_present_flags.push_back(sub_layer_profile_present);
      sub_layer_level_present_flags.push_back(sub_layer_level_present);
  }
  if (sps_max_sub_layers_minus1 > 0) {
      for (uint32_t j = sps_max_sub_layers_minus1; j < 8; j++) {

          RETURN_FALSE_ON_FAIL(parser.ConsumeBits(2));
      }
  }
  for (uint32_t k = 0; k < sps_max_sub_layers_minus1; k++) {
      if(sub_layer_profile_present_flags[k]) {//

        RETURN_FALSE_ON_FAIL(parser.ConsumeBytes(1));

        RETURN_FALSE_ON_FAIL(parser.ConsumeBytes(4));


        RETURN_FALSE_ON_FAIL(parser.ConsumeBits(4));

        RETURN_FALSE_ON_FAIL(parser.ConsumeBits(43));

        RETURN_FALSE_ON_FAIL(parser.ConsumeBits(1));
      }
      if (sub_layer_level_present_flags[k]) {

          RETURN_FALSE_ON_FAIL(parser.ConsumeBytes(1));
      }
  }

  RETURN_FALSE_ON_FAIL(parser.ReadExponentialGolomb(golomb_ignored));

  RETURN_FALSE_ON_FAIL(parser.ReadExponentialGolomb(chroma_format_idc));
  if (chroma_format_idc == 3) {

    RETURN_FALSE_ON_FAIL(parser.ReadBits(1, separate_colour_plane_flag));
  }
  uint32_t pic_width_in_luma_samples = 0;
  uint32_t pic_height_in_luma_samples = 0;

  RETURN_FALSE_ON_FAIL(parser.ReadExponentialGolomb(pic_width_in_luma_samples));

  RETURN_FALSE_ON_FAIL(parser.ReadExponentialGolomb(pic_height_in_luma_samples));

  uint32_t conformance_window_flag = 0;
  RETURN_FALSE_ON_FAIL(parser.ReadBits(1, conformance_window_flag));

  uint32_t conf_win_left_offset = 0;
  uint32_t conf_win_right_offset = 0;
  uint32_t conf_win_top_offset = 0;
  uint32_t conf_win_bottom_offset = 0;
  if (conformance_window_flag) {

      RETURN_FALSE_ON_FAIL(parser.ReadExponentialGolomb(conf_win_left_offset));

      RETURN_FALSE_ON_FAIL(parser.ReadExponentialGolomb(conf_win_right_offset));

      RETURN_FALSE_ON_FAIL(parser.ReadExponentialGolomb(conf_win_top_offset));

      RETURN_FALSE_ON_FAIL(parser.ReadExponentialGolomb(conf_win_bottom_offset));
  }

  int width = 0;
  int height = 0;

  width = pic_width_in_luma_samples;
  height = pic_height_in_luma_samples;

  if (conformance_window_flag) {
    int sub_width_c = ((1 == chroma_format_idc) || (2 == chroma_format_idc)) &&
                        (0 == separate_colour_plane_flag) ? 2 : 1;
    int sub_height_c = (1 == chroma_format_idc) && (0 == separate_colour_plane_flag) ? 2 : 1;

    width -= sub_width_c*(conf_win_right_offset + conf_win_left_offset);
    height -= sub_height_c*(conf_win_top_offset + conf_win_bottom_offset);
  }

  width_ = width;
  height_ = height;
  return true;

}

}  // namespace webrtc
