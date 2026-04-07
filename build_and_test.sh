#!/bin/bash
set -e

cd "$(dirname "$0")"

rm -rf build
mkdir build
cd build

cmake -G Ninja .. \
  -DQt6_DIR="$(brew --prefix qt)/lib/cmake/Qt6" \
  -DPL_GUI=ON \
  -DCMAKE_C_COMPILER="$(brew --prefix llvm)/bin/clang" \
  -DCMAKE_CXX_COMPILER="$(brew --prefix llvm)/bin/clang++" \
  -DBUILD_ALL_PLUGINS=ON \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_TESTS=ON

ninja
