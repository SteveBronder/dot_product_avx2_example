#ifndef BENCH_DOT_PRODUCT_AVX_DOT4_HPP
#define BENCH_DOT_PRODUCT_AVX_DOT4_HPP

#include <dot_product/attributes.hpp>
#include <immintrin.h>

PURE_ ALWAYS_INLINE_ inline double dot_product_avx2_4roll(double *__restrict a, double *__restrict b,
                              const int n) noexcept {
  a = (double *)__builtin_assume_aligned(a, 64);
  b = (double *)__builtin_assume_aligned(b, 64);
  __m256d sum = _mm256_setzero_pd();
  for (int i = 0; i < n; i += 4) {
    sum = _mm256_add_pd(_mm256_mul_pd(_mm256_load_pd(&a[i]), _mm256_load_pd(&b[i])), sum);
  }
  // extract upper 128 bits of result
  auto sum_high = _mm256_extractf128_pd(sum, 1);
  // add upper 128 bits of sum to its lower 128 bits
  __m128d result = _mm_add_pd(sum_high, _mm256_castpd256_pd128(sum));
  return result[0] + result[1];
}

#endif