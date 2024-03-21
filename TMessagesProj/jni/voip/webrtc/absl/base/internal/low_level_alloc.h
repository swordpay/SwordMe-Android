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

#ifndef ABSL_BASE_INTERNAL_LOW_LEVEL_ALLOC_H_
#define ABSL_BASE_INTERNAL_LOW_LEVEL_ALLOC_H_

// mutexes or thread-specific data.  It is intended to be used
// sparingly, and only when malloc() would introduce an unwanted
// dependency, such as inside the heap-checker, or the Mutex
// implementation.


#include <sys/types.h>

#include <cstdint>

#include "absl/base/attributes.h"
#include "absl/base/config.h"

// allocation of virtual memory. Platforms lacking this cannot use
// LowLevelAlloc.
#ifdef ABSL_LOW_LEVEL_ALLOC_MISSING
#error ABSL_LOW_LEVEL_ALLOC_MISSING cannot be directly set
#elif !defined(ABSL_HAVE_MMAP) && !defined(_WIN32)
#define ABSL_LOW_LEVEL_ALLOC_MISSING 1
#endif

// asm.js / WebAssembly.
// See https://kripken.github.io/emscripten-site/docs/porting/pthreads.html
// for more information.
#ifdef ABSL_LOW_LEVEL_ALLOC_ASYNC_SIGNAL_SAFE_MISSING
#error ABSL_LOW_LEVEL_ALLOC_ASYNC_SIGNAL_SAFE_MISSING cannot be directly set
#elif defined(_WIN32) || defined(__asmjs__) || defined(__wasm__)
#define ABSL_LOW_LEVEL_ALLOC_ASYNC_SIGNAL_SAFE_MISSING 1
#endif

#include <cstddef>

#include "absl/base/port.h"

namespace absl {
ABSL_NAMESPACE_BEGIN
namespace base_internal {

class LowLevelAlloc {
 public:
  struct Arena;       // an arena from which memory may be allocated






  static void *Alloc(size_t request) ABSL_ATTRIBUTE_SECTION(malloc_hook);
  static void *AllocWithArena(size_t request, Arena *arena)
      ABSL_ATTRIBUTE_SECTION(malloc_hook);





  static void Free(void *s) ABSL_ATTRIBUTE_SECTION(malloc_hook);








  enum {


    kCallMallocHook = 0x0001,

#ifndef ABSL_LOW_LEVEL_ALLOC_ASYNC_SIGNAL_SAFE_MISSING


    kAsyncSignalSafe = 0x0002,
#endif
  };




  static Arena *NewArena(uint32_t flags);





  static bool DeleteArena(Arena *arena);

  static Arena *DefaultArena();

 private:
  LowLevelAlloc();      // no instances
};

}  // namespace base_internal
ABSL_NAMESPACE_END
}  // namespace absl

#endif  // ABSL_BASE_INTERNAL_LOW_LEVEL_ALLOC_H_
