/*
 * Copyright 2007 ZXing authors
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

package com.google.zxing.qrcode.decoder;

import com.google.zxing.ChecksumException;
import com.google.zxing.DecodeHintType;
import com.google.zxing.FormatException;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.common.DecoderResult;
import com.google.zxing.common.reedsolomon.GenericGF;
import com.google.zxing.common.reedsolomon.ReedSolomonDecoder;
import com.google.zxing.common.reedsolomon.ReedSolomonException;

import java.util.Map;

/**
 * <p>The main class which implements QR Code decoding -- as opposed to locating and extracting
 * the QR Code from an image.</p>
 *
 * @author Sean Owen
 */
public final class Decoder {

  private final ReedSolomonDecoder rsDecoder;

  public Decoder() {
    rsDecoder = new ReedSolomonDecoder(GenericGF.QR_CODE_FIELD_256);
  }

  public DecoderResult decode(boolean[][] image) throws ChecksumException, FormatException {
    return decode(image, null);
  }

  /**
   * <p>Convenience method that can decode a QR Code represented as a 2D array of booleans.
   * "true" is taken to mean a black module.</p>
   *
   * @param image booleans representing white/black QR Code modules
   * @param hints decoding hints that should be used to influence decoding
   * @return text and bytes encoded within the QR Code
   * @throws FormatException if the QR Code cannot be decoded
   * @throws ChecksumException if error correction fails
   */
  public DecoderResult decode(boolean[][] image, Map<DecodeHintType,?> hints)
      throws ChecksumException, FormatException {
    return decode(BitMatrix.parse(image), hints);
  }

  public DecoderResult decode(BitMatrix bits) throws ChecksumException, FormatException {
    return decode(bits, null);
  }

  /**
   * <p>Decodes a QR Code represented as a {@link BitMatrix}. A 1 or "true" is taken to mean a black module.</p>
   *
   * @param bits booleans representing white/black QR Code modules
   * @param hints decoding hints that should be used to influence decoding
   * @return text and bytes encoded within the QR Code
   * @throws FormatException if the QR Code cannot be decoded
   * @throws ChecksumException if error correction fails
   */
  public DecoderResult decode(BitMatrix bits, Map<DecodeHintType,?> hints)
      throws FormatException, ChecksumException {

    BitMatrixParser parser = new BitMatrixParser(bits);
    FormatException fe = null;
    ChecksumException ce = null;
    try {
      return decode(parser, hints);
    } catch (FormatException e) {
      fe = e;
    } catch (ChecksumException e) {
      ce = e;
    }

    try {

      parser.remask();

      parser.setMirror(true);

      parser.readVersion();

      parser.readFormatInformation();

      /*
       * Since we're here, this means we have successfully detected some kind
       * of version and format information when mirrored. This is a good sign,
       * that the QR code may be mirrored, and we should try once more with a
       * mirrored content.
       */

      parser.mirror();

      DecoderResult result = decode(parser, hints);

      result.setOther(new QRCodeDecoderMetaData(true));

      return result;

    } catch (FormatException | ChecksumException e) {

      if (fe != null) {
        throw fe;
      }
      throw ce; // If fe is null, this can't be
    }
  }

  private DecoderResult decode(BitMatrixParser parser, Map<DecodeHintType,?> hints)
      throws FormatException, ChecksumException {
    Version version = parser.readVersion();
    ErrorCorrectionLevel ecLevel = parser.readFormatInformation().getErrorCorrectionLevel();

    byte[] codewords = parser.readCodewords();

    DataBlock[] dataBlocks = DataBlock.getDataBlocks(codewords, version, ecLevel);

    int totalBytes = 0;
    for (DataBlock dataBlock : dataBlocks) {
      totalBytes += dataBlock.getNumDataCodewords();
    }
    byte[] resultBytes = new byte[totalBytes];
    int resultOffset = 0;

    for (DataBlock dataBlock : dataBlocks) {
      byte[] codewordBytes = dataBlock.getCodewords();
      int numDataCodewords = dataBlock.getNumDataCodewords();
      correctErrors(codewordBytes, numDataCodewords);
      for (int i = 0; i < numDataCodewords; i++) {
        resultBytes[resultOffset++] = codewordBytes[i];
      }
    }

    return DecodedBitStreamParser.decode(resultBytes, version, ecLevel, hints);
  }

  /**
   * <p>Given data and error-correction codewords received, possibly corrupted by errors, attempts to
   * correct the errors in-place using Reed-Solomon error correction.</p>
   *
   * @param codewordBytes data and error correction codewords
   * @param numDataCodewords number of codewords that are data bytes
   * @throws ChecksumException if error correction fails
   */
  private void correctErrors(byte[] codewordBytes, int numDataCodewords) throws ChecksumException {
    int numCodewords = codewordBytes.length;

    int[] codewordsInts = new int[numCodewords];
    for (int i = 0; i < numCodewords; i++) {
      codewordsInts[i] = codewordBytes[i] & 0xFF;
    }
    try {
      rsDecoder.decode(codewordsInts, codewordBytes.length - numDataCodewords);
    } catch (ReedSolomonException ignored) {
      throw ChecksumException.getChecksumInstance();
    }


    for (int i = 0; i < numDataCodewords; i++) {
      codewordBytes[i] = (byte) codewordsInts[i];
    }
  }

}
