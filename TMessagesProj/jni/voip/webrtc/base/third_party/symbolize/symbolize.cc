// Copyright (c) 2006, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Author: Satoru Takabayashi
// Stack-footprint reduction work done by Raksit Ashok
//
// Implementation note:
//
// We don't use heaps but only use stacks.  We want to reduce the
// stack consumption so that the symbolizer can run on small stacks.
//
// Here are some numbers collected with GCC 4.1.0 on x86:
// - sizeof(Elf32_Sym)  = 16
// - sizeof(Elf32_Shdr) = 40
// - sizeof(Elf64_Sym)  = 24
// - sizeof(Elf64_Shdr) = 64
//
// This implementation is intended to be async-signal-safe but uses
// some functions which are not guaranteed to be so, such as memchr()
// and memmove().  We assume they are async-signal-safe.
//
// Additional header can be specified by the GLOG_BUILD_CONFIG_INCLUDE
// macro to add platform specific defines (e.g. OS_OPENBSD).

#ifdef GLOG_BUILD_CONFIG_INCLUDE
#include GLOG_BUILD_CONFIG_INCLUDE
#endif  // GLOG_BUILD_CONFIG_INCLUDE

#include "utilities.h"

#if defined(HAVE_SYMBOLIZE)

#include <string.h>

#include <algorithm>
#include <limits>

#include "symbolize.h"
#include "demangle.h"

_START_GOOGLE_NAMESPACE_

// async-signal-safe.  Instead we define a minimal assertion
// macro. So far, we don't need pretty printing for __FILE__, etc.

static int AssertFail() {
  abort();
  return 0;  // Should not reach.
}

#define SAFE_ASSERT(expr) ((expr) ? 0 : AssertFail())

static SymbolizeCallback g_symbolize_callback = NULL;
void InstallSymbolizeCallback(SymbolizeCallback callback) {
  g_symbolize_callback = callback;
}

static SymbolizeOpenObjectFileCallback g_symbolize_open_object_file_callback =
    NULL;
void InstallSymbolizeOpenObjectFileCallback(
    SymbolizeOpenObjectFileCallback callback) {
  g_symbolize_open_object_file_callback = callback;
}

// where the input symbol is demangled in-place.
// To keep stack consumption low, we would like this function to not
// get inlined.
static ATTRIBUTE_NOINLINE void DemangleInplace(char *out, int out_size) {
  char demangled[256];  // Big enough for sane demangled symbols.
  if (Demangle(out, demangled, sizeof(demangled))) {

    size_t len = strlen(demangled);
    if (len + 1 <= (size_t)out_size) {  // +1 for '\0'.
      SAFE_ASSERT(len < sizeof(demangled));
      memmove(out, demangled, len + 1);
    }
  }
}

_END_GOOGLE_NAMESPACE_

#if defined(__ELF__)

#if defined(HAVE_DLFCN_H)
#include <dlfcn.h>
#endif
#if defined(OS_OPENBSD)
#include <sys/exec_elf.h>
#else
#include <elf.h>
#endif
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "symbolize.h"
#include "config.h"
#include "glog/raw_logging.h"

#define NO_INTR(fn)   do {} while ((fn) < 0 && errno == EINTR)

_START_GOOGLE_NAMESPACE_

// descriptor "fd" into the buffer starting at "buf" while handling short reads
// and EINTR.  On success, return the number of bytes read.  Otherwise, return
// -1.
static ssize_t ReadFromOffset(const int fd, void *buf, const size_t count,
                              const off_t offset) {
  SAFE_ASSERT(fd >= 0);
  SAFE_ASSERT(count <= std::numeric_limits<ssize_t>::max());
  char *buf0 = reinterpret_cast<char *>(buf);
  ssize_t num_bytes = 0;
  while (num_bytes < count) {
    ssize_t len;
    NO_INTR(len = pread(fd, buf0 + num_bytes, count - num_bytes,
                        offset + num_bytes));
    if (len < 0) {  // There was an error other than EINTR.
      return -1;
    }
    if (len == 0) {  // Reached EOF.
      break;
    }
    num_bytes += len;
  }
  SAFE_ASSERT(num_bytes <= count);
  return num_bytes;
}

// pointed by "fd" into the buffer starting at "buf" while handling
// short reads and EINTR.  On success, return true. Otherwise, return
// false.
static bool ReadFromOffsetExact(const int fd, void *buf,
                                const size_t count, const off_t offset) {
  ssize_t len = ReadFromOffset(fd, buf, count, offset);
  return len == count;
}

static int FileGetElfType(const int fd) {
  ElfW(Ehdr) elf_header;
  if (!ReadFromOffsetExact(fd, &elf_header, sizeof(elf_header), 0)) {
    return -1;
  }
  if (memcmp(elf_header.e_ident, ELFMAG, SELFMAG) != 0) {
    return -1;
  }
  return elf_header.e_type;
}

// of the specified type is found, set the output to this section header
// and return true.  Otherwise, return false.
// To keep stack consumption low, we would like this function to not get
// inlined.
static ATTRIBUTE_NOINLINE bool
GetSectionHeaderByType(const int fd, ElfW(Half) sh_num, const off_t sh_offset,
                       ElfW(Word) type, ElfW(Shdr) *out) {

  ElfW(Shdr) buf[16];
  for (int i = 0; i < sh_num;) {
    const ssize_t num_bytes_left = (sh_num - i) * sizeof(buf[0]);
    const ssize_t num_bytes_to_read =
        (sizeof(buf) > num_bytes_left) ? num_bytes_left : sizeof(buf);
    const ssize_t len = ReadFromOffset(fd, buf, num_bytes_to_read,
                                       sh_offset + i * sizeof(buf[0]));
    if (len == -1) {
      return false;
    }
    SAFE_ASSERT(len % sizeof(buf[0]) == 0);
    const ssize_t num_headers_in_buf = len / sizeof(buf[0]);
    SAFE_ASSERT(num_headers_in_buf <= sizeof(buf) / sizeof(buf[0]));
    for (int j = 0; j < num_headers_in_buf; ++j) {
      if (buf[j].sh_type == type) {
        *out = buf[j];
        return true;
      }
    }
    i += num_headers_in_buf;
  }
  return false;
}

// but there has (as yet) been no need for anything longer either.
const int kMaxSectionNameLen = 64;

bool GetSectionHeaderByName(int fd, const char *name, size_t name_len,
                            ElfW(Shdr) *out) {
  ElfW(Ehdr) elf_header;
  if (!ReadFromOffsetExact(fd, &elf_header, sizeof(elf_header), 0)) {
    return false;
  }

  ElfW(Shdr) shstrtab;
  off_t shstrtab_offset = (elf_header.e_shoff +
                           elf_header.e_shentsize * elf_header.e_shstrndx);
  if (!ReadFromOffsetExact(fd, &shstrtab, sizeof(shstrtab), shstrtab_offset)) {
    return false;
  }

  for (int i = 0; i < elf_header.e_shnum; ++i) {
    off_t section_header_offset = (elf_header.e_shoff +
                                   elf_header.e_shentsize * i);
    if (!ReadFromOffsetExact(fd, out, sizeof(*out), section_header_offset)) {
      return false;
    }
    char header_name[kMaxSectionNameLen];
    if (sizeof(header_name) < name_len) {
      RAW_LOG(WARNING, "Section name '%s' is too long (%" PRIuS "); "
              "section will not be found (even if present).", name, name_len);

      return false;
    }
    off_t name_offset = shstrtab.sh_offset + out->sh_name;
    ssize_t n_read = ReadFromOffset(fd, &header_name, name_len, name_offset);
    if (n_read == -1) {
      return false;
    } else if (n_read != name_len) {

      continue;
    }
    if (memcmp(header_name, name, name_len) == 0) {
      return true;
    }
  }
  return false;
}

// pc. Iterate over symbols in a symbol table and look for the symbol
// containing "pc".  On success, return true and write the symbol name
// to out.  Otherwise, return false.
// To keep stack consumption low, we would like this function to not get
// inlined.
static ATTRIBUTE_NOINLINE bool
FindSymbol(uint64_t pc, const int fd, char *out, int out_size,
           uint64_t symbol_offset, const ElfW(Shdr) *strtab,
           const ElfW(Shdr) *symtab) {
  if (symtab == NULL) {
    return false;
  }
  const int num_symbols = symtab->sh_size / symtab->sh_entsize;
  for (int i = 0; i < num_symbols;) {
    off_t offset = symtab->sh_offset + i * symtab->sh_entsize;



#if __WORDSIZE == 64
#define NUM_SYMBOLS 32
#else
#define NUM_SYMBOLS 64
#endif

    ElfW(Sym) buf[NUM_SYMBOLS];
    int num_symbols_to_read = std::min(NUM_SYMBOLS, num_symbols - i);
    const ssize_t len =
        ReadFromOffset(fd, &buf, sizeof(buf[0]) * num_symbols_to_read, offset);
    SAFE_ASSERT(len % sizeof(buf[0]) == 0);
    const ssize_t num_symbols_in_buf = len / sizeof(buf[0]);
    SAFE_ASSERT(num_symbols_in_buf <= num_symbols_to_read);
    for (int j = 0; j < num_symbols_in_buf; ++j) {
      const ElfW(Sym)& symbol = buf[j];
      uint64_t start_address = symbol.st_value;
      start_address += symbol_offset;
      uint64_t end_address = start_address + symbol.st_size;
      if (symbol.st_value != 0 &&  // Skip null value symbols.
          symbol.st_shndx != 0 &&  // Skip undefined symbols.
          start_address <= pc && pc < end_address) {
        ssize_t len1 = ReadFromOffset(fd, out, out_size,
                                      strtab->sh_offset + symbol.st_name);
        if (len1 <= 0 || memchr(out, '\0', out_size) == NULL) {
          memset(out, 0, out_size);
          return false;
        }
        return true;  // Obtained the symbol name.
      }
    }
    i += num_symbols_in_buf;
  }
  return false;
}

// both regular and dynamic symbol tables if necessary.  On success,
// write the symbol name to "out" and return true.  Otherwise, return
// false.
static bool GetSymbolFromObjectFile(const int fd,
                                    uint64_t pc,
                                    char* out,
                                    int out_size,
                                    uint64_t base_address) {

  ElfW(Ehdr) elf_header;
  if (!ReadFromOffsetExact(fd, &elf_header, sizeof(elf_header), 0)) {
    return false;
  }

  ElfW(Shdr) symtab, strtab;

  if (GetSectionHeaderByType(fd, elf_header.e_shnum, elf_header.e_shoff,
                             SHT_SYMTAB, &symtab)) {
    if (!ReadFromOffsetExact(fd, &strtab, sizeof(strtab), elf_header.e_shoff +
                             symtab.sh_link * sizeof(symtab))) {
      return false;
    }
    if (FindSymbol(pc, fd, out, out_size, base_address, &strtab, &symtab)) {
      return true;  // Found the symbol in a regular symbol table.
    }
  }

  if (GetSectionHeaderByType(fd, elf_header.e_shnum, elf_header.e_shoff,
                             SHT_DYNSYM, &symtab)) {
    if (!ReadFromOffsetExact(fd, &strtab, sizeof(strtab), elf_header.e_shoff +
                             symtab.sh_link * sizeof(symtab))) {
      return false;
    }
    if (FindSymbol(pc, fd, out, out_size, base_address, &strtab, &symtab)) {
      return true;  // Found the symbol in a dynamic symbol table.
    }
  }

  return false;
}

namespace {
// Thin wrapper around a file descriptor so that the file descriptor
// gets closed for sure.
struct FileDescriptor {
  const int fd_;
  explicit FileDescriptor(int fd) : fd_(fd) {}
  ~FileDescriptor() {
    if (fd_ >= 0) {
      close(fd_);
    }
  }
  int get() { return fd_; }

 private:
  explicit FileDescriptor(const FileDescriptor&);
  void operator=(const FileDescriptor&);
};

//
// Note: we don't use ProcMapsIterator since the object is big (it has
// a 5k array member) and uses async-unsafe functions such as sscanf()
// and snprintf().
class LineReader {
 public:
  explicit LineReader(int fd, char *buf, int buf_len, off_t offset)
      : fd_(fd),
        buf_(buf),
        buf_len_(buf_len),
        offset_(offset),
        bol_(buf),
        eol_(buf),
        eod_(buf) {}





  bool ReadLine(const char **bol, const char **eol) {
    if (BufferIsEmpty()) {  // First time.
      const ssize_t num_bytes = ReadFromOffset(fd_, buf_, buf_len_, offset_);
      if (num_bytes <= 0) {  // EOF or error.
        return false;
      }
      offset_ += num_bytes;
      eod_ = buf_ + num_bytes;
      bol_ = buf_;
    } else {
      bol_ = eol_ + 1;  // Advance to the next line in the buffer.
      SAFE_ASSERT(bol_ <= eod_);  // "bol_" can point to "eod_".
      if (!HasCompleteLine()) {
        const int incomplete_line_length = eod_ - bol_;

        memmove(buf_, bol_, incomplete_line_length);

        char * const append_pos = buf_ + incomplete_line_length;
        const int capacity_left = buf_len_ - incomplete_line_length;
        const ssize_t num_bytes =
            ReadFromOffset(fd_, append_pos, capacity_left, offset_);
        if (num_bytes <= 0) {  // EOF or error.
          return false;
        }
        offset_ += num_bytes;
        eod_ = append_pos + num_bytes;
        bol_ = buf_;
      }
    }
    eol_ = FindLineFeed();
    if (eol_ == NULL) {  // '\n' not found.  Malformed line.
      return false;
    }
    *eol_ = '\0';  // Replace '\n' with '\0'.

    *bol = bol_;
    *eol = eol_;
    return true;
  }

  const char *bol() {
    return bol_;
  }

  const char *eol() {
    return eol_;
  }

 private:
  explicit LineReader(const LineReader&);
  void operator=(const LineReader&);

  char *FindLineFeed() {
    return reinterpret_cast<char *>(memchr(bol_, '\n', eod_ - bol_));
  }

  bool BufferIsEmpty() {
    return buf_ == eod_;
  }

  bool HasCompleteLine() {
    return !BufferIsEmpty() && FindLineFeed() != NULL;
  }

  const int fd_;
  char * const buf_;
  const int buf_len_;
  off_t offset_;
  char *bol_;
  char *eol_;
  const char *eod_;  // End of data in "buf_".
};
}  // namespace

// the first non-hex character or "end" is returned.
static char *GetHex(const char *start, const char *end, uint64_t *hex) {
  *hex = 0;
  const char *p;
  for (p = start; p < end; ++p) {
    int ch = *p;
    if ((ch >= '0' && ch <= '9') ||
        (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f')) {
      *hex = (*hex << 4) | (ch < 'A' ? ch - '0' : (ch & 0xF) + 9);
    } else {  // Encountered the first non-hex character.
      break;
    }
  }
  SAFE_ASSERT(p <= end);
  return const_cast<char *>(p);
}

// the specified pc.  If found, sets |start_address| to the start address
// of where this object file is mapped in memory, sets the module base
// address into |base_address|, copies the object file name into
// |out_file_name|, and attempts to open the object file.  If the object
// file is opened successfully, returns the file descriptor.  Otherwise,
// returns -1.  |out_file_name_size| is the size of the file name buffer
// (including the null-terminator).
static ATTRIBUTE_NOINLINE int
OpenObjectFileContainingPcAndGetStartAddress(uint64_t pc,
                                             uint64_t &start_address,
                                             uint64_t &base_address,
                                             char *out_file_name,
                                             int out_file_name_size) {
  int object_fd;

  int maps_fd;
  NO_INTR(maps_fd = open("/proc/self/maps", O_RDONLY));
  FileDescriptor wrapped_maps_fd(maps_fd);
  if (wrapped_maps_fd.get() < 0) {
    return -1;
  }

  int mem_fd;
  NO_INTR(mem_fd = open("/proc/self/mem", O_RDONLY));
  FileDescriptor wrapped_mem_fd(mem_fd);
  if (wrapped_mem_fd.get() < 0) {
    return -1;
  }


  char buf[1024];  // Big enough for line of sane /proc/self/maps
  int num_maps = 0;
  LineReader reader(wrapped_maps_fd.get(), buf, sizeof(buf), 0);
  while (true) {
    num_maps++;
    const char *cursor;
    const char *eol;
    if (!reader.ReadLine(&cursor, &eol)) {  // EOF or malformed line.
      return -1;
    }







    cursor = GetHex(cursor, eol, &start_address);
    if (cursor == eol || *cursor != '-') {
      return -1;  // Malformed line.
    }
    ++cursor;  // Skip '-'.

    uint64_t end_address;
    cursor = GetHex(cursor, eol, &end_address);
    if (cursor == eol || *cursor != ' ') {
      return -1;  // Malformed line.
    }
    ++cursor;  // Skip ' '.

    const char * const flags_start = cursor;
    while (cursor < eol && *cursor != ' ') {
      ++cursor;
    }

    if (cursor == eol || cursor < flags_start + 4) {
      return -1;  // Malformed line.
    }

    ElfW(Ehdr) ehdr;

    if (flags_start[0] == 'r' &&
        ReadFromOffsetExact(mem_fd, &ehdr, sizeof(ElfW(Ehdr)), start_address) &&
        memcmp(ehdr.e_ident, ELFMAG, SELFMAG) == 0) {
      switch (ehdr.e_type) {
        case ET_EXEC:
          base_address = 0;
          break;
        case ET_DYN:








          base_address = start_address;
          for (unsigned i = 0; i != ehdr.e_phnum; ++i) {
            ElfW(Phdr) phdr;
            if (ReadFromOffsetExact(
                    mem_fd, &phdr, sizeof(phdr),
                    start_address + ehdr.e_phoff + i * sizeof(phdr)) &&
                phdr.p_type == PT_LOAD && phdr.p_offset == 0) {
              base_address = start_address - phdr.p_vaddr;
              break;
            }
          }
          break;
        default:


          break;
      }
    }

    if (!(start_address <= pc && pc < end_address)) {
      continue;  // We skip this map.  PC isn't in this map.
    }

    if (flags_start[0] != 'r' || flags_start[2] != 'x') {
      continue;  // We skip this map.
    }
    ++cursor;  // Skip ' '.

    uint64_t file_offset;
    cursor = GetHex(cursor, eol, &file_offset);
    if (cursor == eol || *cursor != ' ') {
      return -1;  // Malformed line.
    }
    ++cursor;  // Skip ' '.


    int num_spaces = 0;
    while (cursor < eol) {
      if (*cursor == ' ') {
        ++num_spaces;
      } else if (num_spaces >= 2) {


        break;
      }
      ++cursor;
    }
    if (cursor == eol) {
      return -1;  // Malformed line.
    }

    NO_INTR(object_fd = open(cursor, O_RDONLY));
    if (object_fd < 0) {


      strncpy(out_file_name, cursor, out_file_name_size);

      out_file_name[out_file_name_size - 1] = '\0';
      return -1;
    }
    return object_fd;
  }
}

// an integer to ASCII. We'll have to define our own version.
// itoa_r() converts a (signed) integer to ASCII. It returns "buf", if the
// conversion was successful or NULL otherwise. It never writes more than "sz"
// bytes. Output will be truncated as needed, and a NUL character is always
// appended.
// NOTE: code from sandbox/linux/seccomp-bpf/demo.cc.
static char *itoa_r(intptr_t i, char *buf, size_t sz, int base, size_t padding) {

  size_t n = 1;
  if (n > sz)
    return NULL;

  if (base < 2 || base > 16) {
    buf[0] = '\000';
    return NULL;
  }

  char *start = buf;

  uintptr_t j = i;

  if (i < 0 && base == 10) {

    j = static_cast<uintptr_t>(-(i + 1)) + 1;

    if (++n > sz) {
      buf[0] = '\000';
      return NULL;
    }
    *start++ = '-';
  }


  char *ptr = start;
  do {

    if (++n > sz) {
      buf[0] = '\000';
      return NULL;
    }

    *ptr++ = "0123456789abcdef"[j % base];
    j /= base;

    if (padding > 0)
      padding--;
  } while (j > 0 || padding > 0);

  *ptr = '\000';




  while (--ptr > start) {
    char ch = *ptr;
    *ptr = *start;
    *start++ = ch;
  }
  return buf;
}

// buffer size |dest_size| and guarantees that |dest| is null-terminated.
static void SafeAppendString(const char* source, char* dest, int dest_size) {
  int dest_string_length = strlen(dest);
  SAFE_ASSERT(dest_string_length < dest_size);
  dest += dest_string_length;
  dest_size -= dest_string_length;
  strncpy(dest, source, dest_size);

  dest[dest_size - 1] = '\0';
}

// Never writes past the buffer size |dest_size| and guarantees that |dest| is
// null-terminated.
static void SafeAppendHexNumber(uint64_t value, char* dest, int dest_size) {

  char buf[17] = {'\0'};
  SafeAppendString(itoa_r(value, buf, sizeof(buf), 16, 0), dest, dest_size);
}

// successfully finds the symbol containing "pc" and obtains the
// symbol name, returns true and write the symbol name to "out".
// Otherwise, returns false. If Callback function is installed via
// InstallSymbolizeCallback(), the function is also called in this function,
// and "out" is used as its output.
// To keep stack consumption low, we would like this function to not
// get inlined.
static ATTRIBUTE_NOINLINE bool SymbolizeAndDemangle(void *pc, char *out,
                                                    int out_size) {
  uint64_t pc0 = reinterpret_cast<uintptr_t>(pc);
  uint64_t start_address = 0;
  uint64_t base_address = 0;
  int object_fd = -1;

  if (out_size < 1) {
    return false;
  }
  out[0] = '\0';
  SafeAppendString("(", out, out_size);

  if (g_symbolize_open_object_file_callback) {
    object_fd = g_symbolize_open_object_file_callback(pc0, start_address,
                                                      base_address, out + 1,
                                                      out_size - 1);
  } else {
    object_fd = OpenObjectFileContainingPcAndGetStartAddress(pc0, start_address,
                                                             base_address,
                                                             out + 1,
                                                             out_size - 1);
  }

  FileDescriptor wrapped_object_fd(object_fd);

#if defined(PRINT_UNSYMBOLIZED_STACK_TRACES)
  {
#else

  if (object_fd < 0) {
#endif
    if (out[1]) {




      out[out_size - 1] = '\0';  // Making sure |out| is always null-terminated.
      SafeAppendString("+0x", out, out_size);
      SafeAppendHexNumber(pc0 - base_address, out, out_size);
      SafeAppendString(")", out, out_size);
      return true;
    }

    return false;
  }
  int elf_type = FileGetElfType(wrapped_object_fd.get());
  if (elf_type == -1) {
    return false;
  }
  if (g_symbolize_callback) {



    uint64_t relocation = (elf_type == ET_DYN) ? start_address : 0;
    int num_bytes_written = g_symbolize_callback(wrapped_object_fd.get(),
                                                 pc, out, out_size,
                                                 relocation);
    if (num_bytes_written > 0) {
      out += num_bytes_written;
      out_size -= num_bytes_written;
    }
  }
  if (!GetSymbolFromObjectFile(wrapped_object_fd.get(), pc0,
                               out, out_size, base_address)) {
    if (out[1] && !g_symbolize_callback) {




      out[out_size - 1] = '\0';  // Making sure |out| is always null-terminated.
      SafeAppendString("+0x", out, out_size);
      SafeAppendHexNumber(pc0 - base_address, out, out_size);
      SafeAppendString(")", out, out_size);
      return true;
    }
    return false;
  }

  DemangleInplace(out, out_size);
  return true;
}

_END_GOOGLE_NAMESPACE_

#elif defined(OS_MACOSX) && defined(HAVE_DLADDR)

#include <dlfcn.h>
#include <string.h>

_START_GOOGLE_NAMESPACE_

static ATTRIBUTE_NOINLINE bool SymbolizeAndDemangle(void *pc, char *out,
                                                    int out_size) {
  Dl_info info;
  if (dladdr(pc, &info)) {
    if ((int)strlen(info.dli_sname) < out_size) {
      strcpy(out, info.dli_sname);

      DemangleInplace(out, out_size);
      return true;
    }
  }
  return false;
}

_END_GOOGLE_NAMESPACE_

#elif defined(OS_WINDOWS) || defined(OS_CYGWIN)

#include <windows.h>
#include <dbghelp.h>

#ifdef _MSC_VER
#pragma comment(lib, "dbghelp")
#endif

_START_GOOGLE_NAMESPACE_

class SymInitializer {
public:
  HANDLE process;
  bool ready;
  SymInitializer() : process(NULL), ready(false) {


    process = GetCurrentProcess();



    SymSetOptions(SYMOPT_DEFERRED_LOADS);
    if (SymInitialize(process, NULL, true)) {
      ready = true;
    }
  }
  ~SymInitializer() {
    SymCleanup(process);

  }
private:
  SymInitializer(const SymInitializer&);
  SymInitializer& operator=(const SymInitializer&);
};

static ATTRIBUTE_NOINLINE bool SymbolizeAndDemangle(void *pc, char *out,
                                                      int out_size) {
  const static SymInitializer symInitializer;
  if (!symInitializer.ready) {
    return false;
  }


  char buf[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
  SYMBOL_INFO *symbol = reinterpret_cast<SYMBOL_INFO *>(buf);
  symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
  symbol->MaxNameLen = MAX_SYM_NAME;


  BOOL ret = SymFromAddr(symInitializer.process,
                         reinterpret_cast<DWORD64>(pc), 0, symbol);
  if (ret == 1 && static_cast<int>(symbol->NameLen) < out_size) {

    strncpy(out, symbol->Name, static_cast<size_t>(symbol->NameLen) + 1);
    out[static_cast<size_t>(symbol->NameLen)] = '\0';

    DemangleInplace(out, out_size);
    return true;
  }
  return false;
}

_END_GOOGLE_NAMESPACE_

#else
# error BUG: HAVE_SYMBOLIZE was wrongly set
#endif

_START_GOOGLE_NAMESPACE_

bool Symbolize(void *pc, char *out, int out_size) {
  SAFE_ASSERT(out_size >= 0);
  return SymbolizeAndDemangle(pc, out, out_size);
}

_END_GOOGLE_NAMESPACE_

#else  /* HAVE_SYMBOLIZE */

#include <assert.h>

#include "config.h"

_START_GOOGLE_NAMESPACE_

bool Symbolize(void *pc, char *out, int out_size) {
  assert(0);
  return false;
}

_END_GOOGLE_NAMESPACE_

#endif
