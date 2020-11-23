#!/usr/bin/env bash
mkdir -p build && cd build &&
    cmake -DCMAKE_TOOLCHAIN_FILE=../mingw-w64-x86_64.cmake -DCMAKE_BUILD_TYPE=Debug .. &&
    make -j12 &&
    cp astrolograph.exe ..
