#!/bin/bash
pid=`ps -ef | grep "xr_db.exe" | awk '{print $2}'`
kill -TERM $pid
pkill -9 xr_db.exe
