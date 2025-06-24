#!/usr/bin/env bash

#some OS's need this to be set, seems our stock balena doesnt, Raspbian does though
#export LC_ALL=en_GB.UTF-8

for a in *.jar ; do echo $a; export CLASSPATH=$CLASSPATH:$a; done
java com.ktk.Main $BALENA_DEVICE_NAME_AT_INIT #>> /tmp/passportscanner.log 2>&1