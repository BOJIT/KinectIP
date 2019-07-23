#!/bin/bash
#cd $PWD/build/local/
mount -t drvfs Z: /mnt/rock64
cd ./build/remote
cmake -DCMAKE_TOOLCHAIN_FILE=../../aarch64.cmake ../../
make
cd ../..
echo "--------FILE TRANSFER--------"
cd ./bin/aarch64_Linux
scp KinectIP.aarch64.Linux rock64@192.168.1.150:
cd ../..
echo "-----------PROGRAM-----------"
ssh rock64@192.168.1.150 './SetPermissions.exe'
ssh rock64@192.168.1.150 './KinectIP.aarch64.Linux'
exit
umount /mnt/rock64