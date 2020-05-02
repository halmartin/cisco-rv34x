CONFIGFILE="/tmp/etc/strongswan.conf"

#echo $#
[ $# -eq 1 -o $# -eq 3 -o $# -eq 4 ] || {
	echo "Invalid Arguments"
	echo "Help: confStrongswanEapRadius <radius enable/disable(1/0)> <pri/both> <PriIP/secret/port> <SecIP/secret/port>"
	echo "Ex: 	confStrongswanEapRadius 0 =>Radius Disabled"
	echo "	confStrongswanEapRadius 1 pri 2.2.2.2/password/1812 => Only Primary Radius is configured"
	echo "	confStrongswanEapRadius 1 both 2.2.2.2/password/1812 3.3.3.3/password/1812 => Primary and secondary both radius are configured"
	exit
}

[ $# -eq 4 ] && {
	[ $2 != "both" ] && {
	echo "Invalid Arguments"
	echo "Help: confStrongswanEapRadius <radius enable/disable(1/0)> <pri/both> <PriIP/secret/port> <SecIP/secret/port>"
	echo "Ex: 	confStrongswanEapRadius 0 =>Radius Disabled"
	echo "	confStrongswanEapRadius 1 pri 2.2.2.2/password/1812 => Only Primary Radius is configured"
	echo "	confStrongswanEapRadius 1 both 2.2.2.2/password/1812 3.3.3.3/password/1812 => Primary and secondary both radius are configured"
	exit
	}
}

radiusEnable=$1

[ $radiusEnable -eq 1 ] && {
#	nasIdentifier=1.1.1.1
	isSecRad=$2
	primaryRadius=$3
	[ "$isSecRad" = "both" ] && {
		SecondaryRadius=$4
		secIP=${SecondaryRadius%%\/*}
		secPort=${SecondaryRadius##*\/}
		secSecret=${SecondaryRadius%\/*}
		secSecret=${secSecret#*\/}
	}
}

priIP=${primaryRadius%%\/*}
priPort=${primaryRadius##*\/}
priSecret=${primaryRadius%\/*}
priSecret=${priSecret#*\/}

echo "#This is auto generated file based on radius configuration" > $CONFIGFILE
echo "# strongswan.conf - strongSwan configuration file" >> $CONFIGFILE
echo "#" >> $CONFIGFILE
echo "# Refer to the strongswan.conf(5) manpage for details" >> $CONFIGFILE
echo "#" >> $CONFIGFILE
echo "# Configuration changes should be made in the included files" >> $CONFIGFILE
echo "" >> $CONFIGFILE
echo "charon {" >> $CONFIGFILE
echo "	load_modular = yes" >> $CONFIGFILE
echo "	plugins {" >> $CONFIGFILE
echo "		include strongswan.d/charon/*.conf" >> $CONFIGFILE
if [ $radiusEnable -eq 1 ]
then
#{
	echo "		eap-radius {" >> $CONFIGFILE
	echo "			class_group = yes"  >> $CONFIGFILE
	echo "			eap_start = no"  >> $CONFIGFILE
	echo "			servers {" >> $CONFIGFILE
	echo "				primary {" >> $CONFIGFILE
	echo "					address = $priIP" >> $CONFIGFILE
	echo "					secret = $priSecret" >> $CONFIGFILE
	echo "					port = $priPort" >> $CONFIGFILE
#	echo "					nas_identifer = $nasIdentifier" >> $CONFIGFILE
	echo "					#Always prefer the server, unless it gets unreachable" >> $CONFIGFILE
	echo "					preference = 101" >> $CONFIGFILE
	echo "				}" >> $CONFIGFILE
	if [ "$isSecRad" = "both" ] 
	#{
	then
		echo "				secondary  {" >> $CONFIGFILE
		echo "					address = $secIP" >> $CONFIGFILE
		echo "					secret = $secSecret" >> $CONFIGFILE
		echo "					port = $secPort" >> $CONFIGFILE
#		echo "					nas_identifer = $nasIdentifier" >> $CONFIGFILE
		echo "				}" >> $CONFIGFILE
	#}
	fi
	echo "			}" >> $CONFIGFILE
	echo "		}" >> $CONFIGFILE
#}
fi
echo "	}" >> $CONFIGFILE
echo "}" >> $CONFIGFILE
echo "" >> $CONFIGFILE
echo "include strongswan.d/*.conf" >> $CONFIGFILE

isIPSecRunning=$(pgrep ipsec|wc -l)
[ $isIPSecRunning -gt 0 ] && {
	logger -t VPN-cfg "Reloading ipsec after radius configuration"
	kill -SIGHUP `cat /var/run/charon.pid`
}
