#!bin/sh

. /usr/bin/syserr_codes

ASDCLIENT_STATUSFILE="/tmp/asd_download_status"
ASDPROCESS_STATUSFILE="/tmp/asd_process_status"
ASDFILE="/tmp/asd_email_content"
ASDSTATUS="/mnt/configcert/config/asdstatus"
SWDOWNLOAD_STATUS_TMP_FILE="/mnt/configcert/config/downloadstatus"
ASD_VERSION_CHECK_FILE="/etc/verchk"
SWTYP=$1
AVAILABLE_VER=$2
CMD=$3
DOWNLOAD_PATH=$4
TMP_ASD_FRM_UPGRD_FILE=/tmp/firmware_upgrade_state
TMP_ASD_USB_DRV_UPGRD_FILE=/tmp/dirver_upgrade_state

ru_check=`uci get systeminfo.sysinfo.region 2> /dev/null`

# possbile values: only_dwld, dwld_apply, cron
arg5=$5 
EXITSTATUS=1
MTD_ENV=2

if [ "$arg5" = "only_dwld" ];
then
    echo 0 > $ASD_VERSION_CHECK_FILE
fi


if [ -e "$ASDPROCESS_STATUSFILE" ];then
	progress_file=`cat $ASDPROCESS_STATUSFILE`
	logger -t asdclient -p local0.info "$progress_file download still in progress. Hence exiting new $1 download "
    if [ "$SWTYP" = "firmware" ];then
        EXITSTATUS=$ER_FRM_UPGRD_IN_PROGRESS
    elif [ "$SWTYP" = "drivers" ];then
        EXITSTATUS=$ER_USB_DRV_UPGRD_IN_PROGRESS
    fi
	exit $EXITSTATUS
else
	echo "$1" >> $ASDPROCESS_STATUSFILE
fi

reboot_to_image() {
        img_selected="$1"
        logger -t system -p local0.alert "Rebooting the system .."
        active_firmware=`uci get firmware.firminfo.version`
        inactive_firmware=`uci get firmware.firminfo.inactive_version`
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
        TRIGGER_REBOOT=1
        errstring="Success"
        errcode=0
        # Firmware upgrade
        echo 3 > /mnt/configcert/config/rebootstate
        reboot &
}

sendEmailAndLogAlert() {

    local status=$1

    local SWTYP=$2

    local APPLIED_VER=""
    local APPLIED_TIME=""
    local SUBJECT_STR=""
    local EMAIL_BODY_TXT=""
    local BNOTIFY="false"


    if [ "$status" = 0 ];then
        logger -t asdclient -p local0.info "$SWTYP updated successfully"

        if [ "$SWTYP" = "firmware" ];then
            BNOTIFY=`uci get swupdate.firmware.notify`
            SUBJECT_STR="A new firmware updated"
            APPLIED_VER=`grep frm_latest_version "/etc/swupdateinfo" | awk -F'=' '{ print $2 }' |  sed 's/"//g'`
            APPLIED_TIME=`grep frm_latest_update_time "/etc/swupdateinfo" | awk -F'=' '{ print $2 }' |  sed 's/"//g'`
            EMAIL_BODY_TXT="A new firmware version update, version $APPLIED_VER, was applied at $APPLIED_TIME"
        elif [ "$SWTYP" = "signatures" ];then
            BNOTIFY=`uci get swupdate.signature.notify`
            SUBJECT_STR="A new signature updated"
            APPLIED_VER=`grep sig_latest_version "/etc/swupdateinfo" | awk -F'=' '{ print $2 }' |  sed 's/"//g'`
            APPLIED_TIME=`grep sig_latest_update_time "/etc/swupdateinfo" | awk -F'=' '{ print $2 }' |  sed 's/"//g'`
            EMAIL_BODY_TXT="A new signature version update, version $APPLIED_VER, was applied at $APPLIED_TIME"
        elif [ "$SWTYP" = "drivers" ];then
            BNOTIFY=`uci get swupdate.dongle.notify`
            SUBJECT_STR="A new USB dongle driver updated"
            APPLIED_VER=`cat /etc/usb-modem.ver`
            ACTIMG=`uci get firmware.firminfo.active`
            PSFIX="_usb_latest_update_time"
            IMGTYPE=$ACTIMG$PSFIX
            APPLIED_TIME=`grep "$IMGTYPE=" "/etc/swupdateinfo" | awk -F'=' '{ print $2 }' | sed 's/"//g'`
            EMAIL_BODY_TXT="A new USB dongle driver version update, version $APPLIED_VER, was applied at $APPLIED_TIME "
        fi

    else
        logger -t asdclient -p local0.error "Failed to update $SWTYP"

        if [ "$SWTYP" = "firmware" ];then
            BNOTIFY=`uci get swupdate.firmware.notify`
            SUBJECT_STR="Update firmware fail"
            EMAIL_BODY_TXT="Firmware update failed."
        elif [ "$SWTYP" = "signatures" ];then
            BNOTIFY=`uci get swupdate.signature.notify`
            SUBJECT_STR="Update signature fail"
            EMAIL_BODY_TXT="Signature update failed."
        elif [ "$SWTYP" = "drivers" ];then
            BNOTIFY=`uci get swupdate.dongle.notify`
            SUBJECT_STR="Update USB dongle driver fail"
            EMAIL_BODY_TXT="USB dongle driver update failed."
        fi
    fi
    if [ "$BNOTIFY" = "true" ];then
        echo $EMAIL_BODY_TXT > $ASDFILE
        sh /usr/bin/sendAsdmail "$SUBJECT_STR"
    fi
}

FRM_APPLY_SCRIPT="sh /usr/bin/rv340_fw_unpack.sh"
#FRM_APPLY_SCRIPT="sh /usr/bin/rv16x_26x_fw_unpack.sh"
SIG_APPLY_SCRIPT="sh /usr/bin/lcsig.sh"
USB_APPLY_SCRIPT="sh /usr/bin/install_usb_drivers"

rm -rf $ASDCLIENT_STATUSFILE

if [ -e "$SWDOWNLOAD_STATUS_TMP_FILE" ];then
    rm $SWDOWNLOAD_STATUS_TMP_FILE
fi
echo "Status:1" >> $SWDOWNLOAD_STATUS_TMP_FILE
echo "Percentage:0" >> $SWDOWNLOAD_STATUS_TMP_FILE

PID=`uci get systeminfo.sysinfo.pid | cut -d'-' -f1`
VID=`uci get systeminfo.sysinfo.vid`
SNO=`uci get systeminfo.sysinfo.serial_number`

if [ -z "$DOWNLOAD_PATH" ];then
	DOWNLOAD_PATH="/tmp"
fi

if [ "$SWTYP" = "firmware" ];then
	AVAILABLE_VER=`cat /etc/firmware_version`
	if [ "$ru_check" = "RU" ]; then
		#check for RU image and convert frimware version from 1.0.01.16_RUn form to 1.0.01.16
		AVAILABLE_VER=`echo $AVAILABLE_VER | cut -d'_' -f1`
	fi
elif [ "$SWTYP" = "drivers" ];then
	AVAILABLE_VER=`cat /etc/usb-modem.ver`
elif [ "$SWTYP" = "signatures" ];then
	AVAILABLE_VER=`grep sig_latest_version "/etc/swupdateinfo" | awk -F'=' '{ print $2 }' |  sed 's/"//g'`
fi

#echo "/usr/bin/asdclient $PID $VID $SNO $AVAILABLE_VER $SWTYP $DOWNLOAD_PATH $CMD"

echo "$SWTYP update started" > $ASDSTATUS

/usr/bin/asdclient $PID $VID $SNO $AVAILABLE_VER $SWTYP $DOWNLOAD_PATH $CMD
ASDRETSTATUS=$?

EXITSTATUS=$ASDRETSTATUS

# read the status if action
if [ -e "$ASDCLIENT_STATUSFILE" ];then
	tmp=`cat $ASDCLIENT_STATUSFILE` >/dev/null 2>&1
	if [ -n "$tmp" ];then
		errcode=`echo $tmp | cut -d " " -f 1`
		download_path=`echo $tmp | cut -d " " -f 2`
		if [ "$arg5" = "only_dwld" ];then
			EXITSTATUS=$errcode
			echo 1 > $ASD_VERSION_CHECK_FILE
		else
			if [ "$errcode" = 0 ] && [ "$download_path" != "failed" ];then
				if [ "$SWTYP" = "firmware" ];then
					$FRM_APPLY_SCRIPT $download_path		
				elif [ "$SWTYP" = "signatures" ];then
					$SIG_APPLY_SCRIPT $download_path
				elif [ "$SWTYP" = "drivers" ];then
					$USB_APPLY_SCRIPT $download_path
				fi
				EXITSTATUS=$?
			fi
        		sendEmailAndLogAlert $EXITSTATUS $SWTYP
		fi
	fi
fi


if [ "$CMD" = "download" ] && [ "$SWTYP" = "firmware" ];then
    if [ "$EXITSTATUS" = 0 ];then
        echo 1 > $TMP_ASD_FRM_UPGRD_FILE
    else
        echo 0 > $TMP_ASD_FRM_UPGRD_FILE
    fi
fi

if [ -e "$ASDSTATUS" ];then
    rm $ASDSTATUS
fi

SEND_MAIL=1

if [ "$ASDRETSTATUS" = "$ER_FRM_VER_CURRENT_IS_LATEST" ] || [ "$ASDRETSTATUS" = "$ER_USBDRV_VER_CURRENT_IS_LATEST" ] || [ "$ASDRETSTATUS" = "$ER_SIG_VER_CURRENT_IS_LATEST" ];then
    SEND_MAIL=0
fi


check_status=`grep -e "Status:1" $SWDOWNLOAD_STATUS_TMP_FILE`
if [ -n "$check_status" ] && [ "$ASDRETSTATUS" != 0 ] && [ "$SEND_MAIL" = 1 ];then
#    rm $SWDOWNLOAD_STATUS_TMP_FILE
    if [ "$arg5" != "only_dwld" ];then
        sendEmailAndLogAlert -1 $SWTYP
    fi
#else
#    rm $SWDOWNLOAD_STATUS_TMP_FILE
fi

if [ -e "$ASDPROCESS_STATUSFILE" ];then
    rm $ASDPROCESS_STATUSFILE
fi

if [ "$EXITSTATUS" = 0 ];then
    if [ "$SWTYP" = "firmware" ] && [ "$arg5" = "cron" ];then
        reboot_to_image "inactive"
    fi
fi

exit $EXITSTATUS

