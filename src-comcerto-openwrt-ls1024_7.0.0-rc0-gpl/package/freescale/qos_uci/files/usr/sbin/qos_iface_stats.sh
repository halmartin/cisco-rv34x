#!/bin/sh

. /lib/functions/network.sh

if [ "$#" -ne 2 ]
then
	echo "Error: It requires two arguments to run the script."
	echo "Usage: qos_iface_stats.sh <fileID> <apprules | iptablerules>"
	exit
fi

TMP_OUTPUT="/tmp/qosifaces.$1"
TMP_QOSIFACESTATS="/tmp/qosifacestats.$1"

if [ "$2" = "apprules" ]
then
	touch $TMP_OUTPUT
else
	#all interface info is not needed at all. We just bother about USBs.
	ubus list network.interface.* | awk -F "." '!/loopback|usb|gre|wan[1-9]p|wan[1-9]6p/ {print $3}' > $TMP_OUTPUT
	touch $TMP_OUTPUT
fi

usb1proto=`uci get network.usb1.proto`>/dev/null 2>&1
usb2proto=`uci get network.usb2.proto`>/dev/null 2>&1

case $usb1proto in
        3g)
                echo "usb1" >> $TMP_OUTPUT
        ;;
        qmi)
                echo "usb1_4" >> $TMP_OUTPUT
        ;;
	mobile)
		detect=$(cat /var/USBCONNSTATUS 2>/dev/null | grep -i USB1 | awk -F: '{printf $4}')
		if [ "$detect" = "4G" ]; then
			echo "usb1_4" >> $TMP_OUTPUT
		else
			echo "usb1" >> $TMP_OUTPUT
		fi
	;;
esac

case $usb2proto in
        3g)
                echo "usb2" >> $TMP_OUTPUT
        ;;
        qmi)
                echo "usb2_4" >> $TMP_OUTPUT
	;;
	mobile)
		detect=$(cat /var/USBCONNSTATUS 2>/dev/null | grep -i USB2 | awk -F: '{printf $4}')
		if [ "$detect" = "4G" ]; then
			echo "usb2_4" >> $TMP_OUTPUT
		else
			echo "usb2" >> $TMP_OUTPUT
		fi
	;;
esac

touch $TMP_QOSIFACESTATS

while read -r oper 
do
        status="0"
        l2ifname="null"
        l3ifname="null"

	case "$oper" in
		vlan*)
		        __network_get_qosstats_v4 $oper status l2ifname l3ifname
			l2ifname=$(uci get network.$oper.ifname)
			echo "iface:$oper status:$status l2ifname:$l2ifname l3ifname:$l3ifname" >> $TMP_QOSIFACESTATS
		;;

		wan16* | wan26* | wan1_tun1 | wan2_tun1 | wan1_tun2 | wan2_tun2 | *_PD)
			ifname=$(echo $oper | sed s/wan[1-9]6*/\&p/g)
			isavailable=$(ifstatus $ifname | grep -o "available")
			if [ "$isavailable" == "available" ];then
				__network_get_qosstats_v6 $ifname status l2ifname l3ifname 
				#echo "Details are for $ifname: status:$status l2ifname:$l2ifname l3ifname:$l3ifname"
				[ ${#l2ifname} -eq 0 ] && l2ifname=null
				[ ${#l3ifname} -eq 0 ] && l3ifname=null
				echo "iface:$oper status:$status l2ifname:$l2ifname l3ifname:$l3ifname" >> $TMP_QOSIFACESTATS
			else
				__network_get_qosstats_v6 $oper status l2ifname l3ifname 
				[ ${#l2ifname} -eq 0 ] && l2ifname=null
				[ ${#l3ifname} -eq 0 ] && l3ifname=null
				echo "iface:$oper status:$status l2ifname:$l2ifname l3ifname:$l3ifname" >> $TMP_QOSIFACESTATS
			fi
		;;

		*)
			ifname=$(echo $oper | sed s/wan[1-9]*/\&p/g)
			isavailable=$(ifstatus $ifname | grep -o "available")
			if [ "$isavailable" == "available" ];then
		        	__network_get_qosstats_v4 $ifname status l2ifname l3ifname 
				#echo "Details are for $ifname: status:$status l2ifname:$l2ifname l3ifname:$l3ifname"
				[ ${#l2ifname} -eq 0 ] && l2ifname=null
				[ ${#l3ifname} -eq 0 ] && l3ifname=null
				[ ${#status} -eq 0 ] && status=0
				if [ "$oper" == "usb1_4" ]; then
					echo "iface:usb1 status:$status l2ifname:$l2ifname l3ifname:$l3ifname" >> $TMP_QOSIFACESTATS
				elif [ "$oper" == "usb2_4" ]; then
					echo "iface:usb2 status:$status l2ifname:$l2ifname l3ifname:$l3ifname" >> $TMP_QOSIFACESTATS
				else
					echo "iface:$oper status:$status l2ifname:$l2ifname l3ifname:$l3ifname" >> $TMP_QOSIFACESTATS
				fi
			else
		        	__network_get_qosstats_v4 $oper status l2ifname l3ifname 
				[ ${#l2ifname} -eq 0 ] && l2ifname=null
				[ ${#l3ifname} -eq 0 ] && l3ifname=null
				[ ${#status} -eq 0 ] && status=0
				if [ "$oper" == "usb1_4" ]; then
					echo "iface:usb1 status:$status l2ifname:$l2ifname l3ifname:$l3ifname" >> $TMP_QOSIFACESTATS
				elif [ "$oper" == "usb2_4" ]; then
					echo "iface:usb2 status:$status l2ifname:$l2ifname l3ifname:$l3ifname" >> $TMP_QOSIFACESTATS
				else
					echo "iface:$oper status:$status l2ifname:$l2ifname l3ifname:$l3ifname" >> $TMP_QOSIFACESTATS
				fi
			fi
		;;
	esac
done < $TMP_OUTPUT

rm $TMP_OUTPUT
