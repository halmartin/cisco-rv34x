#!/bin/sh

## GPIO Identifiers
GPIO54=54 
GPIO56=56 

## LED <> GPIO Mappings
USB2_LED_GREEN=$GPIO54
USB1_LED_GREEN=$GPIO56

LED_ON=0
LED_OFF=1

device="usb2"
device_node="$1"

UsbActiv()
{
	OldStat=$(grep $device_node /proc/diskstats)
	# Monitor /proc/diskstats
    	while [ 1 ]; do
	    ActStat=$(grep $device_node /proc/diskstats)
	    if [ "$ActStat" != "$OldStat" ]; then
		   OldStat=$ActStat
		   echo ${LED_OFF} >/sys/class/gpio/gpio${USB2_LED_GREEN}/value
		   sleep .5
		   echo ${LED_ON} >/sys/class/gpio/gpio${USB2_LED_GREEN}/value
		   sleep .5
	    else
		   sleep 1
	    fi
	done
}

UsbActiv
