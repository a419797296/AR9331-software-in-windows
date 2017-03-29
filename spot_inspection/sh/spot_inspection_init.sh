#!/bin/sh

cat /etc/ser2net.conf | grep 6666
if [ $? -ne 0 ];then
	echo 6666:raw:0:/dev/ttyATH0:115200 NONE 1STOPBIT 8DATABITS   -RTSCTS >> /etc/ser2net.conf
	kill `pidof ser2net`
	ser2net
#else
#	echo "ser2net server has already started"
fi


pidof ser2net
if [ $? -ne 0 ];then
	ser2net
	echo "ser2net server has not started"
#else
#	echo "ser2net server has already started"
fi


cat /proc/modules | grep ad7606
if [ $? -ne 0 ];then
	insmod ad7606_driver
	echo "the model of ad7606_driver has not been installed"
#else
#	echo "the model of ad7606_driver has already been installed"	
fi


