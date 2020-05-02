#!/bin/sh /etc/rc.common

#As of now, we are handling only for demand interfaces for route syncup with mwan3 infra
if [ "$__this_device" == "RV160" ] || [ "$__this_device" == "RV160W" ]
then
	exit 0
fi

for table in `ip rule show | grep fwmark | grep lookup | grep 10.64. | awk -F ' ' '{print $7}' | sort -u`
do
	[ -n "$(ip -4 route show table $table)" ] || {
		#gwip=$(ip rule show  | grep "lookup 4" | head -n 1 | awk -F ' ' '{print $3}')
		
		routecmd=`ip route show default 0.0.0.0/0 | grep 10.112. | awk -F ' ' '{$6="";$7="";print}'`
		ip route add $routecmd table $table
	}
done

