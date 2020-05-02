#!/bin/sh

errstr="Success"
#rm /var/lock/conntrack.lock 2>/dev/null
#`conntrackd -d`
#`conntrackd -F`
#`killall conntrackd`
conntrack -D -f ipv6
status=$?
if [ "$status" -ne 0 ];then
       errstr="Clear Connection Failed"
fi

if [ "$status" -ne 0 ];then
	echo "error-message \"$errstr\" "
	exit 1
else
	exit 0
fi
