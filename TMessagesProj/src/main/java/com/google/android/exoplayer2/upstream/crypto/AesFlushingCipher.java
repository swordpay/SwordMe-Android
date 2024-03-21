/*
 * Copyright (C) 2016 The Android Open Source Project
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
package com.google.android.exoplayer2.upstream.crypto;

import androidx.annotation.Nullable;

import com.google.android.exoplayer2.util.Assertions;
import com.google.android.exoplayer2.util.Util;

import java.nio.ByteBuffer;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;

import javax.crypto.Cipher;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.ShortBufferException;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

/**
 * A flushing variant of a AES/CTR/NoPadding {@link Cipher}.
 *
 * <p>Unlike a regular {@link Cipher}, the update methods of this class are guaranteed to process
 * all of the bytes input (and hence output the same number of bytes).
 */
public final class AesFlushingCipher {

  private final Cipher cipher;
  private final int blockSize;
  private final byte[] zerosBlock;
  private final byte[] flushedBlock;

  private int pendingXorBytes;

  public AesFlushingCipher(int mode, byte[] secretKey, @Nullable String nonce, long offset) {
    this(mode, secretKey, getFNV64Hash(nonce), offset);
  }

  public AesFlushingCipher(int mode, byte[] secretKey, long nonce, long offset) {
    try {
      cipher = Cipher.getInstance("AES/CTR/NoPadding");
      blockSize = cipher.getBlockSize();
      zerosBlock = new byte[blockSize];
      flushedBlock = new byte[blockSize];
      long counter = offset / blockSize;
      int startPadding = (int) (offset % blockSize);
      cipher.init(
          mode,
          new SecretKeySpec(secretKey, Util.splitAtFirst(cipher.getAlgorithm(), "/")[0]),
          new IvParameterSpec(getInitializationVector(nonce, counter)));
      if (startPadding != 0) {
        updateInPlace(new byte[startPadding], 0, startPadding);
      }
    } catch (NoSuchAlgorithmException
        | NoSuchPaddingException
        | InvalidKeyException
        | InvalidAlgorithmParameterException e) {

      throw new RuntimeException(e);
    }
  }

  public void updateInPlace(byte[] data, int offset, int length) {
    update(data, offset, length, data, offset);
  }

  public void update(byte[] in, int inOffset, int length, byte[] out, int outOffset) {



    while (pendingXorBytes > 0) {
      out[outOffset] = (byte) (in[inOffset] ^ flushedBlock[blockSize - pendingXorBytes]);
      outOffset++;
      inOffset++;
      pendingXorBytes--;
      length--;
      if (length == 0) {
        return;
      }
    }

    int written = nonFlushingUpdate(in, inOffset, length, out, outOffset);
    if (length == written) {
      return;
    }





    int bytesToFlush = length - written;
    Assertions.checkState(bytesToFlush < blockSize);
    outOffset += written;
    pendingXorBytes = blockSize - bytesToFlush;
    written = nonFlushingUpdate(zerosBlock, 0, pendingXorBytes, flushedBlock, 0);
    Assertions.checkState(written == blockSize);


    for (int i = 0; i < bytesToFlush; i++) {
      out[outOffset++] = flushedBlock[i];
    }
  }

  private int nonFlushingUpdate(byte[] in, int inOffset, int length, byte[] out, int outOffset) {
    try {
      return cipher.update(in, inOffset, length, out, outOffset);
    } catch (ShortBufferException e) {

      throw new RuntimeException(e);
    }
  }

  private byte[] getInitializationVector(long nonce, long counter) {
    return ByteBuffer.allocate(16).putLong(nonce).putLong(counter).array();
  }

  /**
   * Returns the hash value of the input as a long using the 64 bit FNV-1a hash function. The hash
   * values produced by this function are less likely to collide than those produced by {@link
   * #hashCode()}.
   */
  private static long getFNV64Hash(@Nullable String input) {
    if (input == null) {
      return 0;
    }

    long hash = 0;
    for (int i = 0; i < input.length(); i++) {
      hash ^= input.charAt(i);

      hash += (hash << 1) + (hash << 4) + (hash << 5) + (hash << 7) + (hash << 8) + (hash << 40);
    }
    return hash;
  }
}
