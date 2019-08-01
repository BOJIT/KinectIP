#!/bin/bash

IP_ADDRESS="192.168.1.150"  # only used if hostname resolution does not work
HOSTNAME="KinectIP"

ssh root@$HOSTNAME << END_SESSION

apt-get install lighttpd -y
apt-get install php7.0-fpm php-cgi -y
apt-get --purge autoremove -y

rm /var/www/html/index.lighttpd.html

lighty-enable-mod fastcgi
lighty-enable-mod fastcgi-php

service lighttpd force-reload

END_SESSION

scp -r ./web_interface/* root@KinectIP:/var/www/html

ssh root@$HOSTNAME << END_SESSION

chown -R www-data:www-data /var/www
gpasswd -a rock64 www-data
chmod -R g+rw /var/www
chmod 775 /var/www

service lighttpd force-reload

if [ "$1" != "" ]; then
  if [ "$1" = "-no_netdata" ]; then
    echo "Skipping Netdata installation"
  else
    echo "Invalid Argument: installing Netdata anyway"
    sleep 2
    echo "----INSTALLING NETDATA----"
    curl -Ss 'https://raw.githubusercontent.com/firehol/netdata-demo-site/master/install-required-packages.sh' >/tmp/kickstart.sh && bash /tmp/kickstart.sh -i netdata
    cd /usr/src
    git clone https://github.com/firehol/netdata.git --depth=1
    cd netdata
    sudo ./netdata-installer.sh
    cd
  fi
else
  echo "----INSTALLING NETDATA----"
  curl -Ss 'https://raw.githubusercontent.com/firehol/netdata-demo-site/master/install-required-packages.sh' >/tmp/kickstart.sh && bash /tmp/kickstart.sh -i netdata
  cd /usr/src
  git clone https://github.com/firehol/netdata.git --depth=1
  cd netdata
  sudo ./netdata-installer.sh
  cd
fi

END_SESSION

exit