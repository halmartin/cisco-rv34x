#!/bin/sh

[ -x /usr/sbin/pppd ] || exit 0

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

ppp_generic_init_config() {
	proto_config_add_string "username"
	proto_config_add_string "password"
	proto_config_add_string "keepalive"
	proto_config_add_int "demand"
	proto_config_add_string "pppd_options"
	proto_config_add_string "connect"
	proto_config_add_string "disconnect"
	proto_config_add_boolean "ipv6"
	proto_config_add_boolean "authfail"
	proto_config_add_boolean "mppe_req"
	proto_config_add_int "mtu"
}

ppp_generic_setup() {
	local config="$1"; shift
	
	json_get_vars ipv6 demand keepalive username password pppd_options mppe_req mtu
	if [ -n "$mppe_req" ]; then
		if [ $mppe_req = "1" ];then
			mppe="mppe required,no56,stateless"
		else
			mppe="nomppe"
		fi
	else
		mppe=""
	fi
	
	[ "$ipv6" = 1 ] || ipv6=""
	demand_route=""
	if [ "${demand:-0}" -gt 0 ]; then
		demand=$(expr 60 "*" "$demand")
		demand_route=1
		demand_conf=""
		demand_conf="precompiled-active-filter /etc/ppp/filter demand idle $demand defaultroute nopersist"
		if [ "$ipv6" = 1 ]; then
				
			if [ "$(echo $config | grep "wan.6p.*")" ]; then
				demand_route=0
			fi

			octet1=$(echo $config | sed s/wan//g | sed s/p.*//)
			octet2=$(echo $config | cut -f 2 -d '_')
			if [ "$(echo $config | grep "_")" = "" ]; then
				octet2="abcd"
			else
				octet1=$octet2
				octet2=$(echo $config | sed s/wan//g | sed s/p.*//)
			fi
			if [ "$(echo $config | grep "wan.p.*")" ]; then
				ipv6_new="${ipv6:+ipv6 ::1:$octet1:$octet2,::1:$octet2:$octet1}"
			else
				ipv6_new="${ipv6:+ipv6 ::1:$octet1:$octet2,::1:$octet2:$octet1 noip}"
			fi
			demand_conf="precompiled-active-filter /etc/ppp/ipv6filter demand idle $demand defaultroute nopersist"
		else
			ipv6_new="${ipv6:++ipv6 noip ipv6cp-use-ipaddr}"
		fi
	else
		demand_conf="persist nodefaultroute"
		if [ "$(echo $config | grep "wan.p.*")" ]; then
                        ipv6_new="${ipv6:++ipv6}"
                else
                        ipv6_new="${ipv6:++ipv6 noip ipv6cp-use-ipaddr}"
                fi

	fi

	if [ "$proto" = "pptp" ]; then
		if [ -n "$mtu" ]; then
			json_get_var mtu mtu
			mtu=$(expr "$mtu" "-" 40)
		else
				mtu=1460
		fi
	else
		[ -n "$mtu" ] || json_get_var mtu mtu
	fi

	[ "$proto" = "pppoe" ] && {
		proto=ppoe
	}

	local interval="${keepalive##*[, ]}"
	[ "$interval" != "$keepalive" ] || interval=5
	[ -n "$connect" ] || json_get_var connect connect
	[ -n "$disconnect" ] || json_get_var disconnect disconnect

	proto_run_command "$config" /usr/sbin/pppd \
		nodetach ipparam "$config" \
		ifname "${proto:-ppp}-$config" \
		lcp-echo-interval 20 lcp-echo-failure 5 \
		$ipv6_new \
		usepeerdns \
		$mppe \
		$demand_conf \
		${username:+user "$username" password "$password"} \
		${connect:+connect "$connect"} \
		${disconnect:+disconnect "$disconnect"} \
		ip-up-script /lib/netifd/ppp-up \
		ipv6-up-script /lib/netifd/ppp-up \
		ip-down-script /lib/netifd/ppp-down \
		ipv6-down-script /lib/netifd/ppp-down \
		${mtu:+mtu $mtu mru $mtu} \
		$pppd_options "$@"

	if [ "$demand_route" = "1" ]; then
		counter=0
		while [ -z "$(ip -4 route list dev ${proto:-ppp}-$config | grep "10.112.112")" -a "$counter" -lt 15 ]; do
			sleep 1
			let counter++
		done

		gateway=$(ifconfig ${proto:-ppp}-$config | grep P-t-P | sed s/' '*/:/g | cut -f 6 -d ':' )
		route=$(ip route show | grep -w ${proto:-ppp}-$config | grep default)
		[ "$route" = "" ] && {
			metric=`uci get network.$config.metric`
			[ "$metric" == "" ] && { metric=0; }
			gateway=$(ifconfig ${proto:-ppp}-$config | grep P-t-P | xargs | cut -f 3 -d ' '| cut -f 2 -d ':')
			route add default gw $gateway dev ${proto:-ppp}-$config metric $metric
		}
		ACTION=ifup INTERFACE=$config DEVICE=${proto:-ppp}-$config /sbin/hotplug-call iface
	fi

	if [[ "$ipv6" = 1 && "$demand_route" = "0" ]]; then
		sleep 2
		ip -6 route add default dev ${proto:-ppp}-$config metric 512
		route6=$(uci show network | grep -E "interface=$config|target=::/0" | awk -F . '{print $2}' | uniq -d)
		[ -n $route6 ] && {
			for i in $route6
			do
				metric=$(uci get network.$i.metric)
				ip -6 route add default dev ${proto:-ppp}-$config metric $metric
			done
		}
	fi
}

ppp_generic_teardown() {
	local interface="$1"

	case "$ERROR" in
		11|19)
			proto_notify_error "$interface" AUTH_FAILED
			json_get_var authfail authfail
			if [ "${authfail:-0}" -gt 0 ]; then
				proto_block_restart "$interface"
			fi
		;;
		2)
			proto_notify_error "$interface" INVALID_OPTIONS
			proto_block_restart "$interface"
		;;
	esac
	proto_kill_command "$interface"
}

# PPP on serial device

proto_ppp_init_config() {
	proto_config_add_string "device"
	ppp_generic_init_config
	no_device=1
	available=1
}

proto_ppp_setup() {
	local config="$1"

	json_get_var device device
	ppp_generic_setup "$config" "$device"
}

proto_ppp_teardown() {
	ppp_generic_teardown "$@"
}

proto_pppoe_init_config() {
	ppp_generic_init_config
	proto_config_add_string "ac"
	proto_config_add_string "service"
}

proto_pppoe_setup() {
	local config="$1"
	local iface="$2"

	for module in slhc ppp_generic pppox pppoe; do
		/sbin/insmod $module 2>&- >&-
	done

	json_get_var mtu mtu
	mtu="${mtu:-1492}"

	json_get_var ac ac
	json_get_var service service

	local isboot=$(uci_get_state system.core.booted)
	[ "1" != "$isboot" ] && {
		local ifaceName=`echo $config | sed 's/p//g'`
		ifdown $ifaceName
	}

	ppp_generic_setup "$config" \
		plugin rp-pppoe.so \
		${ac:+rp_pppoe_ac "$ac"} \
		${service:+rp_pppoe_service "$service"} \
		"nic-$iface"
}

proto_pppoe_teardown() {
	ppp_generic_teardown "$@"
}

proto_pppoa_init_config() {
	ppp_generic_init_config
	proto_config_add_int "atmdev"
	proto_config_add_int "vci"
	proto_config_add_int "vpi"
	proto_config_add_string "encaps"
	no_device=1
	available=1
}

proto_pppoa_setup() {
	local config="$1"
	local iface="$2"

	for module in slhc ppp_generic pppox pppoatm; do
		/sbin/insmod $module 2>&- >&-
	done

	json_get_vars atmdev vci vpi encaps

	case "$encaps" in
		1|vc) encaps="vc-encaps" ;;
		*) encaps="llc-encaps" ;;
	esac

	ppp_generic_setup "$config" \
		plugin pppoatm.so \
		${atmdev:+$atmdev.}${vpi:-8}.${vci:-35} \
		${encaps}
}

proto_pppoa_teardown() {
	ppp_generic_teardown "$@"
}

proto_pptp_init_config() {
	ppp_generic_init_config
	proto_config_add_string "server"
	available=1
	no_device=1
}

proto_pptp_setup() {
	local config="$1"
	local iface="$2"

	local ip serv_addr server
	local nonLogicalIface=$(echo $config | sed s/p//g)
	json_get_var server server && {
		for ip in $(resolveip -4 -t 5 "$server"); do
			( proto_add_host_dependency "$config" "$ip" "$nonLogicalIface" )
			serv_addr=1
		done
	}
	[ -n "$serv_addr" ] || {
		echo "Could not resolve server address"
		sleep 5
		proto_setup_failed "$config"
		exit 1
	}

	local load
	for module in slhc ppp_generic ppp_async ppp_mppe ip_gre gre pptp; do
		grep -q "$module" /proc/modules && continue
		/sbin/insmod $module 2>&- >&-
		load=1
	done
	[ "$load" = "1" ] && sleep 1

	ppp_generic_setup "$config" \
		plugin pptp.so \
		pptp_server $server
#		file /etc/ppp/options.pptp
}

proto_pptp_teardown() {
	ppp_generic_teardown "$@"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol ppp
	[ -f /usr/lib/pppd/*/rp-pppoe.so ] && add_protocol pppoe
	[ -f /usr/lib/pppd/*/pppoatm.so ] && add_protocol pppoa
	[ -f /usr/lib/pppd/*/pptp.so ] && add_protocol pptp
}

