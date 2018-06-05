#!/bin/bash
SCHEDLIST='cfq' #deadline noop'

#compile tests
gcc kalletest.c -o rt -lpthread 
#drop cache
#echo 3 > /proc/sys/vm/drop_caches

#disable Cache
sudo hdparm -W 0 /dev/sdd
mkdir results
runs=$1

for sched in $SCHEDLIST; do
	echo $sched | sudo tee /sys/block/sdd/queue/scheduler
	cat /sys/block/sdd/queue/scheduler

	for (( i = 0; i < $runs; i++ )); do
		for (( j = 0; j < 5; j++ )); do
			echo 3 > /proc/sys/vm/drop_caches
			sudo ./rt $j >> "results/result_rt_"$sched"_"$i"_"$j
			#python parse_results.py "results/result_rt_"$sched"_"$i"_"$j
 			rm "results/result_rt_"$sched"_"$i"_"$j
		done
	done
done

#python merge_results.py

sudo hdparm -W 1 /dev/sdd
rm wt rt rwt
