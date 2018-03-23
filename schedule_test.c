#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

void *write_test(void *args);
pthread_barrier_t barrier;

typedef struct {
    int tid;
    char tids;
} args;

#define BILLION  1000000000L;

int main(void) {
    system("echo noop | sudo tee /sys/block/sda/queue/scheduler");
    system("cat /sys/block/sda/queue/scheduler");
    system("sudo hdparm -W 0 /dev/sda");
    int num_threads = 5;

    pthread_t threads[num_threads];
    args t_args[num_threads];
    pthread_barrier_init(&barrier, NULL, num_threads);

    for(int i = 0; i < num_threads; i++) {
        t_args[i].tid = i;
        t_args[i].tids = i + '0';
        pthread_create(&threads[i], NULL, write_test, &t_args[i]);
    }

    for(int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    system("sudo hdparm -W 1 /dev/sda");
    return 0;
}

void *write_test(void *arg) {
    args *t_args = arg;
    size_t size = 1000000000;
    char *big_boy = malloc(size);

    for(int i = 0; i < size; i++) {
        big_boy[i] = 'X';
    }
        pthread_barrier_wait(&barrier);

    char file_name[9] = {'g','a','r','b','a','g','e','e'};
    file_name[7] = t_args->tids;
    //printf("%s\n", file_name);

	FILE *fp;
	if(t_args->tid%2 == 0) {
		printf("%s\n", "Why you seg?");
        char *filep = malloc(32*sizeof(char));
        strcpy(filep, "../../testdirectory/");
        strcat(filep, file_name);
		fp = fopen(filep, "w");
	   printf("%s\n", filep);
    } else {
		fp = fopen(file_name, "w");
        printf("%s\n", file_name);
	}

    perror("fopen");
    if(fp == NULL) {
        fprintf(stderr, "%s\n", "File not created.");
        return NULL;
    }
    for(int i = 0; i < 1; i++) {
        fwrite(big_boy, size, 1, fp);;
    }

    free(big_boy);

    fclose(fp);

    return NULL;
}
