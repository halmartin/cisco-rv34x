## Usage: sh -x scripts/console-enable.sh

echo -e "Removed barebox console patch,doing barebox clean and re-building"

rm bin/comcerto2000-glibc/RV34X*

rm package/freescale/barebox-comcerto/patches-c2krv340/0059-Disabling-linux-console-from-barebox-as-per-FR-image.patch

make package/utils/busybox/clean

make package/freescale/barebox-comcerto/clean

### Below iptable command insersion into /etc/init.d/zfinish to enable SSH from LAN side 
sed -i '/boot(/a iptables -D delegate_input -j ssh' package/freescale/system/files/finish.init
## Otherwise, below command can be used to enable SSH from both LAN,WAN side
# iptables -I ssh -j ACCEPT

make package/freescale/system/clean

ver=`cat package/base-files/files/etc/firmware_version`;subver=`cat package/base-files/files/etc/firmware_version_minor`;echo ${ver}${subver} >package/base-files/files/etc/firmware_version

echo -e "Modifying version with suffix subver"

cat package/base-files/files/etc/firmware_version

#echo -e "Again Rebuilding"

#make V=99
