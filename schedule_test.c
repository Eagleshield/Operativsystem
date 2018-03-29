#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

typedef void *(*func_ptr)(void*);

void run_threads(func_ptr func, int num_threads);
void *write_test_static(void *args);
void *write_test_dynamic(void *arg);
void *read_test(void *arg);

pthread_barrier_t barrier;

typedef struct {
    int tid;
    char tids;
} args;

int main(void) {
    system("echo noop | sudo tee /sys/block/sda/queue/scheduler");
    system("cat /sys/block/sda/queue/scheduler");
    system("sudo hdparm -W 0 /dev/sda");
    int num_threads = 6;
    pthread_barrier_init(&barrier, NULL, num_threads);
    struct timeval tval_before, tval_after, tval_result;

    printf("%s\n", "Noop write-test...");
    
    gettimeofday(&tval_before, NULL);
    
    //run_threads(&write_test_static, num_threads);
    run_threads(&write_test_dynamic, num_threads);
    //run_threads(&read_test, num_threads);

    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);

    printf("Noop total elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);


    system("echo deadline | sudo tee /sys/block/sda/queue/scheduler");
    system("cat /sys/block/sda/queue/scheduler");
    printf("%s\n", "Deadline write-test...");

    gettimeofday(&tval_before, NULL);
    
    //run_threads(&write_test_static, num_threads);
    run_threads(&write_test_dynamic, num_threads);
    //run_threads(&read_test, num_threads);

    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);

    printf("Deadline total elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

	system("echo cfq | sudo tee /sys/block/sda/queue/scheduler");
    system("cat /sys/block/sda/queue/scheduler");
    printf("%s\n", "Cfq write-test...");

    gettimeofday(&tval_before, NULL);
    
    //run_threads(&write_test_static, num_threads);    
    run_threads(&write_test_dynamic, num_threads);
    //run_threads(&read_test, num_threads);

    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);

    printf("Cfq total elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

    system("sudo hdparm -W 1 /dev/sda");
    return 0;
}

void run_threads(func_ptr func, int num_threads) {
	pthread_t threads[num_threads];
    args t_args[num_threads];

	for(int i = 0; i < num_threads; i++) {
        t_args[i].tid = i;
        t_args[i].tids = i + '0';
        pthread_create(&threads[i], NULL, func, &t_args[i]);
    }
    for(int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

void *write_test_static(void *arg) {
    args *t_args = arg;
    size_t size = 500000000;
    char *big_boy = malloc(size);

    for(int i = 0; i < size; i++) {
        big_boy[i] = 'X';
    }

    char file_name[9] = {'g','a','r','b','a','g','e','e'};
    file_name[7] = t_args->tids;

	FILE *fp;
	if(t_args->tid%2 == 0) {
        char *filep = malloc(32*sizeof(char));
        strcpy(filep, "../testdirectory/");
        strcat(filep, file_name);
		fp = fopen(filep, "w");
    } else {
		fp = fopen(file_name, "w");
	}

    if(fp == NULL) {
        perror("fopen");
        free(big_boy);
        return NULL;
	}
	if(t_args->tid == 0)
		printf("%s\n", "Static size test");
    struct timeval tval_before, tval_after, tval_result;
    pthread_barrier_wait(&barrier);
    /* Timer start */
    gettimeofday(&tval_before, NULL);
    for(int i = 0; i < 4; i++)
    	fwrite(big_boy, size, 1, fp);

    fclose(fp);

    /* Timer end */
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);
    
    pthread_barrier_wait(&barrier);

    printf("(%d)Time elapsed: %ld.%06ld\n", t_args->tid, (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

    free(big_boy);
    return NULL;
}

void *write_test_dynamic(void *arg) {
	args *t_args = arg;
    size_t size[] = {1000000000, 50000000, 1000000};
    char *big_boy = malloc(size[0]);
    char *medium_boy = malloc(size[1]);
    char *small_boy = malloc(size[2]);

    for(int i = 0; i < size[0]; i++) {
        big_boy[i] = 'X';
        if(i < size[1]) {
        	medium_boy[i] = 'Y';
        	if(i < size[2]) {
        		small_boy[i] = 'Z';
        	}
        }
    }

    char file_name[9] = {'g','a','r','b','a','g','e','e'};
    file_name[7] = t_args->tids;

	FILE *fp;
	if(t_args->tid%2 == 0) {
        char *filep = malloc(32*sizeof(char));
        strcpy(filep, "../testdirectory/");
        strcat(filep, file_name);
		fp = fopen(filep, "w");
		//printf("%s\n", filep);
    } else {
		fp = fopen(file_name, "w");
    	//printf("%s\n", file_name);
	}

    if(fp == NULL) {
    	perror("fopen");
        return NULL;
	}

	if(t_args->tid == 0)
		printf("%s\n", "Dynamic size test");

    struct timeval tval_before, tval_after, tval_result;
    pthread_barrier_wait(&barrier);
    
    /* Timer start */
    gettimeofday(&tval_before, NULL);

    if(t_args->tid < 1) {
	    fwrite(big_boy, size[0], 1, fp);
    } else if(t_args->tid < 3 && t_args->tid > 0) {
    	for(int i = 0; i < 20; i++) {
    		fwrite(medium_boy, size[1], 1, fp);
    	}
    } else {
    	for(int i = 0; i < 1000; i++) {
    		fwrite(small_boy, size[2], 1, fp);
    	}
    }

    fclose(fp);

    /* Timer end */
    gettimeofday(&tval_after, NULL);
    timersub(&tval_after, &tval_before, &tval_result);
    pthread_barrier_wait(&barrier);

    printf("(%d)Time elapsed: %ld.%06ld\n", t_args->tid, (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

	free(big_boy);
	free(medium_boy);
	free(small_boy);
    return NULL;
}

// void *read_test(void *arg) {
// 	args *t_args = arg;
// 	int size = 1000000000;
// 	char *big_boy = malloc(size);
	
// 	FILE *fp = open("/dev/urandom", O_DIRECT | O_RDONLY);
	
//     struct timeval tval_before, tval_after, tval_result;

// 	pthread_barrier_wait(&barrier);
//  	/* Timer start */
//     gettimeofday(&tval_before, NULL);
//     read(fp, big_boy, size);

//     fclose(fp);

//     gettimeofday(&tval_after, NULL);
//     timersub(&tval_after, &tval_before, &tval_result);

//     printf("Time elapsed: %ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
//     free(big_boy);
// 	return NULL;
// }