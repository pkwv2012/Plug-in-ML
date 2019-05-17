#!/bin/bash

set -euo pipefail

REPO_HOME=${REPO_HOME:-$(dirname "$0")}
mkdir -p build && cd build

# Configure
cmake -DCMAKE_BUILD_TYPE=Debug ..
# Build (for Make on Unix equivalent to `make -j $(nproc)`)
cmake --build . --config Debug -- -j $(nproc)
# Test (do gtest)
