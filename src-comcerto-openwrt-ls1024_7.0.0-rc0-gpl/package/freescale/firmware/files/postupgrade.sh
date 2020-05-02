#!/bin/sh

# This script is required to do some task only 1 time after firmware upgrade.

POSTUPGRADE_FLAG="/usr/bin/postupgrade/flag"
postupgradeFlag=`cat $POSTUPGRADE_FLAG`
board=`uci get systeminfo.sysinfo.pid | cut -d'-' -f1`

if [ "$postupgradeFlag" -eq 1 ];then
        #  Do all the things here, which need to be done after upgrade.
        #  If any file (binary or data) is required, put them in postupgrade folder. They can be accessed from "/usr/bin/postupgrade" folder.

	if [ "$board" = "RV345P" ] && [ -e "/usr/bin/poe_postupgrade.sh" ];then
	    /bin/sh /usr/bin/poe_postupgrade.sh
	fi

	if [ "$board" = "RV340" -o "$board" = "RV340W" -o "$board" = "RV345" -o "$board" = "RV345P" ] && [ -e "/usr/bin/certsInfraMigration.sh" ]; then
		#this code is been placed here to address MR0-->MR.5/1 update of certificate infra.
		logger -t firmware -p info "Certificates infra migration Start!"
		/bin/sh /usr/bin/certsInfraMigration.sh
		logger -t firmware -p info "Certificates infra migration done!"
	fi

        # Set the flag to 0
        echo "0" > $POSTUPGRADE_FLAG
fi
