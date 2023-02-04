# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -mavx2 -O3 -fno-exceptions -fno-trapping-math -DNDEBUG -flto=auto")
project ("Benchmarks")

# Include sub-projects.
add_subdirectory ("dot_product")