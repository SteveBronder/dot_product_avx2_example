#include <cmath>
#include <cstdlib>
#include <dot_product/avx_dot4.hpp>
#include <dot_product/avx_dot8.hpp>
#include <dot_product/dot.hpp>
#include <immintrin.h>
#include <iostream>
#include <vector>

template <typename Vector> auto mean_sd(const Vector &x) {
  long double err_mean = 0;
  long double err_diff_sq = 0;
  for (int i = 0; i < x.size(); ++i) {
    err_mean += x[i];
    err_diff_sq += std::pow(x[i] - err_mean, 2);
  }
  err_mean /= x.size();
  err_diff_sq /= x.size();
  long double err_sd = std::sqrt(err_diff_sq);
  return std::make_pair(err_mean, err_sd);
}

bool check_abs_diff(double a, long double b, long double tol) {
  return std::abs(static_cast<long double>(a) - b) > tol;
}

int main() {
  int n_max = 32768;
  double *a = (double *)_mm_malloc(sizeof(double) * n_max, 64);
  double *b = (double *)_mm_malloc(sizeof(double) * n_max, 64);
  long double *a_long =
      (long double *)_mm_malloc(sizeof(long double) * n_max, 64);
  long double *b_long =
      (long double *)_mm_malloc(sizeof(long double) * n_max, 64);
  // I know rand and srand are not great but I just need sort of rando nums
  srand(1);
  std::vector<long double> roll8_vs_4_err_vec;
  std::vector<long double> basic_err_vec;
  std::vector<long double> roll8_err_vec;
  std::vector<long double> roll4_err_vec;
  for (int rerun = 0; rerun < 10; ++rerun) {
    for (int i = 0; i < n_max; i++) {
      a_long[i] = 10.0 * (2.0 * rand() - RAND_MAX) / RAND_MAX;
      a[i] = static_cast<double>(a_long[i]);
    }
    for (int i = 0; i < n_max; i++) {
      b_long[i] = 1.0 * (2.0 * rand() - RAND_MAX) / RAND_MAX;
      b[i] = static_cast<double>(b_long[i]);
    }
    for (int n = 8; n < n_max; n += 8) {
      auto sum_roll8 = dot_product_avx2_8roll(a, b, n);
      auto sum_roll4 = dot_product_avx2_4roll(a, b, n);
      auto sum = dot_product(a, b, n);
      auto sum_long = dot_product(a_long, b_long, n);
      if (check_abs_diff(sum_roll4, sum_long, 1e-12) ||
          check_abs_diff(sum_roll8, sum_long, 1e-12)) {
        long double abs_err_roll8 =
            std::fabs(static_cast<long double>(sum_roll8) - sum_long);
        roll8_err_vec.push_back(abs_err_roll8);
        long double abs_err_roll4 =
            std::fabs(static_cast<long double>(sum_roll4) - sum_long);
        roll4_err_vec.push_back(abs_err_roll4);
        long double abs_err =
            std::abs(static_cast<long double>(sum) - sum_long);
        basic_err_vec.push_back(abs_err);
        long double roll8_vs_4_err = abs_err_roll8 - abs_err_roll4;
        roll8_vs_4_err_vec.push_back(roll8_vs_4_err);
      }
    }
  }
  std::cout << "Print out shows roll4 or roll8 absolute error is 1e-12 "
               "greater than dot_product(long double, long double)"
            << std::endl;
  std::cout << "For roll4 vs roll8: With respect to the long double dot "
               "product, Positive values mean roll8 is less precise"
               " negative values mean roll4 is less precise."
            << std::endl;
  auto roll8_vs_4_err_stats = mean_sd(roll8_vs_4_err_vec);
  auto roll8_err_stats = mean_sd(roll8_err_vec);
  auto roll4_err_stats = mean_sd(roll4_err_vec);
  auto basic_err_stats = mean_sd(basic_err_vec);
  std::cout << "Tests with relative absolute error > 1e12: "
            << basic_err_vec.size() << std::endl;
  std::cout << "roll 8 vs roll 4 err mean: " << roll8_vs_4_err_stats.first
            << " sd: " << roll8_vs_4_err_stats.second << std::endl;
  std::cout << "roll 8 err mean:            " << roll8_err_stats.first
            << " sd: " << roll8_err_stats.second << std::endl;
  std::cout << "roll 4 err mean:            " << roll4_err_stats.first
            << " sd: " << roll4_err_stats.second << std::endl;
  std::cout << "roll basic err mean:        " << basic_err_stats.first
            << " sd: " << basic_err_stats.second << std::endl;
  return 0;
}