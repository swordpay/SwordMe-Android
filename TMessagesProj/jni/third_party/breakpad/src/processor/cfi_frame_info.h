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


// set of 'STACK CFI'-derived register recovery rules that apply at a
// given instruction.

#ifndef PROCESSOR_CFI_FRAME_INFO_H_
#define PROCESSOR_CFI_FRAME_INFO_H_

#include <map>
#include <string>

#include "common/using_std_string.h"
#include "google_breakpad/common/breakpad_types.h"

namespace google_breakpad {

using std::map;

class MemoryRegion;

// values, when the PC is at a given address in the current frame's
// function. See the description of 'STACK CFI' records at:
//
// http://code.google.com/p/google-breakpad/wiki/SymbolFiles
//
// To prepare an instance of CFIFrameInfo for use at a given
// instruction, first populate it with the rules from the 'STACK CFI
// INIT' record that covers that instruction, and then apply the
// changes given by the 'STACK CFI' records up to our instruction's
// address. Then, use the FindCallerRegs member function to apply the
// rules to the callee frame's register values, yielding the caller
// frame's register values.
class CFIFrameInfo {
 public:

  template<typename ValueType> class RegisterValueMap: 
    public map<string, ValueType> { };



  void SetCFARule(const string &expression) { cfa_rule_ = expression; }
  void SetRARule(const string &expression)  { ra_rule_ = expression; }
  void SetRegisterRule(const string &register_name, const string &expression) {
    register_rules_[register_name] = expression;
  }

















  template<typename ValueType>
  bool FindCallerRegs(const RegisterValueMap<ValueType> &registers,
                      const MemoryRegion &memory,
                      RegisterValueMap<ValueType> *caller_registers) const;


  string Serialize() const;

 private:

  typedef map<string, string> RuleMap;







  string cfa_rule_;






  string ra_rule_;



  RuleMap register_rules_;
};

// This may seem bureaucratic: there's no legitimate run-time reason
// to use a parser/handler pattern for this, as it's not a likely
// reuse boundary. But doing so makes finer-grained unit testing
// possible.
class CFIRuleParser {
 public:

  class Handler {
   public:
    Handler() { }
    virtual ~Handler() { }

    virtual void CFARule(const string &expression) = 0;
    virtual void RARule(const string &expression) = 0;

    virtual void RegisterRule(const string &name, const string &expression) = 0;
  };

  CFIRuleParser(Handler *handler) : handler_(handler) { }




  bool Parse(const string &rule_set);

 private:

  bool Report();

  Handler *handler_;

  string name_, expression_;
};

// the results.
class CFIFrameInfoParseHandler: public CFIRuleParser::Handler {
 public:

  CFIFrameInfoParseHandler(CFIFrameInfo *frame_info)
      : frame_info_(frame_info) { }

  void CFARule(const string &expression);
  void RARule(const string &expression);
  void RegisterRule(const string &name, const string &expression);

 private:
  CFIFrameInfo *frame_info_;
};

// Given a CFIFrameInfo instance, a table describing the architecture's
// register set, and a context holding the last frame's registers, an
// instance of this class can populate a new context with the caller's
// registers.
//
// This class template doesn't use any internal knowledge of CFIFrameInfo
// or the other stack walking structures; it just uses the public interface
// of CFIFrameInfo to do the usual things. But the logic it handles should
// be common to many different architectures' stack walkers, so wrapping it
// up in a class should allow the walkers to share code.
//
// RegisterType should be the type of this architecture's registers, either
// uint32_t or uint64_t. RawContextType should be the raw context
// structure type for this architecture.
template <typename RegisterType, class RawContextType>
class SimpleCFIWalker {
 public:

  struct RegisterSet {

    const char *name;





    const char *alternate_name;





    bool callee_saves;

    int validity_flag;


    RegisterType RawContextType::*context_member;
  };




  SimpleCFIWalker(const RegisterSet *register_map, size_t map_size)
      : register_map_(register_map), map_size_(map_size) { }













  bool FindCallerRegisters(const MemoryRegion &memory,
                           const CFIFrameInfo &cfi_frame_info,
                           const RawContextType &callee_context,
                           int callee_validity,
                           RawContextType *caller_context,
                           int *caller_validity) const;

 private:
  const RegisterSet *register_map_;
  size_t map_size_;
};

}  // namespace google_breakpad

#include "cfi_frame_info-inl.h"

#endif  // PROCESSOR_CFI_FRAME_INFO_H_
