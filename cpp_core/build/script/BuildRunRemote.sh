#!/bin/bash
#cd $PWD/build/local/
cd ./build/remote
cmake -DCMAKE_TOOLCHAIN_FILE=../../aarch64.cmake ../../
make
cd ../..
echo "--------FILE TRANSFER--------"
cd ./bin/aarch64_Linux
scp KinectIP.aarch64.Linux root@KinectIP:/home/rock64/
cd ../..
echo "-----------PROGRAM-----------"
ssh root@KinectIP '/home/rock64/KinectIP.aarch64.Linux'
exit