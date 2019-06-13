#!/bin/bash
#cd $PWD/build/local/
cd ./build/remote
cmake -DCMAKE_TOOLCHAIN_FILE=../../aarch64.cmake ../../
make
cd ../..
echo "--------FILE TRANSFER--------"
cd ./bin/aarch64_Linux
scp binary.aarch64.Linux rock64@192.168.1.150:
cd ../..
ssh rock64@192.168.1.150 './binary.aarch64.Linux'
exit