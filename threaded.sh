#!/bin/bash
#input 1 -> threads, input 2 -> time

echo noop > /sys/block/sda/queue/scheduler
cat /sys/block/sda/queue/scheduler

bash write_test.sh
bash read_test.sh

echo deadline > /sys/block/sda/queue/scheduler
cat /sys/block/sda/queue/scheduler

bash write_test.sh
bash read_test.sh

echo cfq > /sys/block/sda/queue/scheduler
cat /sys/block/sda/queue/scheduler

bash write_test.sh
bash read_test.sh