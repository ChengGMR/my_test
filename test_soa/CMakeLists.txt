cmake_minimum_required(VERSION 3.5)
project(force_module LANGUAGES CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Werror=return-type)

# Build type default
include_directories(
  include
)

# Threads (for std::thread/condition_variable)
find_package(Threads REQUIRED)

# Your benchmark source file
add_executable(bench_soa src/main.cpp)
    
# Link threads
target_link_libraries(bench_soa PRIVATE Threads::Threads)

# Optional: faster release builds
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(bench_soa PRIVATE
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:RelWithDebInfo>:-O2 -g>
  )
elseif (MSVC)
  target_compile_options(bench_soa PRIVATE
    $<$<CONFIG:Release>:/O2>
  )
endif()
