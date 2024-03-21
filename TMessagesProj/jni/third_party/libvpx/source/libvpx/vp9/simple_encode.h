/*
 *  Copyright (c) 2019 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VP9_SIMPLE_ENCODE_H_
#define VPX_VP9_SIMPLE_ENCODE_H_

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>

namespace vp9 {

enum StatusCode {
  StatusOk = 0,
  StatusError,
};

enum FrameType {
  kFrameTypeKey = 0,
  kFrameTypeInter = 1,
  kFrameTypeAltRef = 2,
  kFrameTypeOverlay = 3,
  kFrameTypeGolden = 4,
};

// This enum numbers have to be contiguous and start from zero except
// kNoneRefFrame.
enum RefFrameType {
  kRefFrameTypeLast = 0,
  kRefFrameTypePast = 1,
  kRefFrameTypeFuture = 2,
  kRefFrameTypeMax = 3,
  kRefFrameTypeNone = -1,
};

enum GopMapFlag {
  kGopMapFlagStart =
      1 << 0,  // Indicate this location is the start of a group of pictures.
  kGopMapFlagUseAltRef =
      1 << 1,  // Indicate this group of pictures will use an alt ref. Only set

};

// This structure contains the information of each 4x4 block.
struct PartitionInfo {
  int row;           // row pixel offset of current 4x4 block
  int column;        // column pixel offset of current 4x4 block
  int row_start;     // row pixel offset of the start of the prediction block
  int column_start;  // column pixel offset of the start of the prediction block
  int width;         // prediction block width
  int height;        // prediction block height
};

constexpr int kMotionVectorSubPixelPrecision = 8;
constexpr int kMotionVectorFullPixelPrecision = 1;

// This structure contains the information of each 16x16 block.
// In the second pass. The frame is split to 4x4 blocks.
// This structure contains the information of each 4x4 block.
struct MotionVectorInfo {


  int mv_count;




  RefFrameType ref_frame[2];


  double mv_row[2];


  double mv_column[2];
};

// For each frame, the tpl stats are computed per 32x32 block.
struct TplStatsInfo {


  int64_t intra_cost;

  int64_t inter_cost;


  int64_t mc_flow;


  int64_t mc_dep_cost;

  int64_t mc_ref_cost;
};

struct RefFrameInfo {
  int coding_indexes[kRefFrameTypeMax];










  int valid_list[kRefFrameTypeMax];
};

bool operator==(const RefFrameInfo &a, const RefFrameInfo &b);

struct EncodeFrameInfo {
  int show_idx;



  int coding_index;
  RefFrameInfo ref_frame_info;
  FrameType frame_type;
};

struct NewMotionvectorComponentCounts {
  std::vector<unsigned int> sign;
  std::vector<unsigned int> classes;
  std::vector<unsigned int> class0;
  std::vector<std::vector<unsigned int>> bits;
  std::vector<std::vector<unsigned int>> class0_fp;
  std::vector<unsigned int> fp;
  std::vector<unsigned int> class0_hp;
  std::vector<unsigned int> hp;
};

struct NewMotionVectorContextCounts {
  std::vector<unsigned int> joints;
  std::vector<NewMotionvectorComponentCounts> comps;
};

using UintArray2D = std::vector<std::vector<unsigned int>>;
using UintArray3D = std::vector<std::vector<std::vector<unsigned int>>>;
using UintArray5D = std::vector<
    std::vector<std::vector<std::vector<std::vector<unsigned int>>>>>;
using UintArray6D = std::vector<std::vector<
    std::vector<std::vector<std::vector<std::vector<unsigned int>>>>>>;

struct TransformSizeCounts {



  UintArray2D p32x32;



  UintArray2D p16x16;



  UintArray2D p8x8;

  std::vector<unsigned int> tx_totals;
};

struct FrameCounts {


  UintArray2D y_mode;


  UintArray2D uv_mode;


  UintArray2D partition;

  UintArray6D coef;

  UintArray5D eob_branch;


  UintArray2D switchable_interp;



  UintArray2D inter_mode;


  UintArray2D intra_inter;



  UintArray2D comp_inter;



  UintArray3D single_ref;


  UintArray2D comp_ref;


  UintArray2D skip;

  TransformSizeCounts tx;

  NewMotionVectorContextCounts mv;
};

struct ImageBuffer {



  std::unique_ptr<unsigned char[]> plane_buffer[3];
  int plane_width[3];
  int plane_height[3];
};

void output_image_buffer(const ImageBuffer &image_buffer, std::FILE *out_file);

struct EncodeFrameResult {
  int show_idx;
  FrameType frame_type;
  int coding_idx;
  RefFrameInfo ref_frame_info;
  size_t coding_data_bit_size;
  size_t coding_data_byte_size;


  std::unique_ptr<unsigned char[]> coding_data;
  double psnr;
  uint64_t sse;
  int quantize_index;
  FrameCounts frame_counts;
  int num_rows_4x4;  // number of row units, in size of 4.
  int num_cols_4x4;  // number of column units, in size of 4.



















  std::vector<PartitionInfo> partition_info;









  std::vector<MotionVectorInfo> motion_vector_info;











  std::vector<TplStatsInfo> tpl_stats_info;
  ImageBuffer coded_frame;


  int recode_count;
  std::vector<int> q_index_history;
  std::vector<int> rate_history;
};

struct GroupOfPicture {





  std::vector<EncodeFrameInfo> encode_frame_list;







  int next_encode_frame_index;

  int show_frame_count;


  int start_show_index;

  int start_coding_index;

  int first_is_key_frame;

  int use_alt_ref;

  int last_gop_use_alt_ref;
};

class SimpleEncode {
 public:


  SimpleEncode(int frame_width, int frame_height, int frame_rate_num,
               int frame_rate_den, int target_bitrate, int num_frames,
               const char *infile_path, const char *outfile_path = nullptr);
  ~SimpleEncode();
  SimpleEncode(SimpleEncode &) = delete;
  SimpleEncode &operator=(const SimpleEncode &) = delete;







  void SetEncodeSpeed(int encode_speed);



















  StatusCode SetEncodeConfig(const char *name, const char *value);



  StatusCode DumpEncodeConfigs(int pass, FILE *fp);



  void ComputeFirstPassStats();




  std::vector<std::vector<double>> ObserveFirstPassStats();




  std::vector<std::vector<MotionVectorInfo>> ObserveFirstPassMotionVectors();




  std::vector<int> ObserveKeyFrameMap() const;














  void SetExternalGroupOfPicturesMap(int *gop_map, int gop_map_size);



  std::vector<int> ObserveExternalGroupOfPicturesMap();


  void StartEncode();


  void EndEncode();




  int GetKeyFrameGroupSize() const;


  GroupOfPicture ObserveGroupOfPicture() const;


  EncodeFrameInfo GetNextEncodeFrameInfo() const;


  void EncodeFrame(EncodeFrameResult *encode_frame_result);


  void EncodeFrameWithQuantizeIndex(EncodeFrameResult *encode_frame_result,
                                    int quantize_index);







  void EncodeFrameWithTargetFrameBits(EncodeFrameResult *encode_frame_result,
                                      int target_frame_bits,
                                      double percent_diff);



  int GetCodingFrameNum() const;

  uint64_t GetFramePixelCount() const;

 private:




  std::vector<int> ComputeKeyFrameMap() const;


  void UpdateKeyFrameGroup(int key_frame_show_index);

  void PostUpdateKeyFrameGroupIndex(FrameType frame_type);

  void PostUpdateState(const EncodeFrameResult &encode_frame_result);

  class EncodeImpl;

  int frame_width_;   // frame width in pixels.
  int frame_height_;  // frame height in pixels.
  int frame_rate_num_;
  int frame_rate_den_;
  int target_bitrate_;
  int num_frames_;
  int encode_speed_;

  std::FILE *in_file_;
  std::FILE *out_file_;
  std::unique_ptr<EncodeImpl> impl_ptr_;

  std::vector<int> key_frame_map_;
  std::vector<int> gop_map_;
  GroupOfPicture group_of_picture_;




  int key_frame_group_size_;

  int key_frame_group_index_;



  int frame_coding_index_;

  int show_frame_count_;



  RefFrameInfo ref_frame_info_;









  std::vector<std::vector<MotionVectorInfo>> fp_motion_vector_info_;
};

}  // namespace vp9

#endif  // VPX_VP9_SIMPLE_ENCODE_H_
