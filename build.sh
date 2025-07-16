#!/bin/bash

# Fast Search Tool Build Script

echo "Building Fast Search Tool..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Executable: ./build/fastsearch"
    echo ""
    echo "To install system-wide:"
    echo "sudo make install"
    echo ""
    echo "To test:"
    echo "./build/fastsearch help"
else
    echo "Build failed!"
    exit 1
fi