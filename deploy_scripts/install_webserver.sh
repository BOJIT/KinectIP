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

END_SESSION