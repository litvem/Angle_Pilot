cmake_minimum_required(VERSION 3.2)

project(mem-share)

# Using C++14
set(CMAKE_CXX_STANDARD 14)

# Enable pthreads and link librt and make it statically linked
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -pthread -lrt")

# Add the 3 executables
add_executable(mem1 ${CMAKE_CURRENT_SOURCE_DIR}/mem1.cpp)
add_executable(mem2 ${CMAKE_CURRENT_SOURCE_DIR}/mem2.cpp)
add_executable(mem3 ${CMAKE_CURRENT_SOURCE_DIR}/mem3.cpp)