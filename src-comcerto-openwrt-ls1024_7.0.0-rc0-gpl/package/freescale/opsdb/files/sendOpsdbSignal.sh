#!/bin/sh

signal="$1"
process="operdb_stats"

opsdb_pid=`pidof $process`

[ -n "$opsdb_pid" ] && {
		`kill -s $signal $opsdb_pid`
	}
