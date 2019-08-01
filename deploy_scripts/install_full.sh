#!/bin/bash

# tested on image: stretch-minimal-rock64-0.8.3-1141-arm64.img.xz

# note for scripts to work fully automatically please ensure that you have key-based ssh authentification.

IP_ADDRESS="192.168.1.150"  # only used if hostname resolution does not work
HOSTNAME="KinectIP"

echo "Installing KinectIP from Source Code:"
echo "Arguments: -no_netdata | skips netdata installation"
sleep 2

ssh root@$HOSTNAME << END_SESSION

  apt-get update && apt-get upgrade -y

  apt-get install build-essential cmake pkg-config -y

  sudo apt-get install autotools-dev -y

  sudo apt-get install autoconf -y

  exit

END_SESSION

./deploy_scripts/install_webserver.sh $1

# copy test images to remote directory
scp -r ./resources/img root@KinectIP:/home/rock64/

# copy pre-compiled NDI libraries to remote local libs
scp -r ./resources/NDI-aarch64-libs/* root@KinectIP:/usr/local/lib/NDI/

./deploy_scripts/install_libfreenect2.sh

exit