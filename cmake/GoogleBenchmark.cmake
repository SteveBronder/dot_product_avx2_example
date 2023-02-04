include(FetchContent)
set(BENCHMARK_ENABLE_TESTING NO)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -flto=auto")
set(INTERPROCEDURAL_OPTIMIZATION TRUE)
FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG origin/main
)

FetchContent_MakeAvailable(googlebenchmark)