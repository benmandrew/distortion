#!/bin/sh

COMPILER=$([ "$(uname -s)" = "Darwin" ] && echo "-D CMAKE_C_COMPILER=gcc-14 -D CMAKE_CXX_COMPILER=g++-14" || echo "")

cmake ${COMPILER} -S . -B build

cmake --build build -j8
