// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/debug/stack_trace.h"

#include <windows.h>
#include <dbghelp.h>
#include <stddef.h>

#include <algorithm>
#include <iostream>
#include <memory>

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/memory/singleton.h"
#include "base/stl_util.h"
#include "base/strings/string_util.h"
#include "base/strings/stringprintf.h"
#include "base/synchronization/lock.h"
#include "build/build_config.h"

namespace base {
namespace debug {

namespace {

// exception. Only used in unit tests.
LPTOP_LEVEL_EXCEPTION_FILTER g_previous_filter = NULL;

bool g_initialized_symbols = false;
DWORD g_init_error = ERROR_SUCCESS;

// This is the unit tests exception filter.
long WINAPI StackDumpExceptionFilter(EXCEPTION_POINTERS* info) {
  DWORD exc_code = info->ExceptionRecord->ExceptionCode;
  std::cerr << "Received fatal exception ";
  switch (exc_code) {
    case EXCEPTION_ACCESS_VIOLATION:
      std::cerr << "EXCEPTION_ACCESS_VIOLATION";
      break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
      std::cerr << "EXCEPTION_ARRAY_BOUNDS_EXCEEDED";
      break;
    case EXCEPTION_BREAKPOINT:
      std::cerr << "EXCEPTION_BREAKPOINT";
      break;
    case EXCEPTION_DATATYPE_MISALIGNMENT:
      std::cerr << "EXCEPTION_DATATYPE_MISALIGNMENT";
      break;
    case EXCEPTION_FLT_DENORMAL_OPERAND:
      std::cerr << "EXCEPTION_FLT_DENORMAL_OPERAND";
      break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
      std::cerr << "EXCEPTION_FLT_DIVIDE_BY_ZERO";
      break;
    case EXCEPTION_FLT_INEXACT_RESULT:
      std::cerr << "EXCEPTION_FLT_INEXACT_RESULT";
      break;
    case EXCEPTION_FLT_INVALID_OPERATION:
      std::cerr << "EXCEPTION_FLT_INVALID_OPERATION";
      break;
    case EXCEPTION_FLT_OVERFLOW:
      std::cerr << "EXCEPTION_FLT_OVERFLOW";
      break;
    case EXCEPTION_FLT_STACK_CHECK:
      std::cerr << "EXCEPTION_FLT_STACK_CHECK";
      break;
    case EXCEPTION_FLT_UNDERFLOW:
      std::cerr << "EXCEPTION_FLT_UNDERFLOW";
      break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:
      std::cerr << "EXCEPTION_ILLEGAL_INSTRUCTION";
      break;
    case EXCEPTION_IN_PAGE_ERROR:
      std::cerr << "EXCEPTION_IN_PAGE_ERROR";
      break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
      std::cerr << "EXCEPTION_INT_DIVIDE_BY_ZERO";
      break;
    case EXCEPTION_INT_OVERFLOW:
      std::cerr << "EXCEPTION_INT_OVERFLOW";
      break;
    case EXCEPTION_INVALID_DISPOSITION:
      std::cerr << "EXCEPTION_INVALID_DISPOSITION";
      break;
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
      std::cerr << "EXCEPTION_NONCONTINUABLE_EXCEPTION";
      break;
    case EXCEPTION_PRIV_INSTRUCTION:
      std::cerr << "EXCEPTION_PRIV_INSTRUCTION";
      break;
    case EXCEPTION_SINGLE_STEP:
      std::cerr << "EXCEPTION_SINGLE_STEP";
      break;
    case EXCEPTION_STACK_OVERFLOW:
      std::cerr << "EXCEPTION_STACK_OVERFLOW";
      break;
    default:
      std::cerr << "0x" << std::hex << exc_code;
      break;
  }
  std::cerr << "\n";

  debug::StackTrace(info).Print();
  if (g_previous_filter)
    return g_previous_filter(info);
  return EXCEPTION_CONTINUE_SEARCH;
}

FilePath GetExePath() {
  wchar_t system_buffer[MAX_PATH];
  GetModuleFileName(NULL, system_buffer, MAX_PATH);
  system_buffer[MAX_PATH - 1] = L'\0';
  return FilePath(system_buffer);
}

bool InitializeSymbols() {
  if (g_initialized_symbols) {


    SymCleanup(GetCurrentProcess());
    g_initialized_symbols = false;
  }
  g_initialized_symbols = true;


  SymSetOptions(SYMOPT_DEFERRED_LOADS |
                SYMOPT_UNDNAME |
                SYMOPT_LOAD_LINES);
  if (!SymInitialize(GetCurrentProcess(), NULL, TRUE)) {
    g_init_error = GetLastError();





    DLOG(ERROR) << "SymInitialize failed: " << g_init_error;
    return false;
  }




  static constexpr size_t kSymbolsArraySize = 1024;
  wchar_t symbols_path[kSymbolsArraySize];


  if (!SymGetSearchPathW(GetCurrentProcess(), symbols_path,
                         kSymbolsArraySize)) {
    g_init_error = GetLastError();
    DLOG(WARNING) << "SymGetSearchPath failed: " << g_init_error;
    return false;
  }

  std::wstring new_path = StringPrintf(L"%ls;%ls", symbols_path,
                                       GetExePath().DirName().value().c_str());
  if (!SymSetSearchPathW(GetCurrentProcess(), new_path.c_str())) {
    g_init_error = GetLastError();
    DLOG(WARNING) << "SymSetSearchPath failed." << g_init_error;
    return false;
  }

  g_init_error = ERROR_SUCCESS;
  return true;
}

// of functions.  The Sym* family of functions may only be invoked by one
// thread at a time.  SymbolContext code may access a symbol server over the
// network while holding the lock for this singleton.  In the case of high
// latency, this code will adversely affect performance.
//
// There is also a known issue where this backtrace code can interact
// badly with breakpad if breakpad is invoked in a separate thread while
// we are using the Sym* functions.  This is because breakpad does now
// share a lock with this function.  See this related bug:
//
//   https://crbug.com/google-breakpad/311
//
// This is a very unlikely edge case, and the current solution is to
// just ignore it.
class SymbolContext {
 public:
  static SymbolContext* GetInstance() {


    return
      Singleton<SymbolContext, LeakySingletonTraits<SymbolContext> >::get();
  }









  void OutputTraceToStream(const void* const* trace,
                           size_t count,
                           std::ostream* os,
                           const char* prefix_string) {
    AutoLock lock(lock_);

    for (size_t i = 0; (i < count) && os->good(); ++i) {
      const int kMaxNameLength = 256;
      DWORD_PTR frame = reinterpret_cast<DWORD_PTR>(trace[i]);


      ULONG64 buffer[
        (sizeof(SYMBOL_INFO) +
          kMaxNameLength * sizeof(wchar_t) +
          sizeof(ULONG64) - 1) /
        sizeof(ULONG64)];
      memset(buffer, 0, sizeof(buffer));

      DWORD64 sym_displacement = 0;
      PSYMBOL_INFO symbol = reinterpret_cast<PSYMBOL_INFO>(&buffer[0]);
      symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
      symbol->MaxNameLen = kMaxNameLength - 1;
      BOOL has_symbol = SymFromAddr(GetCurrentProcess(), frame,
                                    &sym_displacement, symbol);

      DWORD line_displacement = 0;
      IMAGEHLP_LINE64 line = {};
      line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
      BOOL has_line = SymGetLineFromAddr64(GetCurrentProcess(), frame,
                                           &line_displacement, &line);

      if (prefix_string)
        (*os) << prefix_string;
      (*os) << "\t";
      if (has_symbol) {
        (*os) << symbol->Name << " [0x" << trace[i] << "+"
              << sym_displacement << "]";
      } else {

        (*os) << "(No symbol) [0x" << trace[i] << "]";
      }
      if (has_line) {
        (*os) << " (" << line.FileName << ":" << line.LineNumber << ")";
      }
      (*os) << "\n";
    }
  }

 private:
  friend struct DefaultSingletonTraits<SymbolContext>;

  SymbolContext() {
    InitializeSymbols();
  }

  Lock lock_;
  DISALLOW_COPY_AND_ASSIGN(SymbolContext);
};

}  // namespace

bool EnableInProcessStackDumping() {


  g_previous_filter = SetUnhandledExceptionFilter(&StackDumpExceptionFilter);



  return InitializeSymbols();
}

NOINLINE size_t CollectStackTrace(void** trace, size_t count) {

  return CaptureStackBackTrace(0, count, trace, NULL);
}

StackTrace::StackTrace(EXCEPTION_POINTERS* exception_pointers) {
  InitTrace(exception_pointers->ContextRecord);
}

StackTrace::StackTrace(const CONTEXT* context) {
  InitTrace(context);
}

void StackTrace::InitTrace(const CONTEXT* context_record) {




  CONTEXT context_copy;
  memcpy(&context_copy, context_record, sizeof(context_copy));
  context_copy.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;

  count_ = 0;

  STACKFRAME64 stack_frame;
  memset(&stack_frame, 0, sizeof(stack_frame));
#if defined(ARCH_CPU_X86_64)
  int machine_type = IMAGE_FILE_MACHINE_AMD64;
  stack_frame.AddrPC.Offset = context_record->Rip;
  stack_frame.AddrFrame.Offset = context_record->Rbp;
  stack_frame.AddrStack.Offset = context_record->Rsp;
#elif defined(ARCH_CPU_ARM64)
  int machine_type = IMAGE_FILE_MACHINE_ARM64;
  stack_frame.AddrPC.Offset = context_record->Pc;
  stack_frame.AddrFrame.Offset = context_record->Fp;
  stack_frame.AddrStack.Offset = context_record->Sp;
#elif defined(ARCH_CPU_X86)
  int machine_type = IMAGE_FILE_MACHINE_I386;
  stack_frame.AddrPC.Offset = context_record->Eip;
  stack_frame.AddrFrame.Offset = context_record->Ebp;
  stack_frame.AddrStack.Offset = context_record->Esp;
#else
#error Unsupported Windows Arch
#endif
  stack_frame.AddrPC.Mode = AddrModeFlat;
  stack_frame.AddrFrame.Mode = AddrModeFlat;
  stack_frame.AddrStack.Mode = AddrModeFlat;
  while (StackWalk64(machine_type, GetCurrentProcess(), GetCurrentThread(),
                     &stack_frame, &context_copy, NULL,
                     &SymFunctionTableAccess64, &SymGetModuleBase64, NULL) &&
         count_ < size(trace_)) {
    trace_[count_++] = reinterpret_cast<void*>(stack_frame.AddrPC.Offset);
  }

  for (size_t i = count_; i < size(trace_); ++i)
    trace_[i] = NULL;
}

void StackTrace::PrintWithPrefix(const char* prefix_string) const {
  OutputToStreamWithPrefix(&std::cerr, prefix_string);
}

void StackTrace::OutputToStreamWithPrefix(std::ostream* os,
                                          const char* prefix_string) const {
  SymbolContext* context = SymbolContext::GetInstance();
  if (g_init_error != ERROR_SUCCESS) {
    (*os) << "Error initializing symbols (" << g_init_error
          << ").  Dumping unresolved backtrace:\n";
    for (size_t i = 0; (i < count_) && os->good(); ++i) {
      if (prefix_string)
        (*os) << prefix_string;
      (*os) << "\t" << trace_[i] << "\n";
    }
  } else {
    (*os) << "Backtrace:\n";
    context->OutputTraceToStream(trace_, count_, os, prefix_string);
  }
}

}  // namespace debug
}  // namespace base
