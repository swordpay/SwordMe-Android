/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "common_audio/vad/vad_filterbank.h"

#include "rtc_base/checks.h"
#include "common_audio/signal_processing/include/signal_processing_library.h"

static const int16_t kLogConst = 24660;  // 160*log10(2) in Q9.
static const int16_t kLogEnergyIntPart = 14336;  // 14 in Q10

static const int16_t kHpZeroCoefs[3] = { 6631, -13262, 6631 };
static const int16_t kHpPoleCoefs[3] = { 16384, -7756, 5620 };

// Upper: 0.64, Lower: 0.17
static const int16_t kAllPassCoefsQ15[2] = { 20972, 5571 };

static const int16_t kOffsetVector[6] = { 368, 368, 272, 176, 176, 176 };

// sampled at 500 Hz.
//
// - data_in      [i]   : Input audio data sampled at 500 Hz.
// - data_length  [i]   : Length of input and output data.
// - filter_state [i/o] : State of the filter.
// - data_out     [o]   : Output audio data in the frequency interval
//                        80 - 250 Hz.
static void HighPassFilter(const int16_t* data_in, size_t data_length,
                           int16_t* filter_state, int16_t* data_out) {
  size_t i;
  const int16_t* in_ptr = data_in;
  int16_t* out_ptr = data_out;
  int32_t tmp32 = 0;








  for (i = 0; i < data_length; i++) {

    tmp32 = kHpZeroCoefs[0] * *in_ptr;
    tmp32 += kHpZeroCoefs[1] * filter_state[0];
    tmp32 += kHpZeroCoefs[2] * filter_state[1];
    filter_state[1] = filter_state[0];
    filter_state[0] = *in_ptr++;

    tmp32 -= kHpPoleCoefs[1] * filter_state[2];
    tmp32 -= kHpPoleCoefs[2] * filter_state[3];
    filter_state[3] = filter_state[2];
    filter_state[2] = (int16_t) (tmp32 >> 14);
    *out_ptr++ = filter_state[2];
  }
}

// frequency bands (low pass vs high pass).
// Note that `data_in` and `data_out` can NOT correspond to the same address.
//
// - data_in            [i]   : Input audio signal given in Q0.
// - data_length        [i]   : Length of input and output data.
// - filter_coefficient [i]   : Given in Q15.
// - filter_state       [i/o] : State of the filter given in Q(-1).
// - data_out           [o]   : Output audio signal given in Q(-1).
static void AllPassFilter(const int16_t* data_in, size_t data_length,
                          int16_t filter_coefficient, int16_t* filter_state,
                          int16_t* data_out) {






  size_t i;
  int16_t tmp16 = 0;
  int32_t tmp32 = 0;
  int32_t state32 = ((int32_t) (*filter_state) * (1 << 16));  // Q15

  for (i = 0; i < data_length; i++) {
    tmp32 = state32 + filter_coefficient * *data_in;
    tmp16 = (int16_t) (tmp32 >> 16);  // Q(-1)
    *data_out++ = tmp16;
    state32 = (*data_in * (1 << 14)) - filter_coefficient * tmp16;  // Q14
    state32 *= 2;  // Q15.
    data_in += 2;
  }

  *filter_state = (int16_t) (state32 >> 16);  // Q(-1)
}

// an upper (high pass) part and a lower (low pass) part respectively.
//
// - data_in      [i]   : Input audio data to be split into two frequency bands.
// - data_length  [i]   : Length of `data_in`.
// - upper_state  [i/o] : State of the upper filter, given in Q(-1).
// - lower_state  [i/o] : State of the lower filter, given in Q(-1).
// - hp_data_out  [o]   : Output audio data of the upper half of the spectrum.
//                        The length is `data_length` / 2.
// - lp_data_out  [o]   : Output audio data of the lower half of the spectrum.
//                        The length is `data_length` / 2.
static void SplitFilter(const int16_t* data_in, size_t data_length,
                        int16_t* upper_state, int16_t* lower_state,
                        int16_t* hp_data_out, int16_t* lp_data_out) {
  size_t i;
  size_t half_length = data_length >> 1;  // Downsampling by 2.
  int16_t tmp_out;

  AllPassFilter(&data_in[0], half_length, kAllPassCoefsQ15[0], upper_state,
                hp_data_out);

  AllPassFilter(&data_in[1], half_length, kAllPassCoefsQ15[1], lower_state,
                lp_data_out);

  for (i = 0; i < half_length; i++) {
    tmp_out = *hp_data_out;
    *hp_data_out++ -= *lp_data_out;
    *lp_data_out++ += tmp_out;
  }
}

// `total_energy` if necessary.
//
// - data_in      [i]   : Input audio data for energy calculation.
// - data_length  [i]   : Length of input data.
// - offset       [i]   : Offset value added to `log_energy`.
// - total_energy [i/o] : An external energy updated with the energy of
//                        `data_in`.
//                        NOTE: `total_energy` is only updated if
//                        `total_energy` <= `kMinEnergy`.
// - log_energy   [o]   : 10 * log10("energy of `data_in`") given in Q4.
static void LogOfEnergy(const int16_t* data_in, size_t data_length,
                        int16_t offset, int16_t* total_energy,
                        int16_t* log_energy) {

  int tot_rshifts = 0;


  uint32_t energy = 0;

  RTC_DCHECK(data_in);
  RTC_DCHECK_GT(data_length, 0);

  energy = (uint32_t) WebRtcSpl_Energy((int16_t*) data_in, data_length,
                                       &tot_rshifts);

  if (energy != 0) {


    int normalizing_rshifts = 17 - WebRtcSpl_NormU32(energy);



    int16_t log2_energy = kLogEnergyIntPart;

    tot_rshifts += normalizing_rshifts;




    if (normalizing_rshifts < 0) {
      energy <<= -normalizing_rshifts;
    } else {
      energy >>= normalizing_rshifts;
    }




















    log2_energy += (int16_t) ((energy & 0x00003FFF) >> 4);


    *log_energy = (int16_t)(((kLogConst * log2_energy) >> 19) +
        ((tot_rshifts * kLogConst) >> 9));

    if (*log_energy < 0) {
      *log_energy = 0;
    }
  } else {
    *log_energy = offset;
    return;
  }

  *log_energy += offset;



  if (*total_energy <= kMinEnergy) {
    if (tot_rshifts >= 0) {


      *total_energy += kMinEnergy + 1;
    } else {




      *total_energy += (int16_t) (energy >> -tot_rshifts);  // Q0.
    }
  }
}

int16_t WebRtcVad_CalculateFeatures(VadInstT* self, const int16_t* data_in,
                                    size_t data_length, int16_t* features) {
  int16_t total_energy = 0;




  int16_t hp_120[120], lp_120[120];
  int16_t hp_60[60], lp_60[60];
  const size_t half_data_length = data_length >> 1;
  size_t length = half_data_length;  // `data_length` / 2, corresponds to


  int frequency_band = 0;
  const int16_t* in_ptr = data_in;  // [0 - 4000] Hz.
  int16_t* hp_out_ptr = hp_120;  // [2000 - 4000] Hz.
  int16_t* lp_out_ptr = lp_120;  // [0 - 2000] Hz.

  RTC_DCHECK_LE(data_length, 240);
  RTC_DCHECK_LT(4, kNumChannels - 1);  // Checking maximum `frequency_band`.

  SplitFilter(in_ptr, data_length, &self->upper_state[frequency_band],
              &self->lower_state[frequency_band], hp_out_ptr, lp_out_ptr);

  frequency_band = 1;
  in_ptr = hp_120;  // [2000 - 4000] Hz.
  hp_out_ptr = hp_60;  // [3000 - 4000] Hz.
  lp_out_ptr = lp_60;  // [2000 - 3000] Hz.
  SplitFilter(in_ptr, length, &self->upper_state[frequency_band],
              &self->lower_state[frequency_band], hp_out_ptr, lp_out_ptr);

  length >>= 1;  // `data_length` / 4 <=> bandwidth = 1000 Hz.

  LogOfEnergy(hp_60, length, kOffsetVector[5], &total_energy, &features[5]);

  LogOfEnergy(lp_60, length, kOffsetVector[4], &total_energy, &features[4]);

  frequency_band = 2;
  in_ptr = lp_120;  // [0 - 2000] Hz.
  hp_out_ptr = hp_60;  // [1000 - 2000] Hz.
  lp_out_ptr = lp_60;  // [0 - 1000] Hz.
  length = half_data_length;  // `data_length` / 2 <=> bandwidth = 2000 Hz.
  SplitFilter(in_ptr, length, &self->upper_state[frequency_band],
              &self->lower_state[frequency_band], hp_out_ptr, lp_out_ptr);

  length >>= 1;  // `data_length` / 4 <=> bandwidth = 1000 Hz.
  LogOfEnergy(hp_60, length, kOffsetVector[3], &total_energy, &features[3]);

  frequency_band = 3;
  in_ptr = lp_60;  // [0 - 1000] Hz.
  hp_out_ptr = hp_120;  // [500 - 1000] Hz.
  lp_out_ptr = lp_120;  // [0 - 500] Hz.
  SplitFilter(in_ptr, length, &self->upper_state[frequency_band],
              &self->lower_state[frequency_band], hp_out_ptr, lp_out_ptr);

  length >>= 1;  // `data_length` / 8 <=> bandwidth = 500 Hz.
  LogOfEnergy(hp_120, length, kOffsetVector[2], &total_energy, &features[2]);

  frequency_band = 4;
  in_ptr = lp_120;  // [0 - 500] Hz.
  hp_out_ptr = hp_60;  // [250 - 500] Hz.
  lp_out_ptr = lp_60;  // [0 - 250] Hz.
  SplitFilter(in_ptr, length, &self->upper_state[frequency_band],
              &self->lower_state[frequency_band], hp_out_ptr, lp_out_ptr);

  length >>= 1;  // `data_length` / 16 <=> bandwidth = 250 Hz.
  LogOfEnergy(hp_60, length, kOffsetVector[1], &total_energy, &features[1]);

  HighPassFilter(lp_60, length, self->hp_filter_state, hp_120);

  LogOfEnergy(hp_120, length, kOffsetVector[0], &total_energy, &features[0]);

  return total_energy;
}
