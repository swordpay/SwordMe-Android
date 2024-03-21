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
package com.google.android.exoplayer2.extractor.ts;

import android.util.SparseArray;

import androidx.annotation.Nullable;

import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.ParserException;
import com.google.android.exoplayer2.extractor.Extractor;
import com.google.android.exoplayer2.extractor.ExtractorInput;
import com.google.android.exoplayer2.extractor.ExtractorOutput;
import com.google.android.exoplayer2.extractor.ExtractorsFactory;
import com.google.android.exoplayer2.extractor.PositionHolder;
import com.google.android.exoplayer2.extractor.SeekMap;
import com.google.android.exoplayer2.extractor.ts.TsPayloadReader.TrackIdGenerator;
import com.google.android.exoplayer2.util.Assertions;
import com.google.android.exoplayer2.util.ParsableBitArray;
import com.google.android.exoplayer2.util.ParsableByteArray;
import com.google.android.exoplayer2.util.TimestampAdjuster;

import org.checkerframework.checker.nullness.qual.MonotonicNonNull;
import org.checkerframework.checker.nullness.qual.RequiresNonNull;

import java.io.IOException;

/** Extracts data from the MPEG-2 PS container format. */
public final class PsExtractor implements Extractor {

  /** Factory for {@link PsExtractor} instances. */
  public static final ExtractorsFactory FACTORY = () -> new Extractor[] {new PsExtractor()};

  /* package */ static final int PACK_START_CODE = 0x000001BA;
  /* package */ static final int SYSTEM_HEADER_START_CODE = 0x000001BB;
  /* package */ static final int PACKET_START_CODE_PREFIX = 0x000001;
  /* package */ static final int MPEG_PROGRAM_END_CODE = 0x000001B9;
  private static final int MAX_STREAM_ID_PLUS_ONE = 0x100;

  private static final long MAX_SEARCH_LENGTH = 1024 * 1024;


  private static final long MAX_SEARCH_LENGTH_AFTER_AUDIO_AND_VIDEO_FOUND = 8 * 1024;

  public static final int PRIVATE_STREAM_1 = 0xBD;
  public static final int AUDIO_STREAM = 0xC0;
  public static final int AUDIO_STREAM_MASK = 0xE0;
  public static final int VIDEO_STREAM = 0xE0;
  public static final int VIDEO_STREAM_MASK = 0xF0;

  private final TimestampAdjuster timestampAdjuster;
  private final SparseArray<PesReader> psPayloadReaders; // Indexed by pid
  private final ParsableByteArray psPacketBuffer;
  private final PsDurationReader durationReader;

  private boolean foundAllTracks;
  private boolean foundAudioTrack;
  private boolean foundVideoTrack;
  private long lastTrackPosition;

  @Nullable private PsBinarySearchSeeker psBinarySearchSeeker;
  private @MonotonicNonNull ExtractorOutput output;
  private boolean hasOutputSeekMap;

  public PsExtractor() {
    this(new TimestampAdjuster(0));
  }

  public PsExtractor(TimestampAdjuster timestampAdjuster) {
    this.timestampAdjuster = timestampAdjuster;
    psPacketBuffer = new ParsableByteArray(4096);
    psPayloadReaders = new SparseArray<>();
    durationReader = new PsDurationReader();
  }


  @Override
  public boolean sniff(ExtractorInput input) throws IOException {
    byte[] scratch = new byte[14];
    input.peekFully(scratch, 0, 14);

    if (PACK_START_CODE
        != (((scratch[0] & 0xFF) << 24)
            | ((scratch[1] & 0xFF) << 16)
            | ((scratch[2] & 0xFF) << 8)
            | (scratch[3] & 0xFF))) {
      return false;
    }

    if ((scratch[4] & 0xC4) != 0x44) {
      return false;
    }

    if ((scratch[6] & 0x04) != 0x04) {
      return false;
    }

    if ((scratch[8] & 0x04) != 0x04) {
      return false;
    }

    if ((scratch[9] & 0x01) != 0x01) {
      return false;
    }

    if ((scratch[12] & 0x03) != 0x03) {
      return false;
    }

    int packStuffingLength = scratch[13] & 0x07;
    input.advancePeekPosition(packStuffingLength);

    input.peekFully(scratch, 0, 3);
    return (PACKET_START_CODE_PREFIX
        == (((scratch[0] & 0xFF) << 16) | ((scratch[1] & 0xFF) << 8) | (scratch[2] & 0xFF)));
  }

  @Override
  public void init(ExtractorOutput output) {
    this.output = output;
  }

  @Override
  public void seek(long position, long timeUs) {





    boolean resetTimestampAdjuster = timestampAdjuster.getTimestampOffsetUs() == C.TIME_UNSET;
    if (!resetTimestampAdjuster) {
      long adjusterFirstSampleTimestampUs = timestampAdjuster.getFirstSampleTimestampUs();



      resetTimestampAdjuster =
          adjusterFirstSampleTimestampUs != C.TIME_UNSET
              && adjusterFirstSampleTimestampUs != 0
              && adjusterFirstSampleTimestampUs != timeUs;
    }
    if (resetTimestampAdjuster) {
      timestampAdjuster.reset(timeUs);
    }

    if (psBinarySearchSeeker != null) {
      psBinarySearchSeeker.setSeekTargetUs(timeUs);
    }
    for (int i = 0; i < psPayloadReaders.size(); i++) {
      psPayloadReaders.valueAt(i).seek();
    }
  }

  @Override
  public void release() {

  }

  @Override
  public int read(ExtractorInput input, PositionHolder seekPosition) throws IOException {
    Assertions.checkStateNotNull(output); // Asserts init has been called.

    long inputLength = input.getLength();
    boolean canReadDuration = inputLength != C.LENGTH_UNSET;
    if (canReadDuration && !durationReader.isDurationReadFinished()) {
      return durationReader.readDuration(input, seekPosition);
    }
    maybeOutputSeekMap(inputLength);
    if (psBinarySearchSeeker != null && psBinarySearchSeeker.isSeeking()) {
      return psBinarySearchSeeker.handlePendingSeek(input, seekPosition);
    }

    input.resetPeekPosition();
    long peekBytesLeft =
        inputLength != C.LENGTH_UNSET ? inputLength - input.getPeekPosition() : C.LENGTH_UNSET;
    if (peekBytesLeft != C.LENGTH_UNSET && peekBytesLeft < 4) {
      return RESULT_END_OF_INPUT;
    }

    if (!input.peekFully(psPacketBuffer.getData(), 0, 4, true)) {
      return RESULT_END_OF_INPUT;
    }

    psPacketBuffer.setPosition(0);
    int nextStartCode = psPacketBuffer.readInt();
    if (nextStartCode == MPEG_PROGRAM_END_CODE) {
      return RESULT_END_OF_INPUT;
    } else if (nextStartCode == PACK_START_CODE) {

      input.peekFully(psPacketBuffer.getData(), 0, 10);

      psPacketBuffer.setPosition(9);

      int packStuffingLength = psPacketBuffer.readUnsignedByte() & 0x07;

      input.skipFully(packStuffingLength + 14);
      return RESULT_CONTINUE;
    } else if (nextStartCode == SYSTEM_HEADER_START_CODE) {

      input.peekFully(psPacketBuffer.getData(), 0, 2);

      psPacketBuffer.setPosition(0);
      int systemHeaderLength = psPacketBuffer.readUnsignedShort();
      input.skipFully(systemHeaderLength + 6);
      return RESULT_CONTINUE;
    } else if (((nextStartCode & 0xFFFFFF00) >> 8) != PACKET_START_CODE_PREFIX) {
      input.skipFully(1); // Skip bytes until we see a valid start code again.
      return RESULT_CONTINUE;
    }


    int streamId = nextStartCode & 0xFF;

    PesReader payloadReader = psPayloadReaders.get(streamId);
    if (!foundAllTracks) {
      if (payloadReader == null) {
        @Nullable ElementaryStreamReader elementaryStreamReader = null;
        if (streamId == PRIVATE_STREAM_1) {



          elementaryStreamReader = new Ac3Reader();
          foundAudioTrack = true;
          lastTrackPosition = input.getPosition();
        } else if ((streamId & AUDIO_STREAM_MASK) == AUDIO_STREAM) {
          elementaryStreamReader = new MpegAudioReader();
          foundAudioTrack = true;
          lastTrackPosition = input.getPosition();
        } else if ((streamId & VIDEO_STREAM_MASK) == VIDEO_STREAM) {
          elementaryStreamReader = new H262Reader();
          foundVideoTrack = true;
          lastTrackPosition = input.getPosition();
        }
        if (elementaryStreamReader != null) {
          TrackIdGenerator idGenerator = new TrackIdGenerator(streamId, MAX_STREAM_ID_PLUS_ONE);
          elementaryStreamReader.createTracks(output, idGenerator);
          payloadReader = new PesReader(elementaryStreamReader, timestampAdjuster);
          psPayloadReaders.put(streamId, payloadReader);
        }
      }
      long maxSearchPosition =
          foundAudioTrack && foundVideoTrack
              ? lastTrackPosition + MAX_SEARCH_LENGTH_AFTER_AUDIO_AND_VIDEO_FOUND
              : MAX_SEARCH_LENGTH;
      if (input.getPosition() > maxSearchPosition) {
        foundAllTracks = true;
        output.endTracks();
      }
    }

    input.peekFully(psPacketBuffer.getData(), 0, 2);
    psPacketBuffer.setPosition(0);
    int payloadLength = psPacketBuffer.readUnsignedShort();
    int pesLength = payloadLength + 6;

    if (payloadReader == null) {

      input.skipFully(pesLength);
    } else {
      psPacketBuffer.reset(pesLength);

      input.readFully(psPacketBuffer.getData(), 0, pesLength);
      psPacketBuffer.setPosition(6);
      payloadReader.consume(psPacketBuffer);
      psPacketBuffer.setLimit(psPacketBuffer.capacity());
    }

    return RESULT_CONTINUE;
  }


  @RequiresNonNull("output")
  private void maybeOutputSeekMap(long inputLength) {
    if (!hasOutputSeekMap) {
      hasOutputSeekMap = true;
      if (durationReader.getDurationUs() != C.TIME_UNSET) {
        psBinarySearchSeeker =
            new PsBinarySearchSeeker(
                durationReader.getScrTimestampAdjuster(),
                durationReader.getDurationUs(),
                inputLength);
        output.seekMap(psBinarySearchSeeker.getSeekMap());
      } else {
        output.seekMap(new SeekMap.Unseekable(durationReader.getDurationUs()));
      }
    }
  }

  /** Parses PES packet data and extracts samples. */
  private static final class PesReader {

    private static final int PES_SCRATCH_SIZE = 64;

    private final ElementaryStreamReader pesPayloadReader;
    private final TimestampAdjuster timestampAdjuster;
    private final ParsableBitArray pesScratch;

    private boolean ptsFlag;
    private boolean dtsFlag;
    private boolean seenFirstDts;
    private int extendedHeaderLength;
    private long timeUs;

    public PesReader(ElementaryStreamReader pesPayloadReader, TimestampAdjuster timestampAdjuster) {
      this.pesPayloadReader = pesPayloadReader;
      this.timestampAdjuster = timestampAdjuster;
      pesScratch = new ParsableBitArray(new byte[PES_SCRATCH_SIZE]);
    }

    /**
     * Notifies the reader that a seek has occurred.
     *
     * <p>Following a call to this method, the data passed to the next invocation of {@link
     * #consume(ParsableByteArray)} will not be a continuation of the data that was previously
     * passed. Hence the reader should reset any internal state.
     */
    public void seek() {
      seenFirstDts = false;
      pesPayloadReader.seek();
    }

    /**
     * Consumes the payload of a PS packet.
     *
     * @param data The PES packet. The position will be set to the start of the payload.
     * @throws ParserException If the payload could not be parsed.
     */
    public void consume(ParsableByteArray data) throws ParserException {
      data.readBytes(pesScratch.data, 0, 3);
      pesScratch.setPosition(0);
      parseHeader();
      data.readBytes(pesScratch.data, 0, extendedHeaderLength);
      pesScratch.setPosition(0);
      parseHeaderExtension();
      pesPayloadReader.packetStarted(timeUs, TsPayloadReader.FLAG_DATA_ALIGNMENT_INDICATOR);
      pesPayloadReader.consume(data);

      pesPayloadReader.packetFinished();
    }

    private void parseHeader() {




      pesScratch.skipBits(8);
      ptsFlag = pesScratch.readBit();
      dtsFlag = pesScratch.readBit();


      pesScratch.skipBits(6);
      extendedHeaderLength = pesScratch.readBits(8);
    }

    private void parseHeaderExtension() {
      timeUs = 0;
      if (ptsFlag) {
        pesScratch.skipBits(4); // '0010' or '0011'
        long pts = (long) pesScratch.readBits(3) << 30;
        pesScratch.skipBits(1); // marker_bit
        pts |= pesScratch.readBits(15) << 15;
        pesScratch.skipBits(1); // marker_bit
        pts |= pesScratch.readBits(15);
        pesScratch.skipBits(1); // marker_bit
        if (!seenFirstDts && dtsFlag) {
          pesScratch.skipBits(4); // '0011'
          long dts = (long) pesScratch.readBits(3) << 30;
          pesScratch.skipBits(1); // marker_bit
          dts |= pesScratch.readBits(15) << 15;
          pesScratch.skipBits(1); // marker_bit
          dts |= pesScratch.readBits(15);
          pesScratch.skipBits(1); // marker_bit





          timestampAdjuster.adjustTsTimestamp(dts);
          seenFirstDts = true;
        }
        timeUs = timestampAdjuster.adjustTsTimestamp(pts);
      }
    }
  }
}
