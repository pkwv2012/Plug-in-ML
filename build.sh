#!/bin/bash

set -euo pipfail

cd $HOME
mkdir -p build && cd build

# Configure
cmake -DCMAKE_BUILD_TYPE=Debug ..
# Build (for Make on Unix equivalent to `make -j $(nproc)`)
cmake --build . --config Debug -- -j $(nproc)
# Test (do gtest)
