cmake_minimum_required(VERSION 3.2)

project(api_example)

# Using C++14
set(CMAKE_CXX_STANDARD 14)

# Enable pthreads and link librt and make it statically linked
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -pthread -lrt")

# Add the 2 executables
add_executable(producer ${CMAKE_CURRENT_SOURCE_DIR}/producer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/position.cpp)
add_executable(consumer ${CMAKE_CURRENT_SOURCE_DIR}/consumer.cpp ${CMAKE_CURRENT_SOURCE_DIR}/position.cpp)
