#include <benchmark/benchmark.h>
#include <dot_product/dot.hpp>
#include <immintrin.h>

static void BM_dot(benchmark::State &state) {
  srand(123);
  const auto n = state.range(0);
  double *a = (double *)_mm_malloc(sizeof(double) * n, 64);
  double *b = (double *)_mm_malloc(sizeof(double) * n, 64);
  // I know rand and srand are not great but I just need sort of rando nums
  for (int i = 0; i < n; i++) {
    a[i] = 1.0 * rand() / RAND_MAX;
  }
  for (int i = 0; i < n; i++) {
    b[i] = 1.0 * rand() / RAND_MAX;
  }
  for (auto _ : state) {
    // This code gets timed
    auto sum = dot_product(a, b, n);
    benchmark::DoNotOptimize(&sum);
    benchmark::ClobberMemory();
  }
  _mm_free(a);
  _mm_free(b);
}
// Register the function as a benchmark
BENCHMARK(BM_dot)->Range(8, 2 << 15);
// Run the benchmark
BENCHMARK_MAIN();