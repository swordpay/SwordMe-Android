// -*- mode: C++ -*-

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


// it's handler interfaces.
// The DWARF2/3 specification can be found at
// http://dwarf.freestandards.org and should be considered required
// reading if you wish to modify the implementation.
// Only a cursory attempt is made to explain terminology that is
// used here, as it is much better explained in the standard documents
#ifndef COMMON_DWARF_DWARF2READER_H__
#define COMMON_DWARF_DWARF2READER_H__

#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "common/dwarf/bytereader.h"
#include "common/dwarf/dwarf2enums.h"
#include "common/dwarf/types.h"
#include "common/using_std_string.h"

namespace dwarf2reader {
struct LineStateMachine;
class Dwarf2Handler;
class LineInfoHandler;

// the data for the section, and the size of the section.
typedef std::map<string, std::pair<const char*, uint64> > SectionMap;
typedef std::list<std::pair<enum DwarfAttribute, enum DwarfForm> >
    AttributeList;
typedef AttributeList::iterator AttributeIterator;
typedef AttributeList::const_iterator ConstAttributeIterator;

struct LineInfoHeader {
  uint64 total_length;
  uint16 version;
  uint64 prologue_length;
  uint8 min_insn_length; // insn stands for instructin
  bool default_is_stmt; // stmt stands for statement
  int8 line_base;
  uint8 line_range;
  uint8 opcode_base;


  std::vector<unsigned char> *std_opcode_lengths;
};

class LineInfo {
 public:




  LineInfo(const char* buffer_, uint64 buffer_length,
           ByteReader* reader, LineInfoHandler* handler);

  virtual ~LineInfo() {
    if (header_.std_opcode_lengths) {
      delete header_.std_opcode_lengths;
    }
  }



  uint64 Start();









  static bool ProcessOneOpcode(ByteReader* reader,
                               LineInfoHandler* handler,
                               const struct LineInfoHeader &header,
                               const char* start,
                               struct LineStateMachine* lsm,
                               size_t* len,
                               uintptr pc,
                               bool *lsm_passes_pc);

 private:

  void ReadHeader();

  void ReadLines();

  LineInfoHandler* handler_;

  ByteReader* reader_;




  struct LineInfoHeader header_;



  const char* buffer_;
  uint64 buffer_length_;
  const char* after_header_;
};

// the client.  The virtual functions inside this get called for
// interesting events that happen during line info reading.  The
// default implementation does nothing

class LineInfoHandler {
 public:
  LineInfoHandler() { }

  virtual ~LineInfoHandler() { }


  virtual void DefineDir(const string& name, uint32 dir_num) { }







  virtual void DefineFile(const string& name, int32 file_num,
                          uint32 dir_num, uint64 mod_time,
                          uint64 length) { }






  virtual void AddLine(uint64 address, uint64 length,
                       uint32 file_num, uint32 line_num, uint32 column_num) { }
};

// Entry.
// DWARF groups DIE's into a tree and calls the root of this tree a
// "compilation unit".  Most of the time, there is one compilation
// unit in the .debug_info section for each file that had debug info
// generated.
// Each DIE consists of

// DW_TAG_subprogram for functions, DW_TAG_variable for variables, etc
// 2. attributes (such as DW_AT_location for location in memory,
// DW_AT_name for name), and data for each attribute.
// 3. A flag saying whether the DIE has children or not

// each DIE (tag name, attributes and data forms for the attributes)
// are stored in a separate table called the "abbreviation table".
// This is done because a large number of DIEs have the exact same tag
// and list of attributes, but different data for those attributes.
// As a result, the .debug_info section is just a stream of data, and
// requires reading of the .debug_abbrev section to say what the data
// means.

// using absolute offsets from the beginning of .debug_info is that
// DWARF2/3 supports referencing DIE's from other DIE's by their offset
// from either the current compilation unit start, *or* the beginning
// of the .debug_info section.  This means it is possible to reference
// a DIE in one compilation unit from a DIE in another compilation
// unit.  This style of reference is usually used to eliminate
// duplicated information that occurs across compilation
// units, such as base types, etc.  GCC 3.4+ support this with
// -feliminate-dwarf2-dups.  Other toolchains will sometimes do
// duplicate elimination in the linker.

class CompilationUnit {
 public:



  CompilationUnit(const SectionMap& sections, uint64 offset,
                  ByteReader* reader, Dwarf2Handler* handler);
  virtual ~CompilationUnit() {
    if (abbrevs_) delete abbrevs_;
  }






  uint64 Start();

 private:



  struct Abbrev {
    uint64 number;
    enum DwarfTag tag;
    bool has_children;
    AttributeList attributes;
  };



  struct CompilationUnitHeader {
    uint64 length;
    uint16 version;
    uint64 abbrev_offset;
    uint8 address_size;
  } header_;

  void ReadHeader();

  void ReadAbbrevs();


  const char* ProcessDIE(uint64 dieoffset,
                                  const char* start,
                                  const Abbrev& abbrev);


  const char* ProcessAttribute(uint64 dieoffset,
                                        const char* start,
                                        enum DwarfAttribute attr,
                                        enum DwarfForm form);

  void ProcessDIEs();


  const char* SkipDIE(const char* start,
                               const Abbrev& abbrev);


  const char* SkipAttribute(const char* start,
                                     enum DwarfForm form);


  uint64 offset_from_section_start_;



  const char* buffer_;
  uint64 buffer_length_;
  const char* after_header_;

  ByteReader* reader_;

  const SectionMap& sections_;

  Dwarf2Handler* handler_;



  std::vector<Abbrev>* abbrevs_;



  const char* string_buffer_;
  uint64 string_buffer_length_;
};

// client.  The virtual functions inside this get called for
// interesting events that happen during DWARF2 reading.
// The default implementation skips everything.

class Dwarf2Handler {
 public:
  Dwarf2Handler() { }

  virtual ~Dwarf2Handler() { }



  virtual bool StartCompilationUnit(uint64 offset, uint8 address_size,
                                    uint8 offset_size, uint64 cu_length,
                                    uint8 dwarf_version) { return false; }


  virtual bool StartDIE(uint64 offset, enum DwarfTag tag) { return false; }




  virtual void ProcessAttributeUnsigned(uint64 offset,
                                        enum DwarfAttribute attr,
                                        enum DwarfForm form,
                                        uint64 data) { }




  virtual void ProcessAttributeSigned(uint64 offset,
                                      enum DwarfAttribute attr,
                                      enum DwarfForm form,
                                      int64 data) { }





  virtual void ProcessAttributeReference(uint64 offset,
                                         enum DwarfAttribute attr,
                                         enum DwarfForm form,
                                         uint64 data) { }






  virtual void ProcessAttributeBuffer(uint64 offset,
                                      enum DwarfAttribute attr,
                                      enum DwarfForm form,
                                      const char* data,
                                      uint64 len) { }




  virtual void ProcessAttributeString(uint64 offset,
                                      enum DwarfAttribute attr,
                                      enum DwarfForm form,
                                      const string& data) { }




  virtual void ProcessAttributeSignature(uint64 offset,
                                         enum DwarfAttribute attr,
                                         enum DwarfForm form,
                                         uint64 signature) { }




  virtual void EndDIE(uint64 offset) { }

};

// describes how to unwind stack frames --- even for functions that do
// not follow fixed conventions for saving registers, whose frame size
// varies as they execute, etc.
//
// CFI describes, at each machine instruction, how to compute the
// stack frame's base address, how to find the return address, and
// where to find the saved values of the caller's registers (if the
// callee has stashed them somewhere to free up the registers for its
// own use).
//
// For example, suppose we have a function whose machine code looks
// like this (imagine an assembly language that looks like C, for a
// machine with 32-bit registers, and a stack that grows towards lower
// addresses):
//
// func:                                ; entry point; return address at sp
// func+0:      sp = sp - 16            ; allocate space for stack frame
// func+1:      sp[12] = r0             ; save r0 at sp+12
// ...                                  ; other code, not frame-related
// func+10:     sp -= 4; *sp = x        ; push some x on the stack
// ...                                  ; other code, not frame-related
// func+20:     r0 = sp[16]             ; restore saved r0
// func+21:     sp += 20                ; pop whole stack frame
// func+22:     pc = *sp; sp += 4       ; pop return address and jump to it
//
// DWARF CFI is (a very compressed representation of) a table with a
// row for each machine instruction address and a column for each
// register showing how to restore it, if possible.
//
// A special column named "CFA", for "Canonical Frame Address", tells how
// to compute the base address of the frame; registers' entries may
// refer to the CFA in describing where the registers are saved.
//
// Another special column, named "RA", represents the return address.
//
// For example, here is a complete (uncompressed) table describing the
// function above:
// 
//     insn      cfa    r0      r1 ...  ra
//     =======================================
//     func+0:   sp                     cfa[0]
//     func+1:   sp+16                  cfa[0] 
//     func+2:   sp+16  cfa[-4]         cfa[0]
//     func+11:  sp+20  cfa[-4]         cfa[0]
//     func+21:  sp+20                  cfa[0]
//     func+22:  sp                     cfa[0]
//
// Some things to note here:
//
// - Each row describes the state of affairs *before* executing the
//   instruction at the given address.  Thus, the row for func+0
//   describes the state before we allocate the stack frame.  In the
//   next row, the formula for computing the CFA has changed,
//   reflecting that allocation.
//
// - The other entries are written in terms of the CFA; this allows
//   them to remain unchanged as the stack pointer gets bumped around.
//   For example, the rule for recovering the return address (the "ra"
//   column) remains unchanged throughout the function, even as the
//   stack pointer takes on three different offsets from the return
//   address.
//
// - Although we haven't shown it, most calling conventions designate
//   "callee-saves" and "caller-saves" registers. The callee must
//   preserve the values of callee-saves registers; if it uses them,
//   it must save their original values somewhere, and restore them
//   before it returns. In contrast, the callee is free to trash
//   caller-saves registers; if the callee uses these, it will
//   probably not bother to save them anywhere, and the CFI will
//   probably mark their values as "unrecoverable".
//
//   (However, since the caller cannot assume the callee was going to
//   save them, caller-saves registers are probably dead in the caller
//   anyway, so compilers usually don't generate CFA for caller-saves
//   registers.)
// 
// - Exactly where the CFA points is a matter of convention that
//   depends on the architecture and ABI in use. In the example, the
//   CFA is the value the stack pointer had upon entry to the
//   function, pointing at the saved return address. But on the x86,
//   the call frame information generated by GCC follows the
//   convention that the CFA is the address *after* the saved return
//   address.
//
//   But by definition, the CFA remains constant throughout the
//   lifetime of the frame. This makes it a useful value for other
//   columns to refer to. It is also gives debuggers a useful handle
//   for identifying a frame.
//
// If you look at the table above, you'll notice that a given entry is
// often the same as the one immediately above it: most instructions
// change only one or two aspects of the stack frame, if they affect
// it at all. The DWARF format takes advantage of this fact, and
// reduces the size of the data by mentioning only the addresses and
// columns at which changes take place. So for the above, DWARF CFI
// data would only actually mention the following:
// 
//     insn      cfa    r0      r1 ...  ra
//     =======================================
//     func+0:   sp                     cfa[0]
//     func+1:   sp+16
//     func+2:          cfa[-4]
//     func+11:  sp+20
//     func+21:         r0
//     func+22:  sp            
//
// In fact, this is the way the parser reports CFI to the consumer: as
// a series of statements of the form, "At address X, column Y changed
// to Z," and related conventions for describing the initial state.
//
// Naturally, it would be impractical to have to scan the entire
// program's CFI, noting changes as we go, just to recover the
// unwinding rules in effect at one particular instruction. To avoid
// this, CFI data is grouped into "entries", each of which covers a
// specified range of addresses and begins with a complete statement
// of the rules for all recoverable registers at that starting
// address. Each entry typically covers a single function.
//
// Thus, to compute the contents of a given row of the table --- that
// is, rules for recovering the CFA, RA, and registers at a given
// instruction --- the consumer should find the entry that covers that
// instruction's address, start with the initial state supplied at the
// beginning of the entry, and work forward until it has processed all
// the changes up to and including those for the present instruction.
//
// There are seven kinds of rules that can appear in an entry of the
// table:
//
// - "undefined": The given register is not preserved by the callee;
//   its value cannot be recovered.
//
// - "same value": This register has the same value it did in the callee.
//
// - offset(N): The register is saved at offset N from the CFA.
//
// - val_offset(N): The value the register had in the caller is the
//   CFA plus offset N. (This is usually only useful for describing
//   the stack pointer.)
//
// - register(R): The register's value was saved in another register R.
//
// - expression(E): Evaluating the DWARF expression E using the
//   current frame's registers' values yields the address at which the
//   register was saved.
//
// - val_expression(E): Evaluating the DWARF expression E using the
//   current frame's registers' values yields the value the register
//   had in the caller.

class CallFrameInfo {
 public:


  enum EntryKind { kUnknown, kCIE, kFDE, kTerminator };


  class Handler;


  class Reporter;































































  CallFrameInfo(const char *buffer, size_t buffer_length,
                ByteReader *reader, Handler *handler, Reporter *reporter,
                bool eh_frame = false)
      : buffer_(buffer), buffer_length_(buffer_length),
        reader_(reader), handler_(handler), reporter_(reporter),
        eh_frame_(eh_frame) { }

  ~CallFrameInfo() { }



  bool Start();

  static const char *KindName(EntryKind kind);

 private:

  struct CIE;

  struct Entry {


    size_t offset;

    const char *start;





    EntryKind kind;


    const char *fields;

    const char *instructions;




    const char *end;


    uint64 id;


    CIE *cie;
  };

  struct CIE: public Entry {
    uint8 version;                      // CFI data version number
    string augmentation;                // vendor format extension markers
    uint64 code_alignment_factor;       // scale for code address adjustments 
    int data_alignment_factor;          // scale for stack pointer adjustments
    unsigned return_address_register;   // which register holds the return addr

    bool has_z_augmentation;


    bool has_z_lsda;                    // The 'z' augmentation included 'L'.
    bool has_z_personality;             // The 'z' augmentation included 'P'.
    bool has_z_signal_frame;            // The 'z' augmentation included 'S'.


    DwarfPointerEncoding lsda_encoding;


    DwarfPointerEncoding personality_encoding;



    uint64 personality_address;




    DwarfPointerEncoding pointer_encoding;
  };

  struct FDE: public Entry {
    uint64 address;                     // start address of described code
    uint64 size;                        // size of described code, in bytes



    uint64 lsda_address;
  };

  class Rule;
  class UndefinedRule;
  class SameValueRule;
  class OffsetRule;
  class ValOffsetRule;
  class RegisterRule;
  class ExpressionRule;
  class ValExpressionRule;
  class RuleMap;
  class State;






  bool ReadEntryPrologue(const char *cursor, Entry *entry);





  bool ReadCIEFields(CIE *cie);






  bool ReadFDEFields(FDE *fde);



  bool ReportIncomplete(Entry *entry);

  static bool IsIndirectEncoding(DwarfPointerEncoding encoding) {
    return encoding & DW_EH_PE_indirect;
  }

  const char *buffer_;
  size_t buffer_length_;

  ByteReader *reader_;

  Handler *handler_;

  Reporter *reporter_;

  bool eh_frame_;
};

// member functions of a handler object to report the data it finds.
class CallFrameInfo::Handler {
 public:

  enum { kCFARegister = -1 };

  Handler() { }
  virtual ~Handler() { }
















  virtual bool Entry(size_t offset, uint64 address, uint64 length,
                     uint8 version, const string &augmentation,
                     unsigned return_address) = 0;


















  virtual bool UndefinedRule(uint64 address, int reg) = 0;


  virtual bool SameValueRule(uint64 address, int reg) = 0;


  virtual bool OffsetRule(uint64 address, int reg,
                          int base_register, long offset) = 0;



  virtual bool ValOffsetRule(uint64 address, int reg,
                             int base_register, long offset) = 0;





  virtual bool RegisterRule(uint64 address, int reg, int base_register) = 0;


  virtual bool ExpressionRule(uint64 address, int reg,
                              const string &expression) = 0;



  virtual bool ValExpressionRule(uint64 address, int reg,
                                 const string &expression) = 0;




  virtual bool End() = 0;






























  virtual bool PersonalityRoutine(uint64 address, bool indirect) {
    return true;
  }





  virtual bool LanguageSpecificDataArea(uint64 address, bool indirect) {
    return true;
  }








  virtual bool SignalHandler() { return true; }
};

// report errors or warn about problems in the data it is parsing. The
// default definitions of these methods print a message to stderr, but
// you can make a derived class that overrides them.
class CallFrameInfo::Reporter {
 public:







  Reporter(const string &filename,
           const string &section = ".debug_frame")
      : filename_(filename), section_(section) { }
  virtual ~Reporter() { }



  virtual void Incomplete(uint64 offset, CallFrameInfo::EntryKind kind);




  virtual void EarlyEHTerminator(uint64 offset);


  virtual void CIEPointerOutOfRange(uint64 offset, uint64 cie_offset);


  virtual void BadCIEId(uint64 offset, uint64 cie_offset);



  virtual void UnrecognizedVersion(uint64 offset, int version);



  virtual void UnrecognizedAugmentation(uint64 offset,
                                        const string &augmentation);


  virtual void InvalidPointerEncoding(uint64 offset, uint8 encoding);


  virtual void UnusablePointerEncoding(uint64 offset, uint8 encoding);


  virtual void RestoreInCIE(uint64 offset, uint64 insn_offset);


  virtual void BadInstruction(uint64 offset, CallFrameInfo::EntryKind kind,
                              uint64 insn_offset);



  virtual void NoCFARule(uint64 offset, CallFrameInfo::EntryKind kind, 
                         uint64 insn_offset);



  virtual void EmptyStateStack(uint64 offset, CallFrameInfo::EntryKind kind, 
                               uint64 insn_offset);





  virtual void ClearingCFARule(uint64 offset, CallFrameInfo::EntryKind kind, 
                               uint64 insn_offset);

 protected:

  string filename_;

  string section_;
};

}  // namespace dwarf2reader

#endif  // UTIL_DEBUGINFO_DWARF2READER_H__
