#!/bin/sh

source /lib/functions/leds.sh

[ $# -ne 2  ] && echo 'need $1(stat),$2(led_dev)' && exit 1
status_led=$2
case $1 in
led_on)
	echo "what happened?"
	status_led_on $status_led;;
led_off)
	status_led_off $status_led;;
blink_slow)
	status_led_blink_slow $status_led;;
blink_fast)
	status_led_blink_fast $status_led;;
blink_preinit)
	status_led_blink_preinit $status_led;;
blink_fail)
        status_led_blink_failsafe $status_led;;
*)
	echo "error"
esac



