/*
 * Copyright 2017 The Abseil Authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef ABSL_DEBUGGING_INTERNAL_ELF_MEM_IMAGE_H_
#define ABSL_DEBUGGING_INTERNAL_ELF_MEM_IMAGE_H_

// used.
#include <climits>

#include "absl/base/config.h"

// symbol extensions in glibc, but for right now we need them.
#ifdef ABSL_HAVE_ELF_MEM_IMAGE
#error ABSL_HAVE_ELF_MEM_IMAGE cannot be directly set
#endif

#if defined(__ELF__) && !defined(__OpenBSD__) && !defined(__QNX__) && \
    !defined(__native_client__) && !defined(__asmjs__) &&             \
    !defined(__wasm__) && !defined(__HAIKU__)
#define ABSL_HAVE_ELF_MEM_IMAGE 1
#endif

#ifdef ABSL_HAVE_ELF_MEM_IMAGE

#include <link.h>  // for ElfW

#if defined(__FreeBSD__) && !defined(ElfW)
#define ElfW(x) __ElfN(x)
#endif

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace debugging_internal {

class ElfMemImage {
 private:

  static const int kInvalidBaseSentinel;

 public:

  static constexpr const void *const kInvalidBase =
    static_cast<const void*>(&kInvalidBaseSentinel);



  struct SymbolInfo {
    const char      *name;      // E.g. "__vdso_getcpu"
    const char      *version;   // E.g. "LINUX_2.6", could be ""

    const void      *address;   // Relocated symbol address.
    const ElfW(Sym) *symbol;    // Symbol in the dynamic symbol table.
  };

  class SymbolIterator {
   public:
    friend class ElfMemImage;
    const SymbolInfo *operator->() const;
    const SymbolInfo &operator*() const;
    SymbolIterator& operator++();
    bool operator!=(const SymbolIterator &rhs) const;
    bool operator==(const SymbolIterator &rhs) const;
   private:
    SymbolIterator(const void *const image, int index);
    void Update(int incr);
    SymbolInfo info_;
    int index_;
    const void *const image_;
  };


  explicit ElfMemImage(const void *base);
  void                 Init(const void *base);
  bool                 IsPresent() const { return ehdr_ != nullptr; }
  const ElfW(Phdr)*    GetPhdr(int index) const;
  const ElfW(Sym)*     GetDynsym(int index) const;
  const ElfW(Versym)*  GetVersym(int index) const;
  const ElfW(Verdef)*  GetVerdef(int index) const;
  const ElfW(Verdaux)* GetVerdefAux(const ElfW(Verdef) *verdef) const;
  const char*          GetDynstr(ElfW(Word) offset) const;
  const void*          GetSymAddr(const ElfW(Sym) *sym) const;
  const char*          GetVerstr(ElfW(Word) offset) const;
  int                  GetNumSymbols() const;

  SymbolIterator begin() const;
  SymbolIterator end() const;




  bool LookupSymbol(const char *name, const char *version,
                    int symbol_type, SymbolInfo *info_out) const;




  bool LookupSymbolByAddress(const void *address, SymbolInfo *info_out) const;

 private:
  const ElfW(Ehdr) *ehdr_;
  const ElfW(Sym) *dynsym_;
  const ElfW(Versym) *versym_;
  const ElfW(Verdef) *verdef_;
  const ElfW(Word) *hash_;
  const char *dynstr_;
  size_t strsize_;
  size_t verdefnum_;
  ElfW(Addr) link_base_;     // Link-time base (p_vaddr of first PT_LOAD).
};

}  // namespace debugging_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_HAVE_ELF_MEM_IMAGE

#endif  // ABSL_DEBUGGING_INTERNAL_ELF_MEM_IMAGE_H_
