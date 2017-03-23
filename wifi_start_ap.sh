#!/bin/sh

echo "starting ap mode"

if [ ! $# == 2 ]
then
	echo "Please input: wifi_start_ap SSID Key(or none)"
	exit
fi

SSID=$1
key=$2
echo the input SSID is $SSID, key is $key

if [ ! $key = "none" ]
then
#	uci set wireless.ap.encryption=psk2
	uci set wireless.ap.key=$key
else
	
#	uci set wireless.ap.encryption=none
	uci delete wireless.ap.key
	echo starting ap without key
fi


uci set wireless.ap.ssid=$SSID
uci set wireless.ap.disabled=0

#disable the sta mode 
uci set wireless.sta.disabled=1

uci commit wireless
/etc/init.d/network restart
echo "ap mode config is finished"


