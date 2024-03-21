/*
 * Copyright 2009 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.zxing.common;

import com.google.zxing.Binarizer;
import com.google.zxing.LuminanceSource;
import com.google.zxing.NotFoundException;

/**
 * This Binarizer implementation uses the old ZXing global histogram approach. It is suitable
 * for low-end mobile devices which don't have enough CPU or memory to use a local thresholding
 * algorithm. However, because it picks a global black point, it cannot handle difficult shadows
 * and gradients.
 *
 * Faster mobile devices and all desktop applications should probably use HybridBinarizer instead.
 *
 * @author dswitkin@google.com (Daniel Switkin)
 * @author Sean Owen
 */
public class GlobalHistogramBinarizer extends Binarizer {

  private static final int LUMINANCE_BITS = 5;
  private static final int LUMINANCE_SHIFT = 8 - LUMINANCE_BITS;
  private static final int LUMINANCE_BUCKETS = 1 << LUMINANCE_BITS;
  private static final byte[] EMPTY = new byte[0];

  private byte[] luminances;
  private final int[] buckets;

  public GlobalHistogramBinarizer(LuminanceSource source) {
    super(source);
    luminances = EMPTY;
    buckets = new int[LUMINANCE_BUCKETS];
  }

  @Override
  public BitArray getBlackRow(int y, BitArray row) throws NotFoundException {
    LuminanceSource source = getLuminanceSource();
    int width = source.getWidth();
    if (row == null || row.getSize() < width) {
      row = new BitArray(width);
    } else {
      row.clear();
    }

    initArrays(width);
    byte[] localLuminances = source.getRow(y, luminances);
    int[] localBuckets = buckets;
    for (int x = 0; x < width; x++) {
      localBuckets[(localLuminances[x] & 0xff) >> LUMINANCE_SHIFT]++;
    }
    int blackPoint = estimateBlackPoint(localBuckets);

    if (width < 3) {

      for (int x = 0; x < width; x++) {
        if ((localLuminances[x] & 0xff) < blackPoint) {
          row.set(x);
        }
      }
    } else {
      int left = localLuminances[0] & 0xff;
      int center = localLuminances[1] & 0xff;
      for (int x = 1; x < width - 1; x++) {
        int right = localLuminances[x + 1] & 0xff;

        if (((center * 4) - left - right) / 2 < blackPoint) {
          row.set(x);
        }
        left = center;
        center = right;
      }
    }
    return row;
  }

  @Override
  public BitMatrix getBlackMatrix() throws NotFoundException {
    LuminanceSource source = getLuminanceSource();
    int width = source.getWidth();
    int height = source.getHeight();
    BitMatrix matrix = new BitMatrix(width, height, 1);


    initArrays(width);
    int[] localBuckets = buckets;
    for (int y = 1; y < 5; y++) {
      int row = height * y / 5;
      byte[] localLuminances = source.getRow(row, luminances);
      int right = (width * 4) / 5;
      for (int x = width / 5; x < right; x++) {
        int pixel = localLuminances[x] & 0xff;
        localBuckets[pixel >> LUMINANCE_SHIFT]++;
      }
    }
    int blackPoint = estimateBlackPoint(localBuckets);



    byte[] localLuminances = source.getMatrix();
    for (int y = 0; y < height; y++) {
      int offset = y * width;
      for (int x = 0; x < width; x++) {
        int pixel = localLuminances[offset + x] & 0xff;
        if (pixel < blackPoint) {
          matrix.set(x, y);
        }
      }
    }

    return matrix;
  }

  @Override
  public Binarizer createBinarizer(LuminanceSource source) {
    return new GlobalHistogramBinarizer(source);
  }

  private void initArrays(int luminanceSize) {
    if (luminances.length < luminanceSize) {
      luminances = new byte[luminanceSize];
    }
    for (int x = 0; x < LUMINANCE_BUCKETS; x++) {
      buckets[x] = 0;
    }
  }

  private static int estimateBlackPoint(int[] buckets) throws NotFoundException {

    int numBuckets = buckets.length;
    int maxBucketCount = 0;
    int firstPeak = 0;
    int firstPeakSize = 0;
    for (int x = 0; x < numBuckets; x++) {
      if (buckets[x] > firstPeakSize) {
        firstPeak = x;
        firstPeakSize = buckets[x];
      }
      if (buckets[x] > maxBucketCount) {
        maxBucketCount = buckets[x];
      }
    }

    int secondPeak = 0;
    int secondPeakScore = 0;
    for (int x = 0; x < numBuckets; x++) {
      int distanceToBiggest = x - firstPeak;

      int score = buckets[x] * distanceToBiggest * distanceToBiggest;
      if (score > secondPeakScore) {
        secondPeak = x;
        secondPeakScore = score;
      }
    }

    if (firstPeak > secondPeak) {
      int temp = firstPeak;
      firstPeak = secondPeak;
      secondPeak = temp;
    }


    if (secondPeak - firstPeak <= numBuckets / 16) {
      throw NotFoundException.getNotFoundInstance();
    }

    int bestValley = secondPeak - 1;
    int bestValleyScore = -1;
    for (int x = secondPeak - 1; x > firstPeak; x--) {
      int fromFirst = x - firstPeak;
      int score = fromFirst * fromFirst * (secondPeak - x) * (maxBucketCount - buckets[x]);
      if (score > bestValleyScore) {
        bestValley = x;
        bestValleyScore = score;
      }
    }

    return bestValley << LUMINANCE_SHIFT;
  }

}
