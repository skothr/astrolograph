#!/usr/bin/env bash
mkdir -p build && cd build &&
    cmake -DCMAKE_BUILD_TYPE=Release .. &&
    make -j12 &&
    cp astrolograph ..
