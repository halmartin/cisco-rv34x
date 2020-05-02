#!/bin/sh

################  Script Syntax  ###############
# rv340_fw_unpack.sh <firmware_file_name>.img  #
################################################

IMG_FILE=`basename $1`
IMG_DIRNAME=`dirname $1`
DIR_FIRMWARE="/tmp/firmware/"
UPDATE_FROM_USB="$2"

MTD_BAREBOX=0
MTD_BAREBOX_ENV=15
MTD_ENV=2
BAREBOX=barebox-c2krv340.bin
KERNEL=openwrt-comcerto2000-hgw-uImage.img
ROOTFS=openwrt-comcerto2000-hgw-rootfs-ubi_nand.img
CURRENT_ROOTFS=`cat /proc/cmdline | awk -vRS=" " -vORS="\n" '1' | grep ubi.mtd`
ASDSTATUS="/tmp/asdclientstatus"
KERNEL_PART1=3 ROOTFS_PART1=4
KERNEL_PART2=6 ROOTFS_PART2=7
ACTIVE_IMG="`uci get firmware.firminfo.active`"
INACTIVE_IMG="`uci get firmware.firminfo.inactive`"
DIR_IMG="/mnt/configcert/$INACTIVE_IMG"

PRESCRIPT="sh preupgrade.sh"
PREUPGRADEFILE="preupgrade.gz"
PREUPGRADEFILE_MD5="preupgrade_md5sum"

# Checking free space in /tmp (space required for intermediate tar files is 60 MB * 3 = 180 MB)
freetmp=$(df -m /tmp | tail -1 | awk '{print $4}')
if [ $freetmp -lt 180 ];then
	logger -t system -p local0.error "Insufficient space in /tmp (Current space:$freetmp MB, Required space:180 MB)"
	echo "Firmware Update Failed!"
	exit 10
fi

# Check for Firmware Package File
cd $DIR_FIRMWARE

if [ "$IMG_DIRNAME" != "." ];then
	# Absolute path was given
	cp -f $1 /tmp/firmware > /dev/null 2>&1
fi

if [ -f $IMG_FILE ]; then
  echo "$IMG_FILE File there"
else
  logger -t system -p local0.error "$IMG_FILE File Not there"
  echo "$IMG_FILE File Not there"
  echo "Usage: rv340_fw_unpack.sh <*.img>"
  cd - > /dev/null 2>&1
  exit 11
fi

# Start Diag LED Blinking
rv340_led.sh diag slow > /dev/null 2>&1 &

# Setup Inactive Image
if [ x$ACTIVE_IMG = ximage1 ]; then
  MTD_KERNEL_BACKUP=$KERNEL_PART2
  MTD_ROOTFS_BACKUP=$ROOTFS_PART2
  echo "2" > /tmp/active
elif [ x$ACTIVE_IMG = ximage2 ]; then
  MTD_KERNEL_BACKUP=$KERNEL_PART1
  MTD_ROOTFS_BACKUP=$ROOTFS_PART1
  echo "1" > /tmp/active
else
  logger -t system -p local0.error "Error Incorrect Rootfs partition!"
  logger -t system -p local0.error "Firmware Update Failed!"
  echo "Error Incorrect Rootfs partition!"
  rm -rf $DIR_FIRMWARE* > /dev/null 2>&1
  pgrep -f "rv340_led.sh diag slow" | xargs kill -9 > /dev/null 2>&1
  rv340_led.sh diag fast > /dev/null 2>&1 &
  cd - > /dev/null 2>&1
  exit 12
fi

# Calculate md5sum of IMG file
echo `md5sum $IMG_FILE | cut -d " " -f 1` > md5sum_fw-rv340-img

echo -n "Extracting files from $IMG_FILE ..."
dd if=$IMG_FILE of=rv340_fw.gz bs=64 skip=1 > /dev/null 2>&1
[ "$?" -ne 0 ] && {
        logger -t system -p local0.error "image extraction failed"
}

tar -xzvf rv340_fw.gz  > /dev/null 2>&1
[ "$?" -ne 0 ] && {
	logger -t system -p local0.error "Invalid firmware file."
}

# Deleting intermediate tar files to free /tmp memory
rm -rf rv340_fw.gz
echo "done."

echo -n "Image md5sum Verification..."
MD5SUM_IMG="`md5sum fw.gz | cut -d " " -f 1`" >/dev/null 2>&1
[ "$?" -ne 0 ] && {
	logger -t system -p local0.error "can't open fw.gz: No such file or directory."
  	echo "Firmware Update Failed!"
	pgrep -f "rv340_led.sh diag slow" | xargs kill -9 > /dev/null 2>&1
	rv340_led.sh diag fast > /dev/null 2>&1 &
	rm -rf $DIR_FIRMWARE* > /dev/null 2>&1
	cd - > /dev/null 2>&1
	exit 12
}
MD5SUM_PKG="`cat md5sum_fw-rv340 | cut -d " " -f 1`"
[ "$?" -ne 0 ] && {
	logger -t system -p local0.error "can't open md5sum_fw-rv340: No such file or directory."
  	echo "Firmware Update Failed!"
	pgrep -f "rv340_led.sh diag slow" | xargs kill -9 > /dev/null 2>&1
	rv340_led.sh diag fast > /dev/null 2>&1 &
	rm -rf $DIR_FIRMWARE* > /dev/null 2>&1
	cd - > /dev/null 2>&1
	exit 12
}
if [ "$MD5SUM_IMG" = "$MD5SUM_PKG" ]; then
  echo "Successful : $MD5SUM_IMG ."
else
  logger -t system -p local0.error "Failed : MD5SUM_IMG=$MD5SUM_IMG NOT Equal to MD5SUM_PKG=$MD5SUM_PKG !"
  logger -t system -p local0.error "Firmware Update Failed!"
  echo "Failed : MD5SUM_IMG=$MD5SUM_IMG NOT Equal to MD5SUM_PKG=$MD5SUM_PKG !"
  echo "Firmware Update Failed!"
  pgrep -f "rv340_led.sh diag slow" | xargs kill -9 > /dev/null 2>&1
  rv340_led.sh diag fast > /dev/null 2>&1 &
  rm -rf $DIR_FIRMWARE* > /dev/null 2>&1
  cd - > /dev/null 2>&1
  exit 13
fi

tar -xzvf fw.gz  > /dev/null 2>&1
[ "$?" -ne 0 ] && {
        logger -t system -p local0.error "Extraction of individual images failed"
        echo "Firmware Update Failed!"
        pgrep -f "rv340_led.sh diag slow" | xargs kill -9 > /dev/null 2>&1
        rv340_led.sh diag fast > /dev/null 2>&1 &
        rm -rf $DIR_FIRMWARE* > /dev/null 2>&1
        cd - > /dev/null 2>&1
	sleep 10
        pgrep -f "rv340_led.sh diag fast" | xargs kill -9 > /dev/null 2>&1
        exit 12
}

# Deleting intermediate tar files to free /tmp memory
rm -rf fw.gz

checkVersion() {
        local count;
	count=0;
	 if [ $# -eq 1 ]; then                                                                                                         
                for var in $(echo ${1} | sed -e "s/[.-]/ /g"); 
                do      
                        var=$(echo "$var" | sed -e "s/^0*//g")
                        [ -z "$var" ] && var=0
                        count=$(((${count} << 8) + ${var}));
                done;
        fi;                                                                                
        echo ${count};  
}

is_newer() {
	image_version=`cat firmware_version`
	current_version=`cat /etc/firmware_version`
        if [ $(checkVersion "$image_version") -gt $(checkVersion "$current_version") ];then
		logger -t system -p local0.warn "Firmware Version Check Passed."
	else
		logger -t system -p local0.warn "Firmware version ($image_version) is not higher than current firmware version($current_version).Aborting.."
		echo "Firmware Update Failed!"
		pgrep -f "rv340_led.sh diag slow" | xargs kill -9 > /dev/null 2>&1
		rv340_led.sh diag fast > /dev/null 2>&1 &
		rm -rf $DIR_FIRMWARE* > /dev/null 2>&1
		cd - > /dev/null 2>&1
		exit 14
	fi
}


# Autoupgrade from USB where we allow upgrade to higher version only
[ "$UPDATE_FROM_USB" = "1" ] && is_newer

# Check for preupgrade infra
if [ -e "$PREUPGRADEFILE" ] && [ -e "$PREUPGRADEFILE_MD5" ]; then
	logger -t system -p local0.info "Checking md5sum for Preupgrade"
	curr_md5="`md5sum $PREUPGRADEFILE | cut -d " " -f 1`" >/dev/null 2>&1
	buildin_md5="`cat $PREUPGRADEFILE_MD5`" >/dev/null 2>&1
	
	# Verify md5sum
	if [ "$buildin_md5" != "$curr_md5" ]; then
	        logger -t system -p local0.error "Failed : md5sum mismatched for $PREUPGRADEFILE"
    		pgrep -f "rv340_led.sh diag slow" | xargs kill -9 > /dev/null 2>&1
    		rv340_led.sh diag fast > /dev/null 2>&1 &
    		rm -rf $DIR_FIRMWARE* > /dev/null 2>&1
    		cd - > /dev/null 2>&1
		exit 13
	fi
	
	# Untar the preupgrade.tar file
	tar -xzvf $PREUPGRADEFILE  > /dev/null 2>&1
	
	# Run Prescript, which in turns runs all the scripts available in scripts folder. These scrips can expect files & directories from data folder.
	logger -t system -p local0.info "Running Preupgrade Scripts.."
	$PRESCRIPT
	logger -t system -p local0.info "Running Preupgrade Scripts.. Done!"
fi

# Barebox Update
echo "Checking if Barebox binary presence to proceed with Barebox Update..."
if [ -e "$BAREBOX" ]; then
  echo "Barebox image $BAREBOX FOUND!"
  echo "Updating Barebox..."
  logger -t system -p local0.info "Updating Barebox..."
  echo -n "Erasing Barebox partition..."
  flash_erase -q /dev/mtd$MTD_BAREBOX 0 0
  if [ $? -eq 0 ]; then
    echo "done."
  else
    logger -t system -p local0.error "Erasing Barebox Failed!"
    logger -t system -p local0.error "Firmware Update Failed!"
    echo "Failed!"
    echo "Firmware Update Failed!"
    pgrep -f "rv340_led.sh diag slow" | xargs kill -9 > /dev/null 2>&1
    rv340_led.sh diag fast > /dev/null 2>&1 &
    rm -rf $DIR_FIRMWARE* > /dev/null 2>&1
    cd - > /dev/null 2>&1
    exit 15
	fi
  echo -n "Erasing Barebox Env partition..."
  flash_erase -q /dev/mtd$MTD_BAREBOX_ENV 0 0
  if [ $? -eq 0 ]; then
    echo "done."
  else
    logger -t system -p local0.error "Erasing Barebox Env Failed!"
    logger -t system -p local0.error "Firmware Update Failed!"
    echo "Failed!"
    echo "Firmware Update Failed!"
    pgrep -f "rv340_led.sh diag slow" | xargs kill -9 > /dev/null 2>&1
    rv340_led.sh diag fast > /dev/null 2>&1 &
    rm -rf $DIR_FIRMWARE* > /dev/null 2>&1
    cd - > /dev/null 2>&1
    exit 15
        fi
  echo -n "Flashing Barebox partition..."
  nandwrite -p -q /dev/mtd$MTD_BAREBOX $BAREBOX
  if [ $? -eq 0 ]; then
    echo "done."
    echo "Barebox Update Successful."
    rm -rf $BAREBOX
  else
    logger -t system -p local0.error "Flashing Barebox Failed!"
    logger -t system -p local0.error "Firmware Update Failed!"
    echo "Failed!"
    echo "Barebox Update Failed!"
    echo "Firmware Update Failed!"
    pgrep -f "rv340_led.sh diag slow" | xargs kill -9 > /dev/null 2>&1
    rv340_led.sh diag fast > /dev/null 2>&1 &
    rm -rf $DIR_FIRMWARE* > /dev/null 2>&1
    exit 15
  fi
else
  logger -t system -p local0.error "Barebox image $BAREBOX NOT FOUND!"
  echo "Barebox image $BAREBOX NOT FOUND! Continuing with other Images' Update..."
fi

# Kernel Update
echo "Updating Kernel..."
logger -t system -p local0.info "Updating Kernel..."
if [ ! -e "$KERNEL" ]; then
  logger -t system -p local0.error "Kernel image $KERNEL NOT FOUND! Exiting!"
  logger -t system -p local0.error "Firmware Update Failed!"
  echo "Failed!"
  echo "Kernel image $KERNEL NOT FOUND! Exiting!"
  pgrep -f "rv340_led.sh diag slow" | xargs kill -9 > /dev/null 2>&1
  rv340_led.sh diag fast > /dev/null 2>&1 &
  rm -rf $DIR_FIRMWARE* > /dev/null 2>&1
  cd - > /dev/null 2>&1
  exit 12
fi
echo -n "Erasing Kernel partition..."
flash_erase -q /dev/mtd$MTD_KERNEL_BACKUP 0 0
if [ $? -eq 0 ]; then
  echo "done."
else
  logger -t system -p local0.error "Erasing Kernel partition Failed!"
  logger -t system -p local0.error "Firmware Update Failed!"
  echo "Failed!"
  echo "Firmware Update Failed!"
  pgrep -f "rv340_led.sh diag slow" | xargs kill -9 > /dev/null 2>&1
  rv340_led.sh diag fast > /dev/null 2>&1 &
  rm -rf $DIR_FIRMWARE* > /dev/null 2>&1
  cd - > /dev/null 2>&1
  exit 15
fi
echo -n "Flashing Kernel partition..."
nandwrite -p -q /dev/mtd$MTD_KERNEL_BACKUP $KERNEL
if [ $? -eq 0 ]; then
  echo "done."
  echo "Kernel Update Successful."
  rm -rf $KERNEL
else
  logger -t system -p local0.error "Flashing Kernel partition Failed!"
  logger -t system -p local0.error "Firmware Update Failed!"
  echo "Failed!"
  echo "Kernel Update Failed!"
  echo "Firmware Update Failed!"
  pgrep -f "rv340_led.sh diag slow" | xargs kill -9 > /dev/null 2>&1
  rv340_led.sh diag fast > /dev/null 2>&1 &
  rm -rf $DIR_FIRMWARE* > /dev/null 2>&1
  cd - > /dev/null 2>&1
  exit 15
fi

# Rootfs Update
echo "Updating Rootfs..."
logger -t system -p local0.info "Updating Rootfs..."
if [ ! -e "$ROOTFS" ]; then
  logger -t system -p local0.error "Rootfs image $ROOTFS NOT FOUND! Exiting!"
  logger -t system -p local0.error "Firmware Update Failed!"
  echo "Failed!"
  echo "Rootfs image $ROOTFS NOT FOUND! Exiting!"
  pgrep -f "rv340_led.sh diag slow" | xargs kill -9 > /dev/null 2>&1
  rv340_led.sh diag fast > /dev/null 2>&1 &
  rm -rf $DIR_FIRMWARE* > /dev/null 2>&1
  cd - > /dev/null 2>&1
  exit 12
fi
echo -n "Erasing ROOTFS partition..."
flash_erase -q /dev/mtd$MTD_ROOTFS_BACKUP 0 0
if [ $? -eq 0 ]; then
  echo "done."
else
  logger -t system -p local0.error "Erasing ROOTFS partition Failed!"
  logger -t system -p local0.error "Firmware Update Failed!"
  echo "Failed!"
  echo "Firmware Update Failed!"
  pgrep -f "rv340_led.sh diag slow" | xargs kill -9 > /dev/null 2>&1
  rv340_led.sh diag fast > /dev/null 2>&1 &
  rm -rf $DIR_FIRMWARE* > /dev/null 2>&1
  cd - > /dev/null 2>&1
  exit 15
fi
echo -n "Flashing Rootfs partition..."
ubiformat -q /dev/mtd$MTD_ROOTFS_BACKUP -f $ROOTFS -s 2048 -O 2048
if [ $? -eq 0 ]; then
  echo "done."
  echo "Rootfs Update Successful."
  rm -rf $ROOTFS
  echo "Mounting rootfs in /tmp/new_mnt_rootfs, to initiate UBIFS Fixup process "
  mkdir /tmp/new_mnt_rootfs
  ubiattach -p /dev/mtd$MTD_ROOTFS_BACKUP
  #ubidev=`dmesg | grep "UBI: attaching" | tail -1  | cut -d ' ' -f 8`
  #part="_0"
  mount -t ubifs /dev/ubi1_0 /tmp/new_mnt_rootfs
  umount /tmp/new_mnt_rootfs
  ubidetach -p /dev/mtd$MTD_ROOTFS_BACKUP
else
  logger -t system -p local0.error "Flashing Rootfs partition Failed!"
  logger -t system -p local0.error "Firmware Update Failed!"
  echo "Failed!"
  echo "Rootfs Update Failed!"
  echo "Firmware Update Failed!"
  pgrep -f "rv340_led.sh diag slow" | xargs kill -9 > /dev/null 2>&1
  rv340_led.sh diag fast > /dev/null 2>&1 &
  rm -rf $DIR_FIRMWARE* > /dev/null 2>&1
  cd - > /dev/null 2>&1
  exit 15
fi

# Add New Firmware Specific config files in Inactive Config Partition
echo -n "Copying image configs to $DIR_IMG ..."
cp -f firmware_version firmware_time img_version md5sum_fw-rv340-img $DIR_IMG > /dev/null 2>&1
echo "done."

# Set Updated Firmware's details in UCI
if [ x$INACTIVE_IMG = ximage1 ]; then
  uci set firmware.firminfo.active="image2"
  if [ -f /mnt/configcert/image2/firmware_version ]; then
    uci set firmware.firminfo.version="`cat /mnt/configcert/image2/firmware_version`"
  fi
  if [ -f /mnt/configcert/image2/md5sum_fw-rv340-img ]; then
    uci set firmware.firminfo.md5sum="`cat /mnt/configcert/image2/md5sum_fw-rv340-img`"
  fi
  uci set firmware.firminfo.inactive="image1"
  if [ -f /mnt/configcert/image1/firmware_version ]; then
    uci set firmware.firminfo.inactive_version="`cat /mnt/configcert/image1/firmware_version`"
  fi
  if [ -f /mnt/configcert/image1/md5sum_fw-rv340-img ]; then
    uci set firmware.firminfo.inactive_md5sum="`cat /mnt/configcert/image1/md5sum_fw-rv340-img`"
  fi
elif [ x$INACTIVE_IMG = ximage2 ]; then
  uci set firmware.firminfo.active="image1"
  if [ -f /mnt/configcert/image1/firmware_version ]; then
    uci set firmware.firminfo.version="`cat /mnt/configcert/image1/firmware_version`"
  fi
  if [ -f /mnt/configcert/image1/md5sum_fw-rv340-img ]; then
    uci set firmware.firminfo.md5sum="`cat /mnt/configcert/image1/md5sum_fw-rv340-img`"
  fi
    uci set firmware.firminfo.inactive="image2"
  if [ -f /mnt/configcert/image2/firmware_version ]; then
    uci set firmware.firminfo.inactive_version="`cat /mnt/configcert/image2/firmware_version`"
  fi
  if [ -f /mnt/configcert/image2/md5sum_fw-rv340-img ]; then
    uci set firmware.firminfo.inactive_md5sum="`cat /mnt/configcert/image2/md5sum_fw-rv340-img`"
  fi
fi
uci commit firmware

rm -rf $DIR_FIRMWARE* > /dev/null 2>&1
cd - > /dev/null 2>&1

# Firmware Update Done, Stop Diag LED Blinking and Turn it Off
pgrep -f "rv340_led.sh diag slow" | xargs kill -9 > /dev/null 2>&1
rv340_led.sh diag off

logger -t system -p local0.notice "Firmware Update Successful."
SWUPDATE_FILE="/mnt/configcert/config/swupdateinfo"

update_firmwareinfo()
{
        current_frm_version="`uci get firmware.firminfo.inactive_version`" >/dev/null 2>&1
	# this field contains the version of current firmware upgrade
        frm_update_time=`date`
	inactive_firmware=`uci get firmware.firminfo.inactive`
	append_time="_usb_latest_update_time="
	usb_updated_time=$inactive_firmware$append_time
	if [ ! -f "/mnt/configcert/config/swupdateinfo" ];then
		# This case arises, Either factory upgrade including partion upgrade [This covers first time programmming ]
		touch $SWUPDATE_FILE
		echo "frm_available_version=\"\"" >> $SWUPDATE_FILE
		echo "frm_last_check_time=\"\"" >> $SWUPDATE_FILE
		echo "frm_latest_version=\"0.0.0.1\"" >> $SWUPDATE_FILE
		echo "frm_latest_update_time=\"\"" >> $SWUPDATE_FILE
		echo "usb_available_version=\"\"" >> $SWUPDATE_FILE
		echo "usb_last_check_time=\"\"" >> $SWUPDATE_FILE
		echo "image1_usb_latest_version=\"0.0.0.1\"" >> $SWUPDATE_FILE
		echo "image1_usb_latest_update_time=\"\"" >> $SWUPDATE_FILE
		echo "image1_usb_latest_update_timezone=\"\"" >> $SWUPDATE_FILE
		echo "image2_usb_latest_version=\"0.0.0.1\"" >> $SWUPDATE_FILE
		echo "image2_usb_latest_update_time=\"\"" >> $SWUPDATE_FILE
		echo "image2_usb_latest_update_timezone=\"\"" >> $SWUPDATE_FILE
		echo "sig_available_version=\"\"" >> $SWUPDATE_FILE
		echo "sig_last_check_time=\"\"" >> $SWUPDATE_FILE
		echo "sig_latest_version=\"0.0.0.1\"" >> $SWUPDATE_FILE
		echo "sig_latest_update_time=\"\"" >> $SWUPDATE_FILE
		echo "sec_latest_version=\"0.0.0.1\"" >> $SWUPDATE_FILE
		echo "sec_latest_update_time=\"\"" >> $SWUPDATE_FILE
	fi

	sed -i "/^frm_latest_version=/c\frm_latest_version=\"$current_frm_version\"" $SWUPDATE_FILE
	sed -i "/^frm_latest_update_time=/c\frm_latest_update_time=\"$frm_update_time\"" $SWUPDATE_FILE
	# signature is loaded as part of firmware upgrade, hence signature update time is same as firmware update time
	sed -i "/^sig_latest_update_time=/c\sig_latest_update_time=\"$frm_update_time\"" $SWUPDATE_FILE
	sed -i "/^sec_latest_update_time=/c\sec_latest_update_time=\"$frm_update_time\"" $SWUPDATE_FILE
	sed -i "/^$usb_updated_time/ c$usb_updated_time\"$frm_update_time\"" $SWUPDATE_FILE

}
update_firmwareinfo
echo "Firmware Update Successful."

# We are not getting exit status of operation from ASD client. So we are writing to as file.
echo "0" > $ASDSTATUS
exit 0
