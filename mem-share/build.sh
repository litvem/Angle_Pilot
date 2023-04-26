# Build script for the 3 shared memory applications

# Remove any residual build folder to prepare the build process
rm -rf ./build

# Create a new build folder and enter it
mkdir build
cd build

# Build the applications using CMake and Make
cmake -D CMAKE_BUILD_TYPE=Release ..
make
