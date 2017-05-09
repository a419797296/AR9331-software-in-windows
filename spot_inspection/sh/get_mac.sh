#!/bin/sh

echo $#
if [ ! $# == 1 ]
then
	echo "Please input: $0 dev_name"
	exit
fi
mac=`ifconfig $1|grep 'HWaddr'|awk -F '[ ]+' '{print $5}`
echo $mac
uci set spot_inspection.product_info.mac=$mac
uci commit
