# taken from package pptp and modified for accel-pptp

find_gw() {
    route -n | awk '$1 == "0.0.0.0" { print $2; exit }'
}

scan_pptp() {
    config_set "$1" device "pptp-$1"
}

stop_interface_pptp() {
    stop_interface_ppp "$1"
}

coldplug_interface_pptp() {
    setup_interface_pptp "pptp-$1" "$1"
}

setup_interface_pptp() {
    local config="$2"
    local ifname

    local device
    config_get device "$config" device

    local ipproto
    config_get ipproto "$config" ipproto

    local server
    config_get server "$config" server

    for module in slhc ppp_generic pppox pptp; do
	/sbin/insmod $module 2>&- >&-
    done
    sleep 1

    setup_interface "$device" "$config" "${ipproto:-dhcp}"
    local gw="$(find_gw)"
    [ -n "$gw" ] && {
	[ "$gw" != 0.0.0.0 ] && route delete "$server" 2>/dev/null >/dev/null
	route add "$server" gw "$gw"
    }

    # fix up the netmask
    config_get netmask "$config" netmask
    [ -z "$netmask" -o -z "$device" ] || ifconfig $device netmask $netmask

    config_get mtu "$config" mtu
    mtu=${mtu:-1452}
    start_pppd "$config" \
	plugin pptp.so \
        pptp_server $server \
	file /etc/ppp/options.pptp \
	mtu $mtu mru $mtu
}
