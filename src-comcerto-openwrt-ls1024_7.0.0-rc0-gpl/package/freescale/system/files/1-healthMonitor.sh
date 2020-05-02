#!/bin/sh /etc/rc.common

ru_check=`uci get systeminfo.sysinfo.region 2> /dev/null`

# Check for below daemons and run if got killed.
if [ "$ru_check" = "RU" ]; then
	daemons="cmm operdb_stats netifd"
else
daemons="vpnTimer vpnLed starter charon cmm operdb_stats netifd"
fi

OPSDB_STATUS="/tmp/opsdb_status"

backup_and_restart() {
	case $1 in
		operdb_stats)
			daemonName="operdb_stats"

			# create directory /tmp/$daemonName-day-month-year:Hour:Min:Sec
			currentDate=`date +%d-%m-%y-%H:%M:%S`
			OutDirname="/tmp/${daemonName}-${currentDate}"

			# create directory
			mkdir "${OutDirname}"

			# copy core files 
			cp -rf /tmp/${daemonName}*.core ${OutDirname}/

			# take required data
			cp -rf /tmp/stats ${OutDirname}/
			
			# restart the daemon
			operdb_stats &

			# load the data in to certs dll
			sleep 3s
			/usr/bin/certscript boot &

		;;
		netifd)
			/etc/init.d/netifd start &
		;;
	esac
}

vpnGlobalStatus=`uci get strongswan.ipsecGlobal_0.status`

for word in $daemons;do
	local status=0
	died=`pidof $word`
	if [ "$word" = "operdb_stats" ]; then
		status=`cat $OPSDB_STATUS`
		txid=$(confd_cmd -c txid)
		if [ "$status" = "1" ] && [ -n "$txid" ]; then
			[ -z "$died" ] && {
				uptime=`uptime`
				logger -t healthMonitor -p local0.crit "$word got killed .. uptime=$uptime"
				# Restart the daemon and take required backup.
				backup_and_restart $word
			}
		fi
	else
		[ -z "$died" ] && {
			uptime=`uptime`
			[ "$vpnGlobalStatus" = "0" ] && {
				#dont report about "vpnTimer vpnLed starter charon" when ipsec is disabled globally.
				case $word in
					vpn* | starter | charon)
						continue
						;;
				esac
			}
			logger -t healthMonitor -p local0.crit "$word got killed .. uptime=$uptime"
		
			# Restart the daemon and take required backup.
			backup_and_restart $word
		}
	fi
done
