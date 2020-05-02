#!/bin/sh

index=0
PROC_DIR=/proc
proc_min_limit=0

for entry in "$PROC_DIR"/*
do
  if [ -d "$entry" ];
  then
    if [ -f "$entry/status" ]
    then
      index=`expr $index + 1`
      proc_name=`grep "^Name:" $entry/status | cut -d":" -f2,3 | sed -e 's/^[ \t]*//'`
      proc_count=`grep "$proc_name" /proc/*/status | wc -l` >/dev/null 2>&1
      proc_max_limit=`grep processes "$entry/limits" | awk -F' ' '{ print $3 }'`
      echo "$index;$proc_name;$proc_count;$proc_min_limit;$proc_max_limit"  >> /tmp/procstats
    fi
  fi
done
