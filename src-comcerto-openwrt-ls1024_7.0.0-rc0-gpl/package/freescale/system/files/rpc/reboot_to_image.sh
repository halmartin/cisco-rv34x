#!/bin/sh

errcode=1
errstring="Invalid Arguments"

# MTD_ENV needs to be changed, if flash partition changes in future.
MTD_ENV=2
TRIGGER_REBOOT=0


reboot_to_image() {
	img_selected="$1"
	active_firmware=`uci get firmware.firminfo.version`
	inactive_firmware=`uci get firmware.firminfo.inactive_version`
	logger -t system -p local0.alert "Rebooting the system .."
	if [ "$img_selected" = "inactive" ];then
	        curr_img=`uci get firmware.firminfo.active`
	        if [ "$curr_img" = "image1" ];then
	                `uci set firmware.firminfo.active=image2`
	                echo 2 > /tmp/active
	                flash_erase -q /dev/mtd$MTD_ENV 0 0
	                nandwrite -p -q /dev/mtd$MTD_ENV /tmp/active
	        else
	                `uci set firmware.firminfo.active=image1`
	                echo 1 > /tmp/active
	                flash_erase -q /dev/mtd$MTD_ENV 0 0
	                nandwrite -p -q /dev/mtd$MTD_ENV /tmp/active
	        fi
	        uci commit firmware
		logger -t system -p local0.alert "System will boot with inactive image (version $inactive_firmware) after reboot."
	else
		logger -t system -p local0.alert "System will boot with active image (version $active_firmware) after reboot."
	fi
# if reboot is required, need to uncomment the below statement.
#	TRIGGER_REBOOT=1
	errstring="Success"
	errcode=0
}

firmware_state="$1"

reboot_to_image $firmware_state

[ "$TRIGGER_REBOOT" = 1 ] && {
        reboot &
}

if [ "$errcode" -ne 0 ];then
	echo "error-message \"$errstring\""
	exit 1
else
	exit 0
fi
