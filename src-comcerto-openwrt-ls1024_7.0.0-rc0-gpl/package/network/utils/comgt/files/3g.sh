#!/bin/sh

[ -n "$INCLUDE_ONLY" ] || {
	NOT_INCLUDED=1
	INCLUDE_ONLY=1

	. ../netifd-proto.sh
	. ./ppp.sh
	init_proto "$@"
}

proto_3g_init_config() {
	no_device=1
	available=1
	ppp_generic_init_config
	proto_config_add_string "device:device"
	proto_config_add_string "apn"
	proto_config_add_string "service"
	proto_config_add_string "pincode"
	proto_config_add_string "dialnumber"
}

proto_3g_setup() {
	local interface="$1"
	local chat

	json_get_var device device
	json_get_var apn apn
	json_get_var service service
	json_get_var pincode pincode
	json_get_var dialnumber dialnumber

	[ -n "$dat_device" ] && device=$dat_device

	device="$(readlink -f $device)"
	[ -e "$device" ] || {
		proto_set_available "$interface" 0
		return 1
	}


	if [ "$interface" = "usb1" ];then
		TTY_FILE=/tmp/TTY_USB1
		usb_port=1
		temp=`lsusb | grep "Bus 003" | grep -v "Device 001" | cut -d " " -f 6`
	elif [ "$interface" = "usb2" ];then
		TTY_FILE=/tmp/TTY_USB2
		usb_port=2
		temp=`lsusb | grep "Bus 001" | grep -v "Device 001" | cut -d " " -f 6`
	fi
	
	[ -n "$temp" ] && {
		# Huawei CDMA dongle (EC169)
		[ "$temp" = "12d1:1001" ] && cdma_dongle=1
		# Huawei HiLink mode
		[ "$temp" = "12d1:14db" -o "$temp" = "12d1:14dc" ] && return 1
		# Huawei NCM mode
		[ "$temp" = "12d1:1506" ] && return 1
	}

	# We have umts/cdma services.
	service="umts"

	[ "$cdma_dongle" = 1 ] && service="cdma"

	case "$service" in
		cdma|evdo)
			logger -t mobile -p local0.info "$interface: dailing cdma service"
			chat="/etc/chatscripts/evdo.chat"
			# set the dialnumber and apn for auto mode for cdma dongles.
			[ -z "$apn" ] && apn="internet"
			[ -z "$dialnumber=" ] && dialnumber="#777"
		;;
		*)
			logger -t mobile -p local0.info "$interface: dailing umts service"
			chat="/etc/chatscripts/3g.chat"
			cardinfo=$(gcom -d "$device" -s /etc/gcom/getcardinfo_3g.gcom)
			if echo "$cardinfo" | grep -q Novatel; then
				case "$service" in
					umts_only) CODE=2;;
					gprs_only) CODE=1;;
					*) CODE=0;;
				esac
				export MODE="AT\$NWRAT=${CODE},2"
			elif echo "$cardinfo" | grep -q Option; then
				case "$service" in
					umts_only) CODE=1;;
					gprs_only) CODE=0;;
					*) CODE=3;;
				esac
				export MODE="AT_OPSYS=${CODE}"
			elif echo "$cardinfo" | grep -q "Sierra Wireless"; then
				SIERRA=1
			elif echo "$cardinfo" | grep -qi huawei; then
				case "$service" in
					umts_only) CODE="14,2";;
					gprs_only) CODE="13,1";;
					*) CODE="2,2";;
				esac
				export MODE="AT^SYSCFG=${CODE},3FFFFFFF,2,4"
			fi

			if [ -n "$pincode" ]; then
				PINCODE="$pincode" gcom -d "$device" -s /etc/gcom/setpin.gcom || {
					logger -t mobile -p local0.error "$interface: Unable to verify PIN"
					proto_notify_error "$interface" PIN_FAILED
					proto_block_restart "$interface"
					return 1
				}
			fi
			[ -n "$MODE" ] && gcom -d "$device" -s /etc/gcom/setmode.gcom

			# wait for carrier to avoid firmware stability bugs
			[ -n "$SIERRA" ] && {
				gcom -d "$device" -s /etc/gcom/getcarrier.gcom || return 1
			}

			[ -z "$apn" ] && apn="internet"

			if [ -z "$dialnumber" ]; then
				dialnumber="*99***1#"
			fi

		;;
	esac
	
	connect="${apn:+USE_APN=$apn }DIALNUMBER=$dialnumber /usr/sbin/chat -t5 -v -E -f $chat"
	ppp_generic_setup "$interface" \
		noaccomp \
		nopcomp \
		novj \
		nobsdcomp \
		noauth \
		lock \
		crtscts \
		115200 "$device"
	return 0
}

proto_3g_teardown() {
	proto_kill_command "$interface"
}

[ -z "$NOT_INCLUDED" ] || add_protocol 3g
