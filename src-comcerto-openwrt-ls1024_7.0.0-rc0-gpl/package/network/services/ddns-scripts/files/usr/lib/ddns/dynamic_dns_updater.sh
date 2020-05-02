#!/bin/sh 

. /lib/config/uci.sh

# /usr/lib/dynamic_dns/dynamic_dns_updater.sh
#
# Written by Eric Paul Bishop, Janary 2008
# Distributed under the terms of the GNU General Public License (GPL) version 2.0
#
# This script is (loosely) based on the one posted by exobyte in the forums here:
# http://forum.openwrt.org/viewtopic.php?id=14040
#

. /usr/lib/ddns/dynamic_dns_functions.sh
. /lib/functions/network.sh

service_id=$1
if [ -z "$service_id" ]
then
	echo "ERRROR: You must specify a service id (the section name in the /etc/config/ddns file) to initialize dynamic DNS."
	return 1
fi

#default mode is verbose_mode, but easily turned off with second parameter
verbose_mode="1"
if [ -n "$2" ]
then
	verbose_mode="$2"
fi

###############################################################
# Leave this comment here, to clearly document variable names
# that are expected/possible
#
# Now use load_all_config_options to load config
# options, which is a much more flexible solution.
#
#
#config_load "ddns"
#
#config_get enabled $service_id enabled
#config_get service_name $service_id service_name
#config_get update_url $service_id update_url
#
#
#config_get username $service_id username
#config_get password $service_id password
#config_get domain $service_id domain
#
#
#config_get use_https $service_id use_https
#config_get cacert $service_id cacert
#
#config_get ip_source $service_id ip_source
#config_get ip_interface $service_id ip_interface
#config_get ip_network $service_id ip_network
#config_get ip_url $service_id ip_url
#
#config_get force_interval $service_id force_interval
#config_get force_unit $service_id force_unit
#
#config_get check_interval $service_id check_interval
#config_get check_unit $service_id check_unit
#########################################################
load_all_config_options "ddns" "$service_id"

#logger -t ddns -p local0.debug "domain=$domain serviceid=$service_id username=$username passwd=$password servicename=$service_name"


#some defaults
interface=`uci get ddns.$service_id.interface`
tmpIface=`echo $interface | sed s/wan[1-9]*/\&p/g`
getLogicalName=`uci show network | grep -w "$tmpIface"` >/dev/null 2>&1
[ -z "$getLogicalName" ] && rm -rf /var/log/ddns.$tmpIface.logs
[ -n "$getLogicalName" ] && {
	rm -rf /var/log/ddns.$interface.logs
	interface=$tmpIface
}

doForceUpdate=1
doRetry=1

#if this service isn't enabled then quit
if [ "$enabled" != "1" ] 
then
	rm -rf /var/log/ddns.$interface.logs
	logger -t ddns -p "local0.info" "$interface:DDNS service is disabled."
	/etc/hotplug.d/iface/95-wanscript
	/usr/bin/wangetcurrent	#We need to call this script, only then the updated stats can be fetched by GUI
	return 0
fi

#kill old process if it exists & set new pid file
if [ -d /var/run/dynamic_dns ]
then
	#if process is already running, stop it
	if [ -e "/var/run/dynamic_dns/$service_id.pid" ]
	then
		old_pid=$(cat /var/run/dynamic_dns/$service_id.pid)
		test_match=$(ps | grep "^[\t ]*$old_pid")
		if [ -n  "$test_match" ]
		then
			kill -9 $old_pid
		fi
	fi

else
	#make dir since it doesn't exist
	mkdir /var/run/dynamic_dns
fi
echo $$ > /var/run/dynamic_dns/$service_id.pid

if [ -z "$check_interval" ]
then
	check_interval=60
fi

if [ -z "$retry_interval" ]
then
	retry_interval=60
fi

if [ -z "$check_unit" ]
then
	check_unit="seconds"
fi


if [ -z "$force_interval" ]
then
	force_interval=72
fi

if [ -z "$force_unit" ]
then
	force_unit="hours"
fi

if [ -z "$use_https" ]
then
	use_https=0
fi


certfile=$(echo "$service_name" | sed 's/com/pem/g')

#some constants

if [ "x$use_https" = "x1" ]
then
	retrieve_prog="/usr/bin/curl "
	if [ -f "$cacert" ]
	then
		retrieve_prog="${retrieve_prog}--cacert $cacert "
	elif [ -d "$cacert" ]
	then
		retrieve_prog="${retrieve_prog}--capath $cacert "
	fi
else
	#retrieve_prog="/usr/bin/wget -O - ";
	retrieve_prog="/usr/bin/curl "
	retrieve_prog="${retrieve_prog}-k --cacert /etc/ssl/certs/$certfile "
fi

service_file="/usr/lib/ddns/services"

ip_regex="[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}\.[0-9]\{1,3\}"

NEWLINE_IFS='
'

#determine what update url we're using if the service_name is supplied
if [ -n "$service_name" ]
then
	#remove any lines not containing data, and then make sure fields are enclosed in double quotes
	quoted_services=$(cat $service_file |  grep "^[\t ]*[^#]" |  awk ' gsub("\x27", "\"") { if ($1~/^[^\"]*$/) $1="\""$1"\"" }; { if ( $NF~/^[^\"]*$/) $NF="\""$NF"\""  }; { print $0 }' )


	#echo "quoted_services = $quoted_services"
	OLD_IFS=$IFS
	IFS=$NEWLINE_IFS
	for service_line in $quoted_services
	do
		#grep out proper parts of data and use echo to remove quotes
		next_name=$(echo $service_line | grep -o "^[\t ]*\"[^\"]*\"" | xargs -r -n1 echo)
		next_url=$(echo $service_line | grep -o "\"[^\"]*\"[\t ]*$" | xargs -r -n1 echo)

		if [ "$next_name" = "$service_name" ]
		then
			update_url=$next_url
		fi
	done
	IFS=$OLD_IFS
fi

update_url=$(echo $update_url | sed -e 's/^http:/https:/')



verbose_echo "update_url=$update_url"


#compute update interval in seconds
if [ "$force_interval" -eq 0 ];then
	force_interval_seconds=31536000
else
	case "$force_unit" in
		"days" )
			force_interval_seconds=$(($force_interval*60*60*24))
			;;
		"hours" )
			force_interval_seconds=$(($force_interval*60*60))
			;;
		"minutes" )
			force_interval_seconds=$(($force_interval*60))
			;;
		"seconds" )
			force_interval_seconds=$force_interval
			;;
		* )
			#default is hours
			#force_interval_seconds=$(($force_interval*60*60))
			force_interval_seconds=31536000
			;;
	esac
fi



#compute check interval in seconds
case "$check_unit" in
	"days" )
		check_interval_seconds=$(($check_interval*60*60*24))
		;;
	"hours" )
		check_interval_seconds=$(($check_interval*60*60))
		;;
	"minutes" )
		check_interval_seconds=$(($check_interval*60))
		;;
	"seconds" )
		check_interval_seconds=$check_interval
		;;
	* )
		#default is seconds
		check_interval_seconds=$check_interval
		;;
esac



#compute retry interval in seconds
case "$retry_unit" in
	"days" )
		retry_interval_seconds=$(($retry_interval*60*60*24))
		;;
	"hours" )
		retry_interval_seconds=$(($retry_interval*60*60))
		;;
	"minutes" )
		retry_interval_seconds=$(($retry_interval*60))
		;;
	"seconds" )
		retry_interval_seconds=$retry_interval
		;;
	* )
		#default is seconds
		retry_interval_seconds=$retry_interval
		;;
esac



verbose_echo "force seconds = $force_interval_seconds"
verbose_echo "check seconds = $check_interval_seconds"




#determine when the last update was
current_time=$(monotonic_time)
last_update=$(( $current_time - (2*$force_interval_seconds) ))
if [ -e "/var/run/dynamic_dns/$service_id.update" ]
then
	last_update=$(cat /var/run/dynamic_dns/$service_id.update)
fi
time_since_update=$(($current_time - $last_update))


human_time_since_update=$(( $time_since_update / ( 60 ) ))
verbose_echo "time_since_update = $human_time_since_update minutes"
logger -t ddns -p local0.debug "$interface:time_since_update = $human_time_since_update minutes"


echo "" > /var/log/ddns.$interface.logs
verbose_echo_iface "$interface:Update_Registering"
/etc/hotplug.d/iface/95-wanscript
/usr/bin/wangetcurrent	#We need to call this script, only then the updated stats can be fetched by GUI

#do update and then loop endlessly, checking ip every check_interval and forcing an updating once every force_interval
human_time=""
while [ true ]
do
	current_ip=$(get_current_ip)
	if [ "$(cat /var/log/ddns.$interface.logs | grep -E "Successful|nochg")" = "" ]
	then
            if [ "$service_name" != "dnsomatic.com" ]
            then 
		registered_ip=$(echo $(nslookup "$domain" 2>/dev/null) |  grep -o "Name:.*" | grep -o "$ip_regex")
            else
		registered_ip=""
            fi
	else
		registered_ip=$current_ip
		doRetry=1
	fi

	current_time=$(monotonic_time)
	time_since_update=$(($current_time - $last_update))

        if [ "$service_name" != "dnsomatic.com" ]
        then 
	   [ -z "$registered_ip" ] && {
		logger -t ddns -p local0.error "$interface:unable to resolve $domain."
		echo "$interface:Update_Failed" > /var/log/ddns.$interface.logs
		/etc/hotplug.d/iface/95-wanscript
		/usr/bin/wangetcurrent	#We need to call this script, only then the updated stats can be fetched by GUI
		sleep $retry_interval_seconds
		continue
	   }
        fi

	if [ "$current_ip" != "$registered_ip" ]  || [ $force_interval_seconds -lt $time_since_update ] || [ $doForceUpdate -eq 1 ]
	then
		echo "" > /var/log/ddns.$interface.logs
		doForceUpdate=0
		local finaldev
		if [ $doRetry -gt 5 ];then
			verbose_echo_iface "$interface:Update_Failed"
			logger -t ddns -p local0.info "$interface:Exiting after 5 fails"
			exit 0
		fi
		verbose_echo_iface "$interface:update necessary"
		logger -t ddns -p local0.info "$interface:update necessary, performing update ..."
		final_url=$update_url
		for option_var in $ALL_OPTION_VARIABLES
		do
			if [ "$option_var" != "update_url" ]
			then
				replace_name=$(echo "\[$option_var\]" | tr 'a-z' 'A-Z')
				replace_value=$(eval echo "\$$option_var")
				replace_value=$(echo $replace_value | sed -f /usr/lib/ddns/url_escape.sed)
				final_url=$(echo $final_url | sed s^"$replace_name"^"$replace_value"^g )
			fi
		done

		final_url=$(echo $final_url | sed s^"\[HTTPAUTH\]"^"${username//^/\\^}${password:+:${password//^/\\^}}"^g )
		final_url=$(echo $final_url | sed s/"\[IP\]"/"$current_ip"/g )

		network_get_device finaldev $interface

		updated_url=$(echo $update_url | sed s/"\[IP\]"/"$current_ip"/g )
		logger -t ddns -p local0.info "$interface:updating with url=\"$updated_url\""
		#logger -t ddns -p local0.info "$retrieve_prog--interface $finaldev $final_url"

		#here we actually connect, and perform the update
		update_output=$( $retrieve_prog--interface "$finaldev" "$final_url" )
		result="$?"

		doRetry=$(( $doRetry + (1) ))
		#logger -t ddns -p local0.info "$interface Retry value is $doRetry "
		#if [ "$update_output" = "" ];then
		#	logger -t ddns -p local0.info "multiple ddns updates are sent to server from $interface"
		#fi

		update_output=`echo $update_output | sed "s/[\r\n]*//g"`
		tmp=`echo $update_output | grep -i "fail\|not\|nohost\|badauth"`
		[ -n "$tmp" ] && {
			verbose_echo_iface "$interface:Update_Failed"
			logger -t ddns -p local0.error "$interface:update failed update_output=$update_output"
			/etc/hotplug.d/iface/95-wanscript
			/usr/bin/wangetcurrent	#We need to call this script, only then the updated stats can be fetched by GUI
			sleep 180
			if [ "$(echo $tmp | grep -i "badauth\|auth")" != "" ];then
				#logger -t ddns -p local0.info "Exiting because of bad authentication for $interface"
				exit 0
			fi
			if [ "$(echo $tmp | grep -i "nohost\|host")" != "" ];then
				#logger -t ddns -p local0.info "Exiting because of incorrect fqdn for $interface"
				exit 0
			fi
			continue
		}

		[ "$update_output" = "" ] && {
			verbose_echo_iface "$interface:Update_Registering"
			logger -t ddns -p local0.error "$interface:update failed update_output=$update_output"
			/etc/hotplug.d/iface/95-wanscript
			/usr/bin/wangetcurrent	#We need to call this script, only then the updated stats can be fetched by GUI
			sleep 180
			continue
		}



		#save the time of the update
		current_time=$(monotonic_time)
		last_update=$current_time
		time_since_update='0'
		registered_ip=$current_ip

		human_time=$(date)
		echo "Successful" > /var/log/ddns.$interface.logs
		verbose_echo_iface "$interface:Update Output: Update_Success update_output=\"$update_output\""
		verbose_echo_iface "$interface:update complete, time is:$human_time"
		logger -t ddns -p local0.info "$interface:Update Successful"
		logger -t ddns -p local0.info "$interface:update complete, time is:$human_time"
		/etc/hotplug.d/iface/95-wanscript
		/usr/bin/wangetcurrent	#We need to call this script, only then the updated stats can be fetched by GUI
		echo "$last_update" > "/var/run/dynamic_dns/$service_id.update"
	else
		[ -z "$human_time" ] && {
			human_time=$(date)
		}
		human_time_since_update=$(( $time_since_update / ( 60 ) ))
	fi

	sleep $check_interval_seconds

done

#should never get here since we're a daemon, but I'll throw it in anyway
return 0




