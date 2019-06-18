#!/bin/bash
#cd $PWD/build/local/
mount -t drvfs Z: /mnt/rock64
cd ./build/remote
cmake -DCMAKE_TOOLCHAIN_FILE=../../aarch64.cmake ../../
make
cd ../..
umount /mnt/rock64