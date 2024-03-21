// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// http://code.google.com/p/protobuf/
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

//  Based on original Protocol Buffers design by
//  Sanjay Ghemawat, Jeff Dean, and others.
//
// Implements the Protocol Compiler front-end such that it may be reused by
// custom compilers written to support other languages.

#ifndef GOOGLE_PROTOBUF_COMPILER_COMMAND_LINE_INTERFACE_H__
#define GOOGLE_PROTOBUF_COMPILER_COMMAND_LINE_INTERFACE_H__

#include <google/protobuf/stubs/common.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>

namespace google {
namespace protobuf {

class FileDescriptor;        // descriptor.h
class DescriptorPool;        // descriptor.h
class FileDescriptorProto;   // descriptor.pb.h
template<typename T> class RepeatedPtrField;  // repeated_field.h

namespace compiler {

class CodeGenerator;        // code_generator.h
class GeneratorContext;      // code_generator.h
class DiskSourceTree;       // importer.h

// It is designed to make it very easy to create a custom protocol compiler
// supporting the languages of your choice.  For example, if you wanted to
// create a custom protocol compiler binary which includes both the regular
// C++ support plus support for your own custom output "Foo", you would
// write a class "FooGenerator" which implements the CodeGenerator interface,
// then write a main() procedure like this:
//
//   int main(int argc, char* argv[]) {
//     google::protobuf::compiler::CommandLineInterface cli;
//
//     // Support generation of C++ source and headers.
//     google::protobuf::compiler::cpp::CppGenerator cpp_generator;
//     cli.RegisterGenerator("--cpp_out", &cpp_generator,
//       "Generate C++ source and header.");
//
//     // Support generation of Foo code.
//     FooGenerator foo_generator;
//     cli.RegisterGenerator("--foo_out", &foo_generator,
//       "Generate Foo file.");
//
//     return cli.Run(argc, argv);
//   }
//
// The compiler is invoked with syntax like:
//   protoc --cpp_out=outdir --foo_out=outdir --proto_path=src src/foo.proto
//
// For a full description of the command-line syntax, invoke it with --help.
class LIBPROTOC_EXPORT CommandLineInterface {
 public:
  CommandLineInterface();
  ~CommandLineInterface();
















  void RegisterGenerator(const string& flag_name,
                         CodeGenerator* generator,
                         const string& help_text);





























  void AllowPlugins(const string& exe_name_prefix);





  int Run(int argc, const char* const argv[]);








  void SetInputsAreProtoPathRelative(bool enable) {
    inputs_are_proto_path_relative_ = enable;
  }



  void SetVersionInfo(const string& text) {
    version_info_ = text;
  }


 private:


  class ErrorPrinter;
  class GeneratorContextImpl;
  class MemoryOutputStream;

  void Clear();



  bool MakeInputsBeProtoPathRelative(
    DiskSourceTree* source_tree);

  bool ParseArguments(int argc, const char* const argv[]);











  bool ParseArgument(const char* arg, string* name, string* value);

  bool InterpretArgument(const string& name, const string& value);

  void PrintHelpText();

  struct OutputDirective;  // see below
  bool GenerateOutput(const vector<const FileDescriptor*>& parsed_files,
                      const OutputDirective& output_directive,
                      GeneratorContext* generator_context);
  bool GeneratePluginOutput(const vector<const FileDescriptor*>& parsed_files,
                            const string& plugin_name,
                            const string& parameter,
                            GeneratorContext* generator_context,
                            string* error);

  bool EncodeOrDecode(const DescriptorPool* pool);

  bool WriteDescriptorSet(const vector<const FileDescriptor*> parsed_files);






  static void GetTransitiveDependencies(
      const FileDescriptor* file,
      set<const FileDescriptor*>* already_seen,
      RepeatedPtrField<FileDescriptorProto>* output);


  string executable_name_;

  string version_info_;

  struct GeneratorInfo {
    CodeGenerator* generator;
    string help_text;
  };
  typedef map<string, GeneratorInfo> GeneratorMap;
  GeneratorMap generators_;

  string plugin_prefix_;



  map<string, string> plugins_;

  enum Mode {
    MODE_COMPILE,  // Normal mode:  parse .proto files and compile them.
    MODE_ENCODE,   // --encode:  read text from stdin, write binary to stdout.
    MODE_DECODE    // --decode:  read binary from stdin, write text to stdout.
  };

  Mode mode_;

  enum ErrorFormat {
    ERROR_FORMAT_GCC,   // GCC error output format (default).
    ERROR_FORMAT_MSVS   // Visual Studio output (--error_format=msvs).
  };

  ErrorFormat error_format_;

  vector<pair<string, string> > proto_path_;  // Search path for proto files.
  vector<string> input_files_;                // Names of the input proto files.


  struct OutputDirective {
    string name;                // E.g. "--foo_out"
    CodeGenerator* generator;   // NULL for plugins
    string parameter;
    string output_location;
  };
  vector<OutputDirective> output_directives_;


  string codec_type_;


  string descriptor_set_name_;



  bool imports_in_descriptor_set_;

  bool disallow_services_;

  bool inputs_are_proto_path_relative_;

  GOOGLE_DISALLOW_EVIL_CONSTRUCTORS(CommandLineInterface);
};

}  // namespace compiler
}  // namespace protobuf

}  // namespace google
#endif  // GOOGLE_PROTOBUF_COMPILER_COMMAND_LINE_INTERFACE_H__
