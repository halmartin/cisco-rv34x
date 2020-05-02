#!/bin/sh

. /etc/boardInfo

action="$1"
mode="$2"
start_month="$3"
start_day="$4"
start_hour="$5"
start_min="$6"
end_month="$7"
end_day="$8"
end_hour="$9"
end_min="$10"
offset="$11"
start_week="$12"
end_week="$13"
leap_year=0
system_year=`date +%Y`

if [ "$mode" = "Julian" ];then
	tmp="$(/usr/bin/get_timezoneday $mode $system_year $start_month $start_day $start_hour $start_min $end_month $end_day $end_hour $end_min $offset)"
elif [ "$mode" = "Recurring" ];then
	tmp="$(/usr/bin/get_timezoneday $mode $system_year $start_month $start_week $start_day $start_hour $start_min $end_month $end_week $end_day $end_hour $end_min $offset)"
fi


check_leap_year () {
	y="$1"
	a=`expr $y % 4`
	b=`expr $y % 100`
	c=`expr $y % 400`
	if [ $a -eq 0 -a $b -ne 0 -o $c -eq 0 ];then
		return 1
	else
		return 0
	fi
}

# Leap year checks
leap_year=`echo $tmp | cut -d , -f 5`
if [ "$end_month" = "02" ] && [ "$end_day" = "29" ] && [ "$start_month" = "02" ] && [ "$start_day" = "29" ];then
	[ "$leap_year" = 0 ] && {
		return 0
	}
else
	if [ "$action" = "start" ];then
		if  [ "$end_month" = "02" ] && [ "$end_day" = "29" ];then
			temp=$((system_year+1))
	        	check_leap_year $temp
        		is_next_year_leap=$?
			[ "$is_next_year_leap" = 0 ] && {
                     		return 0
			}
		fi
	else
		if [ "$start_month" = "02" ] && [ "$start_day" = "29" ];then
			temp=$((system_year-1))
	        	check_leap_year $temp
        		is_priv_year_leap=$?
			[ "$is_priv_year_leap" = 0 ] && {
                     		return 0
			}
		fi
	fi
fi

if [ "$action" = "start" ];then
	set_time=`echo $tmp | cut -d , -f 2`
else
	set_time=`echo $tmp | cut -d , -f 4`
fi


# Check to allow same cron job next year
if [ "$action" = "start" ];then
	uci_year=`uci get crontabs.DST_START.year`
	if [ "$system_year" -ne "$uci_year" ];then
		`uci set crontabs.DST_START.trigger=1`
		`uci set crontabs.DST_END.trigger=1`
		`uci set crontabs.DST_START.year=$system_year`
		`uci commit crontabs`
		 cp -f $CronTmpConfigFile $CronConfigFile
	fi
	trigger=`uci get crontabs.DST_START.trigger`
else
	trigger=`uci get crontabs.DST_END.trigger`
fi

if [ "$trigger" = 1 ];then
	`date -s $set_time` >/dev/null 2>&1
	`date -k` >/dev/null 2>&1
	`hwclock -w` >/dev/null 2>&1
	if [ "$action" = "start" ];then
		`uci set crontabs.DST_START.trigger=0`
	else
		`uci set crontabs.DST_END.trigger=0`
	fi
	`uci commit crontabs`
	cp -f $CronTmpConfigFile $CronConfigFile
fi
