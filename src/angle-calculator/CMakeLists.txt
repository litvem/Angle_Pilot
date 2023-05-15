cmake_minimum_required(VERSION 3.2)

project(angle-calculator)

# Using C++14
set(CMAKE_CXX_STANDARD 14)

# Enable pthreads and link librt and make it statically linked
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -pthread -lrt")

# Add the 2 executables
add_executable(
    ${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/angle-calculator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../api/position.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/angle-validator.cpp
)