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
package com.google.android.exoplayer2.extractor.ogg;

import static com.google.android.exoplayer2.util.Assertions.checkNotNull;
import static com.google.android.exoplayer2.util.Assertions.checkStateNotNull;

import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;

import com.google.android.exoplayer2.Format;
import com.google.android.exoplayer2.ParserException;
import com.google.android.exoplayer2.extractor.VorbisUtil;
import com.google.android.exoplayer2.extractor.VorbisUtil.Mode;
import com.google.android.exoplayer2.metadata.Metadata;
import com.google.android.exoplayer2.util.MimeTypes;
import com.google.android.exoplayer2.util.ParsableByteArray;
import com.google.common.collect.ImmutableList;

import org.checkerframework.checker.nullness.qual.EnsuresNonNullIf;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;

/** {@link StreamReader} to extract Vorbis data out of Ogg byte stream. */
/* package */ final class VorbisReader extends StreamReader {

  @Nullable private VorbisSetup vorbisSetup;
  private int previousPacketBlockSize;
  private boolean seenFirstAudioPacket;

  @Nullable private VorbisUtil.VorbisIdHeader vorbisIdHeader;
  @Nullable private VorbisUtil.CommentHeader commentHeader;

  public static boolean verifyBitstreamType(ParsableByteArray data) {
    try {
      return VorbisUtil.verifyVorbisHeaderCapturePattern(/* headerType= */ 0x01, data, true);
    } catch (ParserException e) {
      return false;
    }
  }

  @Override
  protected void reset(boolean headerData) {
    super.reset(headerData);
    if (headerData) {
      vorbisSetup = null;
      vorbisIdHeader = null;
      commentHeader = null;
    }
    previousPacketBlockSize = 0;
    seenFirstAudioPacket = false;
  }

  @Override
  protected void onSeekEnd(long currentGranule) {
    super.onSeekEnd(currentGranule);
    seenFirstAudioPacket = currentGranule != 0;
    previousPacketBlockSize = vorbisIdHeader != null ? vorbisIdHeader.blockSize0 : 0;
  }

  @Override
  protected long preparePayload(ParsableByteArray packet) {

    if ((packet.getData()[0] & 0x01) == 1) {
      return -1;
    }

    int packetBlockSize = decodeBlockSize(packet.getData()[0], checkStateNotNull(vorbisSetup));


    int samplesInPacket =
        seenFirstAudioPacket ? (packetBlockSize + previousPacketBlockSize) / 4 : 0;

    appendNumberOfSamples(packet, samplesInPacket);

    seenFirstAudioPacket = true;
    previousPacketBlockSize = packetBlockSize;
    return samplesInPacket;
  }

  @Override
  @EnsuresNonNullIf(expression = "#3.format", result = false)
  protected boolean readHeaders(ParsableByteArray packet, long position, SetupData setupData)
      throws IOException {
    if (vorbisSetup != null) {
      checkNotNull(setupData.format);
      return false;
    }

    vorbisSetup = readSetupHeaders(packet);
    if (vorbisSetup == null) {
      return true;
    }
    VorbisSetup vorbisSetup = this.vorbisSetup;

    VorbisUtil.VorbisIdHeader idHeader = vorbisSetup.idHeader;

    ArrayList<byte[]> codecInitializationData = new ArrayList<>();
    codecInitializationData.add(idHeader.data);
    codecInitializationData.add(vorbisSetup.setupHeaderData);

    @Nullable
    Metadata metadata =
        VorbisUtil.parseVorbisComments(ImmutableList.copyOf(vorbisSetup.commentHeader.comments));

    setupData.format =
        new Format.Builder()
            .setSampleMimeType(MimeTypes.AUDIO_VORBIS)
            .setAverageBitrate(idHeader.bitrateNominal)
            .setPeakBitrate(idHeader.bitrateMaximum)
            .setChannelCount(idHeader.channels)
            .setSampleRate(idHeader.sampleRate)
            .setInitializationData(codecInitializationData)
            .setMetadata(metadata)
            .build();
    return true;
  }

  @VisibleForTesting
  @Nullable
  /* package */ VorbisSetup readSetupHeaders(ParsableByteArray scratch) throws IOException {

    if (vorbisIdHeader == null) {
      vorbisIdHeader = VorbisUtil.readVorbisIdentificationHeader(scratch);
      return null;
    }

    if (commentHeader == null) {
      commentHeader = VorbisUtil.readVorbisCommentHeader(scratch);
      return null;
    }
    VorbisUtil.VorbisIdHeader vorbisIdHeader = this.vorbisIdHeader;
    VorbisUtil.CommentHeader commentHeader = this.commentHeader;

    byte[] setupHeaderData = new byte[scratch.limit()];

    System.arraycopy(scratch.getData(), 0, setupHeaderData, 0, scratch.limit());

    Mode[] modes = VorbisUtil.readVorbisModes(scratch, vorbisIdHeader.channels);

    int iLogModes = VorbisUtil.iLog(modes.length - 1);

    return new VorbisSetup(vorbisIdHeader, commentHeader, setupHeaderData, modes, iLogModes);
  }

  /**
   * Reads an int of {@code length} bits from {@code src} starting at {@code
   * leastSignificantBitIndex}.
   *
   * @param src the {@code byte} to read from.
   * @param length the length in bits of the int to read.
   * @param leastSignificantBitIndex the index of the least significant bit of the int to read.
   * @return the int value read.
   */
  @VisibleForTesting
  /* package */ static int readBits(byte src, int length, int leastSignificantBitIndex) {
    return (src >> leastSignificantBitIndex) & (255 >>> (8 - length));
  }

  @VisibleForTesting
  /* package */ static void appendNumberOfSamples(
      ParsableByteArray buffer, long packetSampleCount) {
    if (buffer.capacity() < buffer.limit() + 4) {
      buffer.reset(Arrays.copyOf(buffer.getData(), buffer.limit() + 4));
    } else {
      buffer.setLimit(buffer.limit() + 4);
    }


    byte[] data = buffer.getData();
    data[buffer.limit() - 4] = (byte) (packetSampleCount & 0xFF);
    data[buffer.limit() - 3] = (byte) ((packetSampleCount >>> 8) & 0xFF);
    data[buffer.limit() - 2] = (byte) ((packetSampleCount >>> 16) & 0xFF);
    data[buffer.limit() - 1] = (byte) ((packetSampleCount >>> 24) & 0xFF);
  }

  private static int decodeBlockSize(byte firstByteOfAudioPacket, VorbisSetup vorbisSetup) {

    int modeNumber = readBits(firstByteOfAudioPacket, vorbisSetup.iLogModes, 1);
    int currentBlockSize;
    if (!vorbisSetup.modes[modeNumber].blockFlag) {
      currentBlockSize = vorbisSetup.idHeader.blockSize0;
    } else {
      currentBlockSize = vorbisSetup.idHeader.blockSize1;
    }
    return currentBlockSize;
  }

  /** Class to hold all data read from Vorbis setup headers. */
  /* package */ static final class VorbisSetup {

    public final VorbisUtil.VorbisIdHeader idHeader;
    public final VorbisUtil.CommentHeader commentHeader;
    public final byte[] setupHeaderData;
    public final Mode[] modes;
    public final int iLogModes;

    public VorbisSetup(
        VorbisUtil.VorbisIdHeader idHeader,
        VorbisUtil.CommentHeader commentHeader,
        byte[] setupHeaderData,
        Mode[] modes,
        int iLogModes) {
      this.idHeader = idHeader;
      this.commentHeader = commentHeader;
      this.setupHeaderData = setupHeaderData;
      this.modes = modes;
      this.iLogModes = iLogModes;
    }
  }
}
