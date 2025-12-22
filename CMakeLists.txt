# 1. Minimum CMake version required
cmake_minimum_required(VERSION 3.10)

# 2. Project Name (Updated as requested)
project(limit-order-book-simulator 
    VERSION 1.0
    DESCRIPTION "Low Latency Limit Order Book Simulator"
    LANGUAGES CXX
)

# 3. Enforce C++17 Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 4. Find Dependencies
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# 5. Compiler Optimizations
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -O3
        -march=native
        -Wall
        -Wextra
    )
endif()

# 6. Include Directories
include_directories(include)

# 7. Build the Executable
add_executable(simulator src/main.cpp)

# 8. Link Libraries
target_link_libraries(simulator PRIVATE Threads::Threads)
