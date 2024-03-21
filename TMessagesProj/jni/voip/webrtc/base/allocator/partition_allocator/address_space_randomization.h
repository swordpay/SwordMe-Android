// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ALLOCATOR_PARTITION_ALLOCATOR_ADDRESS_SPACE_RANDOMIZATION_H_
#define BASE_ALLOCATOR_PARTITION_ALLOCATOR_ADDRESS_SPACE_RANDOMIZATION_H_

#include "base/allocator/partition_allocator/page_allocator.h"
#include "base/base_export.h"
#include "build/build_config.h"

namespace base {

// balance good ASLR against not fragmenting the address space too badly.
BASE_EXPORT void* GetRandomPageBase();

namespace internal {

constexpr uintptr_t AslrAddress(uintptr_t mask) {
  return mask & kPageAllocationGranularityBaseMask;
}
constexpr uintptr_t AslrMask(uintptr_t bits) {
  return AslrAddress((1ULL << bits) - 1ULL);
}

// incomprehensible without indentation. It is also incomprehensible with
// indentation, but the only other option is a combinatorial explosion of
// *_{win,linux,mac,foo}_{32,64}.h files.
//
// clang-format off

#if defined(ARCH_CPU_64_BITS)

  #if defined(MEMORY_TOOL_REPLACES_ALLOCATOR)





    constexpr uintptr_t kASLRMask = AslrAddress(0x007fffffffffULL);
    constexpr uintptr_t kASLROffset = AslrAddress(0x7e8000000000ULL);

  #elif defined(OS_WIN)




    constexpr uintptr_t kASLRMask = AslrMask(47);
    constexpr uintptr_t kASLRMaskBefore8_10 = AslrMask(43);

    constexpr uintptr_t kASLROffset = 0x80000000ULL;

  #elif defined(OS_MACOSX)













    constexpr uintptr_t kASLRMask = AslrMask(38);
    constexpr uintptr_t kASLROffset = AslrAddress(0x1000000000ULL);

  #elif defined(OS_POSIX) || defined(OS_FUCHSIA)

    #if defined(ARCH_CPU_X86_64)


      constexpr uintptr_t kASLRMask = AslrMask(46);
      constexpr uintptr_t kASLROffset = AslrAddress(0);

    #elif defined(ARCH_CPU_ARM64)

      #if defined(OS_ANDROID)


      constexpr uintptr_t kASLRMask = AslrMask(30);
      constexpr uintptr_t kASLROffset = AslrAddress(0x20000000ULL);

      #else


      constexpr uintptr_t kASLRMask = AslrMask(38);
      constexpr uintptr_t kASLROffset = AslrAddress(0x1000000000ULL);

      #endif

    #elif defined(ARCH_CPU_PPC64)

      #if defined(OS_AIX)



        constexpr uintptr_t kASLRMask = AslrMask(30);
        constexpr uintptr_t kASLROffset = AslrAddress(0x400000000000ULL);

      #elif defined(ARCH_CPU_BIG_ENDIAN)

        constexpr uintptr_t kASLRMask = AslrMask(42);
        constexpr uintptr_t kASLROffset = AslrAddress(0);

      #else  // !defined(OS_AIX) && !defined(ARCH_CPU_BIG_ENDIAN)

        constexpr uintptr_t kASLRMask = AslrMask(46);
        constexpr uintptr_t kASLROffset = AslrAddress(0);

      #endif  // !defined(OS_AIX) && !defined(ARCH_CPU_BIG_ENDIAN)

    #elif defined(ARCH_CPU_S390X)



      constexpr uintptr_t kASLRMask = AslrMask(40);
      constexpr uintptr_t kASLROffset = AslrAddress(0);

    #elif defined(ARCH_CPU_S390)


      constexpr uintptr_t kASLRMask = AslrMask(29);
      constexpr uintptr_t kASLROffset = AslrAddress(0);

    #else  // !defined(ARCH_CPU_X86_64) && !defined(ARCH_CPU_PPC64) &&


      constexpr uintptr_t kASLRMask = AslrMask(30);

      #if defined(OS_SOLARIS)










        constexpr uintptr_t kASLROffset = AslrAddress(0x80000000ULL);

      #elif defined(OS_AIX)


        constexpr uintptr_t kASLROffset = AslrAddress(0x90000000ULL);

      #else  // !defined(OS_SOLARIS) && !defined(OS_AIX)



        constexpr uintptr_t kASLROffset = AslrAddress(0x20000000ULL);

      #endif  // !defined(OS_SOLARIS) && !defined(OS_AIX)

    #endif  // !defined(ARCH_CPU_X86_64) && !defined(ARCH_CPU_PPC64) &&


  #endif  // defined(OS_POSIX)

#elif defined(ARCH_CPU_32_BITS)



  constexpr uintptr_t kASLRMask = AslrMask(30);
  constexpr uintptr_t kASLROffset = AslrAddress(0x20000000ULL);

#else

  #error Please tell us about your exotic hardware! Sounds interesting.

#endif  // defined(ARCH_CPU_32_BITS)


}  // namespace internal

}  // namespace base

#endif  // BASE_ALLOCATOR_PARTITION_ALLOCATOR_ADDRESS_SPACE_RANDOMIZATION_H_
