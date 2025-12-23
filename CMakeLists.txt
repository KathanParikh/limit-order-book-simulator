cmake_minimum_required(VERSION 3.10)
project(OrderBookSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler Optimizations
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -O3
        -march=native
        -Wall
        -Wextra
    )
elseif(MSVC)
    add_compile_options(/O2 /W4)
endif()

# 1. Global Include Path (Fixes "File not found" errors)
include_directories(${CMAKE_SOURCE_DIR}/include)

# 2. Main Simulator
add_executable(simulator src/main.cpp)

# 3. Google Benchmark
include(FetchContent)
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.9.1
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

# 4. Benchmark Executable
add_executable(bench_test benchmarks/main.cpp)
target_link_libraries(bench_test benchmark::benchmark)