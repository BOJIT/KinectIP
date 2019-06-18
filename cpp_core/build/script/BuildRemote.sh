#!/bin/bash
#cd $PWD/build/local/
sudo mount -t drvfs Z: /mnt/rock64
cd ./build/remote
cmake -DCMAKE_TOOLCHAIN_FILE=../../aarch64.cmake ../../
make
cd ../..
sudo umount /mnt/rock64