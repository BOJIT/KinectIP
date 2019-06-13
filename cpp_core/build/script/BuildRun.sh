#!/bin/bash
#build and run firmware
cd ./build/local
cmake ../..
make
cd ../..
echo "--------PROGRAM--------"
arch=$(uname -p)
platform=$(uname)
path=$arch"_"$platform
./bin/$path/binary.$arch.$platform