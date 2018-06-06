
default: sched_test

sched_test: schedule_test.c
	gcc -Wall -pthread -o sched_test schedule_test.c -lm

run:
	./sched_test

clean:
	rm -f garb* sched_test