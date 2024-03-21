// -*- mode: C++ -*-

// synth_minidump_unittest_data.h: verbose test data for SynthMinidump tests.

#ifndef PROCESSOR_SYNTH_MINIDUMP_UNITTEST_DATA_H_
#define PROCESSOR_SYNTH_MINIDUMP_UNITTEST_DATA_H_

#include "google_breakpad/common/minidump_format.h"

static const MDRawContextX86 x86_raw_context = {
  0xded5d71b,                           // context_flags
  0x9fdb432e,                           // dr0
  0x26b7a81a,                           // dr1
  0xcac7e348,                           // dr2
  0xcf99ec09,                           // dr3
  0x7dc8c2cd,                           // dr6
  0x21deb880,                           // dr7

  {
    0x8a5d2bb0,                         // control_word
    0x0286c4c9,                         // status_word
    0xf1feea21,                         // tag_word
    0xb2d40576,                         // error_offset
    0x48146cde,                         // error_selector
    0x983f9b21,                         // data_offset
    0x475be12c,                         // data_selector

    {
      0xd9, 0x04, 0x20, 0x6b, 0x88, 0x3a, 0x3f, 0xd5,
      0x59, 0x7a, 0xa9, 0xeb, 0xd0, 0x5c, 0xdf, 0xfe,
      0xad, 0xdd, 0x4a, 0x8b, 0x10, 0xcc, 0x9a, 0x33,
      0xcb, 0xb6, 0xf7, 0x86, 0xcd, 0x69, 0x25, 0xae,
      0x25, 0xe5, 0x7a, 0xa1, 0x8f, 0xb2, 0x84, 0xd9,
      0xf7, 0x2d, 0x8a, 0xa1, 0x80, 0x81, 0x7f, 0x67,
      0x07, 0xa8, 0x23, 0xf1, 0x8c, 0xdc, 0xd8, 0x04,
      0x8b, 0x9d, 0xb1, 0xcd, 0x61, 0x0c, 0x9c, 0x69,
      0xc7, 0x8d, 0x17, 0xb6, 0xe5, 0x0b, 0x94, 0xf7,
      0x78, 0x9b, 0x63, 0x49, 0xba, 0xfc, 0x08, 0x4d
    },

    0x84c53a90,                         // cr0_npx_state
  },

  0x79f71e76,                           // gs
  0x8107bd25,                           // fs
  0x452d2921,                           // es
  0x87ec2875,                           // ds
  0xf8bb73f5,                           // edi
  0xa63ebb88,                           // esi
  0x95d35ebe,                           // ebx
  0x17aa2456,                           // edx
  0x135fa208,                           // ecx
  0x500615e6,                           // eax
  0x66d14205,                           // ebp
  0x000719a5,                           // eip
  0x477b481b,                           // cs
  0x8684dfba,                           // eflags
  0xe33ccddf,                           // esp
  0xc0e65d33,                           // ss

  {
    0x68, 0x63, 0xdf, 0x50, 0xf7, 0x3b, 0xe8, 0xe5,
    0xcb, 0xd6, 0x66, 0x60, 0xe5, 0xa3, 0x58, 0xb3,
    0x6f, 0x34, 0xca, 0x02, 0x9b, 0x5f, 0xd0, 0x41,
    0xbd, 0xc5, 0x2d, 0xf8, 0xff, 0x15, 0xa2, 0xd0,
    0xe3, 0x2b, 0x3b, 0x8a, 0x9f, 0xc3, 0x9e, 0x28,
    0x0a, 0xc2, 0xac, 0x3b, 0x67, 0x37, 0x01, 0xfd,
    0xc3, 0xaf, 0x60, 0xf6, 0x2c, 0x4f, 0xa9, 0x52,
    0x92, 0xe5, 0x28, 0xde, 0x34, 0xb6, 0x2e, 0x44,
    0x15, 0xa4, 0xb6, 0xe4, 0xc9, 0x1a, 0x14, 0xb9,
    0x51, 0x33, 0x3c, 0xe0, 0xc7, 0x94, 0xf0, 0xf7,
    0x78, 0xdd, 0xe5, 0xca, 0xb7, 0xa6, 0xe0, 0x14,
    0xa6, 0x03, 0xab, 0x77, 0xad, 0xbd, 0xd2, 0x53,
    0x3d, 0x07, 0xe7, 0xaf, 0x90, 0x44, 0x71, 0xbe,
    0x0c, 0xdf, 0x2b, 0x97, 0x40, 0x48, 0xd5, 0xf9,
    0x62, 0x03, 0x91, 0x84, 0xd6, 0xdd, 0x29, 0x97,
    0x35, 0x02, 0xfb, 0x59, 0x97, 0xb0, 0xec, 0xa9,
    0x39, 0x6f, 0x81, 0x71, 0x2a, 0xf0, 0xe7, 0x2c,
    0x4e, 0x93, 0x90, 0xcb, 0x67, 0x69, 0xde, 0xd7,
    0x68, 0x3b, 0x0f, 0x69, 0xa8, 0xf4, 0xa8, 0x83,
    0x42, 0x80, 0x47, 0x65, 0x7a, 0xc9, 0x19, 0x5d,
    0xcb, 0x43, 0xa5, 0xff, 0xf8, 0x9e, 0x62, 0xf4,
    0xe2, 0x6c, 0xcc, 0x17, 0x55, 0x7c, 0x0d, 0x5c,
    0x8d, 0x16, 0x01, 0xd7, 0x3a, 0x0c, 0xf4, 0x7f,
    0x71, 0xdc, 0x48, 0xe9, 0x4b, 0xfe, 0x1a, 0xd0,
    0x04, 0x15, 0x33, 0xec, 0x78, 0xc6, 0x7e, 0xde,
    0x7c, 0x23, 0x18, 0x8d, 0x8f, 0xc2, 0x74, 0xc1,
    0x48, 0xcd, 0x5d, 0xee, 0xee, 0x81, 0x9e, 0x49,
    0x47, 0x8a, 0xf8, 0x61, 0xa3, 0x9c, 0x81, 0x96,
    0xbe, 0x2b, 0x5e, 0xbc, 0xcd, 0x34, 0x0a, 0x2a,
    0x3b, 0x8b, 0x7d, 0xa1, 0xf2, 0x8d, 0xb4, 0x51,
    0x9e, 0x14, 0x78, 0xa3, 0x58, 0x65, 0x2d, 0xd6,
    0x50, 0x40, 0x36, 0x32, 0x31, 0xd4, 0x3e, 0xc2,
    0xe0, 0x87, 0x1c, 0x05, 0x95, 0x80, 0x84, 0x24,
    0x08, 0x6f, 0x5b, 0xc7, 0xe1, 0x1d, 0xd5, 0xa3,
    0x94, 0x44, 0xa1, 0x7c, 0xd8, 0x4b, 0x86, 0xd2,
    0xc6, 0xa9, 0xf3, 0xe2, 0x4d, 0x6e, 0x1f, 0x0e,
    0xf2, 0xf5, 0x71, 0xf9, 0x71, 0x05, 0x24, 0xc9,
    0xc1, 0xe8, 0x91, 0x42, 0x61, 0x86, 0x57, 0x68,
    0xd9, 0xc9, 0x1d, 0xd5, 0x5a, 0xe9, 0xba, 0xe6,
    0x15, 0x8f, 0x87, 0xbd, 0x62, 0x56, 0xed, 0xda,
    0xc2, 0xa5, 0xd5, 0x39, 0xac, 0x05, 0x10, 0x14,
    0x4a, 0xe7, 0xe7, 0x3c, 0x3f, 0xb7, 0xbb, 0xed,
    0x01, 0x6e, 0xcd, 0xee, 0x81, 0xb4, 0x62, 0xf4,
    0x62, 0x16, 0xff, 0x20, 0xb4, 0xf0, 0xbc, 0xff,
    0x7d, 0xd9, 0xcf, 0x95, 0x30, 0x27, 0xe0, 0x2f,
    0x98, 0x53, 0x80, 0x15, 0x13, 0xef, 0x44, 0x58,
    0x12, 0x16, 0xdb, 0x11, 0xef, 0x73, 0x51, 0xcd,
    0x42, 0x3f, 0x98, 0x6c, 0xc9, 0x68, 0xc3, 0xf4,
    0x5b, 0x0f, 0x5d, 0x77, 0xed, 0xdf, 0x0f, 0xff,
    0xb8, 0x69, 0x98, 0x50, 0x77, 0x7a, 0xe8, 0x90,
    0x27, 0x46, 0x10, 0xd2, 0xb5, 0x00, 0x3b, 0x36,
    0x43, 0x6d, 0x67, 0x41, 0x20, 0x3a, 0x32, 0xe0,
    0x2e, 0x5a, 0xfb, 0x4e, 0x4f, 0xa4, 0xf7, 0xc2,
    0xe6, 0x81, 0x1a, 0x51, 0xa8, 0x7c, 0xd4, 0x60,
    0x7c, 0x45, 0xe2, 0xba, 0x5b, 0x42, 0xf3, 0xbf,
    0x28, 0xaa, 0xf2, 0x90, 0xe4, 0x94, 0xdd, 0xaa,
    0x22, 0xd3, 0x71, 0x33, 0xa1, 0x01, 0x43, 0x0e,
    0xfa, 0x46, 0xd2, 0x6e, 0x55, 0x5e, 0x49, 0xeb,
    0x94, 0xf0, 0xb0, 0xb1, 0x2e, 0xf2, 0x3d, 0x6c,
    0x00, 0x5e, 0x01, 0x56, 0x3b, 0xfd, 0x5b, 0xa1,
    0x2f, 0x63, 0x1d, 0xbf, 0xf9, 0xd8, 0x13, 0xf7,
    0x4d, 0xb7, 0x1e, 0x3d, 0x98, 0xd2, 0xee, 0xb8,
    0x48, 0xc8, 0x5b, 0x91, 0x0f, 0x54, 0x9e, 0x26,
    0xb2, 0xc7, 0x3a, 0x6c, 0x8a, 0x35, 0xe1, 0xba
  }
};

static const uint8_t x86_expected_contents[] = {
  0x1b, 0xd7, 0xd5, 0xde,
  0x2e, 0x43, 0xdb, 0x9f,
  0x1a, 0xa8, 0xb7, 0x26,
  0x48, 0xe3, 0xc7, 0xca,
  0x09, 0xec, 0x99, 0xcf,
  0xcd, 0xc2, 0xc8, 0x7d,
  0x80, 0xb8, 0xde, 0x21,
  0xb0, 0x2b, 0x5d, 0x8a,
  0xc9, 0xc4, 0x86, 0x02,
  0x21, 0xea, 0xfe, 0xf1,
  0x76, 0x05, 0xd4, 0xb2,
  0xde, 0x6c, 0x14, 0x48,
  0x21, 0x9b, 0x3f, 0x98,
  0x2c, 0xe1, 0x5b, 0x47,

  0xd9, 0x04, 0x20, 0x6b, 0x88, 0x3a, 0x3f, 0xd5,
  0x59, 0x7a, 0xa9, 0xeb, 0xd0, 0x5c, 0xdf, 0xfe,
  0xad, 0xdd, 0x4a, 0x8b, 0x10, 0xcc, 0x9a, 0x33,
  0xcb, 0xb6, 0xf7, 0x86, 0xcd, 0x69, 0x25, 0xae,
  0x25, 0xe5, 0x7a, 0xa1, 0x8f, 0xb2, 0x84, 0xd9,
  0xf7, 0x2d, 0x8a, 0xa1, 0x80, 0x81, 0x7f, 0x67,
  0x07, 0xa8, 0x23, 0xf1, 0x8c, 0xdc, 0xd8, 0x04,
  0x8b, 0x9d, 0xb1, 0xcd, 0x61, 0x0c, 0x9c, 0x69,
  0xc7, 0x8d, 0x17, 0xb6, 0xe5, 0x0b, 0x94, 0xf7,
  0x78, 0x9b, 0x63, 0x49, 0xba, 0xfc, 0x08, 0x4d,

  0x90, 0x3a, 0xc5, 0x84,
  0x76, 0x1e, 0xf7, 0x79,
  0x25, 0xbd, 0x07, 0x81,
  0x21, 0x29, 0x2d, 0x45,
  0x75, 0x28, 0xec, 0x87,
  0xf5, 0x73, 0xbb, 0xf8,
  0x88, 0xbb, 0x3e, 0xa6,
  0xbe, 0x5e, 0xd3, 0x95,
  0x56, 0x24, 0xaa, 0x17,
  0x08, 0xa2, 0x5f, 0x13,
  0xe6, 0x15, 0x06, 0x50,
  0x05, 0x42, 0xd1, 0x66,
  0xa5, 0x19, 0x07, 0x00,
  0x1b, 0x48, 0x7b, 0x47,
  0xba, 0xdf, 0x84, 0x86,
  0xdf, 0xcd, 0x3c, 0xe3,
  0x33, 0x5d, 0xe6, 0xc0,

  0x68, 0x63, 0xdf, 0x50, 0xf7, 0x3b, 0xe8, 0xe5,
  0xcb, 0xd6, 0x66, 0x60, 0xe5, 0xa3, 0x58, 0xb3,
  0x6f, 0x34, 0xca, 0x02, 0x9b, 0x5f, 0xd0, 0x41,
  0xbd, 0xc5, 0x2d, 0xf8, 0xff, 0x15, 0xa2, 0xd0,
  0xe3, 0x2b, 0x3b, 0x8a, 0x9f, 0xc3, 0x9e, 0x28,
  0x0a, 0xc2, 0xac, 0x3b, 0x67, 0x37, 0x01, 0xfd,
  0xc3, 0xaf, 0x60, 0xf6, 0x2c, 0x4f, 0xa9, 0x52,
  0x92, 0xe5, 0x28, 0xde, 0x34, 0xb6, 0x2e, 0x44,
  0x15, 0xa4, 0xb6, 0xe4, 0xc9, 0x1a, 0x14, 0xb9,
  0x51, 0x33, 0x3c, 0xe0, 0xc7, 0x94, 0xf0, 0xf7,
  0x78, 0xdd, 0xe5, 0xca, 0xb7, 0xa6, 0xe0, 0x14,
  0xa6, 0x03, 0xab, 0x77, 0xad, 0xbd, 0xd2, 0x53,
  0x3d, 0x07, 0xe7, 0xaf, 0x90, 0x44, 0x71, 0xbe,
  0x0c, 0xdf, 0x2b, 0x97, 0x40, 0x48, 0xd5, 0xf9,
  0x62, 0x03, 0x91, 0x84, 0xd6, 0xdd, 0x29, 0x97,
  0x35, 0x02, 0xfb, 0x59, 0x97, 0xb0, 0xec, 0xa9,
  0x39, 0x6f, 0x81, 0x71, 0x2a, 0xf0, 0xe7, 0x2c,
  0x4e, 0x93, 0x90, 0xcb, 0x67, 0x69, 0xde, 0xd7,
  0x68, 0x3b, 0x0f, 0x69, 0xa8, 0xf4, 0xa8, 0x83,
  0x42, 0x80, 0x47, 0x65, 0x7a, 0xc9, 0x19, 0x5d,
  0xcb, 0x43, 0xa5, 0xff, 0xf8, 0x9e, 0x62, 0xf4,
  0xe2, 0x6c, 0xcc, 0x17, 0x55, 0x7c, 0x0d, 0x5c,
  0x8d, 0x16, 0x01, 0xd7, 0x3a, 0x0c, 0xf4, 0x7f,
  0x71, 0xdc, 0x48, 0xe9, 0x4b, 0xfe, 0x1a, 0xd0,
  0x04, 0x15, 0x33, 0xec, 0x78, 0xc6, 0x7e, 0xde,
  0x7c, 0x23, 0x18, 0x8d, 0x8f, 0xc2, 0x74, 0xc1,
  0x48, 0xcd, 0x5d, 0xee, 0xee, 0x81, 0x9e, 0x49,
  0x47, 0x8a, 0xf8, 0x61, 0xa3, 0x9c, 0x81, 0x96,
  0xbe, 0x2b, 0x5e, 0xbc, 0xcd, 0x34, 0x0a, 0x2a,
  0x3b, 0x8b, 0x7d, 0xa1, 0xf2, 0x8d, 0xb4, 0x51,
  0x9e, 0x14, 0x78, 0xa3, 0x58, 0x65, 0x2d, 0xd6,
  0x50, 0x40, 0x36, 0x32, 0x31, 0xd4, 0x3e, 0xc2,
  0xe0, 0x87, 0x1c, 0x05, 0x95, 0x80, 0x84, 0x24,
  0x08, 0x6f, 0x5b, 0xc7, 0xe1, 0x1d, 0xd5, 0xa3,
  0x94, 0x44, 0xa1, 0x7c, 0xd8, 0x4b, 0x86, 0xd2,
  0xc6, 0xa9, 0xf3, 0xe2, 0x4d, 0x6e, 0x1f, 0x0e,
  0xf2, 0xf5, 0x71, 0xf9, 0x71, 0x05, 0x24, 0xc9,
  0xc1, 0xe8, 0x91, 0x42, 0x61, 0x86, 0x57, 0x68,
  0xd9, 0xc9, 0x1d, 0xd5, 0x5a, 0xe9, 0xba, 0xe6,
  0x15, 0x8f, 0x87, 0xbd, 0x62, 0x56, 0xed, 0xda,
  0xc2, 0xa5, 0xd5, 0x39, 0xac, 0x05, 0x10, 0x14,
  0x4a, 0xe7, 0xe7, 0x3c, 0x3f, 0xb7, 0xbb, 0xed,
  0x01, 0x6e, 0xcd, 0xee, 0x81, 0xb4, 0x62, 0xf4,
  0x62, 0x16, 0xff, 0x20, 0xb4, 0xf0, 0xbc, 0xff,
  0x7d, 0xd9, 0xcf, 0x95, 0x30, 0x27, 0xe0, 0x2f,
  0x98, 0x53, 0x80, 0x15, 0x13, 0xef, 0x44, 0x58,
  0x12, 0x16, 0xdb, 0x11, 0xef, 0x73, 0x51, 0xcd,
  0x42, 0x3f, 0x98, 0x6c, 0xc9, 0x68, 0xc3, 0xf4,
  0x5b, 0x0f, 0x5d, 0x77, 0xed, 0xdf, 0x0f, 0xff,
  0xb8, 0x69, 0x98, 0x50, 0x77, 0x7a, 0xe8, 0x90,
  0x27, 0x46, 0x10, 0xd2, 0xb5, 0x00, 0x3b, 0x36,
  0x43, 0x6d, 0x67, 0x41, 0x20, 0x3a, 0x32, 0xe0,
  0x2e, 0x5a, 0xfb, 0x4e, 0x4f, 0xa4, 0xf7, 0xc2,
  0xe6, 0x81, 0x1a, 0x51, 0xa8, 0x7c, 0xd4, 0x60,
  0x7c, 0x45, 0xe2, 0xba, 0x5b, 0x42, 0xf3, 0xbf,
  0x28, 0xaa, 0xf2, 0x90, 0xe4, 0x94, 0xdd, 0xaa,
  0x22, 0xd3, 0x71, 0x33, 0xa1, 0x01, 0x43, 0x0e,
  0xfa, 0x46, 0xd2, 0x6e, 0x55, 0x5e, 0x49, 0xeb,
  0x94, 0xf0, 0xb0, 0xb1, 0x2e, 0xf2, 0x3d, 0x6c,
  0x00, 0x5e, 0x01, 0x56, 0x3b, 0xfd, 0x5b, 0xa1,
  0x2f, 0x63, 0x1d, 0xbf, 0xf9, 0xd8, 0x13, 0xf7,
  0x4d, 0xb7, 0x1e, 0x3d, 0x98, 0xd2, 0xee, 0xb8,
  0x48, 0xc8, 0x5b, 0x91, 0x0f, 0x54, 0x9e, 0x26,
  0xb2, 0xc7, 0x3a, 0x6c, 0x8a, 0x35, 0xe1, 0xba
};

static const MDRawContextARM arm_raw_context = {

  0x591b9e6a,

  {
    0xa21594de,
    0x820d8a25,
    0xc4e133b2,
    0x173a1c02,
    0x105fb175,
    0xe871793f,
    0x5def70b3,
    0xcee3a623,
    0x7b3aa9b8,
    0x52518537,
    0x627012c5,
    0x22723dcc,
    0x16fcc971,
    0x20988bcb,
    0xf1ab806b,
    0x99d5fc03,
  },

  0xb70df511,

  {

    0xa1e1f7ce1077e6b5ULL,

    {
      0xbcb8d002eed7fbdeULL,
      0x4dd26a43b96ae97fULL,
      0x8eec22db8b31741cULL,
      0xfd634bd7c5ad66a0ULL,
      0x1681da0daeb3debeULL,
      0x474a32bdf72d0b71ULL,
      0xcaf464f8b1044834ULL,
      0xcaa6592ae5c7582aULL,
      0x4ee46889d877c3dbULL,
      0xf8930cf301645cf5ULL,
      0x4da7e9ebba27f7c7ULL,
      0x69a7b02761944da3ULL,
      0x2cda2b2e78195c06ULL,
      0x66b227ab9b460a42ULL,
      0x7e77e49e52ee0849ULL,
      0xd62cd9663e76f255ULL,
      0xe9370f082451514bULL,
      0x50a1c674dd1b6029ULL,
      0x405db4575829eac4ULL,
      0x67b948764649eee7ULL,
      0x93731885419229d4ULL,
      0xdb0338bad72a4ce7ULL,
      0xa0a451f996fca4c8ULL,
      0xb4508ea668400a45ULL,
      0xbff28c5c7a142423ULL,
      0x4f31b42b96f3a431ULL,
      0x2ce6789d4ea1ff37ULL,
      0xfa150b52e4f82a3cULL,
      0xe9ec40449e6ed4f3ULL,
      0x5ceca87836fe2251ULL,
      0x66f50de463ee238cULL,
      0x42823efcd59ab511ULL,
    },

    {
      0xe9e14cd2,
      0x865bb640,
      0x9f3f0b3e,
      0x94a71c52,
      0x3c012f19,
      0x6436637c,
      0x46ccedcb,
      0x7b341be7,
    }
  }
};

static const uint8_t arm_expected_contents[] = {
  0x6a, 0x9e, 0x1b, 0x59,
  0xde, 0x94, 0x15, 0xa2,
  0x25, 0x8a, 0x0d, 0x82,
  0xb2, 0x33, 0xe1, 0xc4,
  0x02, 0x1c, 0x3a, 0x17,
  0x75, 0xb1, 0x5f, 0x10,
  0x3f, 0x79, 0x71, 0xe8,
  0xb3, 0x70, 0xef, 0x5d,
  0x23, 0xa6, 0xe3, 0xce,
  0xb8, 0xa9, 0x3a, 0x7b,
  0x37, 0x85, 0x51, 0x52,
  0xc5, 0x12, 0x70, 0x62,
  0xcc, 0x3d, 0x72, 0x22,
  0x71, 0xc9, 0xfc, 0x16,
  0xcb, 0x8b, 0x98, 0x20,
  0x6b, 0x80, 0xab, 0xf1,
  0x03, 0xfc, 0xd5, 0x99,
  0x11, 0xf5, 0x0d, 0xb7,
  0xb5, 0xe6, 0x77, 0x10,
  0xce, 0xf7, 0xe1, 0xa1,
  0xde, 0xfb, 0xd7, 0xee,
  0x02, 0xd0, 0xb8, 0xbc,
  0x7f, 0xe9, 0x6a, 0xb9,
  0x43, 0x6a, 0xd2, 0x4d,
  0x1c, 0x74, 0x31, 0x8b,
  0xdb, 0x22, 0xec, 0x8e,
  0xa0, 0x66, 0xad, 0xc5,
  0xd7, 0x4b, 0x63, 0xfd,
  0xbe, 0xde, 0xb3, 0xae,
  0x0d, 0xda, 0x81, 0x16,
  0x71, 0x0b, 0x2d, 0xf7,
  0xbd, 0x32, 0x4a, 0x47,
  0x34, 0x48, 0x04, 0xb1,
  0xf8, 0x64, 0xf4, 0xca,
  0x2a, 0x58, 0xc7, 0xe5,
  0x2a, 0x59, 0xa6, 0xca,
  0xdb, 0xc3, 0x77, 0xd8,
  0x89, 0x68, 0xe4, 0x4e,
  0xf5, 0x5c, 0x64, 0x01,
  0xf3, 0x0c, 0x93, 0xf8,
  0xc7, 0xf7, 0x27, 0xba,
  0xeb, 0xe9, 0xa7, 0x4d,
  0xa3, 0x4d, 0x94, 0x61,
  0x27, 0xb0, 0xa7, 0x69,
  0x06, 0x5c, 0x19, 0x78,
  0x2e, 0x2b, 0xda, 0x2c,
  0x42, 0x0a, 0x46, 0x9b,
  0xab, 0x27, 0xb2, 0x66,
  0x49, 0x08, 0xee, 0x52,
  0x9e, 0xe4, 0x77, 0x7e,
  0x55, 0xf2, 0x76, 0x3e,
  0x66, 0xd9, 0x2c, 0xd6,
  0x4b, 0x51, 0x51, 0x24,
  0x08, 0x0f, 0x37, 0xe9,
  0x29, 0x60, 0x1b, 0xdd,
  0x74, 0xc6, 0xa1, 0x50,
  0xc4, 0xea, 0x29, 0x58,
  0x57, 0xb4, 0x5d, 0x40,
  0xe7, 0xee, 0x49, 0x46,
  0x76, 0x48, 0xb9, 0x67,
  0xd4, 0x29, 0x92, 0x41,
  0x85, 0x18, 0x73, 0x93,
  0xe7, 0x4c, 0x2a, 0xd7,
  0xba, 0x38, 0x03, 0xdb,
  0xc8, 0xa4, 0xfc, 0x96,
  0xf9, 0x51, 0xa4, 0xa0,
  0x45, 0x0a, 0x40, 0x68,
  0xa6, 0x8e, 0x50, 0xb4,
  0x23, 0x24, 0x14, 0x7a,
  0x5c, 0x8c, 0xf2, 0xbf,
  0x31, 0xa4, 0xf3, 0x96,
  0x2b, 0xb4, 0x31, 0x4f,
  0x37, 0xff, 0xa1, 0x4e,
  0x9d, 0x78, 0xe6, 0x2c,
  0x3c, 0x2a, 0xf8, 0xe4,
  0x52, 0x0b, 0x15, 0xfa,
  0xf3, 0xd4, 0x6e, 0x9e,
  0x44, 0x40, 0xec, 0xe9,
  0x51, 0x22, 0xfe, 0x36,
  0x78, 0xa8, 0xec, 0x5c,
  0x8c, 0x23, 0xee, 0x63,
  0xe4, 0x0d, 0xf5, 0x66,
  0x11, 0xb5, 0x9a, 0xd5,
  0xfc, 0x3e, 0x82, 0x42,
  0xd2, 0x4c, 0xe1, 0xe9,
  0x40, 0xb6, 0x5b, 0x86,
  0x3e, 0x0b, 0x3f, 0x9f,
  0x52, 0x1c, 0xa7, 0x94,
  0x19, 0x2f, 0x01, 0x3c,
  0x7c, 0x63, 0x36, 0x64,
  0xcb, 0xed, 0xcc, 0x46,
  0xe7, 0x1b, 0x34, 0x7b
};

#endif // PROCESSOR_SYNTH_MINIDUMP_UNITTEST_DATA_H_
