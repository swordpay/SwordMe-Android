// -*- mode: C++ -*-

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



// all combinations of host system and minidump processor
// architecture, we need to be able to easily generate complex test
// data like debugging information and minidump files.
// 
// For example, if we want our unit tests to provide full code
// coverage for stack walking, it may be difficult to persuade the
// compiler to generate every possible sort of stack walking
// information that we want to support; there are probably DWARF CFI
// opcodes that GCC never emits. Similarly, if we want to test our
// error handling, we will need to generate damaged minidumps or
// debugging information that (we hope) the client or compiler will
// never produce on its own.
//
// google_breakpad::TestAssembler provides a predictable and
// (relatively) simple way to generate complex formatted data streams
// like minidumps and CFI. Furthermore, because TestAssembler is
// portable, developers without access to (say) Visual Studio or a
// SPARC assembler can still work on test data for those targets.

#ifndef PROCESSOR_TEST_ASSEMBLER_H_
#define PROCESSOR_TEST_ASSEMBLER_H_

#include <list>
#include <vector>
#include <string>

#include "common/using_std_string.h"
#include "google_breakpad/common/breakpad_types.h"

namespace google_breakpad {

using std::list;
using std::vector;

namespace test_assembler {

// section. As long as all the labels a section refers to are defined
// by the time we retrieve its contents as bytes, we can use undefined
// labels freely in that section's construction.
//
// A label can be in one of three states:
// - undefined,
// - defined as the sum of some other label and a constant, or
// - a constant.
// 
// A label's value never changes, but it can accumulate constraints.
// Adding labels and integers is permitted, and yields a label.
// Subtracting a constant from a label is permitted, and also yields a
// label. Subtracting two labels that have some relationship to each
// other is permitted, and yields a constant.
//
// For example:
//
//   Label a;               // a's value is undefined
//   Label b;               // b's value is undefined
//   {
//     Label c = a + 4;     // okay, even though a's value is unknown
//     b = c + 4;           // also okay; b is now a+8
//   }
//   Label d = b - 2;       // okay; d == a+6, even though c is gone
//   d.Value();             // error: d's value is not yet known
//   d - a;                 // is 6, even though their values are not known
//   a = 12;                // now b == 20, and d == 18
//   d.Value();             // 18: no longer an error
//   b.Value();             // 20
//   d = 10;                // error: d is already defined.
//
// Label objects' lifetimes are unconstrained: notice that, in the
// above example, even though a and b are only related through c, and
// c goes out of scope, the assignment to a sets b's value as well. In
// particular, it's not necessary to ensure that a Label lives beyond
// Sections that refer to it.
class Label {
 public:
  Label();                      // An undefined label.
  Label(uint64_t value);       // A label with a fixed value
  Label(const Label &value);    // A label equal to another.
  ~Label();








  uint64_t Value() const;

  Label &operator=(uint64_t value);
  Label &operator=(const Label &value);
  Label operator+(uint64_t addend) const;
  Label operator-(uint64_t subtrahend) const;
  uint64_t operator-(const Label &subtrahend) const;




  bool IsKnownConstant(uint64_t *value_p = NULL) const;


















  bool IsKnownOffsetFrom(const Label &label, uint64_t *offset_p = NULL) const;

 private:












  class Binding {
   public:
    Binding();
    Binding(uint64_t addend);
    ~Binding();

    void Acquire() { reference_count_++; };

    bool Release() { return --reference_count_ == 0; }





    void Set(Binding *binding, uint64_t value);










    void Get(Binding **base, uint64_t *addend);

   private:



















    Binding *base_;
    uint64_t addend_;



    int reference_count_;
  };

  Binding *value_;
};

inline Label operator+(uint64_t a, const Label &l) { return l + a; }
// Note that int-Label isn't defined, as negating a Label is not an
// operation we support.

enum Endianness {
  kBigEndian,        // Big-endian: the most significant byte comes first.
  kLittleEndian,     // Little-endian: the least significant byte comes first.
  kUnsetEndian,      // used internally
};

// to the end. Sections have a convenient and flexible set of member
// functions for appending data in various formats: big-endian and
// little-endian signed and unsigned values of different sizes;
// LEB128 and ULEB128 values (see below), and raw blocks of bytes.
//
// If you need to append a value to a section that is not convenient
// to compute immediately, you can create a label, append the
// label's value to the section, and then set the label's value
// later, when it's convenient to do so. Once a label's value is
// known, the section class takes care of updating all previously
// appended references to it.
//
// Once all the labels to which a section refers have had their
// values determined, you can get a copy of the section's contents
// as a string.
//
// Note that there is no specified "start of section" label. This is
// because there are typically several different meanings for "the
// start of a section": the offset of the section within an object
// file, the address in memory at which the section's content appear,
// and so on. It's up to the code that uses the Section class to 
// keep track of these explicitly, as they depend on the application.
class Section {
 public:
  Section(Endianness endianness = kUnsetEndian)
      : endianness_(endianness) { };


  virtual ~Section() { };



  void set_endianness(Endianness endianness) {
    endianness_ = endianness;
  }

  Endianness endianness() const { return endianness_; }


  Section &Append(const uint8_t *data, size_t size) {
    contents_.append(reinterpret_cast<const char *>(data), size);
    return *this;
  };
  Section &Append(const string &data) {
    contents_.append(data);
    return *this;
  };


  Section &Append(size_t size, uint8_t byte) {
    contents_.append(size, (char) byte);
    return *this;
  }



  Section &Append(Endianness endianness, size_t size, uint64_t number);
  Section &Append(Endianness endianness, size_t size, const Label &label);







  Section &Append(const Section &section);


  Section &AppendCString(const string &data) {
    Append(data);
    contents_ += '\0';
    return *this;
  }


  Section &AppendCString(const string &data, size_t size) {
    contents_.append(data, 0, size);
    if (data.size() < size)
      Append(size - data.size(), 0);
    return *this;
  }
















  Section &L8(uint8_t value) { contents_ += value; return *this; }
  Section &B8(uint8_t value) { contents_ += value; return *this; }
  Section &D8(uint8_t value) { contents_ += value; return *this; }
  Section &L16(uint16_t), &L32(uint32_t), &L64(uint64_t),
          &B16(uint16_t), &B32(uint32_t), &B64(uint64_t),
          &D16(uint16_t), &D32(uint32_t), &D64(uint64_t);
  Section &L8(const Label &label),  &L16(const Label &label),
          &L32(const Label &label), &L64(const Label &label),
          &B8(const Label &label),  &B16(const Label &label),
          &B32(const Label &label), &B64(const Label &label),
          &D8(const Label &label),  &D16(const Label &label),
          &D32(const Label &label), &D64(const Label &label);


















  Section &LEB128(long long value);














  Section &ULEB128(uint64_t value);




  Section &Align(size_t alignment, uint8_t pad_byte = 0);

  void Clear();

  size_t Size() const { return contents_.size(); }
















  Label start() const { return start_; }


  Label Here() const { return start_ + Size(); }

  Section &Mark(Label *label) { *label = Here(); return *this; }




  bool GetContents(string *contents);

 private:

  struct Reference {
    Reference(size_t set_offset, Endianness set_endianness,  size_t set_size,
              const Label &set_label)
        : offset(set_offset), endianness(set_endianness), size(set_size),
          label(set_label) { }

    size_t offset;

    Endianness endianness;

    size_t size;

    Label label;
  };

  Endianness endianness_;

  string contents_;

  vector<Reference> references_;

  Label start_;
};

}  // namespace test_assembler
}  // namespace google_breakpad

#endif  // PROCESSOR_TEST_ASSEMBLER_H_
