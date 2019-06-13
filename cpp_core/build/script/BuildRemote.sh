#!/bin/bash
#cd $PWD/build/local/
cd ./build/remote
cmake -DCMAKE_TOOLCHAIN_FILE=../../aarch64.cmake ../../
make
cd ../..