#!/bin/bash

rm -rf build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release

cd ..
