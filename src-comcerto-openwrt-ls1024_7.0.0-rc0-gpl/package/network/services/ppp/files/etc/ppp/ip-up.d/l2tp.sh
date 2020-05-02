#!/bin/sh
# Sends SIGUSR2 signal to l2tp daemon

CMD=l2tpd
[ -f /var/run/xl2tpd.pid ] && {
        CMD=xl2tpd
}


case $CMD in
        xl2tpd)
                kill -12 `cat /var/run/xl2tpd.pid`
        ;;
        l2tpd)
                L2TP_PID=`ps -ax | grep l2tp | grep -v grep |grep -v xl2tpd  | cut -d' ' -f 2 `
                kill -12 $L2TP_PID
        ;;
esac
