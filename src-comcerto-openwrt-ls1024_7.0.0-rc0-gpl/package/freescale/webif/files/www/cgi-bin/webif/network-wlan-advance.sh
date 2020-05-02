#!/usr/bin/webif-page
<?
. /usr/lib/webif/webif.sh

uci_load wireless
uci_load vap0
uci_load vap1
uci_load vap2
uci_load vap3
uci_load vap4
uci_load vap5
uci_load vap6
uci_load vap7

########################################################
# Initializing variables                               #
########################################################
TOTAL_VAP_CONFIGS=7
num_wifi_found=0
radio0_found=0
radio1_found=0
radio2_found=0
radio_gui_config_field=""

########################################################
# Find number of cards                                 #
########################################################

config_get radio0_found   general radio0_found
config_get radio1_found   general radio1_found
config_get radio2_found   general radio2_found

if [ "$radio0_found" -eq 1 ]; then
  num_wifi_found=`expr $num_wifi_found + 1`
fi
if [ "$radio1_found" -eq 1 ]; then
  num_wifi_found=`expr $num_wifi_found + 1`
fi
if [ "$radio2_found" -eq 1 ]; then
  num_wifi_found=`expr $num_wifi_found + 1`
fi

########################################################

! empty "$FORM_change_vap_status" && {
   vap_index=0
   while [ $vap_index -le $TOTAL_VAP_CONFIGS ]
   do
     eval vap_checkbox_val="\$FORM_vap_status_$vap_index"
     config_get old_disable_status vap$vap_index disabled
 #   oldstatus=`uci get ipsec."ipsec$i".StatusEnable` 
     if [ -n "$vap_checkbox_val" ] ; then
       if [ "$old_disable_status" = "1" ] ; then
         uci_set vap$vap_index "vap$vap_index" disabled "0"
         uci_set vap$vap_index "vap$vap_index" status_changed "1"
#         dhcp_server_enable=`uci get dhcp.vap$vap_index.enabled`
#         equal "$dhcp_server_enable" 1 && uci_add "dhcp" "dhcp" "vap$vap_index"
       fi
     else
       if [ "$old_disable_status" = "0" ] ; then
         uci_set vap$vap_index "vap$vap_index" disabled "1"
         uci_set vap$vap_index "vap$vap_index" status_changed "2"
#         dhcp_server_enable=`uci get dhcp.vap$vap_index.enabled`
#         equal "$dhcp_server_enable" 1 && uci_add "dhcp" "dhcp" "vap$vap_index"
       fi
     fi
     vap_index=`expr $vap_index + 1`
   done
}

wep_generate_key="n"
! empty "$FORM_open_generate_wep40_keys" && {
validate <<EOF
string|FORM_open_passphrase_wep40|@TR<<WEP 40 Passphrase>>|required min=1|$FORM_open_passphrase_wep40
EOF
  equal "$?" 0 && {
    FORM_open_wep40_key="1"
    FORM_open_wep40_key1_val=""
    FORM_open_wep40_key2_val=""
    FORM_open_wep40_key3_val=""
    FORM_open_wep40_key4_val=""

    # generate a four 40 bit keys
    textkeys=$(wepkeygen "$FORM_open_passphrase_wep40" | sed s/':'//g)
    keycount=1
    for curkey in $textkeys; do
    case $keycount in
      1) FORM_open_wep40_key1_val=$curkey;;
      2) FORM_open_wep40_key2_val=$curkey;;
      3) FORM_open_wep40_key3_val=$curkey;;
      4) FORM_open_wep40_key4_val=$curkey
         break;;
    esac
    let "keycount+=1"
    done
  }
  FORM_authentication="open"
  FORM_encryption_open="wep40"
  FORM_display_vap=$FORM_vap_id
  wep_generate_key="y"
}

! empty "$FORM_open_generate_wep104_key" && {
validate <<EOF
string|FORM_open_passphrase_wep104|@TR<<WEP 104 Passphrase>>|required min=1|$FORM_open_passphrase_wep104
EOF
  equal "$?" 0 && {
    FORM_open_wep104_key_val=""

    # generate a one 104 bit key
    textkeys=$(wepkeygen -s "$FORM_open_passphrase_wep104" | 
               awk 'BEGIN { count=0 };
                        { total[count]=$1, count+=1; }
                END { print total[0] ":" total[1] ":" total[2] ":" total[3]}')
    FORM_open_wep104_key_val=$(echo "$textkeys" | cut -d ':' -f 0-13 | sed s/':'//g)
  }
  FORM_authentication="open"
  FORM_encryption_open="wep104"
  FORM_display_vap=$FORM_vap_id
  wep_generate_key="y"
}

! empty "$FORM_shared_generate_wep40_keys" && {
validate <<EOF
string|FORM_shared_passphrase_wep40|@TR<<WEP 40 Passphrase>>|required min=1|$FORM_shared_passphrase_wep40
EOF
  equal "$?" 0 && {
    FORM_shared_wep40_key="1"
    FORM_shared_wep40_key1_val=""
    FORM_shared_wep40_key2_val=""
    FORM_shared_wep40_key3_val=""
    FORM_shared_wep40_key4_val=""

    # generate a four 40 bit keys
    textkeys=$(wepkeygen "$FORM_shared_passphrase_wep40" | sed s/':'//g)
    keycount=1
    for curkey in $textkeys; do
    case $keycount in
      1) FORM_shared_wep40_key1_val=$curkey;;
      2) FORM_shared_wep40_key2_val=$curkey;;
      3) FORM_shared_wep40_key3_val=$curkey;;
      4) FORM_shared_wep40_key4_val=$curkey
         break;;
    esac
    let "keycount+=1"
    done
  }
  FORM_authentication="shared"
  FORM_encryption_shared="wep40"
  FORM_display_vap=$FORM_vap_id
  wep_generate_key="y"
}

! empty "$FORM_shared_generate_wep104_key" && {
validate <<EOF
string|FORM_shared_passphrase_wep104|@TR<<WEP 104 Passphrase>>|required min=1|$FORM_shared_passphrase_wep104
EOF
  equal "$?" 0 && {
    FORM_shared_wep104_key_val=""

  # generate a one 104 bit key
    textkeys=$(wepkeygen -s "$FORM_shared_passphrase_wep104" | 
               awk 'BEGIN { count=0 };
                        { total[count]=$1, count+=1; }
                END { print total[0] ":" total[1] ":" total[2] ":" total[3]}')
    FORM_shared_wep104_key_val=$(echo "$textkeys" | cut -d ':' -f 0-13 | sed s/':'//g)
  }
  FORM_authentication="shared"
  FORM_encryption_shared="wep104"
  FORM_display_vap=$FORM_vap_id
  wep_generate_key="y"
}

#bridged=`uci get bridge.general.wifi`
#if [ "$bridged" -eq 1 ]; then
#  brcount=`uci get bridge.general.count`  
#  brindex=1
#  while [ $brindex -le "$brcount" ]
#  do
#    brname=`uci get bridge.general.brname$brindex`
#    brcfg=`uci get network.$brname.wifi`
#    if [ "$brcfg" -eq 1 ];then
#      brstatus=`uci get network.$brname.status`
#      [ "$brstatus" = "0" ] && bridged="0"
#    fi
#    brindex=`expr $brindex + 1`
#  done
#fi
bridged=0
brnames=$(ubus list network.interface.* | cut -d'.' -f 3 | sed "s/loopback//g" | sed "s/lan//g" | sed "s/wan//g" | sed "/^$/d")
for bridge_name in $brnames; do
	brcfg=`uci get network.${bridge_name}.wifi` 
	[ "$brcfg" = "1" ] && bridged=1
done

[ -n "$FORM_save_vap" -a "$wep_generate_key" = "n" ] && {

save_vap_apply="n"
open_wep40_key1=""
open_wep40_key2=""
open_wep40_key3=""
open_wep40_key4=""
if [ "$FORM_authentication" = "open" -a "$FORM_encryption_open" = "wep40" ]; then
open_wep40_key1="wep|FORM_open_wep40_key1_val|@TR<<WEP 40 Key1>>|required|$FORM_open_wep40_key1_val"
open_wep40_key2="wep|FORM_open_wep40_key2_val|@TR<<WEP 40 Key2>>|required|$FORM_open_wep40_key2_val"
open_wep40_key3="wep|FORM_open_wep40_key3_val|@TR<<WEP 40 Key3>>|required|$FORM_open_wep40_key3_val"
open_wep40_key4="wep|FORM_open_wep40_key4_val|@TR<<WEP 40 Key4>>|required|$FORM_open_wep40_key4_val"
fi

open_wep104_key=""
if [ "$FORM_authentication" = "open" -a "$FORM_encryption_open" = "wep104" ]; then
open_wep104_key="wep|FORM_open_wep104_key_val|@TR<<WEP 104 Key>>|required|$FORM_open_wep104_key_val"
fi

shared_wep40_key1=""
shared_wep40_key2=""
shared_wep40_key3=""
shared_wep40_key4=""
if [ "$FORM_authentication" = "shared" -a "$FORM_encryption_shared" = "wep40" ]; then
shared_wep40_key1="wep|FORM_shared_wep40_key1_val|@TR<<WEP 40 Key1>>|required|$FORM_shared_wep40_key1_val"
shared_wep40_key2="wep|FORM_shared_wep40_key2_val|@TR<<WEP 40 Key2>>|required|$FORM_shared_wep40_key2_val"
shared_wep40_key3="wep|FORM_shared_wep40_key3_val|@TR<<WEP 40 Key3>>|required|$FORM_shared_wep40_key3_val"
shared_wep40_key4="wep|FORM_shared_wep40_key4_val|@TR<<WEP 40 Key4>>|required|$FORM_shared_wep40_key4_val"
fi

shared_wep104_key=""
if [ "$FORM_authentication" = "shared" -a "$FORM_encryption_shared" = "wep104" ]; then
shared_wep104_key="wep|FORM_shared_wep104_key_val|@TR<<WEP 104 Key>>|required|$FORM_shared_wep104_key_val"
fi

if [ "$bridged" != "1" ] ; then
  req="required"
fi

validate <<EOF
name|FORM_ssid|@TR<<ESSID>>|required|$FORM_ssid
$open_wep40_key1
$open_wep40_key2
$open_wep40_key3
$open_wep40_key4
$open_wep104_key
$shared_wep40_key1
$shared_wep40_key2
$shared_wep40_key3
$shared_wep40_key4
$shared_wep104_key
ip|FORM_ip_address|@TR<<IP Address>>|$req|$FORM_ip_address
netmask|FORM_netmask|@TR<<Netmask>>|$req|$FORM_netmask
EOF
  equal "$?" 0 && {
    vap_cfg_id="$FORM_vap_id"
    uci_set $vap_cfg_id "$vap_cfg_id" ssid "$FORM_ssid"
    config_get prev_radio_type  $vap_cfg_id radio_type
    uci_set $vap_cfg_id "$vap_cfg_id" radio_type "$FORM_radio_type"
    [ "$prev_radio_type" != "$FORM_radio_type" ] && uci_add "dhcp" "dhcp" "$vap_cfg_id"
    uci_set $vap_cfg_id "$vap_cfg_id" hidden_ssid "$FORM_hidden_ssid"
    uci_set $vap_cfg_id "$vap_cfg_id" authentication "$FORM_authentication"
    if [ "$FORM_authentication" = "open" ] ; then
      uci_set $vap_cfg_id "$vap_cfg_id" encryption "$FORM_encryption_open"
      if [ "$FORM_encryption_open" = "wep40" ] ; then
        uci_set $vap_cfg_id "$vap_cfg_id" key1 "$FORM_open_wep40_key1_val"
        uci_set $vap_cfg_id "$vap_cfg_id" key2 "$FORM_open_wep40_key2_val"
        uci_set $vap_cfg_id "$vap_cfg_id" key3 "$FORM_open_wep40_key3_val"
        uci_set $vap_cfg_id "$vap_cfg_id" key4 "$FORM_open_wep40_key4_val"
#        eval key_val="\$FORM_open_wep40_key${FORM_open_wep40_key}_val"
        uci_set $vap_cfg_id "$vap_cfg_id" key "$FORM_open_wep40_key"
      elif [ "$FORM_encryption_open" = "wep104" ] ; then
        uci_set $vap_cfg_id "$vap_cfg_id" key "1"
        uci_set $vap_cfg_id "$vap_cfg_id" key1 "$FORM_open_wep104_key_val"
      fi
    elif [ "$FORM_authentication" = "shared" ] ; then
      uci_set $vap_cfg_id "$vap_cfg_id" encryption "$FORM_encryption_shared"
      if [ "$FORM_encryption_shared" = "wep40" ] ; then
        uci_set $vap_cfg_id "$vap_cfg_id" key1 "$FORM_shared_wep40_key1_val"
        uci_set $vap_cfg_id "$vap_cfg_id" key2 "$FORM_shared_wep40_key2_val"
        uci_set $vap_cfg_id "$vap_cfg_id" key3 "$FORM_shared_wep40_key3_val"
        uci_set $vap_cfg_id "$vap_cfg_id" key4 "$FORM_shared_wep40_key4_val"
#        eval key_val="\$FORM_shared_wep40_key${FORM_shared_wep40_key}_val"
        uci_set $vap_cfg_id "$vap_cfg_id" key "$FORM_shared_wep40_key"
      elif [ "$FORM_encryption_shared" = "wep104" ] ; then
        uci_set $vap_cfg_id "$vap_cfg_id" key "1"
        uci_set $vap_cfg_id "$vap_cfg_id" key1 "$FORM_shared_wep104_key_val"
      fi
    elif [ "$FORM_authentication" = "wpapsk" ] ; then
      uci_set $vap_cfg_id "$vap_cfg_id" encryption "$FORM_encryption_wpapsk"
      if [ "$FORM_encryption_wpapsk" = "tkip" ] ; then
        uci_set $vap_cfg_id "$vap_cfg_id" key "$FORM_wpa_psk_passphrase_tkip"
      elif [ "$FORM_encryption_wpapsk" = "aes" ] ; then
        uci_set $vap_cfg_id "$vap_cfg_id" key "$FORM_wpa_psk_passphrase_aes"
      fi
    fi
    if [ "$bridged" != "1" ]; then
      config_get prev_ipaddr  $vap_cfg_id ipaddr
      config_get prev_netmask  $vap_cfg_id netmask
      uci_set $vap_cfg_id "$vap_cfg_id" ipaddr "$FORM_ip_address"
      uci_set $vap_cfg_id "$vap_cfg_id" netmask "$FORM_netmask"
      [ "$prev_ipaddr" != "$FORM_ip_address" -o "$prev_netmask" != "$FORM_netmask" ] && { 
#        uci_add "dhcp" "dhcp" "$vap_cfg_id"
        uci_set "$vap_cfg_id" "$vap_cfg_id" status_changed "1"
      }
    fi
    save_vap_apply="y"
  }
  if [ "$save_vap_apply" = "n" ]; then
    FORM_display_vap=$FORM_vap_id
  fi
}

uci_load wireless
uci_load vap0
uci_load vap1
uci_load vap2
uci_load vap3
uci_load vap4
uci_load vap5
uci_load vap6
uci_load vap7
#####################################################################
header "Network" "Wireless" "@TR<<Wireless Advance Configuration>>" 'onload="init();document.forms[0].onsubmit=function(){return checkvalues()}"' "$SCRIPT_NAME"
#####################################################################

cat <<EOF
<script type="text/javascript" src="/webif.js "></script>
<script type="text/javascript">
<!--
function visible_wpa_psk_encryption_mode( status )
{
  if ( status == true )
  {
    if ( document.forms[0].encryption_wpapsk.value == "tkip" ) 
    {
      set_visible('wpa_psk_passphrase_tkip', status);
      set_visible('wpa_psk_passphrase_aes', false);
    }
    else if ( document.forms[0].encryption_wpapsk.value == "aes" ) 
    {
      set_visible('wpa_psk_passphrase_tkip', false);
      set_visible('wpa_psk_passphrase_aes', status);
    }
    else  
    {
      set_visible('wpa_psk_passphrase_tkip', false);
      set_visible('wpa_psk_passphrase_aes', false);
    }
  }
  else  
  {
    set_visible('wpa_psk_passphrase_tkip', false);
    set_visible('wpa_psk_passphrase_aes', false);
  }
}
function visible_encryption_shared_wep104_mode( status )
{
  set_visible('shared_passphrase_wep104', status);
  set_visible('shared_generate_wep104_key', status);
  set_visible('shared_wep104_key', status);
}

function visible_encryption_shared_wep40_mode( status )
{
  set_visible('shared_passphrase_wep40', status);
  set_visible('shared_generate_wep40_keys', status);
  set_visible('shared_wep40_key_1', status);
  set_visible('shared_wep40_key_2', status);
  set_visible('shared_wep40_key_3', status);
  set_visible('shared_wep40_key_4', status);
}

function visible_shared_encryption_mode( status )
{
  if ( status == true )
  {
    if ( document.forms[0].encryption_shared.value == "wep40" ) 
    {
      visible_encryption_shared_wep40_mode(true);
      visible_encryption_shared_wep104_mode(false);
    }
    else if ( document.forms[0].encryption_shared.value == "wep104" ) 
    {
      visible_encryption_shared_wep40_mode(false);
      visible_encryption_shared_wep104_mode(true);
    }
    else  
    {
      visible_encryption_shared_wep40_mode(false);
      visible_encryption_shared_wep104_mode(false);
    }
  }
  else  
  {
    visible_encryption_shared_wep40_mode(false);
    visible_encryption_shared_wep104_mode(false);
  }
}

function visible_encryption_open_wep104_mode( status )
{
  set_visible('open_passphrase_wep104', status);
  set_visible('open_generate_wep104_key', status);
  set_visible('open_wep104_key', status);
}

function visible_encryption_open_wep40_mode( status )
{
  set_visible('open_passphrase_wep40', status);
  set_visible('open_generate_wep40_keys', status);
  set_visible('open_wep40_key_1', status);
  set_visible('open_wep40_key_2', status);
  set_visible('open_wep40_key_3', status);
  set_visible('open_wep40_key_4', status);
}

function visible_open_encryption_mode( status )
{
  if ( status == true )
  {
    if ( document.forms[0].encryption_open.value == "none" ) 
    {
      visible_encryption_open_wep40_mode(false);
      visible_encryption_open_wep104_mode(false);
    }
    else if ( document.forms[0].encryption_open.value == "wep40" ) 
    {
      visible_encryption_open_wep40_mode(true);
      visible_encryption_open_wep104_mode(false);
    }
    else if ( document.forms[0].encryption_open.value == "wep104" ) 
    {
      visible_encryption_open_wep40_mode(false);
      visible_encryption_open_wep104_mode(true);
    }
    else  
    {
      visible_encryption_open_wep40_mode(false);
      visible_encryption_open_wep104_mode(false);
    }
  }
  else  
  {
    visible_encryption_open_wep40_mode(false);
    visible_encryption_open_wep104_mode(false);
  }
}

function vapmodechange()
{
   /* Open Authentication mode */
 /* alert(document.forms[0].hidden_ssid.value);
  alert(document.forms[0].authentication.value);*/
  if ( document.forms[0].authentication.value == "open" ) 
  {
    set_visible('encryption_open', true);
    set_visible('encryption_shared', false);
    set_visible('encryption_wpapsk', false);

    visible_open_encryption_mode(true);
    visible_shared_encryption_mode(false);
    visible_wpa_psk_encryption_mode(false);
  }
  else if ( document.forms[0].authentication.value == "shared" ) /* Shared Authentication mode */
  {
    set_visible('encryption_shared', true);
    set_visible('encryption_open', false);
    set_visible('encryption_wpapsk', false);

    visible_open_encryption_mode(false);
    visible_shared_encryption_mode(true);
    visible_wpa_psk_encryption_mode(false);
  }
  else if ( document.forms[0].authentication.value == "wpapsk" ) /* WPA-PSK Authentication mode */
  {
    set_visible('encryption_wpapsk', true);
    set_visible('encryption_open', false);
    set_visible('encryption_shared', false);

    visible_open_encryption_mode(false);
    visible_shared_encryption_mode(false);
    visible_wpa_psk_encryption_mode(true);
  }
  else
  {
    visible_open_encryption_mode(false);
    visible_shared_encryption_mode(false);
    visible_wpa_psk_encryption_mode(false);
  }
}

function init()
{
  if ( '$num_wifi_found' == 0 )
  {
    alert('No Wi-Fi card present.');
    return true ;
  }
  if ( '$FORM_display_vap' == '' )
  {
    return true ;
  }

 /* if ( ( document.forms[0].radio_type[0].checked != true ) &&
       ( document.forms[0].radio_type[1].checked != true ) &&
       ( document.forms[0].radio_type[2].checked != true ) )
  {
    if ( '$radio0_found' == 1 )
    {
      document.forms[0].radio_type[0].checked = true ;
    }
    else if ( '$radio1_found' == 1 )
    {
      document.forms[0].radio_type[1].checked = true ;
    }
    else if ( '$radio2_found' == 1 )
    {
      document.forms[0].radio_type[2].checked = true ;
    }
  }*/

/*  if ( '$num_wifi_found' == 1 )
  {
    set_visible('radio_type', false);
  } */

  if('$bridged' == '1')
  {
    document.forms[0].ip_address.disabled = true ;
    document.forms[0].netmask.disabled = true ;
   // document.getElementById('ip_address').disabled = true;
   // document.getElementById('netmask').disabled = true;
  }

  vapmodechange();
  return true ;
}
-->
</script>
EOF

empty "$FORM_display_vap" && {

  echo "<div class=\"settings\">"
  echo "<th colspan=\"11\"><h3><strong>" Virtual Access Points: "</strong></h3></th>"
  echo "<div class=\"settings-content-inner\">"
  echo "<table style=\"width: 96%; text-align: left; font-size: 0.8em;\" border=\"0\" cellpadding=\"3\" cellspacing=\"3\" align=\"center\"><form name=\"change_vap_status\" action=\"/cgi-bin/webif/network-wlan-advance.sh\" enctype=\"multipart/form-data\" method=\"post\"> <tbody>"
  echo "<tr class=\"odd\"><th>VAP ID</th><th>Radio ID</th><th>ESSID</th><th>Network</th><th>Authentication</th><th>Encryption</th><th style=\"text-align: center;\">Status</th><th style=\"text-align: center;\">Actions</th></tr>"
  vap_index=0
  while [ $vap_index -le "$TOTAL_VAP_CONFIGS" ]
  do
    vap_id="vap$vap_index"
    config_get radio_index $vap_id radio_type
    if [ "$radio_index" = "radio0" ]; then
      radio_id="Radio0 (PCIe0)"
    elif [ "$radio_index" = "radio1" ]; then
      radio_id="Radio1 (PCIe1)"
    elif [ "$radio_index" = "radio2" ]; then
      radio_id="Radio2 (USB)"
    else
      radio_id=""
    fi
    config_get ssid $vap_id ssid
    config_get ipaddr $vap_id ipaddr
    config_get netmask $vap_id netmask
    config_get authentication $vap_id authentication
    config_get encryption $vap_id encryption
    config_get disable_status $vap_id disabled

    echo "<td class=\"tr_bg\">$vap_id</td>"
    echo "<td class=\"tr_bg\">$radio_id</td>"
    echo "<td class=\"tr_bg\">$ssid</td>"
    if [ "$bridged" -eq 1 ]; then
      echo "<td class=\"tr_bg\">Bridged</td>"
    else
      echo "<td class=\"tr_bg\">${ipaddr}-${netmask}</td>"
    fi
    echo "<td class=\"tr_bg\">$authentication</td>"
    echo "<td class=\"tr_bg\">$encryption</td>"

    if [ "$disable_status" = "0" ] ; then
      status="yes"
    else
      status=""
    fi
    echo "<td class=\"tr_bg\" style=\"text-align: center;\"><input id="vap_status_yes_$vap_index" type="checkbox" name="vap_status_$vap_index" value="$status" checked=""  /></td>"
    echo "<td class=\"tr_bg\" style=\"text-align: center;\"><a href=\"$SCRIPT_NAME?display_vap=$vap_id\"><img alt=\"@TR<<edit>>\" src=\"/images/edit.gif\" title=\"@TR<<edit>>\" /></a></td></tr>"

    vap_index=`expr $vap_index + 1`
  done
  echo "<tr id=\"spacer1\" > <td colspan=\"2\"> <br> <input class=\"button-inner\" name=\"change_vap_status\" value=\"Save\" type=\"submit\"></td></tr>"
  echo "</tbody></form></table></div><div class=\"clearfix\">&nbsp;</div></div>"
}

#! empty "$FORM_display_vap" && {
[ -n "$FORM_display_vap" -a "$wep_generate_key" = "n" ] && {
#  polname="ipsec$FORM_display_vap"
  config_get FORM_radio_type $FORM_display_vap radio_type
  config_get FORM_ssid $FORM_display_vap ssid
  config_get FORM_hidden_ssid $FORM_display_vap hidden_ssid
  config_get FORM_authentication $FORM_display_vap authentication
  if [ "$FORM_authentication" = "open" ] ; then
    config_get FORM_encryption_open $FORM_display_vap encryption
    if [ "$FORM_encryption_open" = "wep40" ] ; then
      config_get FORM_open_wep40_key1_val $FORM_display_vap key1
      config_get FORM_open_wep40_key2_val $FORM_display_vap key2
      config_get FORM_open_wep40_key3_val $FORM_display_vap key3
      config_get FORM_open_wep40_key4_val $FORM_display_vap key4
      config_get FORM_open_wep40_key $FORM_display_vap key
#      config_get key_val $FORM_display_vap key
#      if [ "$key_val" = "$FORM_open_wep40_key1_val" ]; then
#        FORM_open_wep40_key=1
#      elif [ "$key_val" = "$FORM_open_wep40_key2_val" ]; then
#        FORM_open_wep40_key=2
#      elif [ "$key_val" = "$FORM_open_wep40_key3_val" ]; then
#        FORM_open_wep40_key=3
#      elif [ "$key_val" = "$FORM_open_wep40_key4_val" ]; then
#        FORM_open_wep40_key=4
#      fi
    elif [ "$FORM_encryption_open" = "wep104" ] ; then
      config_get FORM_open_wep104_key_val $FORM_display_vap key1
    fi
  elif [ "$FORM_authentication" = "shared" ] ; then
    config_get FORM_encryption_shared $FORM_display_vap encryption
    if [ "$FORM_encryption_shared" = "wep40" ] ; then
      config_get FORM_shared_wep40_key1_val $FORM_display_vap key1
      config_get FORM_shared_wep40_key2_val $FORM_display_vap key2
      config_get FORM_shared_wep40_key3_val $FORM_display_vap key3
      config_get FORM_shared_wep40_key4_val $FORM_display_vap key4
      config_get FORM_shared_wep40_key $FORM_display_vap key
#      config_get key_val $FORM_display_vap key
#      if [ "$key_val" = "$FORM_shared_wep40_key1_val" ]; then
#        FORM_shared_wep40_key=1
#      elif [ "$key_val" = "$FORM_shared_wep40_key2_val" ]; then
#        FORM_shared_wep40_key=2
#      elif [ "$key_val" = "$FORM_shared_wep40_key3_val" ]; then
#        FORM_shared_wep40_key=3
#      elif [ "$key_val" = "$FORM_shared_wep40_key4_val" ]; then
#        FORM_shared_wep40_key=4
#      fi
    elif [ "$FORM_encryption_shared" = "wep104" ] ; then
      config_get FORM_shared_wep104_key_val $FORM_display_vap key1
    fi
  elif [ "$FORM_authentication" = "wpapsk" ] ; then
    config_get FORM_encryption_wpapsk $FORM_display_vap encryption
    if [ "$FORM_encryption_wpapsk" = "tkip" ] ; then
      config_get FORM_wpa_psk_passphrase_tkip $FORM_display_vap key
    elif [ "$FORM_encryption_wpapsk" = "aes" ] ; then
      config_get FORM_wpa_psk_passphrase_aes $FORM_display_vap key
    fi
  fi
  config_get FORM_ip_address $FORM_display_vap ipaddr
  config_get FORM_netmask $FORM_display_vap netmask
}

########################################################
# add radio buttons                                    #
########################################################

radio_gui_config_field="field|@TR<<Radio>>|radio_type"
if [ "$radio0_found" -eq 1 ]; then
  radio_gui_config_field="$radio_gui_config_field
                          radio|radio_type|$FORM_radio_type|radio0|@TR<<Radio0 (PCIe0)>>"
else
  radio_gui_config_field="$radio_gui_config_field
                          radio|radio_type|$FORM_radio_type|radio0|@TR<<Radio0 (PCIe0)>>|disabled"
fi
radio_gui_config_field="$radio_gui_config_field
                        string|<br>"
if [ "$radio1_found" -eq 1 ]; then
  radio_gui_config_field="$radio_gui_config_field
                          radio|radio_type|$FORM_radio_type|radio1|@TR<<Radio1 (PCIe1)>>"
else
  radio_gui_config_field="$radio_gui_config_field
                          radio|radio_type|$FORM_radio_type|radio1|@TR<<Radio1 (PCIe1)>>|disabled"
fi
radio_gui_config_field="$radio_gui_config_field
                        string|<br>"
if [ "$radio2_found" -eq 1 ]; then
  radio_gui_config_field="$radio_gui_config_field
                          radio|radio_type|$FORM_radio_type|radio2|@TR<<Radio2 (USB)>>"
else
  radio_gui_config_field="$radio_gui_config_field
                          radio|radio_type|$FORM_radio_type|radio2|@TR<<Radio2 (USB)>>|disabled"
fi
radio_gui_config_field="$radio_gui_config_field
                        string|<br>"


save_vap_failed="n"
[ -n "$FORM_save_vap" -a "$save_vap_apply" = "n" ] && save_vap_failed="y"
[ -n "$FORM_display_vap" -o "$save_vap_failed" = "y" ] && {
[ "$save_vap_failed" = "y" ] && {
  FORM_display_vap=$FORM_vap_id
}
display_form <<EOF
onchange|vapmodechange
start_form|@TR<<Edit $FORM_display_vap Configuration>>
formtag_begin|save_vap|$SCRIPT_NAME
field|@TR<<VAP ID>>|vap_id|hidden
text|vap_id|$FORM_display_vap|||readonly

$radio_gui_config_field

field|@TR<<ESSID>>|ssid
text|ssid|$FORM_ssid
field|@TR<<ESSID Broadcast>>|hidden_ssid
select|hidden_ssid|$FORM_hidden_ssid
option|1|@TR<<Hide>>
option|0|@TR<<Show>>
field|@TR<<Authentication>>|authentication
select|authentication|$FORM_authentication
option|open|@TR<<Open>>
option|shared|@TR<<Shared>>
option|wpapsk|@TR<<WPA-PSK>>
field|@TR<<Encryption>>|encryption_open|hidden
select|encryption_open|$FORM_encryption_open
option|none|@TR<<None>>
option|wep40|@TR<<WEP 40>>
option|wep104|@TR<<WEP 104>>
field|@TR<<Encryption>>|encryption_shared|hidden
select|encryption_shared|$FORM_encryption_shared
option|wep40|@TR<<WEP 40>>
option|wep104|@TR<<WEP 104>>
field|@TR<<Encryption>>|encryption_wpapsk|hidden
select|encryption_wpapsk|$FORM_encryption_wpapsk
option|tkip|@TR<<TKIP>>
option|aes|@TR<<AES>>

field|@TR<<Passphrase>>|open_passphrase_wep40|hidden
text|open_passphrase_wep40|$FORM_open_passphrase_wep40
field|&nbsp;|open_generate_wep40_keys|hidden
submit|open_generate_wep40_keys|@TR<<Generate 40bit Keys>>
field|@TR<<Passphrase>>|open_passphrase_wep104|hidden
text|open_passphrase_wep104|$FORM_open_passphrase_wep104
field|&nbsp;|open_generate_wep104_key|hidden
submit|open_generate_wep104_key|@TR<<Generate 104bit Key>>
field|@TR<<WEP40 Key 1>>|open_wep40_key_1|hidden
radio|open_wep40_key|$FORM_open_wep40_key|1
text|open_wep40_key1_val|$FORM_open_wep40_key1_val|<br />
field|@TR<<WEP40 Key 2>>|open_wep40_key_2|hidden
radio|open_wep40_key|$FORM_open_wep40_key|2
text|open_wep40_key2_val|$FORM_open_wep40_key2_val|<br />
field|@TR<<WEP40 Key 3>>|open_wep40_key_3|hidden
radio|open_wep40_key|$FORM_open_wep40_key|3
text|open_wep40_key3_val|$FORM_open_wep40_key3_val|<br />
field|@TR<<WEP40 Key 4>>|open_wep40_key_4|hidden
radio|open_wep40_key|$FORM_open_wep40_key|4
text|open_wep40_key4_val|$FORM_open_wep40_key4_val|<br />
field|@TR<<WEP104 Key>>|open_wep104_key|hidden
text|open_wep104_key_val|$FORM_open_wep104_key_val|<br />

field|@TR<<Passphrase>>|shared_passphrase_wep40|hidden
text|shared_passphrase_wep40|$FORM_shared_passphrase_wep40
field|&nbsp;|shared_generate_wep40_keys|hidden
submit|shared_generate_wep40_keys|@TR<<Generate 40bit Keys>>
field|@TR<<Passphrase>>|shared_passphrase_wep104|hidden
text|shared_passphrase_wep104|$FORM_shared_passphrase_wep104
field|&nbsp;|shared_generate_wep104_key|hidden
submit|shared_generate_wep104_key|@TR<<Generate 104bit Key>>
field|@TR<<WEP40 Key 1>>|shared_wep40_key_1|hidden
radio|shared_wep40_key|$FORM_shared_wep40_key|1
text|shared_wep40_key1_val|$FORM_shared_wep40_key1_val|<br />
field|@TR<<WEP40 Key 2>>|shared_wep40_key_2|hidden
radio|shared_wep40_key|$FORM_shared_wep40_key|2
text|shared_wep40_key2_val|$FORM_shared_wep40_key2_val|<br />
field|@TR<<WEP40 Key 3>>|shared_wep40_key_3|hidden
radio|shared_wep40_key|$FORM_shared_wep40_key|3
text|shared_wep40_key3_val|$FORM_shared_wep40_key3_val|<br />
field|@TR<<WEP40 Key 4>>|shared_wep40_key_4|hidden
radio|shared_wep40_key|$FORM_shared_wep40_key|4
text|shared_wep40_key4_val|$FORM_shared_wep40_key4_val|<br />
field|@TR<<WEP104 Key>>|shared_wep104_key|hidden
text|shared_wep104_key_val|$FORM_shared_wep104_key_val|<br />

field|@TR<<Passphrase>>|wpa_psk_passphrase_tkip|hidden
text|wpa_psk_passphrase_tkip|$FORM_wpa_psk_passphrase_tkip
field|@TR<<Passphrase>>|wpa_psk_passphrase_aes|hidden
text|wpa_psk_passphrase_aes|$FORM_wpa_psk_passphrase_aes

field|@TR<<IP Address>>|
text|ip_address|$FORM_ip_address
field|@TR<<Netmask>>|
text|netmask|$FORM_netmask

helpitem|Encryption Type
helptext|HelpText Encryption Type#WEP key should be alpha-numeric and should not end with "0" and it should be either 10 or 26 characters or you can type something in WEP PASS and generate it through GUI.
helptext|HelpText Encryption Type#WPA-PSK key should be alpha-numeric and minimum 8 to maximum 64 characters.


field||spacer1
string|<br />
submit|save_vap|@TR<<Save>>
submit||@TR<<Cancel>>
formtag_end
end_form
field|@TR<<>>
string|<a href="network-wlan-advance.sh" ><b>VAP configuration table</b></a>
EOF
}

footer ?>
<!--
##WEBIF:name:xNetwork:350:Wireless
-->
