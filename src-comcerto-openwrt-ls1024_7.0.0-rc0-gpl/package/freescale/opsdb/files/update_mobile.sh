#!/bin/sh

action="$1"
device="$2"
manufacture="$3"
firmware_version="$4"
sim_status="$5"
imsi="$6"
carrier="$7"
service_name="$8"
sig_strength="$9"
card_info="$10"

if [ "$action" = "add" ];then
	exist="`confd_cmd -o -c "exists /usb-modems-state/usb-modem{$device}"`"

	[ "x$exist" = "xno" ] && {
		confd_cmd -o -c "create /usb-modems-state/usb-modem{$device}"
	}

confd_cmd -o <<EOF
set /usb-modems-state/usb-modem{$device}/manufacture "$manufacture"
set /usb-modems-state/usb-modem{$device}/firmware-version "$firmware_version"
set /usb-modems-state/usb-modem{$device}/sim-status "$sim_status"
set /usb-modems-state/usb-modem{$device}/imsi "$imsi"
set /usb-modems-state/usb-modem{$device}/carrier "$carrier"
set /usb-modems-state/usb-modem{$device}/signal-strength "$sig_strength"
set /usb-modems-state/usb-modem{$device}/card-model "$card_info"
set /usb-modems-state/usb-modem{$device}/network-mode "$service_name"
EOF
elif [ "$action" = "del" ];then
	confd_cmd -o -c "del /usb-modems-state/usb-modem{$device}"
fi
