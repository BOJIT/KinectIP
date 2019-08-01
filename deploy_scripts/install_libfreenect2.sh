#!/bin/bash

# installs from local repository instead of official libfreenect2 repository
SOURCE="LOCAL"

IP_ADDRESS="192.168.1.150"  # only used if hostname resolution does not work
HOSTNAME="KinectIP"

ssh root@$HOSTNAME << END_SESSION

  #install dependencies
  apt-get install libusb-1.0-0-dev -y
  apt-get install libturbojpeg0-dev -y
  apt-get install libglfw3-dev -y
  
  if [ $SOURCE != "LOCAL" ]; then
    git clone https://github.com/OpenKinect/libfreenect2.git
  fi

END_SESSION

if [ $SOURCE = "LOCAL" ]; then
  scp -r ./resources/libfreenect2 root@KinectIP:/home/rock64/
fi

ssh root@$HOSTNAME << END_SESSION

  cd /home/rock64/libfreenect2
  mkdir build && cd build
  cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local/lib/freenect2
  make
  make install
  cp ../platform/linux/udev/90-kinect2.rules /etc/udev/rules.d/

END_SESSION
