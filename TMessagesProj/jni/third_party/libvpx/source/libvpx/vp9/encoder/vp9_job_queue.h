/*
 *  Copyright (c) 2017 The WebM project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef VPX_VP9_ENCODER_VP9_JOB_QUEUE_H_
#define VPX_VP9_ENCODER_VP9_JOB_QUEUE_H_

typedef enum {
  FIRST_PASS_JOB,
  ENCODE_JOB,
  ARNR_JOB,
  NUM_JOB_TYPES,
} JOB_TYPE;

typedef struct {
  int vert_unit_row_num;  // Index of the vertical unit row
  int tile_col_id;        // tile col id within a tile
  int tile_row_id;        // tile col id within a tile
} JobNode;

typedef struct {

  void *next;

  JobNode job_info;
} JobQueue;

typedef struct {

  void *next;

  int num_jobs_acquired;
} JobQueueHandle;

#endif  // VPX_VP9_ENCODER_VP9_JOB_QUEUE_H_
