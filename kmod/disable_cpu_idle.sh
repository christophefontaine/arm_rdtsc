#!/bin/sh
for cpu in 0 1 2 3 ; do
for state in 0 1 ; do
echo 1 > /sys/devices/system/cpu/cpu$cpu/cpuidle/state$state/disable
done
done
