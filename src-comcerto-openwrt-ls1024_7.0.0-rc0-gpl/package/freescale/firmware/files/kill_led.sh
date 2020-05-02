#!/bin/sh

if [ "$_this_board" = "RV160" -o "$_this_board" = "RV160W" -o "$_this_board" = "RV260" -o "$_this_board" = "RV260W" -o "$_this_board" = "RV260P" ];then
	platform_led_script="rv16x_26x_led.sh"
else
	platform_led_script="rv340_led.sh"
fi

kill_led() {
  for p in `pgrep -f $platform_led_script | xargs`
	do
    LED=`cat /proc/$p/cmdline | grep $1`
		if [ $LED == "$1" ]; then
#      echo -n "Killing $LED process with PID $p..."
      kill -9 $p > /dev/null
      echo "done."
	  else
      echo "NOT killing LED process PID $p"
    fi
  done
}

kill_led $1
