#!/bin/sh
. /lib/functions/network.sh

network_get_ipaddr ip wlan0
echo $ip
network_get_ipaddrs ip wlan0
echo $ip
network_get_ipaddr $ip wlan0
echo $ip
network_get_ipaddrs $ip wlan0
echo $ip
