// Copyright (c) 2012 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#ifndef MKVMUXER_MKVMUXER_H_
#define MKVMUXER_MKVMUXER_H_

#include <stdint.h>

#include <cstddef>
#include <list>
#include <map>

#include "common/webmids.h"
#include "mkvmuxer/mkvmuxertypes.h"

// http://www.webmproject.org/code/specs/container/.

namespace mkvparser {
class IMkvReader;
}  // namespace mkvparser

namespace mkvmuxer {

class MkvWriter;
class Segment;

const uint64_t kMaxTrackNumber = 126;

// Interface used by the mkvmuxer to write out the Mkv data.
class IMkvWriter {
 public:

  virtual int32 Write(const void* buf, uint32 len) = 0;


  virtual int64 Position() const = 0;

  virtual int32 Position(int64 position) = 0;

  virtual bool Seekable() const = 0;





  virtual void ElementStartNotify(uint64 element_id, int64 position) = 0;

 protected:
  IMkvWriter();
  virtual ~IMkvWriter();

 private:
  LIBWEBM_DISALLOW_COPY_AND_ASSIGN(IMkvWriter);
};

// DocType. This function must be called before any other libwebm writing
// functions are called.
bool WriteEbmlHeader(IMkvWriter* writer, uint64_t doc_type_version,
                     const char* const doc_type);

// before any other libwebm writing functions are called.
bool WriteEbmlHeader(IMkvWriter* writer, uint64_t doc_type_version);

// kDefaultDocTypeVersion. Exists for backward compatibility.
bool WriteEbmlHeader(IMkvWriter* writer);

bool ChunkedCopy(mkvparser::IMkvReader* source, IMkvWriter* dst, int64_t start,
                 int64_t size);

// Class to hold data the will be written to a block.
class Frame {
 public:
  Frame();
  ~Frame();


  bool CopyFrom(const Frame& frame);

  bool Init(const uint8_t* frame, uint64_t length);

  bool AddAdditionalData(const uint8_t* additional, uint64_t length,
                         uint64_t add_id);

  bool IsValid() const;


  bool CanBeSimpleBlock() const;

  uint64_t add_id() const { return add_id_; }
  const uint8_t* additional() const { return additional_; }
  uint64_t additional_length() const { return additional_length_; }
  void set_duration(uint64_t duration);
  uint64_t duration() const { return duration_; }
  bool duration_set() const { return duration_set_; }
  const uint8_t* frame() const { return frame_; }
  void set_is_key(bool key) { is_key_ = key; }
  bool is_key() const { return is_key_; }
  uint64_t length() const { return length_; }
  void set_track_number(uint64_t track_number) { track_number_ = track_number; }
  uint64_t track_number() const { return track_number_; }
  void set_timestamp(uint64_t timestamp) { timestamp_ = timestamp; }
  uint64_t timestamp() const { return timestamp_; }
  void set_discard_padding(int64_t discard_padding) {
    discard_padding_ = discard_padding;
  }
  int64_t discard_padding() const { return discard_padding_; }
  void set_reference_block_timestamp(int64_t reference_block_timestamp);
  int64_t reference_block_timestamp() const {
    return reference_block_timestamp_;
  }
  bool reference_block_timestamp_set() const {
    return reference_block_timestamp_set_;
  }

 private:

  uint64_t add_id_;

  uint8_t* additional_;

  uint64_t additional_length_;

  uint64_t duration_;



  bool duration_set_;

  uint8_t* frame_;

  bool is_key_;

  uint64_t length_;

  uint64_t track_number_;

  uint64_t timestamp_;

  int64_t discard_padding_;

  int64_t reference_block_timestamp_;

  bool reference_block_timestamp_set_;

  LIBWEBM_DISALLOW_COPY_AND_ASSIGN(Frame);
};

// Class to hold one cue point in a Cues element.
class CuePoint {
 public:
  CuePoint();
  ~CuePoint();

  uint64_t Size() const;

  bool Write(IMkvWriter* writer) const;

  void set_time(uint64_t time) { time_ = time; }
  uint64_t time() const { return time_; }
  void set_track(uint64_t track) { track_ = track; }
  uint64_t track() const { return track_; }
  void set_cluster_pos(uint64_t cluster_pos) { cluster_pos_ = cluster_pos; }
  uint64_t cluster_pos() const { return cluster_pos_; }
  void set_block_number(uint64_t block_number) { block_number_ = block_number; }
  uint64_t block_number() const { return block_number_; }
  void set_output_block_number(bool output_block_number) {
    output_block_number_ = output_block_number;
  }
  bool output_block_number() const { return output_block_number_; }

 private:

  uint64_t PayloadSize() const;

  uint64_t time_;

  uint64_t track_;

  uint64_t cluster_pos_;

  uint64_t block_number_;


  bool output_block_number_;

  LIBWEBM_DISALLOW_COPY_AND_ASSIGN(CuePoint);
};

// Cues element.
class Cues {
 public:
  Cues();
  ~Cues();

  bool AddCue(CuePoint* cue);


  CuePoint* GetCueByIndex(int32_t index) const;

  uint64_t Size();

  bool Write(IMkvWriter* writer) const;

  int32_t cue_entries_size() const { return cue_entries_size_; }
  void set_output_block_number(bool output_block_number) {
    output_block_number_ = output_block_number;
  }
  bool output_block_number() const { return output_block_number_; }

 private:

  int32_t cue_entries_capacity_;

  int32_t cue_entries_size_;

  CuePoint** cue_entries_;


  bool output_block_number_;

  LIBWEBM_DISALLOW_COPY_AND_ASSIGN(Cues);
};

// ContentEncAESSettings element
class ContentEncAESSettings {
 public:
  enum { kCTR = 1 };

  ContentEncAESSettings();
  ~ContentEncAESSettings() {}

  uint64_t Size() const;


  bool Write(IMkvWriter* writer) const;

  uint64_t cipher_mode() const { return cipher_mode_; }

 private:


  uint64_t PayloadSize() const;

  uint64_t cipher_mode_;

  LIBWEBM_DISALLOW_COPY_AND_ASSIGN(ContentEncAESSettings);
};

// ContentEncoding element
// Elements used to describe if the track data has been encrypted or
// compressed with zlib or header stripping.
// Currently only whole frames can be encrypted with AES. This dictates that
// ContentEncodingOrder will be 0, ContentEncodingScope will be 1,
// ContentEncodingType will be 1, and ContentEncAlgo will be 5.
class ContentEncoding {
 public:
  ContentEncoding();
  ~ContentEncoding();


  bool SetEncryptionID(const uint8_t* id, uint64_t length);

  uint64_t Size() const;


  bool Write(IMkvWriter* writer) const;

  uint64_t enc_algo() const { return enc_algo_; }
  uint64_t encoding_order() const { return encoding_order_; }
  uint64_t encoding_scope() const { return encoding_scope_; }
  uint64_t encoding_type() const { return encoding_type_; }
  ContentEncAESSettings* enc_aes_settings() { return &enc_aes_settings_; }

 private:

  uint64_t EncodingSize(uint64_t compresion_size,
                        uint64_t encryption_size) const;

  uint64_t EncryptionSize() const;

  uint64_t enc_algo_;
  uint8_t* enc_key_id_;
  uint64_t encoding_order_;
  uint64_t encoding_scope_;
  uint64_t encoding_type_;

  ContentEncAESSettings enc_aes_settings_;

  uint64_t enc_key_id_length_;

  LIBWEBM_DISALLOW_COPY_AND_ASSIGN(ContentEncoding);
};

// Colour element.
class PrimaryChromaticity {
 public:
  static const float kChromaticityMin;
  static const float kChromaticityMax;

  PrimaryChromaticity(float x_val, float y_val) : x_(x_val), y_(y_val) {}
  PrimaryChromaticity() : x_(0), y_(0) {}
  ~PrimaryChromaticity() {}

  uint64_t PrimaryChromaticitySize(libwebm::MkvId x_id,
                                   libwebm::MkvId y_id) const;
  bool Valid() const;
  bool Write(IMkvWriter* writer, libwebm::MkvId x_id,
             libwebm::MkvId y_id) const;

  float x() const { return x_; }
  void set_x(float new_x) { x_ = new_x; }
  float y() const { return y_; }
  void set_y(float new_y) { y_ = new_y; }

 private:
  float x_;
  float y_;
};

class MasteringMetadata {
 public:
  static const float kValueNotPresent;
  static const float kMinLuminance;
  static const float kMinLuminanceMax;
  static const float kMaxLuminanceMax;

  MasteringMetadata()
      : luminance_max_(kValueNotPresent),
        luminance_min_(kValueNotPresent),
        r_(NULL),
        g_(NULL),
        b_(NULL),
        white_point_(NULL) {}
  ~MasteringMetadata() {
    delete r_;
    delete g_;
    delete b_;
    delete white_point_;
  }

  uint64_t MasteringMetadataSize() const;
  bool Valid() const;
  bool Write(IMkvWriter* writer) const;

  bool SetChromaticity(const PrimaryChromaticity* r,
                       const PrimaryChromaticity* g,
                       const PrimaryChromaticity* b,
                       const PrimaryChromaticity* white_point);
  const PrimaryChromaticity* r() const { return r_; }
  const PrimaryChromaticity* g() const { return g_; }
  const PrimaryChromaticity* b() const { return b_; }
  const PrimaryChromaticity* white_point() const { return white_point_; }

  float luminance_max() const { return luminance_max_; }
  void set_luminance_max(float luminance_max) {
    luminance_max_ = luminance_max;
  }
  float luminance_min() const { return luminance_min_; }
  void set_luminance_min(float luminance_min) {
    luminance_min_ = luminance_min;
  }

 private:

  uint64_t PayloadSize() const;

  float luminance_max_;
  float luminance_min_;
  PrimaryChromaticity* r_;
  PrimaryChromaticity* g_;
  PrimaryChromaticity* b_;
  PrimaryChromaticity* white_point_;
};

class Colour {
 public:
  enum MatrixCoefficients {
    kGbr = 0,
    kBt709 = 1,
    kUnspecifiedMc = 2,
    kReserved = 3,
    kFcc = 4,
    kBt470bg = 5,
    kSmpte170MMc = 6,
    kSmpte240MMc = 7,
    kYcocg = 8,
    kBt2020NonConstantLuminance = 9,
    kBt2020ConstantLuminance = 10,
  };
  enum ChromaSitingHorz {
    kUnspecifiedCsh = 0,
    kLeftCollocated = 1,
    kHalfCsh = 2,
  };
  enum ChromaSitingVert {
    kUnspecifiedCsv = 0,
    kTopCollocated = 1,
    kHalfCsv = 2,
  };
  enum Range {
    kUnspecifiedCr = 0,
    kBroadcastRange = 1,
    kFullRange = 2,
    kMcTcDefined = 3,  // Defined by MatrixCoefficients/TransferCharacteristics.
  };
  enum TransferCharacteristics {
    kIturBt709Tc = 1,
    kUnspecifiedTc = 2,
    kReservedTc = 3,
    kGamma22Curve = 4,
    kGamma28Curve = 5,
    kSmpte170MTc = 6,
    kSmpte240MTc = 7,
    kLinear = 8,
    kLog = 9,
    kLogSqrt = 10,
    kIec6196624 = 11,
    kIturBt1361ExtendedColourGamut = 12,
    kIec6196621 = 13,
    kIturBt202010bit = 14,
    kIturBt202012bit = 15,
    kSmpteSt2084 = 16,
    kSmpteSt4281Tc = 17,
    kAribStdB67Hlg = 18,
  };
  enum Primaries {
    kReservedP0 = 0,
    kIturBt709P = 1,
    kUnspecifiedP = 2,
    kReservedP3 = 3,
    kIturBt470M = 4,
    kIturBt470Bg = 5,
    kSmpte170MP = 6,
    kSmpte240MP = 7,
    kFilm = 8,
    kIturBt2020 = 9,
    kSmpteSt4281P = 10,
    kJedecP22Phosphors = 22,
  };
  static const uint64_t kValueNotPresent;
  Colour()
      : matrix_coefficients_(kValueNotPresent),
        bits_per_channel_(kValueNotPresent),
        chroma_subsampling_horz_(kValueNotPresent),
        chroma_subsampling_vert_(kValueNotPresent),
        cb_subsampling_horz_(kValueNotPresent),
        cb_subsampling_vert_(kValueNotPresent),
        chroma_siting_horz_(kValueNotPresent),
        chroma_siting_vert_(kValueNotPresent),
        range_(kValueNotPresent),
        transfer_characteristics_(kValueNotPresent),
        primaries_(kValueNotPresent),
        max_cll_(kValueNotPresent),
        max_fall_(kValueNotPresent),
        mastering_metadata_(NULL) {}
  ~Colour() { delete mastering_metadata_; }

  uint64_t ColourSize() const;
  bool Valid() const;
  bool Write(IMkvWriter* writer) const;

  bool SetMasteringMetadata(const MasteringMetadata& mastering_metadata);

  const MasteringMetadata* mastering_metadata() const {
    return mastering_metadata_;
  }

  uint64_t matrix_coefficients() const { return matrix_coefficients_; }
  void set_matrix_coefficients(uint64_t matrix_coefficients) {
    matrix_coefficients_ = matrix_coefficients;
  }
  uint64_t bits_per_channel() const { return bits_per_channel_; }
  void set_bits_per_channel(uint64_t bits_per_channel) {
    bits_per_channel_ = bits_per_channel;
  }
  uint64_t chroma_subsampling_horz() const { return chroma_subsampling_horz_; }
  void set_chroma_subsampling_horz(uint64_t chroma_subsampling_horz) {
    chroma_subsampling_horz_ = chroma_subsampling_horz;
  }
  uint64_t chroma_subsampling_vert() const { return chroma_subsampling_vert_; }
  void set_chroma_subsampling_vert(uint64_t chroma_subsampling_vert) {
    chroma_subsampling_vert_ = chroma_subsampling_vert;
  }
  uint64_t cb_subsampling_horz() const { return cb_subsampling_horz_; }
  void set_cb_subsampling_horz(uint64_t cb_subsampling_horz) {
    cb_subsampling_horz_ = cb_subsampling_horz;
  }
  uint64_t cb_subsampling_vert() const { return cb_subsampling_vert_; }
  void set_cb_subsampling_vert(uint64_t cb_subsampling_vert) {
    cb_subsampling_vert_ = cb_subsampling_vert;
  }
  uint64_t chroma_siting_horz() const { return chroma_siting_horz_; }
  void set_chroma_siting_horz(uint64_t chroma_siting_horz) {
    chroma_siting_horz_ = chroma_siting_horz;
  }
  uint64_t chroma_siting_vert() const { return chroma_siting_vert_; }
  void set_chroma_siting_vert(uint64_t chroma_siting_vert) {
    chroma_siting_vert_ = chroma_siting_vert;
  }
  uint64_t range() const { return range_; }
  void set_range(uint64_t range) { range_ = range; }
  uint64_t transfer_characteristics() const {
    return transfer_characteristics_;
  }
  void set_transfer_characteristics(uint64_t transfer_characteristics) {
    transfer_characteristics_ = transfer_characteristics;
  }
  uint64_t primaries() const { return primaries_; }
  void set_primaries(uint64_t primaries) { primaries_ = primaries; }
  uint64_t max_cll() const { return max_cll_; }
  void set_max_cll(uint64_t max_cll) { max_cll_ = max_cll; }
  uint64_t max_fall() const { return max_fall_; }
  void set_max_fall(uint64_t max_fall) { max_fall_ = max_fall; }

 private:

  uint64_t PayloadSize() const;

  uint64_t matrix_coefficients_;
  uint64_t bits_per_channel_;
  uint64_t chroma_subsampling_horz_;
  uint64_t chroma_subsampling_vert_;
  uint64_t cb_subsampling_horz_;
  uint64_t cb_subsampling_vert_;
  uint64_t chroma_siting_horz_;
  uint64_t chroma_siting_vert_;
  uint64_t range_;
  uint64_t transfer_characteristics_;
  uint64_t primaries_;
  uint64_t max_cll_;
  uint64_t max_fall_;

  MasteringMetadata* mastering_metadata_;
};

// Projection element.
class Projection {
 public:
  enum ProjectionType {
    kTypeNotPresent = -1,
    kRectangular = 0,
    kEquirectangular = 1,
    kCubeMap = 2,
    kMesh = 3,
  };
  static const uint64_t kValueNotPresent;
  Projection()
      : type_(kRectangular),
        pose_yaw_(0.0),
        pose_pitch_(0.0),
        pose_roll_(0.0),
        private_data_(NULL),
        private_data_length_(0) {}
  ~Projection() { delete[] private_data_; }

  uint64_t ProjectionSize() const;
  bool Write(IMkvWriter* writer) const;

  bool SetProjectionPrivate(const uint8_t* private_data,
                            uint64_t private_data_length);

  ProjectionType type() const { return type_; }
  void set_type(ProjectionType type) { type_ = type; }
  float pose_yaw() const { return pose_yaw_; }
  void set_pose_yaw(float pose_yaw) { pose_yaw_ = pose_yaw; }
  float pose_pitch() const { return pose_pitch_; }
  void set_pose_pitch(float pose_pitch) { pose_pitch_ = pose_pitch; }
  float pose_roll() const { return pose_roll_; }
  void set_pose_roll(float pose_roll) { pose_roll_ = pose_roll; }
  uint8_t* private_data() const { return private_data_; }
  uint64_t private_data_length() const { return private_data_length_; }

 private:

  uint64_t PayloadSize() const;

  ProjectionType type_;
  float pose_yaw_;
  float pose_pitch_;
  float pose_roll_;
  uint8_t* private_data_;
  uint64_t private_data_length_;
};

// Track element.
class Track {
 public:

  explicit Track(unsigned int* seed);
  virtual ~Track();

  virtual bool AddContentEncoding();


  ContentEncoding* GetContentEncodingByIndex(uint32_t index) const;

  virtual uint64_t PayloadSize() const;

  virtual uint64_t Size() const;

  virtual bool Write(IMkvWriter* writer) const;


  bool SetCodecPrivate(const uint8_t* codec_private, uint64_t length);

  void set_codec_id(const char* codec_id);
  const char* codec_id() const { return codec_id_; }
  const uint8_t* codec_private() const { return codec_private_; }
  void set_language(const char* language);
  const char* language() const { return language_; }
  void set_max_block_additional_id(uint64_t max_block_additional_id) {
    max_block_additional_id_ = max_block_additional_id;
  }
  uint64_t max_block_additional_id() const { return max_block_additional_id_; }
  void set_name(const char* name);
  const char* name() const { return name_; }
  void set_number(uint64_t number) { number_ = number; }
  uint64_t number() const { return number_; }
  void set_type(uint64_t type) { type_ = type; }
  uint64_t type() const { return type_; }
  void set_uid(uint64_t uid) { uid_ = uid; }
  uint64_t uid() const { return uid_; }
  void set_codec_delay(uint64_t codec_delay) { codec_delay_ = codec_delay; }
  uint64_t codec_delay() const { return codec_delay_; }
  void set_seek_pre_roll(uint64_t seek_pre_roll) {
    seek_pre_roll_ = seek_pre_roll;
  }
  uint64_t seek_pre_roll() const { return seek_pre_roll_; }
  void set_default_duration(uint64_t default_duration) {
    default_duration_ = default_duration;
  }
  uint64_t default_duration() const { return default_duration_; }

  uint64_t codec_private_length() const { return codec_private_length_; }
  uint32_t content_encoding_entries_size() const {
    return content_encoding_entries_size_;
  }

 private:

  char* codec_id_;
  uint8_t* codec_private_;
  char* language_;
  uint64_t max_block_additional_id_;
  char* name_;
  uint64_t number_;
  uint64_t type_;
  uint64_t uid_;
  uint64_t codec_delay_;
  uint64_t seek_pre_roll_;
  uint64_t default_duration_;

  uint64_t codec_private_length_;

  ContentEncoding** content_encoding_entries_;

  uint32_t content_encoding_entries_size_;

  LIBWEBM_DISALLOW_COPY_AND_ASSIGN(Track);
};

// Track that has video specific elements.
class VideoTrack : public Track {
 public:

  enum StereoMode {
    kMono = 0,
    kSideBySideLeftIsFirst = 1,
    kTopBottomRightIsFirst = 2,
    kTopBottomLeftIsFirst = 3,
    kSideBySideRightIsFirst = 11
  };

  enum AlphaMode { kNoAlpha = 0, kAlpha = 1 };

  explicit VideoTrack(unsigned int* seed);
  virtual ~VideoTrack();


  virtual uint64_t PayloadSize() const;

  virtual bool Write(IMkvWriter* writer) const;

  bool SetStereoMode(uint64_t stereo_mode);

  bool SetAlphaMode(uint64_t alpha_mode);

  void set_display_height(uint64_t height) { display_height_ = height; }
  uint64_t display_height() const { return display_height_; }
  void set_display_width(uint64_t width) { display_width_ = width; }
  uint64_t display_width() const { return display_width_; }
  void set_pixel_height(uint64_t height) { pixel_height_ = height; }
  uint64_t pixel_height() const { return pixel_height_; }
  void set_pixel_width(uint64_t width) { pixel_width_ = width; }
  uint64_t pixel_width() const { return pixel_width_; }

  void set_crop_left(uint64_t crop_left) { crop_left_ = crop_left; }
  uint64_t crop_left() const { return crop_left_; }
  void set_crop_right(uint64_t crop_right) { crop_right_ = crop_right; }
  uint64_t crop_right() const { return crop_right_; }
  void set_crop_top(uint64_t crop_top) { crop_top_ = crop_top; }
  uint64_t crop_top() const { return crop_top_; }
  void set_crop_bottom(uint64_t crop_bottom) { crop_bottom_ = crop_bottom; }
  uint64_t crop_bottom() const { return crop_bottom_; }

  void set_frame_rate(double frame_rate) { frame_rate_ = frame_rate; }
  double frame_rate() const { return frame_rate_; }
  void set_height(uint64_t height) { height_ = height; }
  uint64_t height() const { return height_; }
  uint64_t stereo_mode() { return stereo_mode_; }
  uint64_t alpha_mode() { return alpha_mode_; }
  void set_width(uint64_t width) { width_ = width; }
  uint64_t width() const { return width_; }
  void set_colour_space(const char* colour_space);
  const char* colour_space() const { return colour_space_; }

  Colour* colour() { return colour_; }

  bool SetColour(const Colour& colour);

  Projection* projection() { return projection_; }

  bool SetProjection(const Projection& projection);

 private:

  uint64_t VideoPayloadSize() const;

  uint64_t display_height_;
  uint64_t display_width_;
  uint64_t pixel_height_;
  uint64_t pixel_width_;
  uint64_t crop_left_;
  uint64_t crop_right_;
  uint64_t crop_top_;
  uint64_t crop_bottom_;
  double frame_rate_;
  uint64_t height_;
  uint64_t stereo_mode_;
  uint64_t alpha_mode_;
  uint64_t width_;
  char* colour_space_;

  Colour* colour_;
  Projection* projection_;

  LIBWEBM_DISALLOW_COPY_AND_ASSIGN(VideoTrack);
};

// Track that has audio specific elements.
class AudioTrack : public Track {
 public:

  explicit AudioTrack(unsigned int* seed);
  virtual ~AudioTrack();


  virtual uint64_t PayloadSize() const;

  virtual bool Write(IMkvWriter* writer) const;

  void set_bit_depth(uint64_t bit_depth) { bit_depth_ = bit_depth; }
  uint64_t bit_depth() const { return bit_depth_; }
  void set_channels(uint64_t channels) { channels_ = channels; }
  uint64_t channels() const { return channels_; }
  void set_sample_rate(double sample_rate) { sample_rate_ = sample_rate; }
  double sample_rate() const { return sample_rate_; }

 private:

  uint64_t bit_depth_;
  uint64_t channels_;
  double sample_rate_;

  LIBWEBM_DISALLOW_COPY_AND_ASSIGN(AudioTrack);
};

// Tracks element
class Tracks {
 public:

  enum { kVideo = 0x1, kAudio = 0x2 };

  static const char kOpusCodecId[];
  static const char kVorbisCodecId[];
  static const char kAv1CodecId[];
  static const char kVp8CodecId[];
  static const char kVp9CodecId[];
  static const char kWebVttCaptionsId[];
  static const char kWebVttDescriptionsId[];
  static const char kWebVttMetadataId[];
  static const char kWebVttSubtitlesId[];

  Tracks();
  ~Tracks();




  bool AddTrack(Track* track, int32_t number);

  const Track* GetTrackByIndex(uint32_t idx) const;


  Track* GetTrackByNumber(uint64_t track_number) const;

  bool TrackIsAudio(uint64_t track_number) const;

  bool TrackIsVideo(uint64_t track_number) const;

  bool Write(IMkvWriter* writer) const;

  uint32_t track_entries_size() const { return track_entries_size_; }

 private:

  Track** track_entries_;

  uint32_t track_entries_size_;

  mutable bool wrote_tracks_;

  LIBWEBM_DISALLOW_COPY_AND_ASSIGN(Tracks);
};

// Chapter element
//
class Chapter {
 public:




  bool set_id(const char* id);


  void set_time(const Segment& segment, uint64_t start_time_ns,
                uint64_t end_time_ns);


  void set_uid(const uint64_t uid) { uid_ = uid; }
















  bool add_string(const char* title, const char* language, const char* country);

 private:
  friend class Chapters;

  class Display {
   public:

    void Init();

    void Clear();


    bool set_title(const char* title);


    bool set_language(const char* language);


    bool set_country(const char* country);



    uint64_t WriteDisplay(IMkvWriter* writer) const;

   private:
    char* title_;
    char* language_;
    char* country_;
  };

  Chapter();
  ~Chapter();



  void Init(unsigned int* seed);


  void ShallowCopy(Chapter* dst) const;


  void Clear();




  bool ExpandDisplaysArray();



  uint64_t WriteAtom(IMkvWriter* writer) const;


  char* id_;

  uint64_t start_timecode_;

  uint64_t end_timecode_;

  uint64_t uid_;


  Display* displays_;

  int displays_size_;


  int displays_count_;

  LIBWEBM_DISALLOW_COPY_AND_ASSIGN(Chapter);
};

// Chapters element
//
class Chapters {
 public:
  Chapters();
  ~Chapters();

  Chapter* AddChapter(unsigned int* seed);

  int Count() const;

  bool Write(IMkvWriter* writer) const;

 private:


  bool ExpandChaptersArray();



  uint64_t WriteEdition(IMkvWriter* writer) const;

  int chapters_size_;

  int chapters_count_;

  Chapter* chapters_;

  LIBWEBM_DISALLOW_COPY_AND_ASSIGN(Chapters);
};

// Tag element
//
class Tag {
 public:
  bool add_simple_tag(const char* tag_name, const char* tag_string);

 private:

  friend class Tags;

  class SimpleTag {
   public:

    void Init();

    void Clear();


    bool set_tag_name(const char* tag_name);


    bool set_tag_string(const char* tag_string);



    uint64_t Write(IMkvWriter* writer) const;

   private:
    char* tag_name_;
    char* tag_string_;
  };

  Tag();
  ~Tag();


  void ShallowCopy(Tag* dst) const;


  void Clear();




  bool ExpandSimpleTagsArray();



  uint64_t Write(IMkvWriter* writer) const;

  SimpleTag* simple_tags_;

  int simple_tags_size_;


  int simple_tags_count_;

  LIBWEBM_DISALLOW_COPY_AND_ASSIGN(Tag);
};

// Tags element
//
class Tags {
 public:
  Tags();
  ~Tags();

  Tag* AddTag();

  int Count() const;

  bool Write(IMkvWriter* writer) const;

 private:


  bool ExpandTagsArray();

  int tags_size_;

  int tags_count_;

  Tag* tags_;

  LIBWEBM_DISALLOW_COPY_AND_ASSIGN(Tags);
};

// Cluster element
//
// Notes:
//  |Init| must be called before any other method in this class.
class Cluster {
 public:



  Cluster(uint64_t timecode, int64_t cues_pos, uint64_t timecode_scale,
          bool write_last_frame_with_duration = false,
          bool fixed_size_timecode = false);
  ~Cluster();

  bool Init(IMkvWriter* ptr_writer);


  bool AddFrame(const Frame* frame);










  bool AddFrame(const uint8_t* data, uint64_t length, uint64_t track_number,
                uint64_t timecode,  // timecode units (absolute)
                bool is_key);













  bool AddFrameWithAdditional(const uint8_t* data, uint64_t length,
                              const uint8_t* additional,
                              uint64_t additional_length, uint64_t add_id,
                              uint64_t track_number, uint64_t abs_timecode,
                              bool is_key);











  bool AddFrameWithDiscardPadding(const uint8_t* data, uint64_t length,
                                  int64_t discard_padding,
                                  uint64_t track_number, uint64_t abs_timecode,
                                  bool is_key);














  bool AddMetadata(const uint8_t* data, uint64_t length, uint64_t track_number,
                   uint64_t timecode, uint64_t duration);

  void AddPayloadSize(uint64_t size);




  bool Finalize();









  bool Finalize(bool set_last_frame_duration, uint64_t duration);

  uint64_t Size() const;


  int64_t GetRelativeTimecode(int64_t abs_timecode) const;

  int64_t size_position() const { return size_position_; }
  int32_t blocks_added() const { return blocks_added_; }
  uint64_t payload_size() const { return payload_size_; }
  int64_t position_for_cues() const { return position_for_cues_; }
  uint64_t timecode() const { return timecode_; }
  uint64_t timecode_scale() const { return timecode_scale_; }
  void set_write_last_frame_with_duration(bool write_last_frame_with_duration) {
    write_last_frame_with_duration_ = write_last_frame_with_duration;
  }
  bool write_last_frame_with_duration() const {
    return write_last_frame_with_duration_;
  }

 private:

  typedef std::map<uint64_t, std::list<Frame*> >::iterator FrameMapIterator;



  bool PreWriteBlock();


  void PostWriteBlock(uint64_t element_size);

  bool DoWriteFrame(const Frame* const frame);


  bool QueueOrWriteFrame(const Frame* const frame);

  bool WriteClusterHeader();

  int32_t blocks_added_;

  bool finalized_;


  bool fixed_size_timecode_;

  bool header_written_;

  uint64_t payload_size_;

  const int64_t position_for_cues_;

  int64_t size_position_;

  const uint64_t timecode_;

  const uint64_t timecode_scale_;




  bool write_last_frame_with_duration_;

  std::map<uint64_t, std::list<Frame*> > stored_frames_;


  std::map<uint64_t, uint64_t> last_block_timestamp_;

  IMkvWriter* writer_;

  LIBWEBM_DISALLOW_COPY_AND_ASSIGN(Cluster);
};

// SeekHead element
class SeekHead {
 public:
  SeekHead();
  ~SeekHead();






  bool AddSeekEntry(uint32_t id, uint64_t pos);

  bool Finalize(IMkvWriter* writer) const;


  uint32_t GetId(int index) const;


  uint64_t GetPosition(int index) const;


  bool SetSeekEntry(int index, uint32_t id, uint64_t position);


  bool Write(IMkvWriter* writer);

  const static int32_t kSeekEntryCount = 5;

 private:

  uint64_t MaxEntrySize() const;

  uint32_t seek_entry_id_[kSeekEntryCount];

  uint64_t seek_entry_pos_[kSeekEntryCount];

  int64_t start_pos_;

  LIBWEBM_DISALLOW_COPY_AND_ASSIGN(SeekHead);
};

// Segment Information element
class SegmentInfo {
 public:
  SegmentInfo();
  ~SegmentInfo();

  bool Finalize(IMkvWriter* writer) const;

  bool Init();


  bool Write(IMkvWriter* writer);

  void set_duration(double duration) { duration_ = duration; }
  double duration() const { return duration_; }
  void set_muxing_app(const char* app);
  const char* muxing_app() const { return muxing_app_; }
  void set_timecode_scale(uint64_t scale) { timecode_scale_ = scale; }
  uint64_t timecode_scale() const { return timecode_scale_; }
  void set_writing_app(const char* app);
  const char* writing_app() const { return writing_app_; }
  void set_date_utc(int64_t date_utc) { date_utc_ = date_utc; }
  int64_t date_utc() const { return date_utc_; }

 private:



  double duration_;

  char* muxing_app_;
  uint64_t timecode_scale_;

  char* writing_app_;

  int64_t date_utc_;

  int64_t duration_pos_;

  LIBWEBM_DISALLOW_COPY_AND_ASSIGN(SegmentInfo);
};

// This class represents the main segment in a WebM file. Currently only
// supports one Segment element.
//
// Notes:
//  |Init| must be called before any other method in this class.
class Segment {
 public:
  enum Mode { kLive = 0x1, kFile = 0x2 };

  enum CuesPosition {
    kAfterClusters = 0x0,  // Position Cues after Clusters - Default
    kBeforeClusters = 0x1  // Position Cues before Clusters
  };

  static const uint32_t kDefaultDocTypeVersion = 4;
  static const uint64_t kDefaultMaxClusterDuration = 30000000000ULL;

  Segment();
  ~Segment();


  bool Init(IMkvWriter* ptr_writer);





  Track* AddTrack(int32_t number);




  uint64_t AddAudioTrack(int32_t sample_rate, int32_t channels, int32_t number);



  Chapter* AddChapter();



  Tag* AddTag();




  bool AddCuePoint(uint64_t timestamp, uint64_t track);








  bool AddFrame(const uint8_t* data, uint64_t length, uint64_t track_number,
                uint64_t timestamp_ns, bool is_key);














  bool AddMetadata(const uint8_t* data, uint64_t length, uint64_t track_number,
                   uint64_t timestamp_ns, uint64_t duration_ns);













  bool AddFrameWithAdditional(const uint8_t* data, uint64_t length,
                              const uint8_t* additional,
                              uint64_t additional_length, uint64_t add_id,
                              uint64_t track_number, uint64_t timestamp,
                              bool is_key);











  bool AddFrameWithDiscardPadding(const uint8_t* data, uint64_t length,
                                  int64_t discard_padding,
                                  uint64_t track_number, uint64_t timestamp,
                                  bool is_key);




  bool AddGenericFrame(const Frame* frame);




  uint64_t AddVideoTrack(int32_t width, int32_t height, int32_t number);











  bool CopyAndMoveCuesBeforeClusters(mkvparser::IMkvReader* reader,
                                     IMkvWriter* writer);



  bool CuesTrack(uint64_t track_number);


  void ForceNewClusterOnNextFrame();



  bool Finalize();

  Cues* GetCues() { return &cues_; }

  const SegmentInfo* GetSegmentInfo() const { return &segment_info_; }
  SegmentInfo* GetSegmentInfo() { return &segment_info_; }


  Track* GetTrackByNumber(uint64_t track_number) const;

  void OutputCues(bool output_cues);

  void AccurateClusterDuration(bool accurate_cluster_duration);

  void UseFixedSizeClusterTimecode(bool fixed_size_cluster_timecode);









  bool SetChunking(bool chunking, const char* filename);

  bool chunking() const { return chunking_; }
  uint64_t cues_track() const { return cues_track_; }
  void set_max_cluster_duration(uint64_t max_cluster_duration) {
    max_cluster_duration_ = max_cluster_duration;
  }
  uint64_t max_cluster_duration() const { return max_cluster_duration_; }
  void set_max_cluster_size(uint64_t max_cluster_size) {
    max_cluster_size_ = max_cluster_size;
  }
  uint64_t max_cluster_size() const { return max_cluster_size_; }
  void set_mode(Mode mode) { mode_ = mode; }
  Mode mode() const { return mode_; }
  CuesPosition cues_position() const { return cues_position_; }
  bool output_cues() const { return output_cues_; }
  void set_estimate_file_duration(bool estimate_duration) {
    estimate_file_duration_ = estimate_duration;
  }
  bool estimate_file_duration() const { return estimate_file_duration_; }
  const SegmentInfo* segment_info() const { return &segment_info_; }
  void set_duration(double duration) { duration_ = duration; }
  double duration() const { return duration_; }

  bool DocTypeIsWebm() const;

 private:



  bool CheckHeaderInfo();

  void UpdateDocTypeVersion();



  bool UpdateChunkName(const char* ext, char** name) const;



  int64_t MaxOffset();

  bool QueueFrame(Frame* frame);


  int WriteFramesAll();



  bool WriteFramesLessThan(uint64_t timestamp);


  bool WriteSegmentHeader();








  int TestFrame(uint64_t track_num, uint64_t timestamp_ns, bool key) const;


  bool MakeNewCluster(uint64_t timestamp_ns);



  bool DoNewClusterProcessing(uint64_t track_num, uint64_t timestamp_ns,
                              bool key);


  void MoveCuesBeforeClusters();








  void MoveCuesBeforeClustersHelper(uint64_t diff, int index,
                                    uint64_t* cue_size);

  unsigned int seed_;

  Cues cues_;
  SeekHead seek_head_;
  SegmentInfo segment_info_;
  Tracks tracks_;
  Chapters chapters_;
  Tags tags_;

  int chunk_count_;

  char* chunk_name_;


  MkvWriter* chunk_writer_cluster_;


  MkvWriter* chunk_writer_cues_;


  MkvWriter* chunk_writer_header_;


  bool chunking_;

  char* chunking_base_name_;

  int64_t cluster_end_offset_;

  Cluster** cluster_list_;

  int32_t cluster_list_capacity_;

  int32_t cluster_list_size_;

  CuesPosition cues_position_;

  uint64_t cues_track_;

  bool force_new_cluster_;




  Frame** frames_;

  int32_t frames_capacity_;

  int32_t frames_size_;

  bool has_video_;

  bool header_written_;

  uint64_t last_block_duration_;

  uint64_t last_timestamp_;

  uint64_t last_track_timestamp_[kMaxTrackNumber];

  uint64_t track_frames_written_[kMaxTrackNumber];



  uint64_t max_cluster_duration_;



  uint64_t max_cluster_size_;


  Mode mode_;

  bool new_cuepoint_;


  bool output_cues_;


  bool accurate_cluster_duration_;

  bool fixed_size_cluster_timecode_;

  bool estimate_file_duration_;


  int32_t ebml_header_size_;

  int64_t payload_pos_;

  int64_t size_position_;




  uint32_t doc_type_version_;
  uint32_t doc_type_version_written_;

  double duration_;

  IMkvWriter* writer_cluster_;
  IMkvWriter* writer_cues_;
  IMkvWriter* writer_header_;

  LIBWEBM_DISALLOW_COPY_AND_ASSIGN(Segment);
};

}  // namespace mkvmuxer

#endif  // MKVMUXER_MKVMUXER_H_
