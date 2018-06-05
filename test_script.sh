#!/bin/bash

SCHEDLIST='cfq deadline noop'

for sched in $SCHEDLIST; do
	echo $sched | sudo tee /sys/block/sda/queue/scheduler
	cat /sys/block/sda/queue/sheduler
done

