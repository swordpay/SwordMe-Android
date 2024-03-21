// Copyright 2018 The Abseil Authors.
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

#include "absl/debugging/internal/demangle.h"

#include <cstdlib>
#include <string>

#include "gtest/gtest.h"
#include "absl/base/config.h"
#include "absl/base/internal/raw_logging.h"
#include "absl/debugging/internal/stack_consumption.h"
#include "absl/memory/memory.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace debugging_internal {
namespace {

static const char *DemangleIt(const char * const mangled) {
  static char demangled[4096];
  if (Demangle(mangled, demangled, sizeof(demangled))) {
    return demangled;
  } else {
    return mangled;
  }
}

TEST(Demangle, CornerCases) {
  char tmp[10];
  EXPECT_TRUE(Demangle("_Z6foobarv", tmp, sizeof(tmp)));

  EXPECT_STREQ("foobar()", tmp);
  EXPECT_TRUE(Demangle("_Z6foobarv", tmp, 9));
  EXPECT_STREQ("foobar()", tmp);
  EXPECT_FALSE(Demangle("_Z6foobarv", tmp, 8));  // Not enough.
  EXPECT_FALSE(Demangle("_Z6foobarv", tmp, 1));
  EXPECT_FALSE(Demangle("_Z6foobarv", tmp, 0));
  EXPECT_FALSE(Demangle("_Z6foobarv", nullptr, 0));  // Should not cause SEGV.
  EXPECT_FALSE(Demangle("_Z1000000", tmp, 9));
}

// by GCC 4.5.x (and our locally-modified version of GCC 4.4.x), and
// .constprop.N and .isra.N, which are used by GCC 4.6.x.  These
// suffixes are used to indicate functions which have been cloned
// during optimization.  We ignore these suffixes.
TEST(Demangle, Clones) {
  char tmp[20];
  EXPECT_TRUE(Demangle("_ZL3Foov", tmp, sizeof(tmp)));
  EXPECT_STREQ("Foo()", tmp);
  EXPECT_TRUE(Demangle("_ZL3Foov.clone.3", tmp, sizeof(tmp)));
  EXPECT_STREQ("Foo()", tmp);
  EXPECT_TRUE(Demangle("_ZL3Foov.constprop.80", tmp, sizeof(tmp)));
  EXPECT_STREQ("Foo()", tmp);
  EXPECT_TRUE(Demangle("_ZL3Foov.isra.18", tmp, sizeof(tmp)));
  EXPECT_STREQ("Foo()", tmp);
  EXPECT_TRUE(Demangle("_ZL3Foov.isra.2.constprop.18", tmp, sizeof(tmp)));
  EXPECT_STREQ("Foo()", tmp);

  EXPECT_TRUE(Demangle("_ZL3Foov.__uniq.12345", tmp, sizeof(tmp)));
  EXPECT_STREQ("Foo()", tmp);
  EXPECT_TRUE(Demangle("_ZL3Foov.__uniq.12345.isra.2.constprop.18", tmp,
                       sizeof(tmp)));
  EXPECT_STREQ("Foo()", tmp);

  EXPECT_TRUE(Demangle("_ZL3Foov.clo", tmp, sizeof(tmp)));
  EXPECT_STREQ("Foo()", tmp);

  EXPECT_TRUE(Demangle("_ZL3Foov.123", tmp, sizeof(tmp)));
  EXPECT_STREQ("Foo()", tmp);

  EXPECT_TRUE(Demangle("_ZL3Foov.clone.foo", tmp, sizeof(tmp)));
  EXPECT_STREQ("Foo()", tmp);

  EXPECT_TRUE(Demangle("_ZL3Foov.clone.123.456", tmp, sizeof(tmp)));
  EXPECT_STREQ("Foo()", tmp);

  EXPECT_TRUE(Demangle("_ZL3Foov.part.9.165493.constprop.775.31805", tmp,
                       sizeof(tmp)));
  EXPECT_STREQ("Foo()", tmp);

  EXPECT_FALSE(Demangle("_ZL3Foov.", tmp, sizeof(tmp)));

  EXPECT_FALSE(Demangle("_ZL3Foov.abc123", tmp, sizeof(tmp)));

  EXPECT_FALSE(Demangle("_ZL3Foov.clone.", tmp, sizeof(tmp)));

  EXPECT_FALSE(Demangle("_ZL3Foov.isra.2.constprop.", tmp, sizeof(tmp)));
}

TEST(Demangle, AbiTags) {
  char tmp[80];



  EXPECT_TRUE(Demangle("_Z1aB3abc", tmp, sizeof(tmp)));
  EXPECT_STREQ("a[abi:abc]", tmp);





  EXPECT_TRUE(Demangle("_ZN1BC2B3xyzEv", tmp, sizeof(tmp)));
  EXPECT_STREQ("B::B[abi:xyz]()", tmp);


  EXPECT_TRUE(Demangle("_Z1CB3barB3foov", tmp, sizeof(tmp)));
  EXPECT_STREQ("C[abi:bar][abi:foo]()", tmp);
}

// They are not to be run under sanitizers as the sanitizers increase
// stack consumption by about 4x.
#if defined(ABSL_INTERNAL_HAVE_DEBUGGING_STACK_CONSUMPTION) && \
    !defined(ABSL_HAVE_ADDRESS_SANITIZER) &&                   \
    !defined(ABSL_HAVE_MEMORY_SANITIZER) &&                    \
    !defined(ABSL_HAVE_THREAD_SANITIZER)

static const char *g_mangled;
static char g_demangle_buffer[4096];
static char *g_demangle_result;

static void DemangleSignalHandler(int signo) {
  if (Demangle(g_mangled, g_demangle_buffer, sizeof(g_demangle_buffer))) {
    g_demangle_result = g_demangle_buffer;
  } else {
    g_demangle_result = nullptr;
  }
}

static const char *DemangleStackConsumption(const char *mangled,
                                            int *stack_consumed) {
  g_mangled = mangled;
  *stack_consumed = GetSignalHandlerStackConsumption(DemangleSignalHandler);
  ABSL_RAW_LOG(INFO, "Stack consumption of Demangle: %d", *stack_consumed);
  return g_demangle_result;
}

// with some level of nesting. With alternate signal stack we have 64K,
// but some signal handlers run on thread stack, and could have arbitrarily
// little space left (so we don't want to make this number too large).
const int kStackConsumptionUpperLimit = 8192;

static std::string NestedMangledName(int depth) {
  std::string mangled_name = "_Z1a";
  if (depth > 0) {
    mangled_name += "IXL";
    mangled_name += NestedMangledName(depth - 1);
    mangled_name += "EEE";
  }
  return mangled_name;
}

TEST(Demangle, DemangleStackConsumption) {





  int stack_consumed = 0;

  const char *demangled =
      DemangleStackConsumption("_Z6foobarv", &stack_consumed);
  EXPECT_STREQ("foobar()", demangled);
  EXPECT_GT(stack_consumed, 0);
  EXPECT_LT(stack_consumed, kStackConsumptionUpperLimit);

  const std::string nested_mangled_name0 = NestedMangledName(0);
  demangled = DemangleStackConsumption(nested_mangled_name0.c_str(),
                                       &stack_consumed);
  EXPECT_STREQ("a", demangled);
  EXPECT_GT(stack_consumed, 0);
  EXPECT_LT(stack_consumed, kStackConsumptionUpperLimit);

  const std::string nested_mangled_name1 = NestedMangledName(1);
  demangled = DemangleStackConsumption(nested_mangled_name1.c_str(),
                                       &stack_consumed);
  EXPECT_STREQ("a<>", demangled);
  EXPECT_GT(stack_consumed, 0);
  EXPECT_LT(stack_consumed, kStackConsumptionUpperLimit);

  const std::string nested_mangled_name2 = NestedMangledName(2);
  demangled = DemangleStackConsumption(nested_mangled_name2.c_str(),
                                       &stack_consumed);
  EXPECT_STREQ("a<>", demangled);
  EXPECT_GT(stack_consumed, 0);
  EXPECT_LT(stack_consumed, kStackConsumptionUpperLimit);

  const std::string nested_mangled_name3 = NestedMangledName(3);
  demangled = DemangleStackConsumption(nested_mangled_name3.c_str(),
                                       &stack_consumed);
  EXPECT_STREQ("a<>", demangled);
  EXPECT_GT(stack_consumed, 0);
  EXPECT_LT(stack_consumed, kStackConsumptionUpperLimit);
}

#endif  // Stack consumption tests

static void TestOnInput(const char* input) {
  static const int kOutSize = 1048576;
  auto out = absl::make_unique<char[]>(kOutSize);
  Demangle(input, out.get(), kOutSize);
}

TEST(DemangleRegression, NegativeLength) {
  TestOnInput("_ZZn4");
}

TEST(DemangleRegression, DeeplyNestedArrayType) {
  const int depth = 100000;
  std::string data = "_ZStI";
  data.reserve(data.size() + 3 * depth + 1);
  for (int i = 0; i < depth; i++) {
    data += "A1_";
  }
  TestOnInput(data.c_str());
}

}  // namespace
}  // namespace debugging_internal
ABSL_NAMESPACE_END
}  // namespace absl
