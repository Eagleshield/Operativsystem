#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>

pthread_barrier_t barrier;

double timer(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return 1000000 * (double)tv.tv_sec + (double)tv.tv_usec;
}

unsigned int get_rand_block(const long nr_of_blocks) {
	return (unsigned int)rand() % nr_of_blocks;
}

long get_disk_size() {
	int fd = open("/dev/sda5", O_RDONLY);
	assert(fd >= 0);

	unsigned long numblocks=0;
	const long disk_sz = lseek(fd, 0, SEEK_END);
	assert(disk_sz > 0);
	close(fd);
	return disk_sz;
}

long get_nr_of_blocks(const long block_size, const long disk_sz) {
	return (disk_sz/block_size)-1;
}

void* pread_file(void* param) {
	long* buf = (long*)param;
	long block_sz = buf[0]; long nr_blocks = buf[1];
	int flags = O_RDONLY|O_SYNC;
	int fd = open("/dev/sda5", flags);
	unsigned int rand_val = get_rand_block(nr_blocks);
	char* buffer = malloc(block_sz);

	pthread_barrier_wait(&barrier);
	double start = timer();
	pread(fd, buffer, block_sz, rand_val*block_sz);
	close(fd);	
	double time = ((timer() - start)/1000000);
	pthread_barrier_wait(&barrier);

	printf("%ld %lf\n",block_sz/1000000, time);
	free(buffer);

	return NULL;
}


int main(int argc, const char *argv[]) {	
	int pt = atoi(argv[1]);
	int nr_threads = 1 << pt;
	const long gigabyte = 1 << 30;
	const long total_size = (double)(gigabyte << 2)/((double)(pt+2)*0.5);
	pthread_t threads[nr_threads];
	long param[nr_threads][2];
	long mult = 0;
	pthread_barrier_init(&barrier, NULL, nr_threads);
	long disk_sz = get_disk_size();
	srand(time(NULL));
	for (int i = 0; i < nr_threads; i++) { 
		if(i == 0) {
			param[i][0] = total_size;
			param[i][1] = get_nr_of_blocks(total_size, disk_sz);
		}else if((1<<mult) > i) {
			param[i][0] = total_size >> mult;
			param[i][1] = get_nr_of_blocks(total_size >> mult, disk_sz);
		} else {
			param[i][0] = total_size >> ++mult;
			param[i][1] = get_nr_of_blocks(total_size >> mult, disk_sz);		
		}
		pthread_create(&threads[i], NULL, pread_file, param[i]);
	}

	double start_tot = timer();
	for (int i = 0; i < nr_threads; i++) {
		pthread_join(threads[i], NULL);
	}
	double stop = ((timer() - start_tot)/1000000);
	printf("Total %lf\n", stop);
	pthread_barrier_destroy(&barrier);
	return 0;
}