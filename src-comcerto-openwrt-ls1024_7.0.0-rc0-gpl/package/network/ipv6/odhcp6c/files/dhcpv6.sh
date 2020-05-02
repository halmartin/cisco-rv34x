#!/bin/sh

. /lib/functions.sh
. ../netifd-proto.sh
init_proto "$@"

proto_dhcpv6_init_config() {
	renew_handler=1

	proto_config_add_string 'reqaddress:or("try","force","none")'
	proto_config_add_string 'reqprefix:or("auto","no",range(0, 64))'
	proto_config_add_string clientid
	proto_config_add_string 'reqopts:list(uinteger)'
	proto_config_add_string 'noslaaconly:bool'
	proto_config_add_string 'forceprefix:bool'
	proto_config_add_string 'norelease:bool'
	proto_config_add_string 'ip6prefix:ip6addr'
	proto_config_add_string iface_dslite
	proto_config_add_string zone_dslite
	proto_config_add_string iface_map
	proto_config_add_string zone_map
	proto_config_add_string zone
	proto_config_add_string 'ifaceid:ip6addr'
	proto_config_add_string 'sourcerouting:bool'
	proto_config_add_string "userclass"
	proto_config_add_string "vendorclass"
	proto_config_add_boolean delegate
	proto_config_add_int "soltimeout"
	proto_config_add_boolean fakeroutes
}

proto_dhcpv6_setup() {
	local config="$1"
	local iface="$2"

	local reqaddress reqprefix clientid reqopts noslaaconly forceprefix norelease ip6prefix iface_dslite iface_map ifaceid sourcerouting userclass vendorclass delegate zone_dslite zone_map zone soltimeout fakeroutes
	json_get_vars reqaddress reqprefix clientid reqopts noslaaconly forceprefix norelease ip6prefix iface_dslite iface_map ifaceid sourcerouting userclass vendorclass delegate zone_dslite zone_map zone soltimeout fakeroutes


	# Configure
	local opts=""
	[ -n "$reqaddress" ] && append opts "-N$reqaddress"

	[ -n "$3" ] && reqprefix="$3"

	[ -n "$clientid" ] && append opts "-c$clientid"

	[ "$noslaaconly" = "1" ] && append opts "-S"

	[ "$forceprefix" = "1" ] && append opts "-F"

	[ "$norelease" = "1" ] && append opts "-k"

	[ -n "$ifaceid" ] && append opts "-i$ifaceid"

	[ -n "$vendorclass" ] && append opts "-V$vendorclass"

	[ -n "$userclass" ] && append opts "-u$userclass"

	for opt in $reqopts; do
		append opts "-r$opt"
	done

	append opts "-t${soltimeout:-120}"

	[ "$reqprefix" != "auto" ] && [ "$config" != "usb1_6" ] && [ "$config" != "usb2_6" ] && {
		proto=$(uci get network.$config.proto )
		slaacmode=$(uci get network.$config.reqaddress )
		iface=$(uci get network.$config.ifname )
		if [[ "$proto" = "dhcpv6" && "$slaacmode" = "" ]]; then
			pd_interface=$(uci show network-bkp | grep "ifname=$iface$" | grep "PD" | awk -F . '{print $2}')
			[ -n "$pd_interface" ] && {
				proto_kill_command "$pd_interface"
			}
				append opts "-P0"
		fi
	}

	[ "$reqprefix" = "auto" ] && [ "$config" != "usb1_6" ] && [ "$config" != "usb2_6" ] && {
		act_int=$(uci show network | grep -wE "ifname=$iface$" | grep -E "wan16|wan26" | awk -F . '{print $2}')
		proto=$(uci get network.$act_int.proto )
		slaacmode=$(uci get network.$act_int.reqaddress )
		if [[ "$proto" = "dhcpv6" && "$slaacmode" = "" ]]; then
			exit 0
		else
			append opts "-P0 -Nnone"
		fi
	}

	[ "$config" = "usb1_6" ] || [ "$config" = "usb2_6" ] && {
		[ "$reqprefix" = "auto" ] && append opts "-P0"
	}

	[ -n "$ip6prefix" ] && proto_export "USERPREFIX=$ip6prefix"
	[ -n "$iface_dslite" ] && proto_export "IFACE_DSLITE=$iface_dslite"
	[ -n "$iface_map" ] && proto_export "IFACE_MAP=$iface_map"
	[ "$sourcerouting" != "0" ] && proto_export "SOURCE_ROUTING=1"
	[ "$delegate" = "0" ] && proto_export "IFACE_DSLITE_DELEGATE=0"
	[ "$delegate" = "0" ] && proto_export "IFACE_MAP_DELEGATE=0"
	[ -n "$zone_dslite" ] && proto_export "ZONE_DSLITE=$zone_dslite"
	[ -n "$zone_map" ] && proto_export "ZONE_MAP=$zone_map"
	[ -n "$zone" ] && proto_export "ZONE=$zone"
	[ "$fakeroutes" != "0" ] && proto_export "FAKE_ROUTES=1"

	proto_export "INTERFACE=$config"
	proto_run_command "$config" odhcp6c \
		-s /lib/netifd/dhcpv6.script \
		$opts $iface

}

proto_dhcpv6_renew() {
	local interface="$1"
	# SIGUSR1 forces odhcp6c to renew its lease
	local sigusr1="$(kill -l SIGUSR1)"
	[ -n "$sigusr1" ] && proto_kill_command "$interface" $sigusr1
}

proto_dhcpv6_teardown() {
	local interface="$1"
	proto_kill_command "$interface"

	[ "$interface" != "usb1_6" ] && [ "$interface" != "usb2_6" ] && {

		pppoev6=$(echo $interface | sed s/wan16/wan16p/g | sed s/wan26/wan26p/g)
		pppoe_proto=$(uci get network.$pppoev6.proto)
		[ $pppoe_proto != "" ] && {
			interface=$pppoev6
		}
		proto=$(uci get network.$interface.proto )
		slaacmode=$(uci get network.$interface.reqaddress )
		iface=$(uci get network.$interface.ifname )
		if [[  "$proto" != "dhcpv6"  || "$slaacmode" != "" ]]; then
			pd_newinterface=$(uci show network | grep "ifname=$iface$" | grep "PD" | awk -F . '{print $2}')
			[ -n "$pd_newinterface" ] && {
				proto_dhcpv6_setup "$pd_newinterface" "$iface" "auto"
			}
		fi
	}

}

add_protocol dhcpv6

