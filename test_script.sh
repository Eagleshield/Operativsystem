#!/bin/bash

SCHEDLIST='noop deadline cfq'

sudo hdparm -W 0 /dev/sda

for sched in SCHEDLIST; do
	echo 3 > sudo tee /proc/sys/vm/drop_caches

	echo $sched | sudo tee /sys/block/sda/queue/scheduler
	cat /sys/block/sda/queue/scheduler

	for threads in {1..8}; do
		./sched_test $threads 1000000000 $sched
	done
	
done

sudo hdparm -W 1 /dev/sda