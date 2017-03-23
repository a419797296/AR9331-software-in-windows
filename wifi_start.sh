#!/bin/sh


if [ ! $# == 3 ]
then
	echo "Please input: wifi_start ap/sta SSID Key(or none)"
	exit
fi

host_ip="115.239.210.27"
mode=$1
SSID=$2
key=$3

if [[ $mode = "ap" ]]
then
	echo ap mode
	./wifi_start_ap.sh $SSID $key
fi

if [[ $mode = "sta" ]]
then
	echo sta mode
	./wifi_start_sta.sh $SSID $key

	i=0
	while [ $i -ne 20 ]
	do
		ping $host_ip -c 1 -w 1
		if [ $? -eq 0 ]
		then
			echo 'the network have already connected'
			ip=`ifconfig wlan0|awk -F'[ :]+' '/inet addr/{print $4}'`
			echo "the current ip is $ip"
			exit
		fi
		i=$(($i+1))
		sleep 1
	done
	echo 'can not connect the wifi,maybe the input ssid or key is wrong, changeing to the ap mode...'

	./wifi_start_ap.sh Spot_Inpection none
fi


