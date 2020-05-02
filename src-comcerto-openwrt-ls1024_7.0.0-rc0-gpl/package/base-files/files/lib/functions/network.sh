# 2017-06-28: li.zhang <li.zhang@deltaww.com>
# fix bug of the AC320U get the mwan3 iface error
# 2017-04-20: li.zhang <li.zhang@deltaww.com>
# fix bug of the 3g/4g connection the auto service type can not get mwaniface and modify the 4g interface path 
# 2017-03-13: li.zhang <li.zhang@deltaww.com>
# modify the network_is_up() that use to detect interfaces connection

. /usr/share/libubox/jshn.sh
# 1: destination variable
# 2: interface
# 3: path
# 4: separator
# 5: limit
__network_ifstatus() {
	local __tmp

	[ -z "$__NETWORK_CACHE" ] && \
		export __NETWORK_CACHE="$(ubus call network.interface dump)"

	__tmp="$(jsonfilter ${4:+-F "$4"} ${5:+-l "$5"} -s "$__NETWORK_CACHE" -e "$1=@.interface${2:+[@.interface='$2']}$3")"

	[ -z "$__tmp" ] && \
		unset "$1" && \
		return 1

	eval "$__tmp"
}

# determine first IPv4 address of given logical interface
# 1: destination variable
# 2: interface
network_get_ipaddr() {
	__network_ifstatus "$1" "$2" "['ipv4-address'][0].address";
}

# determine first IPv6 address of given logical interface
# 1: destination variable
# 2: interface
network_get_ipaddr6() {
	local __addr

	if __network_ifstatus "__addr" "$2" "['ipv6-address','ipv6-prefix-assignment'][0].address"; then
		case "$__addr" in
			*:)	export "$1=${__addr}1" ;;
			*)	export "$1=${__addr}" ;;
		esac
		return 0
	fi

	unset $1
	return 1
}

# determine first IPv4 subnet of given logical interface
# 1: destination variable
# 2: interface
network_get_subnet() {
	__network_ifstatus "$1" "$2" "['ipv4-address'][0]['address','mask']" "/"
}

# determine first IPv6 subnet of given logical interface
# 1: destination variable
# 2: interface
network_get_subnet6() {
	__network_ifstatus "$1" "$2" "['ipv6-address'][0]['address','mask']" "/"
}

# determine first IPv6 prefix of given logical interface
# 1: destination variable
# 2: interface
network_get_prefix6() {
	__network_ifstatus "$1" "$2" "['ipv6-prefix'][0]['address','mask']" "/"
}

# determine all IPv4 addresses of given logical interface
# 1: destination variable
# 2: interface
network_get_ipaddrs() {
	__network_ifstatus "$1" "$2" "['ipv4-address'][*].address"
}

# determine all IPv6 addresses of given logical interface
# 1: destination variable
# 2: interface
network_get_ipaddrs6() {
	local __addr
	local __list=""

	if __network_ifstatus "__addr" "$2" "['ipv6-address','ipv6-prefix-assignment'][*].address"; then
		for __addr in $__addr; do
			case "$__addr" in
				*:) __list="${__list:+$__list }${__addr}1" ;;
				*)  __list="${__list:+$__list }${__addr}"  ;;
			esac
		done

		export "$1=$__list"
		return 0
	fi

	unset "$1"
	return 1
}

# determine all IP addresses of given logical interface
# 1: destination variable
# 2: interface
network_get_ipaddrs_all() {
	local __addr
	local __list=""

	if __network_ifstatus "__addr" "$2" "['ipv4-address','ipv6-address','ipv6-prefix-assignment'][*].address"; then
		for __addr in $__addr; do
			case "$__addr" in
				*:) __list="${__list:+$__list }${__addr}1" ;;
				*)  __list="${__list:+$__list }${__addr}"  ;;
			esac
		done

		export "$1=$__list"
		return 0
	fi

	unset "$1"
	return 1
}

# determine all IPv4 subnets of given logical interface
# 1: destination variable
# 2: interface
network_get_subnets() {
	__network_ifstatus "$1" "$2" "['ipv4-address'][*]['address','mask']" "/ "
}

# determine all IPv6 subnets of given logical interface
# 1: destination variable
# 2: interface
network_get_subnets6() {
	local __addr
	local __list=""

	if __network_ifstatus "__addr" "$2" "['ipv6-address','ipv6-prefix-assignment'][*]['address','mask']" "/ "; then
		for __addr in $__addr; do
			case "$__addr" in
				*:/*) __list="${__list:+$__list }${__addr%/*}1/${__addr##*/}" ;;
				*)    __list="${__list:+$__list }${__addr}"                   ;;
			esac
		done

		export "$1=$__list"
		return 0
	fi

	unset "$1"
	return 1
}

network_get_prefixes6_assignment() {
	__network_ifstatus "$1" "$2" "['ipv6-prefix-assignment'][*]['address','mask']" "/ "
}

# determine all IPv6 prefixes of given logical interface
# 1: destination variable
# 2: interface
network_get_prefixes6() {
	__network_ifstatus "$1" "$2" "['ipv6-prefix'][*]['address','mask']" "/ "
}

# determine IPv4 gateway of given logical interface
# 1: destination variable
# 2: interface
# 3: consider inactive gateway if "true" (optional)
network_get_gateway() {
	__network_ifstatus "$1" "$2" ".route[@.target='0.0.0.0' && !@.table].nexthop" "" 1 && \
		return 0

	[ "$3" = 1 -o "$3" = "true" ] && \
		__network_ifstatus "$1" "$2" ".inactive.route[@.target='0.0.0.0' && !@.table].nexthop" "" 1
}

# determine IPv6 gateway of given logical interface
# 1: destination variable
# 2: interface
# 3: consider inactive gateway if "true" (optional)
network_get_gateway6() {
	__network_ifstatus "$1" "$2" ".route[@.target='::' && !@.table].nexthop" "" 1 && \
		return 0

	[ "$3" = 1 -o "$3" = "true" ] && \
		__network_ifstatus "$1" "$2" ".inactive.route[@.target='::' && !@.table].nexthop" "" 1
}

# determine the DNS servers of the given logical interface
# 1: destination variable
# 2: interface
# 3: consider inactive servers if "true" (optional)
network_get_dnsserver() {
	__network_ifstatus "$1" "$2" "['dns-server'][*]" && return 0

	[ "$3" = 1 -o "$3" = "true" ] && \
		__network_ifstatus "$1" "$2" ".inactive['dns-server'][*]"
}

# determine the domains of the given logical interface
# 1: destination variable
# 2: interface
# 3: consider inactive domains if "true" (optional)
network_get_dnssearch() {
	__network_ifstatus "$1" "$2" "['dns-search'][*]" && return 0

	[ "$3" = 1 -o "$3" = "true" ] && \
		__network_ifstatus "$1" "$2" ".inactive['dns-search'][*]"
}


# 1: destination variable
# 2: addr
# 3: inactive
__network_wan()
{
	__network_ifstatus "$1" "" \
		"[@.route[@.target='$2' && !@.table]].interface" "" 1 && \
			return 0

	[ "$3" = 1 -o "$3" = "true" ] && \
		__network_ifstatus "$1" "" \
			"[@.inactive.route[@.target='$2' && !@.table]].interface" "" 1
}

# find the logical interface which holds the current IPv4 default route
# 1: destination variable
# 2: consider inactive default routes if "true" (optional)
network_find_wan() { __network_wan "$1" "0.0.0.0" "$2"; }

# find the logical interface which holds the current IPv6 default route
# 1: destination variable
# 2: consider inactive dafault routes if "true" (optional)
network_find_wan6() { __network_wan "$1" "::" "$2"; }

# test whether the given logical interface is running
# 1: interface
network_is_up()
{
	local __up
	__network_device __up "$1" up && [ $__up -eq 1 ]
}

# determine the protocol of the given logical interface
# 1: destination variable
# 2: interface
network_get_protocol() { __network_ifstatus "$1" "$2" ".proto"; }

#determine the autostart variable of the given logical interface
# 1: destination variable
# 2: interface
network_get_autostart() { __network_ifstatus "$1" "$2" ".autostart"; }

# determine the layer 3 linux network device of the given logical interface
# 1: destination variable
# 2: interface
network_get_device() { __network_ifstatus "$1" "$2" ".l3_device"; }

# determine the layer 2 linux network device of the given logical interface
# 1: destination variable
# 2: interface
network_get_physdev() { __network_ifstatus "$1" "$2" ".device"; }

# defer netifd actions on the given linux network device
# 1: device name
network_defer_device()
{
	ubus call network.device set_state \
		"$(printf '{ "name": "%s", "defer": true }' "$1")" 2>/dev/null
}

# continue netifd actions on the given linux network device
# 1: device name
network_ready_device()
{
	ubus call network.device set_state \
		"$(printf '{ "name": "%s", "defer": false }' "$1")" 2>/dev/null
}

# flush the internal value cache to force re-reading values from ubus
network_flush_cache() { unset __NETWORK_CACHE; }

#NXP custom functions:
__network_device()
{
	local __var="$1"
	local __iface="$2"
	local __field="$3"

	local __tmp="$(ubus call network.interface."$__iface" status 2>/dev/null)"
	[ -n "$__tmp" ] || return 1
	json_load "$__tmp"
	json_get_var "$__var" "$__field"
}

network_active_wan_interfaces() {
	local up
	local ifname 
	local __iface
	local i	
	
	ifname=$(ubus list | sed -ne 's/^network\.interface\.//p' | grep "wan\|usb" | grep -v "_PD")
	for __iface in $ifname; do
		proto=`uci get network.$__iface.proto 2>/dev/null`
		case $proto in
			qmi)
				__iface="$__iface"_4
			;;
			mobile)
				detect=$(cat /var/USBCONNSTATUS 2>/dev/null | grep -i $__iface | awk -F: '{printf $4}')
				if [ "$detect" = "4G" ]; then
					__iface="$__iface"_4
				fi
			;;
			"")
				continue
			;;
		esac
		__network_device up "$__iface" up
		if [ "$up" == "1" ];then
			i=$i' '$__iface
		fi
	done
	i=$(echo $i | sed s/^' '//g)
	eval "export -- \"$1=\$i\""
}

network_active_wan_v6_interfaces() {
	local up
	local ifname
	local __iface
	local i

	ifname=$(ubus list | grep -v "_PD" | grep "wan16\|wan26\|wan1_tun1\|wan1_tun2\|wan2_tun1\|wan2_tun2" | awk -F . '{print $3}')
	for __iface in $ifname; do
		__network_device up "$__iface" up
		if [ "$up" == "1" ];then
			i=$i' '$__iface
		fi
	done
	i=$(echo $i | sed s/^' '//g)
	eval "export -- \"$1=\$i\""
}

network_get_physdev_cached()
{
	local cache
	eval cache=\$interface_physdev_$2
	if [ -n "$cache" ]
	then
		eval $1=\$cache
	else
		network_get_physdev $1 $2
		eval interface_physdev_$2=\$$1
	fi
}

network_active_wan_interfaces_cached()
{
	local cache
	eval cache=\$active_wan_interfaces
	if [ -n "$cache" ]
	then
		eval $1=\$cache
	else
		network_active_wan_interfaces $1 
		eval active_wan_interfaces=\$$1
	fi
}

network_all_wan_interfaces() {
	local up
	local ifname
	local __iface
	local i

	ifname=$(ubus list | sed -ne 's/^network\.interface\.//p' | grep "wan\|usb" | grep -v "_PD")
	for __iface in $ifname; do
		proto=`uci get network.$__iface.proto 2>/dev/null`
		case $proto in
			qmi)
				__iface="$__iface"_4
			;;
			mobile)
				detect=$(cat /var/USBCONNSTATUS 2>/dev/null | grep -i $__iface | awk -F: '{printf $4}')
				if [ "$detect" = "4G" ]; then
					__iface="$__iface"_4
				fi
			;;
			"")
				continue
			;;
		esac
		i=$i' '$__iface
	done

	i=$(echo $i | sed s/^' '//g)
	eval "export -- \"$1=\$i\""
}

network_all_wan_interfaces_cached(){
	local cache
	eval cache=\$all_wan_interfaces
	if [ -n "$cache" ]
	then
		eval $1=\$cache
	else
		network_all_wan_interfaces $1 
		eval all_wan_interfaces=\$$1
	fi
}

network_active_lan_interfaces() {
	local up
	local ifname 
	local __iface
	local i	
	
	ifname=$(ubus list | sed -ne 's/^network\.interface\.//p')
	ifname=$(echo $ifname | grep -o vlan[0-9]*)
	for __iface in $ifname; do
		__network_device up "$__iface" up
		if [ "$up" == "1" ];then
			i=$i' '$__iface
		fi
	done

	i=$(echo $i | sed s/^' '//g)
	eval "export -- \"$1=\$i\""
}

network_active_lan_interfaces_cached() {
	local cache
	eval cache=\$active_lan_interfaces
	if [ -n "$cache" ]
	then
		eval $1=\$cache
	else
		network_active_lan_interfaces $1 
		eval active_lan_interfaces=\$$1
	fi
}

network_active_lan_subnets() {
	local allLANIfaces __subnets
	network_active_lan_interfaces allLANIfaces
	for iface in $allLANIfaces
	do
		#echo "iface is $iface"
		network_get_subnet lan_subnet $iface
		local network=`ipcalc.sh $lan_subnet | grep NETWORK | cut -d = -f 2`
		local mask=`ipcalc.sh $lan_subnet | grep PREFIX | cut -d = -f 2`
		if [ "$__subnets" = "" ]
		then
			__subnets="$network/$mask"
		else
			__subnets="$network/$mask,$__subnets"
		fi
	done

	eval "export -- \"$1=\$__subnets\""
}

network_get_waniface()
{
	local iface
	local ifname
	local up
	local isUSB=`echo $2 | grep usb`
	
	[ -n "$isUSB" ] && {
		iface=$(echo $2 | awk '{print tolower($0)}' | sed s/-/_/g )
		proto=`uci get network.$iface.proto 2>/dev/null`
		case $proto in
			qmi)
				iface="$iface"_4
			;;
			mobile)
				detect=$(cat /var/USBCONNSTATUS 2>/dev/null | grep -i $iface | awk -F: '{printf $4}')
				if [ "$detect" = "4G" ]; then
					iface="$iface"_4
				fi
			;;
		esac

		network_get_device $1 $iface
		return
	}
	iface=$(echo $2 | awk '{print tolower($0)}' | sed s/-/_/g )
	ifname=$(echo $iface | sed s/wan[1-9]*/\&p/g)
	up=0
	__network_device up "$ifname" up
	if [ "$up" == "1" ];then
		network_get_device $1 $ifname
	else
		network_get_device $1 $iface
	fi
}	

network_get_waniface_cached()
{
	local cache
	eval cache=\$waniface_$2
	if [ -n "$cache" ]
	then
		eval $1=\$cache
	else
		network_get_waniface $1 $2
		eval waniface_$2=\$$1
	fi
}

network_get_device_cached()
{
	local cache
	eval cache=\$device_$2
	if [ -n "$cache" ]
	then
		eval $1=\$cache
	else
		network_get_device $1 $2
		eval device_$2=\$$1
	fi
}

network_get_mwaniface()
{
	local iface
	local ifname
	local up
	local isUSB=`echo $2 | grep usb`
	
	[ -n "$isUSB" ] && {
		iface=$(echo $2 | awk '{print tolower($0)}' | sed s/-/_/g )
		proto=`uci get network.$iface.proto 2>/dev/null`
		device=`uci get network.$iface.device 2>/dev/null`
		if [ "$proto" = "mobile" ]; then
			detect=$(cat /var/USBCONNSTATUS 2>/dev/null | grep -i $iface | awk -F: '{printf $4}')
			if [ "$detect" = "4G" ]; then
				proto=qmi
			else
				proto=3g
			fi
		fi
		if [ "$proto" = "3g" ]; then
			eval "export -- \"$1=3g-$iface\""
		elif [ "$proto" = "qmi" ]
		then
			network_get_device $1 "$iface"_4
		fi
		return
	}
	iface=$(echo $2 | awk '{print tolower($0)}' | sed s/-/_/g )
	ifname=$(echo $iface | sed s/wan[1-9]*/\&p/g)
	status=$(ifstatus $ifname | grep -o "available")
	if [ "$status" == "available" ];then
		proto=`uci get network.$ifname.proto 2>/dev/null`
		if [ "$proto" = "pppoe" ]; then
			eval "export -- \"$1=ppoe-$ifname\""
		else
			eval "export -- \"$1=\$proto-$ifname\""
		fi
	else
		network_get_physdev $1 $iface
	fi
}

network_get_mwaniface_cached()
{
	local cache
	eval cache=\$mwaniface_$2
	if [ -n "$cache" ]
	then
		eval $1=\$cache
	else
		network_get_mwaniface $1 $2
		eval mwaniface_$2=\$$1
	fi
}

network_get_mwanproto()
{
	#This function returns current active protocol on the given interface
	local iface
	local ifname
	local proto
	local isUSB=`echo $2 | grep usb`
	
	iface=$(echo $2 | awk '{print tolower($0)}' | sed s/-/_/g )
	[ -n "$isUSB" ] && {
		proto=`uci get network.$iface.proto 2>/dev/null`
		eval "export -- \"$1=$proto\""
		return
	}
	ifname=$(echo $iface | sed s/wan[1-9]*/\&p/g)
	status=$(ifstatus $ifname | grep -o "available")
	if [ "$status" == "available" ];then
		proto=`uci get network.$ifname.proto 2>/dev/null`
		eval "export -- \"$1=$proto\""
	else
		proto=`uci get network.$iface.proto 2>/dev/null`
		eval "export -- \"$1=$proto\""
	fi
}

network_get_mwanproto_cached()
{
	local cache
	eval cache=\$mwanproto_$2
	if [ -n "$cache" ]
	then
		eval $1=\$cache
	else
		network_get_mwanproto $1 $2
		eval mwanproto_$2=\$$1
	fi
}

network_get_wanip()
{
	local iface
	local ifname
	local up
	local isUSB=`echo $2 | grep usb`

	[ -n "$isUSB" ] && {
		iface=$(echo $2 | awk '{print tolower($0)}' | sed s/-/_/g )
		proto=`uci get network.$iface.proto 2>/dev/null`
		case $proto in
			qmi)
				iface="$iface"_4
			;;
			mobile)
				detect=$(cat /var/USBCONNSTATUS 2>/dev/null | grep -i $iface | awk -F: '{printf $4}')
				if [ "$detect" = "4G" ]; then
					iface="$iface"_4
				fi
			;;
		esac

		__network_ifstatus "$1" "$iface" "['ipv4-address'][0].address";
		return
	}

	iface=$(echo $2 | awk '{print tolower($0)}' | sed s/-/_/g )
	ifname=$(echo $iface | sed s/wan[1-9]*/\&p/g)
	__network_device up "$ifname" up
	if [ "$up" == "1" ];then
		#__network_ipaddr "$1" "$ifname" 4 0
		__network_ifstatus "$1" "$ifname" "['ipv4-address'][0].address";
	else
		#__network_ipaddr "$1" "$iface" 4 0
		__network_ifstatus "$1" "$iface" "['ipv4-address'][0].address";
	fi
}	

network_get_wanip_cached()
{
	local cache
	eval cache=\$_wan_iface_ipaddr_$2
	if [ -n "$cache" ]
	then
		eval $1=\$cache
	else
		network_get_wanip $1 $2
		eval _wan_iface_ipaddr_$2=\$$1
	fi
}

network_get_mwanip()
{
	local iface
	local ifname
	local up
	local isUSB=`echo $2 | grep usb`

	[ -n "$isUSB" ] && {

		iface=$(echo $2 | awk '{print tolower($0)}' | sed s/-/_/g )
		proto=`uci get network.$iface.proto 2>/dev/null`
		case $proto in
			qmi)
				iface="$iface"_4
			;;
			mobile)
				detect=$(cat /var/USBCONNSTATUS 2>/dev/null | grep -i $iface | awk -F: '{printf $4}')
				if [ "$detect" = "4G" ]; then
					iface="$iface"_4
				fi
			;;
		esac

		__network_ifstatus "$1" "$iface" "['ipv4-address'][0].address";
		return
	}

	iface=$(echo $2 | awk '{print tolower($0)}' | sed s/-/_/g )
	ifname=$(echo $iface | sed s/wan[1-9]*/\&p/g)
	status=$(ifstatus $ifname | grep -o "available")
	if [ "$status" == "available" ];then
		demand=`uci get network.$ifname.demand 2>/dev/null`
		if [ "$demand" = "" ]; then
			#__network_ipaddr "$1" "$ifname" 4 0
			__network_ifstatus "$1" "$ifname" "['ipv4-address'][0].address";
		else
			proto=`uci get network.$ifname.proto 2>/dev/null`
			if [ "$proto" = "pppoe" ]; then
				device_name=ppoe-$ifname
			else
				device_name=$proto-$ifname
			fi
			demand_ip=$(ifconfig $device_name | grep inet | awk -F : '{print $2}' | awk -F ' ' '{print $1}')
			eval "export -- \"$1=\$demand_ip\""
		fi
	else
		#__network_ipaddr "$1" "$iface" 4 0
		__network_ifstatus "$1" "$iface" "['ipv4-address'][0].address";
	fi
}

network_get_wanip6()
{
	local iface
	local ifname
	local up

	iface=$(echo $2 | awk '{print tolower($0)}' | sed s/-/_/g )
	ifname=$(echo $iface | sed s/wan[1-9]*/\&p/g)
	__network_device up "$ifname" up
	if [ "$up" == "1" ];then
		#__network_ipaddr "$1" "$ifname" 6 0
		network_get_ipaddr6 "$1" "$ifname"
	else
		ifname=$(echo $iface)
		#__network_ipaddr "$1" "$ifname" 6 0
		network_get_ipaddr6 "$1" "$ifname"
	fi
}

network_get_wanip6_cached(){
	local cache
	eval cache=\$_wan_iface_ipaddr6_$2
	if [ -n "$cache" ]
	then
		eval $1=\$cache
	else
		network_get_wanip6 $1 $2
		eval _wan_iface_ipaddr6_$2=\$$1
	fi
}

network_get_cli_to_gui_iface()
{
	local iface

	iface=$(echo $2 | awk '{print toupper($0)}' | sed s/_/-/g | sed s/P//g )
	eval "export -- \"$1=\$iface\""
}

network_get_subnet_cached()
{
	local cache
	eval cache=\$_iface_subnet_$2
	if [ -n "$cache" ]
	then
		eval $1=\$cache
	else
 		#__network_ipaddr "$1" "$2" 4 1; 
		network_get_subnet "$1" "$2";
		eval _iface_subnet_$2=\$$1
	fi
}

network_get_ipaddr_cached()
{
	local cache
	eval cache=\$_iface_ipaddr_$2
	if [ -n "$cache" ]
	then
		eval $1=\$cache
	else
 		#__network_ipaddr "$1" "$2" 4 0;
		network_get_ipaddr "$1" "$2";
		eval _iface_ipaddr_$2=\$$1
	fi
}

__network_get_qosstats_v4()
{
	local __iface="$1"
	local __ifstatus="$2"
	local __phy_ifname_l2="$3"
	local __phy_ifname_l3="$4"

	local __tmp="$(ubus call network.interface."$__iface" status 2>/dev/null)"

	json_load "${__tmp:-{}}"

	json_get_var "$__phy_ifname_l3" "l3_device"
	json_get_var "$__phy_ifname_l2" "device"
	json_get_var "$__ifstatus" "up"
}

__network_get_qosstats_v6()
{
	local __iface="$1"
	local __ifstatus="$2"
	local __phy_ifname_l2="$3"
	local __phy_ifname_l3="$4"

	local __tmp1="$(ubus call network.interface."$__iface" status 2>/dev/null)"

	json_load "${__tmp1:-{}}"
	json_get_var "$__phy_ifname_l3" "l3_device"
	json_get_var "$__phy_ifname_l2" "device"
	json_get_var "$__ifstatus" "up"
}

__network_get_fwstats_v4()
{
	local __iface="$1"
	local __ifstatus="$2"
	local __ip4="$3"
	local __ip4_mask="$4"
	local __phy_ifname_l2="$5"
	local __phy_ifname_l3="$6"

	local __tmp="$(ubus call network.interface."$__iface" status 2>/dev/null)"

	json_load "${__tmp:-{}}"

	json_get_var "$__phy_ifname_l3" "l3_device"
	json_get_var "$__phy_ifname_l2" "device"
	json_get_var "$__ifstatus" "up"

	json_get_type __tmp "ipv4_address"
	if [ "$__tmp" = array ]; then
		json_select "ipv4_address"
		json_get_type __tmp 1
		if [ "$__tmp" = object ]; then
			json_select 1
			json_get_var $__ip4 address
			json_get_var $__ip4_mask mask
		fi
	fi
}

__network_get_fwstats_v6()
{
	local __iface="$1"
	local __ifstatus="$2"
	local __ip6="$3"
	local __ip6_prefix="$4"
	local __phy_ifname_l2="$5"
	local __phy_ifname_l3="$6"

	local __tmp1="$(ubus call network.interface."$__iface" status 2>/dev/null)"

	json_load "${__tmp1:-{}}"
	json_get_var "$__phy_ifname_l3" "l3_device"
	json_get_var "$__phy_ifname_l2" "device"
	json_get_var "$__ifstatus" "up"

	json_get_type __tmp "ipv6_address"
	if [ "$__tmp" = array ]; then
		json_select "ipv6_address"
		json_get_type __tmp 1
		if [ "$__tmp" = object ]; then
			json_select 1
			json_get_var $__ip6 address
			json_get_var $__ip6_prefix mask
		fi
	fi
}

__network_getall_stats_v4()
{
	local __iface="$1"
	local __ifstatus="$2"
	local __ip4="$3"
	local __ip4_mask="$4"
	local __phy_ifname_l2="$5"
	local __phy_ifname_l3="$6"
        local __dns_out1="$7"
        local __dns_out2="$8"
	local __gw="$9"
        local __dns1=""
        local __dns2=""
	local __idx=1
	local __dnscount=1
	local __isvlan=

	local __tmp="$(ubus call network.interface."$__iface" status 2>/dev/null)"
	local __tmp1="$__tmp"
	local __tmp2="$__tmp"

	json_load "${__tmp:-{}}"
	
	json_get_var "$__phy_ifname_l3" "l3_device"
	json_get_var "$__phy_ifname_l2" "device"
	json_get_var "$__ifstatus" "up"

        json_get_type __tmp "ipv4_address"
	if [ "$__tmp" = array ]; then
	        json_select "ipv4_address"
        	json_get_type __tmp 1
        	if [ "$__tmp" = object ]; then
	                json_select 1
        	        json_get_var $__ip4 address
                        json_get_var $__ip4_mask mask
        	fi
	fi

	#dns
        __field="dns_server"
        json_load "${__tmp1:-{}}"
        if json_get_type __tmp1 "$__field" && [ "$__tmp1" = array ]; then
		json_select "$__field"
		__isvlan=`echo $__iface | grep "vlan"`
		if [ -z "$__isvlan" ];then
               		 while json_get_type __tmp1 "$__idx" && [ "$__tmp1" = string ]; do
	       		 	json_get_var __tmp "$((__idx++))"
	    			is_v6=`echo $__tmp | grep ":"`
	    			[ -z "$is_v6" ] && {
	       			 	[ "$__dnscount" = 1 ] && __dns1=$__tmp
               		  		[ "$__dnscount" = 2 ] && __dns2=$__tmp && break
		       		 	__dnscount=$((__dnscount+1))
	    			}
               		 done
		else
               		 while json_get_type __tmp1 "$__idx" && [ "$__tmp1" = string ]; do
	       		 	json_get_var __tmp "$((__idx++))"
	       		 	# avoid v6 address
	       		 	__isvlan=`echo $__tmp | grep ":"`
	       		 	[ -z "$__isvlan" ] && {	 
	       		 		[ "$__dnscount" = 1 ] && __dns1=$__tmp 
               		         	[ "$__dnscount" = 2 ] && __dns2=$__tmp && break
	       		 		__dnscount=$((__dnscount+1))
	       		 	}
               		 done
		fi
        fi
	eval "export -- \"$__dns_out1=$__dns1\""
        eval "export -- \"$__dns_out2=$__dns2\""

	# gateway
	local __family="4"
	__idx=1
	json_load "${__tmp2:-{}}"
        if json_get_type __tmp2 route && [ "$__tmp2" = array ]; then
                json_select route
                while json_get_type __tmp2 "$__idx" && [ "$__tmp2" = object ]; do
                        json_select "$((__idx++))"
                        json_get_var __tmp target
                        case "${__family}/${__tmp}" in
                                4/0.0.0.0|6/::)
                                        json_get_var "$__gw" nexthop
                                        return $?
                                ;;
                        esac
                        json_select ".."
                done
        fi
}

__network_getall_stats_v6()
{
	local __iface="$1"
	local __ifstatus="$2"
	local __phy_ifname_l2="$3"
	local __phy_ifname_l3="$4"
        local __dns_out1="$5"
        local __dns_out2="$6"
	local __gw="$7"
        local __dns1=""
        local __dns2=""
	local __idx=1
	local __dnscount=1
	local __isvlan=
	local __temp_pd=

	local __tmp1="$(ubus call network.interface."$__iface" status 2>/dev/null)"
	local __tmp2="$__tmp1"

	json_load "${__tmp1:-{}}"
	json_get_var "$__phy_ifname_l3" "l3_device"
	json_get_var "$__phy_ifname_l2" "device"
	json_get_var "$__ifstatus" "up"

	#dns
        __field="dns_server"
        if json_get_type __tmp1 "$__field" && [ "$__tmp1" = array ]; then
		json_select "$__field"
		__isvlan=`echo $__iface | grep "vlan"`
		if [ -z "$__isvlan" ];then
               	 	while json_get_type __tmp1 "$__idx" && [ "$__tmp1" = string ]; do
	       	 		json_get_var __tmp "$((__idx++))"
		    		is_v4=`echo $__tmp | grep ":"`
			    	[ -z "$is_v4" ] || {
	       	 			[ "$__dnscount" = 1 ] && __dns1=$__tmp 
               	         		[ "$__dnscount" = 2 ] && __dns2=$__tmp && break
	       	 			__dnscount=$((__dnscount+1))
			    	}
               	 	done
		else
                	while json_get_type __tmp1 "$__idx" && [ "$__tmp1" = string ]; do
				json_get_var __tmp "$((__idx++))"
				# avoid v4 address
				__isvlan=`echo $__tmp | grep ":"`
				[ -n "$__isvlan" ] && {	 
					[ "$__dnscount" = 1 ] && __dns1=$__tmp 
                        		[ "$__dnscount" = 2 ] && __dns2=$__tmp && break
					__dnscount=$((__dnscount+1))
				}
                	done
		fi
        fi
	eval "export -- \"$__dns_out1=$__dns1\""
        eval "export -- \"$__dns_out2=$__dns2\""

	# gateway
	local __family="6"
	__idx=1
	json_load "${__tmp2:-{}}"
        if json_get_type __tmp2 route && [ "$__tmp2" = array ]; then
                json_select route
                while json_get_type __tmp2 "$__idx" && [ "$__tmp2" = object ]; do
                        json_select "$((__idx++))"
                        json_get_var __tmp target
                        case "${__family}/${__tmp}" in
                                4/0.0.0.0|6/::)
                                        json_get_var "$__gw" nexthop
                                        return $?
                                ;;
                        esac
                        json_select ".."
                done
        fi
}

