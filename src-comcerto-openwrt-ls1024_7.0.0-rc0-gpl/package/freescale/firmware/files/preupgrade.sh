#!/bin/sh

PREUPGRADESCRIPTS_DIR="preupgrade/scripts"
PREUPGRADEDATA_DIR="preupgrade/data"

CURR_DIR=$(pwd)
PREUPGRADEDATA_DIR_PATH=$CURR_DIR/$PREUPGRADEDATA_DIR
export PREUPGRADEDATA_DIR_PATH=$PREUPGRADEDATA_DIR_PATH

# Notes:
# 1 - Put all scripts in preupgrade/scripts directory, which need to be execute before upgrade.
# 2 - Using NUMBER-SCRIPTNAME is Recommended in order to achive sequencial execution. Ex: 1-poeUpgrade.sh , 2-abc.sh 
# 3 - If a script in preupgrade/scripts directory needs any data (binary, config etc), keep then in preupgrade/data directory.
# 4 - path to preupgrade/data can be accessed using PREUPGRADEDATA_DIR_PATH environment variable.

for script in $(ls $PREUPGRADESCRIPTS_DIR/* 2>&-); do (
        [ -f $script ] && . $script
); done

