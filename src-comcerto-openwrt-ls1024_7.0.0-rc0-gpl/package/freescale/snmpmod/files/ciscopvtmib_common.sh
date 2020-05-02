#!/bin/sh

cmn_frmver=`cat /etc/firmware_version`
cmn_modelid=`uci get systeminfo.sysinfo.pid | cut -d'-' -f1`
cmn_datetime=`date`
cmn_conflastchange=$(cat /tmp/snmp_audit_last_log) >/dev/null 2>&1
cmn_rebootreason=$(cat /mnt/configcert/config/rebootstate)

echo "$cmn_frmver,$cmn_modelid,$cmn_datetime,$cmn_conflastchange,$cmn_rebootreason"
