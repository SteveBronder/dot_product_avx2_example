cmake_minimum_required (VERSION 3.11)

# Include Google Benchmark
include(../cmake/GoogleBenchmark.cmake)
include_directories(./include/)
set(INTERPROCEDURAL_OPTIMIZATION TRUE)
# Add source to this project's executable.
add_executable (dot_product_err "avx_dot_err.cpp")
add_executable (dot_product_4 "avx_dot4_bench.cpp")
add_executable (dot_product_8 "avx_dot8_bench.cpp")
add_executable (dot_product "dot_bench.cpp")

# Link Google Benchmark to the project
target_link_libraries(dot_product_4 benchmark::benchmark)
target_link_libraries(dot_product_8 benchmark::benchmark)
target_link_libraries(dot_product benchmark::benchmark)
