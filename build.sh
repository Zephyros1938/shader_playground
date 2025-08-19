#!/bin/bash
rm -rf build
set -e

mkdir -p build
cd build

cmake ..
# cmake --build . --verbose
make
