#!/bin/sh
. /etc/boardInfo

# Change log
#
# 2017/07/28: li.zhang<li.zhang@deltaww.com>
# - Fixed bug: after the LAN dhcp change the huawei nat dogle should reconnect to get readonly stats
# 2017/03/23: qiuhong.su<qiuhong.su@deltaww.com>
# - Fixed bug: after modify LAN/VPN Web Management, it not take effect.
#   Soultion: Add "/etc/init.d/mini_httpd.init reload" into function firewall()

# 2017/04/06: jiangzhe.guo<jiangzhe.guo@deltaww.com>
# - Add license part

. /lib/functions/network.sh
. /lib/config/uci.sh

board=`uci get systeminfo.sysinfo.pid | awk -F '-' '{print $1}'`

dhcp() {
	local retval=1
	uci commit dhcp
	/etc/init.d/dnsmasq reload
	
	# 4G nat dongle reconnect
	temp=`lsusb | grep "Bus 003" | grep -v "Device 001" | cut -d " " -f 6`
	if [ "$temp" = "12d1:14db" ] || [ "$temp" = "12d1:14dc" ] || [ "$temp" = "1199:9055" ];then
		echo "1" > /sys/devices/platform/dwc_otg.0/usb3/bConfigurationValue
	fi

	retval=$?
	return $retval
}

radvd() {
	local retval=1
	uci commit radvd
	/etc/init.d/radvd reload
	
	retval=$?
	return $retval
}

swupdate() {
	local retval=1
	/etc/init.d/swupdateinit restart
	retval=$?
	return $retval
}

poeconf() {
	local retval=1
	/etc/init.d/poeinit reload
	retval=$?
	return $retval
}

snmpconf() {
	local retval=1
	uci commit snmpconf
	/etc/init.d/snmpinit reload
	retval=$?
	return $retval
}

aaa() {
	local retval=1
	uci commit aaa
	retval=$?
	return $retval
}

switch() {
	local retval=1
	local old_status
	local new_status
	local old_dev_mgmt

	if [ "$board" = "RV160" -o "$board" = "RV160W" -o "$board" = "RV260" -o "$board" = "RV260P" -o "$board" = "RV260W" ]; then
		old_status=$(uci get /tmp/etc/config/switch.dot1x_status.global_status)
		cp /tmp/.uci/switch /tmp/switch_config
		cp /tmp/.uci/qos_switch /tmp/qos_switch_config
		new_status=$(uci get switch.dot1x_status.global_status)
        elif [ "$board" = "RV340" -o "$board" = "RV340W" -o "$board" = "RV345" -o "$board" = "RV345P" ]; then
		old_status=$(uci get /tmp/etc/config/switch.dot1x_status.state)
	        cp /tmp/.uci/switch /tmp/switch_config
        	cp /tmp/.uci/qos_switch /tmp/qos_switch_config
        	new_status=$(uci get switch.dot1x_status.state)
        fi

	# To avoid multiple ip table rules in device_management
	touch /tmp/switch-bkp
	cp /tmp/etc/config/switch /tmp/switch-bkp


	local inter_vlan_cmd=`cat /tmp/.uci/switch |grep routing |cut -f 1 -d=`
	if [ "$inter_vlan_cmd" != "" ]; then
   		local old_inter_vlan=`uci get /tmp/etc/config/$inter_vlan_cmd`
	fi

	local dev_mgmt_cmd=`cat /tmp/.uci/switch |grep dev_mgmt |cut -f 1 -d=`
	if [ "$dev_mgmt_cmd" != "" ]; then
   		old_dev_mgmt=`uci get /tmp/etc/config/$dev_mgmt_cmd`
	fi

	uci commit switch
	uci commit qos_switch
	/etc/init.d/switch reload
	
	if [ "$inter_vlan_cmd" != "" ]; then                                  
		local new_inter_vlan=`uci get $inter_vlan_cmd`                 
		if [ "$new_inter_vlan" == "disable" ] && [ "$old_inter_vlan" == "enable" ]; then        
			/usr/bin/clearconnection.sh 2>&- >&-                                            
		fi                                                             
	fi    
	
	if [ "$dev_mgmt_cmd" != "" ]; then                                  
		local new_dev_mgmt=`uci get $dev_mgmt_cmd`                 
		if [ "$new_dev_mgmt" == "disable" ] && [ "$old_dev_mgmt" == "enable" ]; then        
			/usr/bin/clearconnection.sh 2>&- >&-                                            
		fi
	fi

	if [ $new_status != $old_status ]
	then
		/etc/init.d/dot1x restart
	else
		/etc/init.d/dot1x reload
	fi
	
	retval=$?
	return $retval
}

wanport() {
       local retval=1
       /etc/init.d/wanport reload
       retval=$?
       return $retval
}

bonjour() {
	local retval=1
	uci commit bonjour
	/etc/init.d/bonjour reload
	
	retval=$?
	return $retval
}

dmz() {
	local retval=1
	#uci commit dmz
	/etc/init.d/hardwaredmz reload
	
	retval=$?
	return $retval
}


systemconfig() {
	local retval=1
	/etc/init.d/systemconfig reload
	
	retval=$?
	
	if [ "$?" = 0 ];then
		dhcp
                retval=$?
                if [ $retval -gt 0 ]
                then
			return $retval
		fi	
		
		bonjour
                retval=$?
                if [ $retval -gt 0 ]
                then
			return $retval
		fi	
	else
		return $retval
	fi
	
}
email() {
	local retval=1
	uci commit email
	/etc/init.d/email reload
	
	retval=$?
	return $retval
}

syslog() {
	SYSLOG_NG_VERSION="3.0.8"
        old_value=`uci get /tmp/etc/config/syslog.log_server.enable`
        new_value=`uci get syslog.log_server.enable`
        if [ "$new_value" -eq 0 ];then
                logger -t system -p local0.notice "syslog-ng version:$SYSLOG_NG_VERSION stopped."
        fi

	local retval=1
	uci commit syslog
#	/etc/init.d/syslog reload
	/etc/init.d/syslog restart
	retval=$?

	if [ "$old_value" -ne "$new_value" ] && [ "$new_value" -eq 1 ];then
                logger -t system -p local0.notice "syslog-ng version:$SYSLOG_NG_VERSION started."
        fi
	return $retval
}

system() {
	local retval=1
	uci commit system
	/etc/init.d/system reload
	
	retval=$?
	return $retval
}

testserver() {
	local retval=1
	/usr/bin/testserver

	retval=$?
	return $retval
}

certificate() {
	local retval=1
	local updateCert=1	
	if [ ! -e "/tmp/.uci/certificate" ];then
		echo "no config"
		return $retval
	fi
		
	check_action=`cat /tmp/.uci/certificate | grep "delete_certificate"`
	if [ -n "$check_action" ];then
		/etc/init.d/certificate delete_certificate
		retval=$?
	else
		check_action=`cat /tmp/.uci/certificate | grep "=generated_certificate"`
		if [ -n "$check_action" ];then
			/etc/init.d/certificate generate_certificate
			retval=$?
		else
			check_action=`cat /tmp/.uci/certificate | grep "imported_certificate"`
			if [ -n "$check_action" ];then
				/etc/init.d/certificate import_certificate
				retval=$?
			else
				check_action=`cat /tmp/.uci/certificate | grep "exported_certificate"`
				if [ -n "$check_action" ];then
					/etc/init.d/certificate export_certificate
					retval=$?
					updateCert=0
				fi
			fi
		fi
	fi


	# copy certificate to config partition for success cases
	[ "$retval" = 0 ] && {
		cp -f $TMP_CERT_FILE $MNT_CERT_FILE
		cp -f $TMP_PREINSTALLED_CERT_FILE $MNT_PREINSTALLED_CERT_FILE
	}
	
	# Update the certs opsdb
	#[ "$updateCert" = 1 ] && /usr/bin/certscript

	return $retval
}

schedule() {

	local retval=0
	
	cp /tmp/.uci/schedule /tmp/scheduledeltaconfig
	cp /tmp/.uci/schedule /tmp/scheddeltaconfig
	uci commit schedule	
	# Calling Dependent module function to update the changed schedule configuration
	# <modulescriptname>  <command>  <arg1:subcommand> <arg2:scheduledeltaconfigfilepath>
	/etc/init.d/webfilter sched update /tmp/scheduledeltaconfig
	
	retval=$?
	
	if [ $retval -gt 0 ]
	then
		return $retval
	else
		avc
		retval=$?
		if [ $retval -gt 0 ]
		then
			return $retval
		fi
		
		firewall_sched	
		retval=$?
		if [ $retval -gt 0 ]
		then
			return $retval
		fi
		
		/etc/init.d/schedule reload	
		retval=$?
		if [ $retval -gt 0 ]
		then
			return $retval
		fi
		
		
	fi
	return $retval
}

policynat(){
	local retval=0                                                                       
        /etc/init.d/firewall firewall_policynat_reload                                          
        retval=$?                                                                            
        return $retval
}

ipgroup() {

	local retval=0
	
	cp /tmp/.uci/ipgroup /tmp/ipgroupdeltaconfig
	uci commit ipgroup	
	# Calling Dependent module function to update the changed ipgroup configuration
	# <modulescriptname>  <command>  <arg1:subcommand> <arg2:ipgrpdeltaconfigfilepath>
	/etc/init.d/webfilter   ipgrp     update      /tmp/ipgroupdeltaconfig
	
	retval=$?
	
	if [ $retval -gt 0 ]
	then
		return $retval
	else
		policynat
		retval=$?                                                                    
                if [ $retval -gt 0 ]                                                         
                then                                                                         
                        return $retval                                                       
                fi  

		avc	
		retval=$?
		if [ $retval -gt 0 ]
		then
			return $retval
		fi
		
		/etc/init.d/ipgroup reload	
		retval=$?
		if [ $retval -gt 0 ]
		then
			return $retval
		fi
		
		
	fi
	return $retval
}

schedule_run() {

	local retval=0
	local depmod=$@

	cp /tmp/.uci/schedule /tmp/scheduledeltaconfig
	cp /tmp/.uci/schedule /tmp/scheddeltaconfig
	uci commit schedule
	# Calling Dependent module function to update the changed schedule configuration
	# <modulescriptname>  <command>  <arg1:subcommand> <arg2:scheduledeltaconfigfilepath>
	if [ $(echo $depmod | grep webfilter) ];then
		/etc/init.d/webfilter sched update /tmp/scheduledeltaconfig
		retval=$?
	else
		retval=0
	fi


	if [ $retval -gt 0 ]
	then
		return $retval
	else
		if [ $(echo $depmod | grep avc) ];then
			avc
			retval=$?
		else
			retval=0
		fi
		if [ $retval -gt 0 ]
		then
			return $retval
		fi

		if [ $(echo $depmod | grep firewall) ];then
			firewall_sched
			retval=$?
		else
			retval=0
		fi
		if [ $retval -gt 0 ]
		then
			return $retval
		fi

		/etc/init.d/schedule reload
		retval=$?
		if [ $retval -gt 0 ]
		then
			return $retval
		fi


	fi
	return $retval
}

ipgroup_run() {

	local retval=0
	local depmod=$@

	cp /tmp/.uci/ipgroup /tmp/ipgroupdeltaconfig
	uci commit ipgroup
	# Calling Dependent module function to update the changed ipgroup configuration
	# <modulescriptname>  <command>  <arg1:subcommand> <arg2:ipgrpdeltaconfigfilepath>

	if [ $(echo $depmod | grep webfilter) ];then
		/etc/init.d/webfilter   ipgrp     update      /tmp/ipgroupdeltaconfig
		retval=$?
	else
		retval=0
	fi


	if [ $retval -gt 0 ]
	then
		return $retval
	else
		if [ $(echo $depmod | grep avc) ];then
			avc
			retval=$?
		else
			retval=0
		fi

		if [ $retval -gt 0 ]
		then
			return $retval
		fi

		/etc/init.d/ipgroup reload
		retval=$?
		if [ $retval -gt 0 ]
		then
			return $retval
		fi


	fi
	return $retval
}

webfilter() {

	local retval=0
	
	/etc/init.d/webfilter reload
	uci commit webfilter
	
	retval=$?
	return $retval
	
}

webfilter_run() {

	local retval=0

	/etc/init.d/webfilter reload
	uci commit webfilter
	logger -t ucicfg "Completed webfilter module"

	return 0

}

avc() {

	local retval=0

    if [ "$board" = "RV160" -o "$board" = "RV160W" -o "$board" = "RV260" -o "$board" = "RV260P" -o "$board" = "RV260W" ]; then
        return 0
    fi
#all lionic configuration's are present in avc file	
	uci commit avc
	/etc/init.d/avc reload
	
	retval=$?
	return $retval
	
}

mwan3_full_tunnel_config() {
	#This function has to figure out a GRE route addition/deletion event and act appropriately.
	if [ "$board" = "RV260" -o "$board" = "RV260P" -o "$board" = "RV260W" ]; then
		default_route_iface=`cat /tmp/networkconfig | sed s/"'"//g | grep -E "network.route_[0-9]+.target=0.0.0.0" -A 6 | grep "interface=" | sed s/_static// | awk -F '=' '{print $2}'`
		[ -n "$default_route_iface" ] && {
			#This is a default route addition.
			is_gre_default_route=`uci get network.$default_route_iface.proto`
			if [ "$is_gre_default_route" = "gre" ]
			then # GRE default route addition.
			#{
				#If we are inside this, it means that we are dealing with a default route on GRE interface. Set mwan3 module for RV260 device.
				is_already_configured=`uci_get_state mwan3.full_tunnel.tunnel_name`
				if [ "$is_already_configured" = "" ]
				then #Which means it is not configured earlier. Now, is the time to configure.
				#{
					uci set mwan3.$default_route_iface=interface
					uci set mwan3.$default_route_iface.enabled=1
					uci set mwan3.$default_route_iface.log=1
					uci set mwan3."$default_route_iface"_mem=member
					uci set mwan3."$default_route_iface"_mem.interface=$default_route_iface
					uci set mwan3."$default_route_iface"_mem.metric=0
					uci set mwan3."$default_route_iface"_mem.weight=100
					uci set mwan3."$default_route_iface"_only=policy
					uci add_list mwan3."$default_route_iface"_only.use_member="$default_route_iface"_mem
					uci set mwan3.default_gre=rule
					uci set mwan3.default_gre.dest_ip=0.0.0.0/0
					uci set mwan3.default_gre.use_policy="$default_route_iface"_only

					uci_toggle_state mwan3 full_tunnel tunnel_name "$default_route_iface"
					mwan3

					logger -t mwan3 "Configuring mwan3 for Full tunnel over GRE interface $default_route_iface."
					return 0
				#}
				fi
			#}
			fi

		}

		del_route=`grep -E "\-network.route_[0-9]+$" /tmp/networkconfig`
		[ -n "$del_route" ] && {
			#A route deletion case.
			#Check if a default route on GRE is already added. Only then we need to act further, else leave it.
			is_already_configured=`uci_get_state mwan3.full_tunnel.tunnel_name`
			if [ "$is_already_configured" != "" ]
			then
			#{
				#There is a default route on GRE added earlier. So figure out if the same route is deleted.
				#	If the same route is deleted then we may need to deconfigure the route from mwan3.
				find=0
				local get_iface
				all_default_routes=`uci show network | grep -E "network.route_[0-9]+.target=0.0.0.0" | awk -F '.' '{print $2}'`
				if [ "$all_default_routes" != "" ] 
				then
				#{
					for section in $all_default_routes
					do
						get_iface=`uci get network.$section.interface | sed s/_static//`
						if [ "gre_$get_iface" = "gre_$is_already_configured" ]
						then
						#Your default route on GRE is not deleted.
							find=1
						fi
					done
				#}
				else
					#No default route after some deletions. So, even our route of interest also got deleted.
					get_iface="$is_already_configured"
				fi
				if [ $find -eq 0 ]
				then
				#{
				#your route is the one which is deleted. Hence deconfigure in mwan3.
					uci delete mwan3.$get_iface
					uci delete mwan3."$get_iface"_mem
					uci delete mwan3."$get_iface"_only
					uci delete mwan3.default_gre

					uci_toggle_state mwan3 full_tunnel tunnel_name ""
					mwan3

					logger -t mwan3 "Deleted Full tunnel over GRE interface $get_iface from mwan3 module."
					return 0
				#}
				fi
			#}
			fi
		}
	fi
}

network_onboot() {
	/etc/init.d/abootpattern1 boot

	/etc/init.d/wanport start_setstatus
	local retval=0

        ifaces=$(cat /tmp/networkconfig | sed s/"'"//g | grep "=interface" | cut -f 2 -d . | cut -f 1 -d = )
        for if in $ifaces
        do
		enabled=`uci get network.$if.enable` 2>&- >&-
                mtu_p=`uci get /tmp/etc/config/network.$if.mtu` 2>&- >&-
                mtu_n=`uci get network.$if.mtu` 2>&- >&-
                [ -n "$mtu_p" ] && [ "$(cat /tmp/networkconfig  | grep "network.$if.mtu")" = "" ] && {
	                if [ "$enabled" != 0 ];then
				echo "network.$if.mtunew=1500" >>/tmp/networkconfig
                        fi
                }
                #logger -t testing "############# prev mtu $mtu_p new mtu $mtu_n"
                if [ "$mtu_p" = "$mtu_n" ]; then
			cat /tmp/networkconfig  | grep -v "network.$if.mtu" >/tmp/networkconfig1
                fi
                cp /tmp/networkconfig1 /tmp/networkconfig

                mac_p=`uci get /tmp/etc/config/network.$if.macaddr` 2>&- >&-
                mac_n=`uci get network.$if.macaddr` 2>&- >&-
                [ -n "$mac_p" ] && [ "$(cat /tmp/networkconfig  | grep "network.$if.macaddr")" = "" ] && {
                        [ -n "$(echo $if | grep wan1)" ] && {
                                macwan=$(uci get systeminfo.sysinfo.macwan1)
                        }

                        [ -n "$(echo $if | grep wan2)" ] && {
                                macwan=$(uci get systeminfo.sysinfo.macwan2)
                        }

	                if [ "$enabled" != 0 ];then
	                        echo "network.$if.macaddrnew=$macwan" >>/tmp/networkconfig
	                fi
                }
                if [ "$mac_p" = "$mac_n" ]; then
                        cat /tmp/networkconfig  | grep -v "network.$if.macaddr" >/tmp/networkconfig1
                fi
                cp /tmp/networkconfig1 /tmp/networkconfig

        done
        rm /tmp/networkconfig1 >/dev/null 2>&1

	/sbin/mtu-mac-enable reload "gui" &

	/etc/init.d/network reload
	/etc/init.d/wanport start_setwanport
	mwan3_full_tunnel_config

	rm /tmp/networkconfig >/dev/null 2>&1
	retval=$?
}

network() {

	local retval=0

	cp /tmp/etc/config/network /tmp/etc/config/network-bkp
	cp /tmp/.uci/network /tmp/networkconfig	

	isvlan=$(grep "network.vlan" /tmp/networkconfig)
        ifaces=$(cat /tmp/networkconfig | sed s/"'"//g | grep "=interface" | cut -f 2 -d . | cut -f 1 -d = )
        for if in $ifaces
        do
		enabled=`uci get network.$if.enable` 2>&- >&-
                mtu_p=`uci get /tmp/etc/config/network.$if.mtu` 2>&- >&-
                mtu_n=`uci get network.$if.mtu` 2>&- >&-
                [ -n "$mtu_p" ] && [ "$(cat /tmp/networkconfig  | grep "network.$if.mtu")" = "" ] && {
	                if [ "$enabled" != 0 ];then
                        	echo "network.$if.mtunew=1500" >>/tmp/networkconfig
                        fi
                }
                #logger -t testing "############# prev mtu $mtu_p new mtu $mtu_n"
                if [ "$mtu_p" = "$mtu_n" ]; then
                        cat /tmp/networkconfig  | grep -v "network.$if.mtu" >/tmp/networkconfig1
			cp /tmp/networkconfig1 /tmp/networkconfig
                fi

                mac_p=`uci get /tmp/etc/config/network.$if.macaddr` 2>&- >&-
                mac_n=`uci get network.$if.macaddr` 2>&- >&-
                [ -n "$mac_p" ] && [ "$(cat /tmp/networkconfig  | grep "network.$if.macaddr")" = "" ] && {
                        [ -n "$(echo $if | grep wan1)" ] && {
                                macwan=$(uci get systeminfo.sysinfo.macwan1)
                        }

                        [ -n "$(echo $if | grep wan2)" ] && {
                                macwan=$(uci get systeminfo.sysinfo.macwan2)
                        }

	                if [ "$enabled" != 0 ];then
	                        echo "network.$if.macaddrnew=$macwan" >>/tmp/networkconfig
	                fi
                }
                if [ "$mac_p" = "$mac_n" ]; then
                        cat /tmp/networkconfig  | grep -v "network.$if.macaddr" >/tmp/networkconfig1
			cp /tmp/networkconfig1 /tmp/networkconfig
                fi

        done
        rm /tmp/networkconfig1

	/sbin/mtu-mac-enable reload "gui" &
	
	/etc/init.d/network reload
	
	retval=$?
	
	if [ $retval -gt 0 ]
	then
		return $retval
	else
		[ -n "$isvlan" ] && {
			dhcp
			retval=$?
			if [ $retval -gt 0 ]
			then
				return $retval
			fi
		
			bonjour
			retval=$?
			if [ $retval -gt 0 ]
			then
				return $retval
			fi
		}
	fi
	/sbin/netifd-sync reload &

	# Run the wanscript to get updated data
        /usr/bin/wanscript
        /usr/bin/sendOpsdbSignal.sh SIGHUP

	bwmgmt
	mwan3_full_tunnel_config

	rm /tmp/networkconfig
	return $retval
}

firewall_sched() {

	local retval=0 
	/etc/init.d/firewall firewall_sched_reload
	retval=$?
	return $retval
}

firewall() {
	local retval
	#uci commit firewall moved to firewallstart function.	
	/etc/init.d/firewall reload
	/etc/init.d/nginx reload
	/etc/init.d/mini_httpd.init reload
	retval=$?

	if [ $retval -gt 0 ]
	then
		return $retval
	else

		bonjour
		retval=$?
		if [ $retval -gt 0 ]
		then
			return $retval
		fi
	fi
	return $retval


}

upnpd() {
	local retval
    uci commit upnpd	
    /etc/init.d/miniupnpd reload
    retval=$?
    return $retval
}
radius() {
    local retval
    local pid
    uci commit radius
	/etc/init.d/radius reload
    [ -x "/usr/bin/rtdot1xd" ] && {
       pid=$(pgrep rtdot1xd)
       if [ -n "$pid" ]; then
          `kill -SIGUSR1 $pid`
       fi
    }
    retval=$?
    return $retval
}

ldap() {
    local retval
    uci commit ldap
    /etc/init.d/ldapclient reload
    retval=$?
    return $retval
}   

bwmgmt() {
    local retval
    uci commit bwmgmt
    /etc/init.d/bwmgmt reload
    retval=$?
    return $retval
}   

ad() {
   local retval
   uci commit ad
   /etc/init.d/ad reload
   retval=$?
   return $retval
}   

strongswan() {
	local retval

	/etc/init.d/strongswan reload
	retval=$?
	return $retval
}

pptpd() {
	local retval

	uci commit pptpd
	/etc/init.d/pptpd reload
	retval=$?
	return $retval
}

l2tpd() {
	local retval

	uci commit l2tpd
	/etc/init.d/l2tpd reload

	retval=$?

	if [ $retval -gt 0 ]
	then
		return $retval
	else
		strongswan
		retval=$?
		if [ $retval -gt 0 ]
		then
			return $retval
		fi
	fi
	return $retval
}

vpnpassthrough() {
	local retval

	uci commit vpnpassthrough
	/etc/init.d/vpnpassthrough reload
	retval=$?
	return $retval
}

vpnresourcemgmt() {
	local retval

	/etc/init.d/vpnresourcemgmt reload
	retval=$?
	return $retval
}

lldpd() {
	local retval

	uci commit lldpd
	/etc/init.d/lldpd restart
	retval=$?
	return $retval
}

ddns() {
	local retval
	
	cp /tmp/.uci/ddns /tmp/ddns
	uci commit ddns
   	/etc/init.d/ddns reload

     
	retval=$?
	return $retval
}

rip() {
	local retval
	
   	/etc/init.d/rip reload

     
	retval=$?
	return $retval
}

qos() {
	local retval
	
   	/etc/init.d/qos reload

     
	retval=$?
	return $retval
}


mwan3() {
	local retval

	uci reorder mwan3.default_rule=2000 2>&- >&-
	uci commit mwan3

	/etc/init.d/mwan3 reload &

	return 0
}

igmpproxy() {
	local retval

	uci commit igmpproxy

	/etc/init.d/mcproxy reload

	retval=$?
	return $retval
}

sslvpn() {
	local retval=1

	uci commit sslvpn
	uci show sslvpn > /tmp/1.log
	/etc/init.d/sslvpn reload

	retval=$?
	return $retval
}

firewall_dmz(){                                                                                         
        local retval=0                                                                                                            
        /etc/init.d/firewall firewall_dmz_reload                                                     
        retval=$?                                                                                          
        return $retval                                                                                     
}  
openvpn() {
	local retval=1
	local new_status
	local old_status
	local cipher
	local local_addr
	local temp
	local ca_cert
	local local_cert
	local ca_cert_bkup
	local local_cert_bkup

	old_status=$(uci get /tmp/etc/config/openvpn.global.enable)
	new_status=$(uci get openvpn.global.enable)

	if [ "$new_status" = "1" ]; then
	#{
		if [ $new_status != $old_status ]
		then
			#clear the default openvpn uci config
			echo > /tmp/etc/config/openvpn
		fi

		if [ -f "/tmp/etc/config/openvpn" ]; then
			cp /tmp/etc/config/openvpn /tmp/etc/config/openvpn_bkup	
		fi

		# set the Values, not provided by the GUI
		uci set openvpn.global.dh=/etc/openvpn/dh2048.pem
		uci set openvpn.global.dev=tun
		uci set openvpn.global.topology=subnet
		uci set openvpn.global.status=/tmp/openvpn.global.status
		ca_cert=$(uci get openvpn.global.ca)
		local_cert=$(uci get openvpn.global.cert)
		if [ -n "$ca_cert" ]
		then 
		#As we will get certificate with full path and extension, removing path and extension
			ca_cert=${ca_cert##*/}
			ca_cert=${ca_cert%.pem}
			updateCertUsage add $ca_cert "OpenVPN"
		fi

		if [ -n "$local_cert" ]
		then
			local_cert=${local_cert##*/}
			local_cert=${local_cert%.pem}
			updateCertUsage add $local_cert "OpenVPN"	
		fi
		

		uci set openvpn.global.plugin='/usr/lib/openvpn/plugins/openvpn-plugin-auth-pam.so openvpn'
		uci delete openvpn.global.auth_user_pass_verify
		uci set openvpn.global.management='127.0.0.1 4567'
#		uci set openvpn.global.inactive=60
		uci set openvpn.global.keepalive='10 30'
		uci set openvpn.global.verb=3

		uci set openvpn.global.tun_mtu=45000
		uci set openvpn.global.fragment=0
		uci set openvpn.global.mssfix=0
		uci set openvpn.global.txqueuelen=1000
		uci set openvpn.global.nice=-20

		uci set openvpn.global.mute_replay_warnings=1

		proto=$(uci get openvpn.global.proto)
		if [ "$proto" = "udp" ]
		then
			uci add_list openvpn.global.push='explicit-exit-notify 3'
		fi

		client_isolation=$(uci get openvpn.global.client_isolation)
		if [ "$client_isolation" = "0" ]; then
			uci set openvpn.global.client_to_client=1
		fi

		ca_cert_bkup=$(uci get openvpn_bkup.global.ca)

		local_cert_bkup=$(uci get openvpn_bkup.global.cert)

		if [ "$local_cert" != "$local_cert_bkup" ]; then
			local_cert_bkup=${local_cert_bkup##*/}
			local_cert_bkup=${local_cert_bkup%.pem}
			updateCertUsage del $local_cert_bkup "OpenVPN"
		fi

		if [ "$ca_cert" != "$ca_cert_bkup" ]; then
			ca_cert_bkup=${ca_cert_bkup##*/}
			ca_cert_bkup=${ca_cert_bkup%.pem}
			updateCertUsage del $ca_cert_bkup "OpenVPN"
		fi

		number_of_conns=$(uci get vpnresourcemgmt.global.ssl_vpn)
		uci set openvpn.global.max_clients=$number_of_conns

		temp=$(uci get openvpn.global.interface)
		if [ "$temp" = "WAN" ]; then
			network_get_wanip_cached local_addr "wan1"
			uci set openvpn.global.local=$local_addr
		elif [ "$temp" = "USB" ]; then
			network_get_wanip_cached local_addr "usb1"
			uci set openvpn.global.local=$local_addr
		fi

		if [ "$temp" = "WAN" -o "$temp" = "USB" ]; then
			if [ "$local_addr" = "" ]
			then
				uci commit openvpn
				/etc/init.d/openvpn stop
				return 0;
			fi
		fi

		uci commit openvpn

		if [ $new_status != $old_status ]
		then
			/etc/init.d/openvpn start
		else
			/etc/init.d/openvpn reload
		fi
	#}
	else
	#{
		uci commit openvpn
		/etc/init.d/openvpn stop

		ca_cert=$(uci get openvpn.global.ca)
		local_cert=$(uci get openvpn.global.cert)
		if [ -n "$ca_cert" ]
		then 
			ca_cert=${ca_cert##*/}
			ca_cert=${ca_cert%.pem}
			updateCertUsage del $ca_cert "OpenVPN"
		fi

		if [ -n "$local_cert" ]
		then
			local_cert=${local_cert##*/}
			local_cert=${local_cert%.pem}
			updateCertUsage del $local_cert "OpenVPN"	
		fi
	#}
	fi

	retval=$?
	if [ $retval -gt 0 ]                                                                               
    then                                                                                               
        return $retval                                                                             
    else
		firewall_dmz		
		retval=$?
	fi
	return $retval
}

wireless() {
	local retval

	uci set wireless.wl0.type="broadcom"
	uci set wireless.wl1.type="broadcom"

	uci commit wireless

	if [ "$board" = "RV340W" ] || [ "$board" = "RV160W" ] || [ "$board" = "RV260W" ] ;then
		if [ "$(nvram get wps_reload_lock)" != "1" ]; then
			if [ -e /tmp/etc/config/wireless_old ];then
				/sbin/wifi reload_part >/dev/null 2>&1
			else
				/sbin/wifi down >/dev/null 2>&1
				/sbin/wifi up >/dev/null 2>&1
			fi
		else
			nvram set wps_reload_lock=0 >/dev/null 2>&1
		fi
 	fi

	retval=$?

	[ -e /mnt/initial_setup_done ] && {
		count=$(cat /mnt/configcert/b_count)
		if [ $count == 1 ]; then
			ssid=$(uci get wireless.@wifi-iface[0].ssid)
			if [ "$ssid" != "CiscoSB-Setup" ]; then
				sh /usr/bin/initial_ssid_flush_rules.sh
				echo 2 > /mnt/configcert/b_count
			fi
		fi
	}

	return $retval
}

cpprofile() {
	local retval

	uci commit cpprofile

	if [ "$board" = "RV340W" ] || [ "$board" = "RV160W" ] || [ "$board" = "RV260W" ] ;then
		if [ -e /var/run/captive.pid ];then
		/etc/init.d/captive_portal reload >/dev/null 2>&1 &
		fi
 	fi

	retval=$?

	return $retval
}

lobby() {
	local retval

	uci commit lobby

	killall -18 cportald

	retval=$?

	return $retval
}

license() {
	local retval=1
	/etc/init.d/license reload

	retval=$?
	return $retval
}

pnp() {
        local retval
        local pnp_current_config
        local pnp_new_config
        pnp_current_config=`uci get /tmp/etc/config/pnp.general.enabled`

        uci commit pnp

        pnp_new_config=`uci get pnp.general.enabled`

        if [ "$pnp_new_config" = "0" ];then
                if [ "$pnp_current_config" = "1" ];then
                        /etc/init.d/pnpd stop
                fi
        fi

        if [ "$pnp_new_config" = "1" ];then
                create_pnp_config
                if [ "$pnp_current_config" = "1" ];then
                        /etc/init.d/pnpd restart
                else
                        /etc/init.d/pnpd start
                fi
        fi
        retval=$?
        return $retval
}



update_pc2run() {

	local modules
	local main_mod
	modules=$@
	if [ -n "$(echo $modules | grep "network")" ];then
		modules=$(echo $modules | sed s/"dhcp"/""/ | sed s/"bonjour"/""/ | sed s/"bwmgmt"//)
	fi

	if [ -n "$(echo $modules | grep "systemconfig")" ];then
		modules=$(echo $modules | sed s/"dhcp"/""/ | sed s/"bonjour"/""/)
	fi

	if [ -n "$(echo $modules | grep "firewall")" ];then
		modules=$(echo $modules | sed s/"bonjour"/""/)
	fi

	if [ -n "$(echo $modules | grep "switch")" ];then
		modules=$(echo $modules | sed s/"qos_switch"/""/)
	fi

	if [ -n "$(echo $modules | grep ipgroup)" ];then
		modules=$(echo $modules | sed s/ipgroup//)
		modules="ipgroup $modules"
	fi
	if [ -n "$(echo $modules | grep schedule)" ];then
		modules=$(echo $modules | sed s/schedule//)
		modules="schedule $modules"
	fi
	if [ -n "$(echo $modules | grep network)" ];then
		modules=$(echo $modules | sed s/network//)
		modules="network $modules"
	fi

	logger -t ucicfg "Final modules are $modules"

	for main_mod in $modules
	do
		if [ "$main_mod" = "schedule" ];then
			logger -t ucicfg "Reloading $main_mod ..."
			schedule_run $modules
		elif [ "$main_mod" = "ipgroup" ];then
			logger -t ucicfg "Reloading $main_mod ..."
			ipgroup_run $modules
		elif [ "$main_mod" = "webfilter" ];then
			logger -t ucicfg "Reloading $main_mod ..."
			webfilter_run
		elif [ "$main_mod" = "qos" ];then
			logger -t ucicfg "Reloading $main_mod ..."
			uci commit qos
			/etc/init.d/qos restart
		else
			logger -t ucicfg "Reloading $main_mod ..."
			$main_mod
		fi
	done
}
	local cfg="$1"
	local retval
	
if [ $# -eq 1 ]
then
	if [ $cfg = "dhcp" ]
	then
			dhcp
			exit $?
	elif [ $cfg = "network" ]
	then
			network
			exit $?
	elif [ $cfg = "network_onboot" ]
	then
			network_onboot
			exit $?
	elif [ $cfg = "ddns" ]
	then
			ddns
			exit $?
     	elif [ $cfg = "rip" ]
	then
			rip
			exit $?
	elif [ $cfg = "qos" ]
	then
			qos
			exit $?
	elif [ $cfg = "dmz" ]
	then
			dmz
			exit $?
	elif [ $cfg = "qos_switch" ]
	then
			switch
			exit $?
	elif [ $cfg = "switch" ]
	then
			switch
			exit $?
	elif [ $cfg = "wanport" ]
	then
			wanport
			exit $?
	elif [ $cfg = firewall ]
	then
			firewall
			exit $?
	elif [ $cfg = mwan3 ]
	then
			mwan3
			exit $?
	elif [ $cfg = igmpproxy ]
	then
			igmpproxy
			exit $?
	elif [ $cfg = radvd ]
	then
			radvd
			exit $?
	elif [ $cfg = upnpd ]
	then
			upnpd
			exit $?
	elif [ $cfg = bonjour ]
	then
			bonjour
			exit $?
	elif [ $cfg = certificate ]
	then
			certificate
			exit $?
	elif [ $cfg = testserver ]
	then
			testserver
			exit $?
	elif [ $cfg = email ]
	then
			email
			exit $?
	elif [ $cfg = webfilter ]
	then
		webfilter
		exit $?
	elif [ $cfg = avc ]
	then
		avc
		exit $?
	elif [ $cfg = systemconfig ]
	then
		systemconfig
		exit $?
	elif [ $cfg = swupdate ]
	then
		swupdate
		exit $?
	elif [ $cfg = snmpconf ]
	then
		snmpconf
		exit $?
	elif [ $cfg = poeconf ]
	then
		poeconf
		exit $?
	elif [ $cfg = schedule ]
	then
		schedule
		exit $?
	elif [ $cfg = ipgroup ]
	then
		ipgroup
		exit $?
	elif [ $cfg = strongswan ]
	then
		strongswan
		exit $?
	elif [ $cfg = pptpd ]
	then
		pptpd
		exit $?
	elif [ $cfg = l2tpd ]
	then
		l2tpd
		exit $?
	elif [ $cfg = vpnpassthrough ]
	then
		vpnpassthrough
		exit $?
	elif [ $cfg = vpnresourcemgmt ]
	then
		vpnresourcemgmt
		exit $?
	elif [ $cfg = syslog ]
	then
		syslog
		exit $?
	elif [ $cfg = system ]
	then
		system
		exit $?
	elif [ $cfg = lldpd ]
	then
		lldpd
		exit $?
	elif [ $cfg = sslvpn ]
	then
		sslvpn
		exit $?
	elif [ $cfg = openvpn ]
	then
		openvpn
		exit $?
	elif [ $cfg = radius ]
	then
		radius
		exit $?
	elif [ $cfg = ldap ]      
	then                         
	      ldap           
	      exit $?   
	elif [ $cfg = bwmgmt ]
	then
		bwmgmt
		exit $?
	elif [ $cfg = ad ]        
	then                                          
	      ad                    
	      exit $?     
	elif [ $cfg = wireless ]
	then
		wireless
		exit $?
	elif [ $cfg = update_pc2run ]
	then
		update_pc2run $@
		exit $?
	elif [ $cfg = cpprofile ]
	then
		cpprofile
		exit $?
	elif [ $cfg = lobby ]
	then
		lobby
		exit $?
	elif [ $cfg = pnp ]
	then
		pnp
		exit $?
	elif [ $cfg = aaa ]
	then
		aaa
		exit $?
	elif [ $cfg = license ]
	then
		license
		exit $?
	fi

else
	logger -t ucicfg "Reloading config_update.sh with args:$@"
	update_pc2run $@
	exit 0
fi
