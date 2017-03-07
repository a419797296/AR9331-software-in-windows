#!/bin/sh

kill_num=`ps | grep spot_inspection | awk -F '[ ]+' '{print $2}'`
kill $kill_num
#echo "test"

#for i in $kill_num
#do
#	echo this num is $i
#	sleep 2s
#done
