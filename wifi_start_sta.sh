#!/bin/sh

echo "starting sta mode"
host_ip="115.239.210.27"

if [ ! $# == 2 ]
then
	echo "Please input: wifi_start_sta SSID Key"
	exit
fi

ping $host_ip -c 1 -w 1 > nul

if [ $? == 0 ]
then
	SSID=`uci get wireless.sta.ssid`
	key=`uci get wireless.sta.key`
	echo the network have already connected,which SSID is $SSID, key is $key
	exit
fi

SSID=$1
key=$2
echo the input SSID is $SSID, key is $key
uci set wireless.sta.ssid=$SSID
uci set wireless.sta.key=$key
uci set wireless.sta.disabled=0
uci commit wireless
/etc/init.d/network restart

