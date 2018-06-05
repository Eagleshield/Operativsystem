#!/bin/bash

SCHEDLIST='cfq deadline noop'

sudo hdparm -W 0 /dev/sda

for sched in $SCHEDLIST; do
	echo 3 > sudo tee /proc/sys/vm/drop_caches

	echo $sched | sudo tee /sys/block/sda/queue/scheduler
	cat /sys/block/sda/queue/scheduler


done

sudo hdparm -W 1 /dev/sda