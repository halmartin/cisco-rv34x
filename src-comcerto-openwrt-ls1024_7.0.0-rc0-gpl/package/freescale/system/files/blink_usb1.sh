#!/bin/sh

## GPIO Identifiers
GPIO54=54 
GPIO56=56 

## LED <> GPIO Mappings
USB1_LED_GREEN=/sys/class/leds/rv160w::usb2_2-gled/brightness

LED_ON=1
LED_OFF=0

device="usb1"
device_node="$1"


UsbActiv()
{
	OldStat=$(grep $device_node /proc/diskstats)
	# Monitor /proc/diskstats
    	while [ 1 ]; do
	    ActStat=$(grep $device_node /proc/diskstats)
	    if [ "$ActStat" != "$OldStat" ]; then
		   OldStat=$ActStat
		   echo ${LED_OFF} > ${USB1_LED_GREEN}
		   sleep .5
		   echo ${LED_ON} > ${USB1_LED_GREEN}
		   sleep .5
	    else
		   sleep 1
	    fi
	done
}

UsbActiv
