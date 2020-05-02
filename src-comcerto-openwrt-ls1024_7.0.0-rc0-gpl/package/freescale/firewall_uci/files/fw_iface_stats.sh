#!/bin/sh

. /lib/functions/network.sh

TMP_OUTPUT="/tmp/fwifaces.$1"
TMP_FWIFACESTATS="/tmp/fwifacestats.$1"

ubus list network.interface.* | awk -F "." '!/loopback|usb|^gre|wan[1-9]p|wan[1-9]6p/ {print $3}' > $TMP_OUTPUT

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

#rm $TMP_FWIFACESTATS
touch $TMP_FWIFACESTATS

while read -r oper 
do
        status="0"
	ip4addr="0.0.0.0"
       	ip4subnet="0"
        l2ifname="null"
        l3ifname="null"
	ip6addr="0::0"
	ip6mask="0"

	case "$oper" in
		vlan*)
		        __network_get_fwstats_v4 $oper status ip4addr ip4subnet l2ifname l3ifname
			__network_get_fwstats_v6 $oper status ip6addr ip6mask l2ifname l3ifname  #let some parameters may overwrite. It will not impact
			l2ifname=$(uci get network.$oper.ifname)
			echo "iface:$oper status:$status ip4addr:$ip4addr ip4subnet:$ip4subnet ip6addr:$ip6addr ip6mask:$ip6mask l2ifname:$l2ifname l3ifname:$l3ifname" >> $TMP_FWIFACESTATS 
		;;

		wan16* | wan26* | wan1_tun1 | wan2_tun1 | wan1_tun2 | wan2_tun2 | *_PD)
			ifname=$(echo $oper | sed s/wan[1-9]6*/\&p/g)
			isavailable=$(ifstatus $ifname | grep -o "available")
			if [ "$isavailable" == "available" ];then
				__network_get_fwstats_v6 $ifname status ip6addr ip6mask l2ifname l3ifname 
				#echo "Details are for $ifname: status:$status ip6addr:$ip6addr ip6mask:$ip6mask l2ifname:$l2ifname l3ifname:$l3ifname"
				[ ${#l2ifname} -eq 0 ] && l2ifname=null
				[ ${#l3ifname} -eq 0 ] && l3ifname=null
				echo "iface:$oper status:$status ip4addr:null ip4subnet:0 ip6addr:$ip6addr ip6mask:$ip6mask l2ifname:$l2ifname l3ifname:$l3ifname" >> $TMP_FWIFACESTATS
			else
				__network_get_fwstats_v6 $oper status ip6addr ip6mask l2ifname l3ifname 
				[ ${#l2ifname} -eq 0 ] && l2ifname=null
				[ ${#l3ifname} -eq 0 ] && l3ifname=null
				echo "iface:$oper status:$status ip4addr:null ip4subnet:0 ip6addr:$ip6addr ip6mask:$ip6mask l2ifname:$l2ifname l3ifname:$l3ifname" >> $TMP_FWIFACESTATS
			fi
		;;

		*)
			ifname=$(echo $oper | sed s/wan[1-9]*/\&p/g)
			isavailable=$(ifstatus $ifname | grep -o "available")
			if [ "$isavailable" == "available" ];then
		        	__network_get_fwstats_v4 $ifname status ip4addr ip4subnet l2ifname l3ifname 
				#echo "Details are for $ifname: status:$status ip4addr:$ip4addr ip4subnet:$ip4subnet l2ifname:$l2ifname l3ifname:$l3ifname"
				[ ${#l2ifname} -eq 0 ] && l2ifname=null
				[ ${#l3ifname} -eq 0 ] && l3ifname=null
                [ ${#status} -eq 0 ] && status=0
                if [ "$oper" == "usb1_4" ]; then
				    echo "iface:usb1 status:$status ip4addr:$ip4addr ip4subnet:$ip4subnet ip6addr:null ip6mask:0 l2ifname:$l2ifname l3ifname:$l3ifname" >> $TMP_FWIFACESTATS
                elif [ "$oper" == "usb2_4" ]; then
				    echo "iface:usb2 status:$status ip4addr:$ip4addr ip4subnet:$ip4subnet ip6addr:null ip6mask:0 l2ifname:$l2ifname l3ifname:$l3ifname" >> $TMP_FWIFACESTATS
                else
    				echo "iface:$oper status:$status ip4addr:$ip4addr ip4subnet:$ip4subnet ip6addr:null ip6mask:0 l2ifname:$l2ifname l3ifname:$l3ifname" >> $TMP_FWIFACESTATS
                fi
			else
		        	__network_get_fwstats_v4 $oper status ip4addr ip4subnet l2ifname l3ifname 
				[ ${#l2ifname} -eq 0 ] && l2ifname=null
				[ ${#l3ifname} -eq 0 ] && l3ifname=null
                [ ${#status} -eq 0 ] && status=0
                if [ "$oper" == "usb1_4" ]; then
				    echo "iface:usb1 status:$status ip4addr:$ip4addr ip4subnet:$ip4subnet ip6addr:null ip6mask:0 l2ifname:$l2ifname l3ifname:$l3ifname" >> $TMP_FWIFACESTATS
                elif [ "$oper" == "usb2_4" ]; then
				    echo "iface:usb2 status:$status ip4addr:$ip4addr ip4subnet:$ip4subnet ip6addr:null ip6mask:0 l2ifname:$l2ifname l3ifname:$l3ifname" >> $TMP_FWIFACESTATS
                else
				    echo "iface:$oper status:$status ip4addr:$ip4addr ip4subnet:$ip4subnet ip6addr:null ip6mask:0 l2ifname:$l2ifname l3ifname:$l3ifname" >> $TMP_FWIFACESTATS
                fi
			fi
		;;
	esac
done < $TMP_OUTPUT

rm $TMP_OUTPUT
