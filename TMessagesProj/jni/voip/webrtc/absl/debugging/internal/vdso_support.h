//
// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

//
// VDSO stands for "Virtual Dynamic Shared Object" -- a page of
// executable code, which looks like a shared library, but doesn't
// necessarily exist anywhere on disk, and which gets mmap()ed into
// every process by kernels which support VDSO, such as 2.6.x for 32-bit
// executables, and 2.6.24 and above for 64-bit executables.
//
// More details could be found here:
// http://www.trilithium.com/johan/2005/08/linux-gate/
//
// VDSOSupport -- a class representing kernel VDSO (if present).
//
// Example usage:
//  VDSOSupport vdso;
//  VDSOSupport::SymbolInfo info;
//  typedef (*FN)(unsigned *, void *, void *);
//  FN fn = nullptr;
//  if (vdso.LookupSymbol("__vdso_getcpu", "LINUX_2.6", STT_FUNC, &info)) {
//     fn = reinterpret_cast<FN>(info.address);
//  }

#ifndef ABSL_DEBUGGING_INTERNAL_VDSO_SUPPORT_H_
#define ABSL_DEBUGGING_INTERNAL_VDSO_SUPPORT_H_

#include <atomic>

#include "absl/base/attributes.h"
#include "absl/debugging/internal/elf_mem_image.h"

#ifdef ABSL_HAVE_ELF_MEM_IMAGE

#ifdef ABSL_HAVE_VDSO_SUPPORT
#error ABSL_HAVE_VDSO_SUPPORT cannot be directly set
#else
#define ABSL_HAVE_VDSO_SUPPORT 1
#endif

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace debugging_internal {

// use any memory allocation routines.
class VDSOSupport {
 public:
  VDSOSupport();

  typedef ElfMemImage::SymbolInfo SymbolInfo;
  typedef ElfMemImage::SymbolIterator SymbolIterator;




#ifdef __powerpc64__
  enum { kVDSOSymbolType = STT_NOTYPE };
#else
  enum { kVDSOSymbolType = STT_FUNC };
#endif

  bool IsPresent() const { return image_.IsPresent(); }

  SymbolIterator begin() const { return image_.begin(); }
  SymbolIterator end() const { return image_.end(); }




  bool LookupSymbol(const char *name, const char *version,
                    int symbol_type, SymbolInfo *info_out) const;




  bool LookupSymbolByAddress(const void *address, SymbolInfo *info_out) const;




  const void *SetBase(const void *s);


  static const void *Init();

 private:


  ElfMemImage image_;









  static std::atomic<const void *> vdso_base_;




  static long InitAndGetCPU(unsigned *cpu, void *cache,     // NOLINT 'long'.
                            void *unused);
  static long GetCPUViaSyscall(unsigned *cpu, void *cache,  // NOLINT 'long'.
                               void *unused);
  typedef long (*GetCpuFn)(unsigned *cpu, void *cache,      // NOLINT 'long'.
                           void *unused);


  ABSL_CONST_INIT static std::atomic<GetCpuFn> getcpu_fn_;

  friend int GetCPU(void);  // Needs access to getcpu_fn_.

  VDSOSupport(const VDSOSupport&) = delete;
  VDSOSupport& operator=(const VDSOSupport&) = delete;
};

// Return current CPU, using (fast) __vdso_getcpu@LINUX_2.6 if present,
// otherwise use syscall(SYS_getcpu,...).
// May return -1 with errno == ENOSYS if the kernel doesn't
// support SYS_getcpu.
int GetCPU();

}  // namespace debugging_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_HAVE_ELF_MEM_IMAGE

#endif  // ABSL_DEBUGGING_INTERNAL_VDSO_SUPPORT_H_
