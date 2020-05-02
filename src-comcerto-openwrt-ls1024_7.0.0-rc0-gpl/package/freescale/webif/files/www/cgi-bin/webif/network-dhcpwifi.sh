#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh
header "xNetwork" "DHCP" "@TR<<DHCP Wifi Interfaces>>" '' "$SCRIPT_NAME"
#ShowNotUpdatedWarning


uci_load network
uci_load dhcp

vap_list="vap0 vap1 vap2 vap3 vap4 vap5 vap6 vap7"


if [ -n "$FORM_iface" ]; then
		if empty "$FORM_submit"; then
			config_get FORM_dhcp_enabled ${FORM_iface} enabled
			config_get FORM_dhcp_start ${FORM_iface} start
			config_get FORM_dhcp_num ${FORM_iface} limit
			config_get FORM_dhcp_lease ${FORM_iface} leasetime
		fi
		
#		FORM_dhcp_lease=${FORM_dhcp_lease:-12h}
		# cut is to fix for cases where an IP address got stuck in this instead of mere integer
		FORM_dhcp_start=$(echo "$FORM_dhcp_start" | cut -d '.' -f 4)
		
		# convert lease time to minutes
		lease_int=$(echo "$FORM_dhcp_lease" | tr -d [a-z][A-Z])			
		time_units=$(echo "$FORM_dhcp_lease" | tr -d [0-9])
		if [ -n "$lease_int" ]; then
		case "$time_units" in
			"h" | "H" ) let "FORM_dhcp_lease=$lease_int*60";;
			"d" | "D" ) let "FORM_dhcp_lease=$lease_int*24*60";;
			"s" | "S" ) let "FORM_dhcp_lease=$lease_int/60";;
			"w" | "W" ) let "FORM_dhcp_lease=$lease_int*7*24*60";;
			"m" | "M" | "" ) let "FORM_dhcp_lease=$lease_int";;  # minutes 			
#			*) FORM_dhcp_lease="$lease_int"; echo "<br />WARNING: Unknown suffix found on dhcp lease time: $FORM_dhcp_lease";;
		esac					
		fi
			
fi

# For page 
#string|FORM_${FORM_iface}_dhcp_iface|DHCP iface||$FORM_dhcp_iface
if [ -n "$FORM_submit" ]; then
	FAILED=0
	validate <<EOF
int|FORM_${FORM_iface}_dhcp_enabled|DHCP enabled||$FORM_dhcp_enabled
int|FORM_${FORM_iface}_dhcp_start|DHCP start|required min=1 max=254|$FORM_dhcp_start
EOF
	if [ "$?" -ne 0 ]; then
	  FAIL_MESSAGE="Please enter valid number in \"DHCP start\" field"
	  FAILED=1
	  max_dhcp_num=254
	else
	  max_dhcp_num=`expr 255 - $FORM_dhcp_start`
	fi
	validate <<EOF
int|FORM_${FORM_iface}_dhcp_num|DHCP num|required min=1 max=$max_dhcp_num|$FORM_dhcp_num
EOF
	if [ "$?" -ne 0 ]; then
	  FAIL_MESSAGE="$FAIL_MESSAGE <br>Please enter valid number in \"DHCP NUM\" field"
	  FAILED=1
	fi
	validate <<EOF
int|FORM_${FORM_iface}_dhcp_lease|DHCP lease time|min=1 max=2147483647 required|$FORM_dhcp_lease
EOF
	if [ "$?" -ne 0 ]; then
	  FAIL_MESSAGE="$FAIL_MESSAGE <br>Please enter valid number in \"lease time\" field"
	  FAILED=1
	fi
	if equal "$FAILED" 0; then
		SAVED=1
		uci_add "dhcp" "dhcp" "${FORM_iface}"
		uci_set "dhcp" "${FORM_iface}" "enabled" "$FORM_dhcp_enabled"
		uci_set "dhcp" "${FORM_iface}" "interface" "$FORM_iface"
		uci_set "dhcp" "${FORM_iface}" "start" "$FORM_dhcp_start"
		uci_set "dhcp" "${FORM_iface}" "limit" "$FORM_dhcp_num"
		uci_set "dhcp" "${FORM_iface}" "leasetime" "${FORM_dhcp_lease}m"

		config_set "${FORM_iface}" "enabled" "$FORM_dhcp_enabled"
	else
#		echo "<div class=\"failed-validation\">Validation failed on one or more variables. On this page a common error is putting an IP address in \"DHCP Start\" instead of a simple number.</div>"
		echo "<div class=\"failed-validation\">$FAIL_MESSAGE</div>"
	fi
fi

awk -v "name=@TR<<Name>>" \
	-v "interface=@TR<<Interface>>" \
	-v "interfaces=@TR<<Interfaces>>" \
	-v "status=@TR<<Status>>" \
	-v "action=@TR<<Action>>" \
	-f /usr/lib/webif/common.awk -f - /etc/dnsmasq.options <<EOF
BEGIN{
	start_form("@TR<<Interfaces>>")
	print "<table style=\\"width: 90%\\">"
	print "<tr class=\"odd\"><th>" name "</th><th>" interface "</th><th>" interfaces "</th><th>" status "</th><th>" action "</th></tr>"
}
EOF



wifi_dhcp_cfg=1


equal "$wifi_dhcp_cfg" 1 && append network_devices "wifi"
	for ifname in $network_devices; do
		config_get type $ifname type
		if [ "$type" = "bridge" ]; then
			IFACE="br-$ifname"
			IFACES=$(uci get network.$ifname.ifname)
		else 
			IFACE="$ifname"
			config_get IFACES $ifname ifname
		fi
		echo "IFACES=$IFACES ifname=$ifname" >/test1.log
		if [ "$ifname" = "$FORM_iface" ]; then
			style="class=\"settings-title\""
		else
			style=""
		fi
		config_get name $ifname name
		if [ "$name" ]; then
			NAME="br-$name"
		else
			NAME="$ifname"
		fi
		config_get status $ifname enabled
		equal "$status" 1 && STATUS="Enabled" || STATUS="Disabled"
		config_get proto $ifname proto
		if [ "$ifname" = "wifi" ]; then
		#NAME="wifi"
		#IFACE="wifi"
		#IFACES=`uci get wireless.general.type`
		  proto="static"
		  for vap_id in $vap_list; do
		    config_get status $vap_id enabled
		    equal "$status" 1 && STATUS="Enabled" || STATUS="Disabled"
                    disable=`uci get ${vap_id}.${vap_id}.disabled`
                    if [ "$disable" -eq 0 ]; then
                      IFACES=`uci get ${vap_id}.${vap_id}.interface`
		      IFACE=$vap_id
                    else
                      IFACES=""
                      IFACE=""
                    fi
		    echo "<tr class=\"tr_bg\"><td $style>$vap_id</td><td $style>$IFACE</td><td $style>$IFACES</td><td>$STATUS</td><td $style align=\"center\"><a href=\"network-dhcpwifi.sh?action=modify&amp;iface=$vap_id\">@TR<<<img title="edit" src="/images/edit.gif" alt="edit">>></a></td></tr>"
                  done
                fi
	done

awk -f /usr/lib/webif/common.awk -f - /etc/dnsmasq.options <<EOF
BEGIN{
	print "</table><br />"
	end_form();
}
EOF

if [ -n "$FORM_iface" ]; then
	config_get ipaddr $FORM_iface ipaddr
	config_get netmask $FORM_iface netmask
	if [ "$FORM_iface" = "vap0" -o "$FORM_iface" = "vap1" -o "$FORM_iface" = "vap2" -o "$FORM_iface" = "vap3" -o "$FORM_iface" = "vap4" -o "$FORM_iface" = "vap5" -o "$FORM_iface" = "vap6" -o "$FORM_iface" = "vap7"  ]; then
	  ipaddr=`uci get ${FORM_iface}.${FORM_iface}.ipaddr`
	  netmask=`uci get ${FORM_iface}.${FORM_iface}.netmask`
	fi
	if [ -n "$ipaddr" ]; then
		config_get start $FORM_iface start
		config_get num $FORM_iface limit
		
		eval $(ipcalc.sh $ipaddr $netmask ${start:-100} ${num:-150})


#NET=`echo $NETWORK | cut -d "." -f 1-3`
NET=`echo $ipaddr | cut -d "." -f 1-3`
display_form<<EOF
start_form|@TR<<DHCP Server For $FORM_iface>>
field|@TR<<Interface>>|iface_field|hidden
text|iface|$FORM_iface
field|@TR<<DHCP Service>>|dhcp_enabled_field
select|dhcp_enabled|$FORM_dhcp_enabled
option|0|@TR<<Disabled>>
option|1|@TR<<Enabled>>
field|@TR<<DHCP Start>>|dhcp_start_field
string|$NET.
text|dhcp_start|$FORM_dhcp_start
field|@TR<<DHCP Num>>|dhcp_num_field
text|dhcp_num|$FORM_dhcp_num
field|@TR<<DHCP Lease Minutes>>
text|dhcp_lease|$FORM_dhcp_lease
end_form
submit|save|@TR<<Save>>
EOF
fi
fi

footer ?>
<!--
##WEBIF:name:xNetwork:425:DHCP
-->
