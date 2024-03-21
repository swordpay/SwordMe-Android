/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "modules/rtp_rtcp/source/flexfec_header_reader_writer.h"

#include <string.h>

#include "api/scoped_refptr.h"
#include "modules/rtp_rtcp/source/byte_io.h"
#include "modules/rtp_rtcp/source/forward_error_correction_internal.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"

namespace webrtc {

namespace {

constexpr size_t kMaxMediaPackets = 48;  // Since we are reusing ULPFEC masks.

// Maintain a sufficiently larger tracking window than `kMaxMediaPackets`
// to account for packet reordering in pacer/ network.
constexpr size_t kMaxTrackedMediaPackets = 4 * kMaxMediaPackets;

constexpr size_t kMaxFecPackets = kMaxMediaPackets;

constexpr size_t kFlexfecPacketMaskSizes[] = {2, 6, 14};

constexpr size_t kBaseHeaderSize = 12;

constexpr size_t kStreamSpecificHeaderSize = 6;

// the number of K-bits set.
constexpr size_t kHeaderSizes[] = {
    kBaseHeaderSize + kStreamSpecificHeaderSize + kFlexfecPacketMaskSizes[0],
    kBaseHeaderSize + kStreamSpecificHeaderSize + kFlexfecPacketMaskSizes[1],
    kBaseHeaderSize + kStreamSpecificHeaderSize + kFlexfecPacketMaskSizes[2]};

// TODO(brandtr): Update this when we support multistream protection.
constexpr uint8_t kSsrcCount = 1;

constexpr uint32_t kReservedBits = 0;

constexpr size_t kPacketMaskOffset =
    kBaseHeaderSize + kStreamSpecificHeaderSize;

// This can be used in conjunction with FlexfecHeaderWriter::MinPacketMaskSize,
// which calculates a bound on the needed packet mask size including K-bits,
// given a packet mask without K-bits.
size_t FlexfecHeaderSize(size_t packet_mask_size) {
  RTC_DCHECK_LE(packet_mask_size, kFlexfecPacketMaskSizes[2]);
  if (packet_mask_size <= kFlexfecPacketMaskSizes[0]) {
    return kHeaderSizes[0];
  } else if (packet_mask_size <= kFlexfecPacketMaskSizes[1]) {
    return kHeaderSizes[1];
  }
  return kHeaderSizes[2];
}

}  // namespace

FlexfecHeaderReader::FlexfecHeaderReader()
    : FecHeaderReader(kMaxTrackedMediaPackets, kMaxFecPackets) {}

FlexfecHeaderReader::~FlexfecHeaderReader() = default;

// retransmissions, and/or several protected SSRCs.
bool FlexfecHeaderReader::ReadFecHeader(
    ForwardErrorCorrection::ReceivedFecPacket* fec_packet) const {
  if (fec_packet->pkt->data.size() <=
      kBaseHeaderSize + kStreamSpecificHeaderSize) {
    RTC_LOG(LS_WARNING) << "Discarding truncated FlexFEC packet.";
    return false;
  }
  uint8_t* const data = fec_packet->pkt->data.MutableData();
  bool r_bit = (data[0] & 0x80) != 0;
  if (r_bit) {
    RTC_LOG(LS_INFO)
        << "FlexFEC packet with retransmission bit set. We do not yet "
           "support this, thus discarding the packet.";
    return false;
  }
  bool f_bit = (data[0] & 0x40) != 0;
  if (f_bit) {
    RTC_LOG(LS_INFO)
        << "FlexFEC packet with inflexible generator matrix. We do "
           "not yet support this, thus discarding packet.";
    return false;
  }
  uint8_t ssrc_count = ByteReader<uint8_t>::ReadBigEndian(&data[8]);
  if (ssrc_count != 1) {
    RTC_LOG(LS_INFO)
        << "FlexFEC packet protecting multiple media SSRCs. We do not "
           "yet support this, thus discarding packet.";
    return false;
  }
  uint32_t protected_ssrc = ByteReader<uint32_t>::ReadBigEndian(&data[12]);
  uint16_t seq_num_base = ByteReader<uint16_t>::ReadBigEndian(&data[16]);










  if (fec_packet->pkt->data.size() < kHeaderSizes[0]) {
    RTC_LOG(LS_WARNING) << "Discarding truncated FlexFEC packet.";
    return false;
  }
  uint8_t* const packet_mask = data + kPacketMaskOffset;
  bool k_bit0 = (packet_mask[0] & 0x80) != 0;
  uint16_t mask_part0 = ByteReader<uint16_t>::ReadBigEndian(&packet_mask[0]);

  mask_part0 <<= 1;
  ByteWriter<uint16_t>::WriteBigEndian(&packet_mask[0], mask_part0);
  size_t packet_mask_size;
  if (k_bit0) {



    packet_mask_size = kFlexfecPacketMaskSizes[0];
  } else {
    if (fec_packet->pkt->data.size() < kHeaderSizes[1]) {
      return false;
    }
    bool k_bit1 = (packet_mask[2] & 0x80) != 0;




    uint8_t bit15 = (packet_mask[2] >> 6) & 0x01;
    packet_mask[1] |= bit15;
    uint32_t mask_part1 = ByteReader<uint32_t>::ReadBigEndian(&packet_mask[2]);

    mask_part1 <<= 2;
    ByteWriter<uint32_t>::WriteBigEndian(&packet_mask[2], mask_part1);
    if (k_bit1) {



      packet_mask_size = kFlexfecPacketMaskSizes[1];
    } else {
      if (fec_packet->pkt->data.size() < kHeaderSizes[2]) {
        RTC_LOG(LS_WARNING) << "Discarding truncated FlexFEC packet.";
        return false;
      }
      bool k_bit2 = (packet_mask[6] & 0x80) != 0;
      if (k_bit2) {



        packet_mask_size = kFlexfecPacketMaskSizes[2];
      } else {
        RTC_LOG(LS_WARNING)
            << "Discarding FlexFEC packet with malformed header.";
        return false;
      }





      uint8_t tail_bits = (packet_mask[6] >> 5) & 0x03;
      packet_mask[5] |= tail_bits;
      uint64_t mask_part2 =
          ByteReader<uint64_t>::ReadBigEndian(&packet_mask[6]);


      mask_part2 <<= 3;
      ByteWriter<uint64_t>::WriteBigEndian(&packet_mask[6], mask_part2);
    }
  }

  fec_packet->fec_header_size = FlexfecHeaderSize(packet_mask_size);
  fec_packet->protected_ssrc = protected_ssrc;
  fec_packet->seq_num_base = seq_num_base;
  fec_packet->packet_mask_offset = kPacketMaskOffset;
  fec_packet->packet_mask_size = packet_mask_size;

  fec_packet->protection_length =
      fec_packet->pkt->data.size() - fec_packet->fec_header_size;

  return true;
}

FlexfecHeaderWriter::FlexfecHeaderWriter()
    : FecHeaderWriter(kMaxMediaPackets, kMaxFecPackets, kHeaderSizes[2]) {}

FlexfecHeaderWriter::~FlexfecHeaderWriter() = default;

size_t FlexfecHeaderWriter::MinPacketMaskSize(const uint8_t* packet_mask,
                                              size_t packet_mask_size) const {
  if (packet_mask_size == kUlpfecPacketMaskSizeLBitClear &&
      (packet_mask[1] & 0x01) == 0) {


    return kFlexfecPacketMaskSizes[0];
  } else if (packet_mask_size == kUlpfecPacketMaskSizeLBitClear) {


    return kFlexfecPacketMaskSizes[1];
  } else if (packet_mask_size == kUlpfecPacketMaskSizeLBitSet &&
             (packet_mask[5] & 0x03) == 0) {


    return kFlexfecPacketMaskSizes[1];
  } else if (packet_mask_size == kUlpfecPacketMaskSizeLBitSet) {


    return kFlexfecPacketMaskSizes[2];
  }
  RTC_DCHECK_NOTREACHED() << "Incorrect packet mask size: " << packet_mask_size
                          << ".";
  return kFlexfecPacketMaskSizes[2];
}

size_t FlexfecHeaderWriter::FecHeaderSize(size_t packet_mask_size) const {
  return FlexfecHeaderSize(packet_mask_size);
}

// FlexFEC header standard. Note that the header size is computed by
// FecHeaderSize(), so in this function we can be sure that we are
// writing in space that is intended for the header.
//
// TODO(brandtr): Update this function when we support offset-based masks,
// retransmissions, and protecting multiple SSRCs.
void FlexfecHeaderWriter::FinalizeFecHeader(
    uint32_t media_ssrc,
    uint16_t seq_num_base,
    const uint8_t* packet_mask,
    size_t packet_mask_size,
    ForwardErrorCorrection::Packet* fec_packet) const {
  uint8_t* data = fec_packet->data.MutableData();
  data[0] &= 0x7f;  // Clear R bit.
  data[0] &= 0xbf;  // Clear F bit.
  ByteWriter<uint8_t>::WriteBigEndian(&data[8], kSsrcCount);
  ByteWriter<uint32_t, 3>::WriteBigEndian(&data[9], kReservedBits);
  ByteWriter<uint32_t>::WriteBigEndian(&data[12], media_ssrc);
  ByteWriter<uint16_t>::WriteBigEndian(&data[16], seq_num_base);




  uint8_t* const written_packet_mask = data + kPacketMaskOffset;
  if (packet_mask_size == kUlpfecPacketMaskSizeLBitSet) {

    uint16_t tmp_mask_part0 =
        ByteReader<uint16_t>::ReadBigEndian(&packet_mask[0]);
    uint32_t tmp_mask_part1 =
        ByteReader<uint32_t>::ReadBigEndian(&packet_mask[2]);

    tmp_mask_part0 >>= 1;  // Shift, thus clearing K-bit 0.
    ByteWriter<uint16_t>::WriteBigEndian(&written_packet_mask[0],
                                         tmp_mask_part0);
    tmp_mask_part1 >>= 2;  // Shift, thus clearing K-bit 1 and bit 15.
    ByteWriter<uint32_t>::WriteBigEndian(&written_packet_mask[2],
                                         tmp_mask_part1);
    bool bit15 = (packet_mask[1] & 0x01) != 0;
    if (bit15)
      written_packet_mask[2] |= 0x40;  // Set bit 15.
    bool bit46 = (packet_mask[5] & 0x02) != 0;
    bool bit47 = (packet_mask[5] & 0x01) != 0;
    if (!bit46 && !bit47) {
      written_packet_mask[2] |= 0x80;  // Set K-bit 1.
    } else {
      memset(&written_packet_mask[6], 0, 8);  // Clear all trailing bits.
      written_packet_mask[6] |= 0x80;         // Set K-bit 2.
      if (bit46)
        written_packet_mask[6] |= 0x40;  // Set bit 46.
      if (bit47)
        written_packet_mask[6] |= 0x20;  // Set bit 47.
    }
  } else if (packet_mask_size == kUlpfecPacketMaskSizeLBitClear) {

    uint16_t tmp_mask_part0 =
        ByteReader<uint16_t>::ReadBigEndian(&packet_mask[0]);

    tmp_mask_part0 >>= 1;  // Shift, thus clearing K-bit 0.
    ByteWriter<uint16_t>::WriteBigEndian(&written_packet_mask[0],
                                         tmp_mask_part0);
    bool bit15 = (packet_mask[1] & 0x01) != 0;
    if (!bit15) {
      written_packet_mask[0] |= 0x80;  // Set K-bit 0.
    } else {
      memset(&written_packet_mask[2], 0U, 4);  // Clear all trailing bits.
      written_packet_mask[2] |= 0x80;          // Set K-bit 1.
      written_packet_mask[2] |= 0x40;          // Set bit 15.
    }
  } else {
    RTC_DCHECK_NOTREACHED()
        << "Incorrect packet mask size: " << packet_mask_size << ".";
  }
}

}  // namespace webrtc
