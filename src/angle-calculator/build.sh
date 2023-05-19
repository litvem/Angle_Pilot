#! /usr/bin/sh

rm -rf build/
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=Release ..
make
