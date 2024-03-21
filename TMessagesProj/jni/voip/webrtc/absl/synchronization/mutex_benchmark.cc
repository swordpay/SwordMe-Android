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

#include <cstdint>
#include <mutex>  // NOLINT(build/c++11)
#include <vector>

#include "absl/base/config.h"
#include "absl/base/internal/cycleclock.h"
#include "absl/base/internal/spinlock.h"
#include "absl/synchronization/blocking_counter.h"
#include "absl/synchronization/internal/thread_pool.h"
#include "absl/synchronization/mutex.h"
#include "benchmark/benchmark.h"

namespace {

void BM_Mutex(benchmark::State& state) {
  static absl::Mutex* mu = new absl::Mutex;
  for (auto _ : state) {
    absl::MutexLock lock(mu);
  }
}
BENCHMARK(BM_Mutex)->UseRealTime()->Threads(1)->ThreadPerCpu();

static void DelayNs(int64_t ns, int* data) {
  int64_t end = absl::base_internal::CycleClock::Now() +
                ns * absl::base_internal::CycleClock::Frequency() / 1e9;
  while (absl::base_internal::CycleClock::Now() < end) {
    ++(*data);
    benchmark::DoNotOptimize(*data);
  }
}

template <typename MutexType>
class RaiiLocker {
 public:
  explicit RaiiLocker(MutexType* mu) : mu_(mu) { mu_->Lock(); }
  ~RaiiLocker() { mu_->Unlock(); }
 private:
  MutexType* mu_;
};

template <>
class RaiiLocker<std::mutex> {
 public:
  explicit RaiiLocker(std::mutex* mu) : mu_(mu) { mu_->lock(); }
  ~RaiiLocker() { mu_->unlock(); }
 private:
  std::mutex* mu_;
};

class ScopedThreadMutexPriority {
 public:
  explicit ScopedThreadMutexPriority(int priority) {
    absl::base_internal::ThreadIdentity* identity =
        absl::synchronization_internal::GetOrCreateCurrentThreadIdentity();
    identity->per_thread_synch.priority = priority;



    identity->per_thread_synch.next_priority_read_cycles =
        std::numeric_limits<int64_t>::max();
  }
  ~ScopedThreadMutexPriority() {




    absl::synchronization_internal::GetOrCreateCurrentThreadIdentity()
        ->per_thread_synch.next_priority_read_cycles =
        std::numeric_limits<int64_t>::min();
  }
};

void BM_MutexEnqueue(benchmark::State& state) {









  const bool multiple_priorities = state.range(0);
  ScopedThreadMutexPriority priority_setter(
      (multiple_priorities && state.thread_index() != 0) ? 1 : 0);

  struct Shared {
    absl::Mutex mu;
    std::atomic<int> looping_threads{0};
    std::atomic<int> blocked_threads{0};
    std::atomic<bool> thread_has_mutex{false};
  };
  static Shared* shared = new Shared;









  absl::synchronization_internal::PerThreadSem::SetThreadBlockedCounter(
      &shared->blocked_threads);




  ABSL_RAW_CHECK(
      shared->looping_threads.load(std::memory_order_relaxed) == 0 &&
          shared->blocked_threads.load(std::memory_order_relaxed) == 0 &&
          !shared->thread_has_mutex.load(std::memory_order_relaxed),
      "Shared state isn't zeroed at start of benchmark iteration");

  static constexpr int kBatchSize = 1000;
  while (state.KeepRunningBatch(kBatchSize)) {
    shared->looping_threads.fetch_add(1);
    for (int i = 0; i < kBatchSize; i++) {
      {
        absl::MutexLock l(&shared->mu);
        shared->thread_has_mutex.store(true, std::memory_order_relaxed);




        while (shared->looping_threads.load(std::memory_order_relaxed) -
                   shared->blocked_threads.load(std::memory_order_relaxed) !=
               1) {
        }
        shared->thread_has_mutex.store(false);
      }



      while (!shared->thread_has_mutex.load(std::memory_order_relaxed) &&
             shared->looping_threads.load(std::memory_order_relaxed) > 1) {
      }
    }






    shared->looping_threads.fetch_add(-1);
  }
  absl::synchronization_internal::PerThreadSem::SetThreadBlockedCounter(
      nullptr);
}

BENCHMARK(BM_MutexEnqueue)
    ->Threads(4)
    ->Threads(64)
    ->Threads(128)
    ->Threads(512)
    ->ArgName("multiple_priorities")
    ->Arg(false)
    ->Arg(true);

template <typename MutexType>
void BM_Contended(benchmark::State& state) {
  int priority = state.thread_index() % state.range(1);
  ScopedThreadMutexPriority priority_setter(priority);

  struct Shared {
    MutexType mu;
    int data = 0;
  };
  static auto* shared = new Shared;
  int local = 0;
  for (auto _ : state) {










    DelayNs(100 * state.threads(), &local);
    RaiiLocker<MutexType> locker(&shared->mu);
    DelayNs(state.range(0), &shared->data);
  }
}
void SetupBenchmarkArgs(benchmark::internal::Benchmark* bm,
                        bool do_test_priorities) {
  const int max_num_priorities = do_test_priorities ? 2 : 1;
  bm->UseRealTime()

      ->Threads(1)
      ->Threads(2)
      ->Threads(4)
      ->Threads(6)
      ->Threads(8)
      ->Threads(12)
      ->Threads(16)
      ->Threads(24)
      ->Threads(32)
      ->Threads(48)
      ->Threads(64)
      ->Threads(96)
      ->Threads(128)
      ->Threads(192)
      ->Threads(256)
      ->ArgNames({"cs_ns", "num_prios"});


  for (int critical_section_ns : {1, 20, 50, 200, 2000}) {
    for (int num_priorities = 1; num_priorities <= max_num_priorities;
         num_priorities++) {
      bm->ArgPair(critical_section_ns, num_priorities);
    }
  }
}

BENCHMARK_TEMPLATE(BM_Contended, absl::Mutex)
    ->Apply([](benchmark::internal::Benchmark* bm) {
      SetupBenchmarkArgs(bm, /*do_test_priorities=*/true);
    });

BENCHMARK_TEMPLATE(BM_Contended, absl::base_internal::SpinLock)
    ->Apply([](benchmark::internal::Benchmark* bm) {
      SetupBenchmarkArgs(bm, /*do_test_priorities=*/false);
    });

BENCHMARK_TEMPLATE(BM_Contended, std::mutex)
    ->Apply([](benchmark::internal::Benchmark* bm) {
      SetupBenchmarkArgs(bm, /*do_test_priorities=*/false);
    });

// evaluated).  Mutex has (some) support for equivalence classes allowing
// Conditions with the same function/argument to potentially not be multiply
// evaluated.
//
// num_classes==0 is used for the special case of every waiter being distinct.
void BM_ConditionWaiters(benchmark::State& state) {
  int num_classes = state.range(0);
  int num_waiters = state.range(1);

  struct Helper {
    static void Waiter(absl::BlockingCounter* init, absl::Mutex* m, int* p) {
      init->DecrementCount();
      m->LockWhen(absl::Condition(
          static_cast<bool (*)(int*)>([](int* v) { return *v == 0; }), p));
      m->Unlock();
    }
  };

  if (num_classes == 0) {

    num_classes = num_waiters;
  }

  absl::BlockingCounter init(num_waiters);
  absl::Mutex mu;
  std::vector<int> equivalence_classes(num_classes, 1);

  absl::synchronization_internal::ThreadPool pool(num_waiters);

  for (int i = 0; i < num_waiters; i++) {


    pool.Schedule([&, i] {
      Helper::Waiter(&init, &mu, &equivalence_classes[i % num_classes]);
    });
  }
  init.Wait();

  for (auto _ : state) {
    mu.Lock();
    mu.Unlock();  // Each unlock requires Condition evaluation for our waiters.
  }

  mu.Lock();
  for (int i = 0; i < num_classes; i++) {
    equivalence_classes[i] = 0;
  }
  mu.Unlock();
}

#if defined(__linux__) && !defined(ABSL_HAVE_THREAD_SANITIZER)
constexpr int kMaxConditionWaiters = 8192;
#else
constexpr int kMaxConditionWaiters = 1024;
#endif
BENCHMARK(BM_ConditionWaiters)->RangePair(0, 2, 1, kMaxConditionWaiters);

}  // namespace
