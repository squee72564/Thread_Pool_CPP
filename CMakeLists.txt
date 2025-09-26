cmake_minimum_required(VERSION 3.10)

project(thread_pool)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(thread_pool src/thread_pool.cpp)
target_include_directories(thread_pool PUBLIC include)
target_compile_options(thread_pool PRIVATE -Wall -Wextra -Wpedantic -O3)

if(BUILD_TESTS)
    add_executable(thread_pool_test tests/test_thread_pool.cpp)
    target_link_libraries(thread_pool_test PRIVATE thread_pool)
    target_compile_options(thread_pool_test PRIVATE -Wall -Wextra -Wpedantic -O3)

    add_executable(vector_sum_test tests/vector_sum.cpp)
    target_link_libraries(vector_sum_test PRIVATE thread_pool)
    target_compile_options(vector_sum_test PRIVATE -Wall -Wextra -Wpedantic -O3)
endif()
