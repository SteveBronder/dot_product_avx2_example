#ifndef BENCH_DOT_PRODUCT_DOT_HPP
#define BENCH_DOT_PRODUCT_DOT_HPP

template <typename T> T dot_product(T *a, T *b, int n) {
  T sum = 0;
  for (int i = 0; i < n; ++i) {
    sum += a[i] * b[i];
  }
  return sum;
}

#endif