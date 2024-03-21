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
package com.google.android.exoplayer2.metadata.scte35;

import androidx.annotation.Nullable;

import com.google.android.exoplayer2.metadata.Metadata;
import com.google.android.exoplayer2.metadata.MetadataInputBuffer;
import com.google.android.exoplayer2.metadata.SimpleMetadataDecoder;
import com.google.android.exoplayer2.util.ParsableBitArray;
import com.google.android.exoplayer2.util.ParsableByteArray;
import com.google.android.exoplayer2.util.TimestampAdjuster;

import org.checkerframework.checker.nullness.qual.MonotonicNonNull;

import java.nio.ByteBuffer;

/** Decodes splice info sections and produces splice commands. */
public final class SpliceInfoDecoder extends SimpleMetadataDecoder {

  private static final int TYPE_SPLICE_NULL = 0x00;
  private static final int TYPE_SPLICE_SCHEDULE = 0x04;
  private static final int TYPE_SPLICE_INSERT = 0x05;
  private static final int TYPE_TIME_SIGNAL = 0x06;
  private static final int TYPE_PRIVATE_COMMAND = 0xFF;

  private final ParsableByteArray sectionData;
  private final ParsableBitArray sectionHeader;

  private @MonotonicNonNull TimestampAdjuster timestampAdjuster;

  public SpliceInfoDecoder() {
    sectionData = new ParsableByteArray();
    sectionHeader = new ParsableBitArray();
  }

  @Override
  @SuppressWarnings("ByteBufferBackingArray") // Buffer validated by SimpleMetadataDecoder.decode
  protected Metadata decode(MetadataInputBuffer inputBuffer, ByteBuffer buffer) {

    if (timestampAdjuster == null
        || inputBuffer.subsampleOffsetUs != timestampAdjuster.getTimestampOffsetUs()) {
      timestampAdjuster = new TimestampAdjuster(inputBuffer.timeUs);
      timestampAdjuster.adjustSampleTimestamp(inputBuffer.timeUs - inputBuffer.subsampleOffsetUs);
    }

    byte[] data = buffer.array();
    int size = buffer.limit();
    sectionData.reset(data, size);
    sectionHeader.reset(data, size);


    sectionHeader.skipBits(39);
    long ptsAdjustment = sectionHeader.readBits(1);
    ptsAdjustment = (ptsAdjustment << 32) | sectionHeader.readBits(32);

    sectionHeader.skipBits(20);
    int spliceCommandLength = sectionHeader.readBits(12);
    int spliceCommandType = sectionHeader.readBits(8);
    @Nullable SpliceCommand command = null;

    sectionData.skipBytes(14);
    switch (spliceCommandType) {
      case TYPE_SPLICE_NULL:
        command = new SpliceNullCommand();
        break;
      case TYPE_SPLICE_SCHEDULE:
        command = SpliceScheduleCommand.parseFromSection(sectionData);
        break;
      case TYPE_SPLICE_INSERT:
        command =
            SpliceInsertCommand.parseFromSection(sectionData, ptsAdjustment, timestampAdjuster);
        break;
      case TYPE_TIME_SIGNAL:
        command = TimeSignalCommand.parseFromSection(sectionData, ptsAdjustment, timestampAdjuster);
        break;
      case TYPE_PRIVATE_COMMAND:
        command = PrivateCommand.parseFromSection(sectionData, spliceCommandLength, ptsAdjustment);
        break;
      default:

        break;
    }
    return command == null ? new Metadata() : new Metadata(command);
  }
}
