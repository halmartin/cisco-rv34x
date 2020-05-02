#!/bin/sh
. /lib/functions/network.sh

local device="/dev/modem1"
local status
local card_status
local INTERFACE
local NETWORK
local ipaddr

exist="`confd_cmd -o -c "exists /usb-modems-state/usb-modem{$device}"`"

[ "x$exist" = "xno" ] && {
	confd_cmd -o -c "create /usb-modems-state/usb-modem{$device}"
}

while true
do
	INTERFACE=`cat /tmp/tmpwanstats | grep usb1`
	NETWORK="$(ubus call network.interface.$INTERFACE status)"
	ipaddr="$(jsonfilter -s "$NETWORK" -e "$['ipv4-address'][0].address")"

	if [ -n "$ipaddr" ];then
		card_status=connected
	else
		card_status=disconnected
	fi
	confd_cmd -o <<EOF
	set /usb-modems-state/usb-modem{$device}/card-status "$card_status"
EOF
	sleep 10
done

