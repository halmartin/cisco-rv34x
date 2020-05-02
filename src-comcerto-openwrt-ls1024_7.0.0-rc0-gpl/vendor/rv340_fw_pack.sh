#!/bin/sh

# Run the script as ./rv340_fw_upgrade.sh

######################## NOTE ########################
# Before initiating build Please edit script to set / export below environement Variables to required Values
#TARGET=rv340
#VERSION_MAJOR=0
#VERSION_MINOR=0
#VERSION_SUBMINOR=00
#VERSION_REVISION=01
###################### NOTE END ###################### 

##### Custom Header contents  ######
#Version
#Time stamp
#Board_name
##################################	

DIR_VENDOR="$1"
DIR_IMAGES=$PWD
MKIMAGE_PATH=$PWD/../../build_dir/target-arm_cortex-a9_glibc-2.19_eabi/barebox-bareboxc2krv340/barebox-2011.06.0/scripts
TARGET=RV34X
FRM_VERSION=`cat $PWD/../../package/base-files/files/etc/firmware_version`
VERSION_MAJOR=`echo $FRM_VERSION | cut -d'.' -f1`
VERSION_MINOR=`echo $FRM_VERSION | cut -d'.' -f2`
VERSION_SUBMINOR=`echo $FRM_VERSION | cut -d'.' -f3`
VERSION_REVISION=`echo $FRM_VERSION | cut -d'.' -f4 | cut -d'_' -f1`
#VERSION_MAJOR=1
#VERSION_MINOR=0
#VERSION_SUBMINOR=01
#VERSION_REVISION=1601
VERSION=${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_SUBMINOR}.${VERSION_REVISION}

# Below lines append suffix after minor version based on value given in /etc/firmware_version
VERSION_SUFFIX=`echo $FRM_VERSION | cut -d'.' -f4 | cut -d'_' -f2`
if [ "$VERSION_SUFFIX" != "$VERSION_REVISION" ]; then
	VERSION=${VERSION}_${VERSION_SUFFIX}
	echo "Appending suffix to image version ..."
fi

TIME_NOW=`date +%F-%H-%M-%S-%p`
BAREBOX_PATH=barebox-comcerto-bareboxc2krv340
BAREBOX_IMAGEPATH=$PWD
BAREBOX_IMAGE=barebox-c2krv340.bin
KERNEL_IMAGE=openwrt-comcerto2000-hgw-uImage.img
ROOTFS_IMAGE=openwrt-comcerto2000-hgw-rootfs-ubi_nand.img
RV340_FW_VERSION=${TARGET}-v${VERSION}-${TIME_NOW}
RV340_FW_IMAGE=${RV340_FW_VERSION}.img

PREUPGRADE_TAR_FILE="preupgrade.gz"
PREUPGRADE_md5sum="preupgrade_md5sum"
FIRMWARE_PKG_PATH=$PWD/../../package/freescale/firmware
PREUPGRADE_SCRIPT_PATH=$FIRMWARE_PKG_PATH/files/
PREUPGRADE_SCRIPT="preupgrade.sh"
PREUPGRADE_DIR="preupgrade"


# Clear existing md5sum_fw file
if [ -f md5sums_fw ]; then
  rm -rf md5sums_fw
fi

# Copy New md5sum file as md5sums_fw
cp md5sums md5sums_fw;

# Clear existing tmp_fw directory
if [ -d tmp_fw ]; then
  rm -rf tmp_fw
fi

# Create New directory
mkdir tmp_fw


# Check preupgrade infra inclusion flag FLAG_PREUPGRADE_INC : if FLAG_PREUPGRADE_INC = 1 include preupgrade.gz file else skip
FLAG_PREUPGRADE_INC=1
if [ x$FLAG_PREUPGRADE_INC = x1 ]; then
	# Delete previous file (if any)
	rm -rf $PREUPGRADE_TAR_FILE $PREUPGRADE_md5sum

	echo "Preupgrade infra is build with the image"
	cp -f ${PREUPGRADE_SCRIPT_PATH}/${PREUPGRADE_SCRIPT} .
	cp -rf ${PREUPGRADE_SCRIPT_PATH}/${PREUPGRADE_DIR} .
	tar -czvf $PREUPGRADE_TAR_FILE $PREUPGRADE_SCRIPT $PREUPGRADE_DIR > /dev/null

	md5sum $PREUPGRADE_TAR_FILE | cut -d " " -f 1 > $PREUPGRADE_md5sum
	cp -rf $PREUPGRADE_TAR_FILE $PREUPGRADE_md5sum tmp_fw/
	rm -rf $PREUPGRADE_SCRIPT $PREUPGRADE_DIR
else
	echo "Preupgrade infra is not build with the image"
fi


# Check Barebox binary inclusion flag FLAG_BAREBOX_INC : if FLAG_BAREBOX_INC = 1 include barebox in FM Package else skip
FLAG_BAREBOX_INC=1
if [ x$FLAG_BAREBOX_INC = x1 ]; then
  cd $BAREBOX_PATH;
  if [ -f $BAREBOX_IMAGE ]; then
    echo "$BAREBOX_IMAGE File there"
    ls *.bin | xargs md5sum >> ../md5sums_fw;
    cd -;cat md5sums_fw;
    cp md5sums_fw $BAREBOX_PATH/$BAREBOX_IMAGE $KERNEL_IMAGE $ROOTFS_IMAGE tmp_fw/
  else
    echo "$BAREBOX_IMAGE File Not there"
    echo "Stopping Firmware Packaging!"
    cd - > /dev/null
    exit 1
  fi
else
  echo "Excluded Barebox Image $BAREBOX_IMAGE from Firmware Package and Continuing..."
  cp md5sums_fw $KERNEL_IMAGE $ROOTFS_IMAGE tmp_fw/
  echo "NOTE: To include $BAREBOX_IMAGE export FLAG_BAREBOX_INC=1 and Re-Run make V=99"
fi

cd tmp_fw
echo $VERSION > firmware_version
echo $TIME_NOW > firmware_time
echo $RV340_FW_VERSION > img_version 
echo -n "Tarring images..."
tar -czvf fw.gz * > /dev/null
md5sum fw.gz > md5sum_fwrv340
cat md5sum_fwrv340 | cut -d " " -f 1 > md5sum_fw-rv340

if [ x$FLAG_PREUPGRADE_INC = x1 ]; then
	tar -czvf rv340_fw.gz md5sum_fw-rv340 fw.gz $PREUPGRADE_TAR_FILE $PREUPGRADE_md5sum> /dev/null 
else
	tar -czvf rv340_fw.gz md5sum_fw-rv340 fw.gz > /dev/null
fi
echo "done."

echo -n "Copying image config files to $DIR_VENDOR/root_yaffs2/image1 ..."
cat firmware_version firmware_time img_version md5sum_fw-rv340
cp -f firmware_version firmware_time img_version md5sum_fw-rv340 $DIR_VENDOR/root_yaffs2/yaffs2_configcert/image1
echo "done."

echo "Creating Firmware .img..."
$MKIMAGE_PATH/mkimage -A arm -O linux -T firmware -C gzip -n 'RV340 Firmware Package' -d rv340_fw.gz $RV340_FW_IMAGE
if [ $? = 0 ]; then
  md5=`md5sum $RV340_FW_IMAGE | cut -d " " -f 1`
  echo $md5","$VERSION > md5sum_fw-rv340-img
  mv $RV340_FW_IMAGE md5sum_fw-rv340-img $DIR_IMAGES
  cd -
  ls -l $RV340_FW_IMAGE
  echo "done."
  echo "$TARGET Firmware Package $RV340_FW_IMAGE Created Successfully!"
else
	echo "$TARGET Firmware Package Creation Failed!!!"
fi
