#!bin/sh
ASDFILE="/tmp/asd_email_content"
. /etc/boardInfo

CMD="check"
PID=`uci get systeminfo.sysinfo.pid | cut -d'-' -f1`
VID=`uci get systeminfo.sysinfo.vid`
SNO=`uci get systeminfo.sysinfo.serial_number`

FRM_NOTIFY=`uci get swupdate.firmware.notify`
USBVER_NOTIFY=`uci get swupdate.dongle.notify`
SIGVER_NOTIFY=`uci get swupdate.signature.notify`

ru_check=`uci get systeminfo.sysinfo.region 2> /dev/null`

REL_LINK="http://www.cisco.com/c/en/us/support/routers/small-business-rv-series-routers/products-release-notes-list.html"
SWUPDATE_INFO_FILE="/etc/swupdateinfo"
frmerrorcode=0
usberrorcode=0
sigrrorcode=0
EXITSTATUS=0

FRMVER=`cat /etc/firmware_version`
if [ "$ru_check" = "RU" ]; then
	#check for RU image and convert frimware version from 1.0.01.16_RUn form to 1.0.01.16
	FRMVER=`echo $FRMVER | cut -d'_' -f1`
fi

USBVER=`cat /etc/usb-modem.ver`
if [ "$IS_MR1" = "1" ]; then
SIGVER=`grep sec_latest_version "/etc/swupdateinfo" | awk -F'=' '{ print $2 }' |  sed 's/"//g'`
else
SIGVER=`grep sig_latest_version "/etc/swupdateinfo" | awk -F'=' '{ print $2 }' |  sed 's/"//g'`
fi
  
#echo "/usr/bin/asdclient $PID $VID $SNO $FRMVER firmware /tmp $CMD"
#echo "/usr/bin/asdclient $PID $VID $SNO $USBVER drivers /tmp $CMD"
#echo "/usr/bin/asdclient $PID $VID $SNO $SIGVER signatures /tmp $CMD"

#Check whether board is RV26X based on PID
if [ "$PID" = "RV260" -o "$PID" = "RV260W" -o "$PID" = "RV260P" ]; then
    RV260_series=1
else
    RV260_series=0
fi
BB2=0

if [ "$PID" = "RV340" -o "$PID" = "RV340W" -o "$PID" = "RV345" -o "$PID" = "RV345P" ]; then
	RV260_series=1
    BB2=1
fi
/usr/bin/asdclient $PID $VID $SNO $FRMVER firmware /tmp $CMD >/dev/null 2>&1
frmerrorcode=$?
#echo "frmerrorcode = $frmerrorcode "

#Invoke ASD client for USB only if Board series is RV26X
if [ "$RV260_series" = 1 ];then
    /usr/bin/asdclient $PID $VID $SNO $USBVER drivers /tmp $CMD
    usberrorcode=$?
fi
#echo "usberrorcode= $usberrorcode"

if [ "$BB2" = 1 ];then
/usr/bin/asdclient $PID $VID $SNO $SIGVER signatures /tmp $CMD
sigrrorcode=$?
fi
#echo "sigrrorcode= $sigrrorcode"

if [ "$BB2" = 1 ];then
if [ "$frmerrorcode" -ne 0 ] || [ "$usberrorcode" -ne 0 ] || [ "$sigrrorcode" -ne 0 ];then
    EXITSTATUS=1
    exit $EXITSTATUS
fi
else
if [ "$frmerrorcode" -ne 0 ] || [ "$usberrorcode" -ne 0 ];then
    EXITSTATUS=1
    exit $EXITSTATUS
fi
fi



AUTO_UPDATE_NOTIFY=`uci get swupdate.autoupdate.notify_type`
NOTIFY_EMAIL=`uci get swupdate.autoupdate.notify_emailid`
if [ "$AUTO_UPDATE_NOTIFY" != "email" ] || [ -z "$NOTIFY_EMAIL" ];then
    exit $EXITSTATUS
fi


NEW_FRMVER=`grep frm_available_version $SWUPDATE_INFO_FILE | awk -F'=' '{ print $2 }' |  sed 's/"//g'`
NEW_USBVER=`grep usb_available_version $SWUPDATE_INFO_FILE | awk -F'=' '{ print $2 }' |  sed 's/"//g'`  
if [ "$BB2" = 1 ];then
NEW_SIGVER=`grep sig_available_version $SWUPDATE_INFO_FILE | awk -F'=' '{ print $2 }' |  sed 's/"//g'`
fi

CHKVER=`cat /etc/verchk`

#echo " CHKVER= $CHKVER"
#echo " FRM_NOTIFY= $FRM_NOTIFY"
#echo " USBVER_NOTIFY= $USBVER_NOTIFY"
#echo " SIGVER_NOTIFY= $SIGVER_NOTIFY"

SUBJECT_STR=""

if [ "$CHKVER" -eq 0 ];then

    if [ "$FRM_NOTIFY" = "true" ];then
        SUBJECT_STR="A new firmware version, release $NEW_FRMVER, is available"
        echo  "A new firmware version, release $NEW_FRMVER, is available on cisco.com. The release note for this version is available for download from: $REL_LINK" > $ASDFILE
        sh /usr/bin/sendAsdmail "$SUBJECT_STR"
    fi

    if [ "$RV260_series" = 1 ];then
        if [ "$USBVER_NOTIFY" = "true" ];then
            SUBJECT_STR="A new USB dongle driver version, release $NEW_USBVER, is available"
            echo  "A new USB dongle driver version, release $NEW_USBVER, is available on cisco.com. The release note for this version is available for download from: $REL_LINK" > $ASDFILE
            sh /usr/bin/sendAsdmail "$SUBJECT_STR"
        fi
    fi

if [ "$BB2" = 1 ];then
    if [ "$SIGVER_NOTIFY" = "true" ];then
        SUBJECT_STR="A new signature version, release $NEW_SIGVER, is available"
        echo  "A new signature version, release $NEW_SIGVER, is available on cisco.com. The release note for this version is available for download from: $REL_LINK" > $ASDFILE
        sh /usr/bin/sendAsdmail "$SUBJECT_STR"
    fi
fi

    exit $EXITSTATUS
fi

RETSTATUS=0
sh /usr/bin/compswvers.sh $FRMVER $NEW_FRMVER
RETSTATUS=$?
if [ "$RETSTATUS" = 2 ];then
    if [ "$FRM_NOTIFY" = "true" ];then
        SUBJECT_STR="A new firmware version, release $NEW_FRMVER, is available"
        #echo "New available firmware version: $NEW_FRMVER" > $ASDFILE
        echo  "A new firmware version, release $NEW_FRMVER, is available on cisco.com. The release note for this version is available for download from: $REL_LINK" > $ASDFILE
        sh /usr/bin/sendAsdmail "$SUBJECT_STR"
    fi
fi

if [ "$RV260_series" = 1 ];then
    sh /usr/bin/compswvers.sh $USBVER $NEW_USBVER
    RETSTATUS=$?
    if [ "$RETSTATUS" = 2 ];then
        if [ "$USBVER_NOTIFY" = "true" ];then
            SUBJECT_STR="A new USB dongle driver version, release $NEW_USBVER, is available"
            #echo "New available USB Dongle Driver version: $NEW_USBVER" > $ASDFILE
            echo  "A new USB dongle driver version, release $NEW_USBVER, is available on cisco.com. The release note for this version is available for download from: $REL_LINK" > $ASDFILE
            sh /usr/bin/sendAsdmail "$SUBJECT_STR"
        fi
    fi
fi

if [ "$BB2" = 1 ];then
sh /usr/bin/compswvers.sh $SIGVER $NEW_SIGVER
RETSTATUS=$?
if [ "$RETSTATUS" = 2 ];then
    if [ "$SIGVER_NOTIFY" = "true" ];then
        #echo "New available Signature version: $NEW_SIGVER" > $ASDFILE
        SUBJECT_STR="A new signature version, release $NEW_SIGVER, is available"
        echo  "A new signature version, release $NEW_SIGVER, is available on cisco.com. The release note for this version is available for download from: $REL_LINK" > $ASDFILE
        sh /usr/bin/sendAsdmail "$SUBJECT_STR"
    fi
fi
fi

exit $EXITSTATUS
