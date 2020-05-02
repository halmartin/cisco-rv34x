#!/bin/sh

. /etc/boardInfo

SWTYPE=$1
SWVER=$2

SWUPDATE_FILE="/mnt/configcert/config/swupdateinfo"
SWUPDATTIME=$(date -D '%s' +"%a %b %d %T %Y" -d "$(( `date +%s` ))")

    case $SWTYPE in
    firmware)
        sed -i "/^frm_available_version=/c\frm_available_version=\"$SWVER\"" $SWUPDATE_FILE
        sed -i "/^frm_last_check_time=/c\frm_last_check_time=\"$SWUPDATTIME\"" $SWUPDATE_FILE
        logger -t asdclient -p local0.info " Firmware Last Check Time: $SWUPDATTIME"
    ;;
    drivers)
        sed -i "/^usb_available_version=/c\usb_available_version=\"$SWVER\"" $SWUPDATE_FILE
        sed -i "/^usb_last_check_time=/c\usb_last_check_time=\"$SWUPDATTIME\"" $SWUPDATE_FILE
        logger -t asdclient -p local0.info " USB Dongle Driver Last Check Time: $SWUPDATTIME"
    ;;
    signatures)
	## Irrespective of MR0.5 or MR1, use the same sig_available_version, sig_last_check_time to fill.
        sed -i "/^sig_available_version=/c\sig_available_version=\"$SWVER\"" $SWUPDATE_FILE
        sed -i "/^sig_last_check_time=/c\sig_last_check_time=\"$SWUPDATTIME\"" $SWUPDATE_FILE
        logger -t asdclient -p local0.info " Signature Last Check Time: $SWUPDATTIME"
    ;;
    *)
    ;;
    esac
