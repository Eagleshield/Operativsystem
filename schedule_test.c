#define _GNU_SOURCE

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
#include <math.h>

typedef void *(*func_ptr)(void*);

void run_threads(func_ptr func, int num_threads, char *sched, int size);
void *write_test_static(void *arg);
void *write_test_dynamic(void *arg);
void *read_test(void *arg);
int stringToInt(char *string);

pthread_barrier_t barrier;

typedef struct {
    int size;
    int tid;
    char tids;
    int res_fd;
    char *sched;
} args;

int main(int argc, char **args) {
    //ARGS: num_threads total_size scheduler
	if(argc != 4)
		return -1;

    int num_threads	= stringToInt(args[1]);
    int total_size	= stringToInt(args[2]);
    char *sched 	= args[3];

    pthread_barrier_init(&barrier, NULL, num_threads);
    struct timeval tval_before, tval_after, tval_result;
    
    //-------------------------------------------------------------------------

    char *results = malloc(32*sizeof(char));

    strcpy(results, sched);
    strcat(results, "write");
    strcat(results, args[1]);

    int fd = open(results, O_CREAT | O_APPEND, 0777);
    
    gettimeofday(&tval_before, NULL);
    run_threads(&write_test_dynamic, num_threads, sched, total_size);
    gettimeofday(&tval_after, NULL);

    timersub(&tval_after, &tval_before, &tval_result);
    char *str = malloc(30*sizeof(char));
    sprintf(str, "%ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
    write(fd, str, 30);
    free(results);
    //-------------------------------------------------------------------------
    char *results2 = malloc(32*sizeof(char));

    strcpy(results2, sched);
    strcat(results2, "read");
    strcat(results2, args[1]);

    int fd2 = open(results2, O_CREAT | O_APPEND, 0777);
    
    gettimeofday(&tval_before, NULL);
    run_threads(&read_test, num_threads, sched, total_size);
    gettimeofday(&tval_after, NULL);

    timersub(&tval_after, &tval_before, &tval_result);

    char *str2 = malloc(30*sizeof(char));
    sprintf(str2, "%ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);
    write(fd2, str2, 30);

    free(results2);

    return 0;
}

int stringToInt(char *string) {
	int len = strlen(string);
	double res = 0;
	for (int i = 0; i < len; i++) {
		res += (pow(10, (len - i)) * (string[i] - '0'));
	}
	return (int)res;
}

void run_threads(func_ptr func, int num_threads, char *sched, int size) {
	pthread_t threads[num_threads];
    args t_args[num_threads];

	for(int i = 0; i < num_threads; i++) {
        t_args[i].tid = i;
        t_args[i].tids = i + '0';
        t_args[i].sched = sched;
        t_args[i].size = (int)(size/num_threads);
        pthread_create(&threads[i], NULL, func, &t_args[i]);
    }
    for(int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}


void *write_test_dynamic(void *arg) {
	args *t_args = arg;
    
    char *filep = malloc(32*sizeof(char));
    char file_name[9] = {'w','r','i','t','e','e'};
    
    file_name[5] = t_args->tids;

	int fp;

	if(t_args->tid%2 == 0) {
        strcpy(filep, "../testdirectory/");
        strcat(filep, t_args->sched);
        strcat(filep, file_name);
		fp = open(filep, O_WRONLY);
    } else {
    	strcpy(filep, t_args->sched);
    	strcat(filep, file_name);
		fp = open(filep, O_WRONLY);
	}

    pthread_barrier_wait(&barrier);
    
    char *file = malloc(t_args->size*sizeof(char));

	write(fp, file, t_args->size);    

    close(fp);

    pthread_barrier_wait(&barrier);

    free(filep);
    free(file);
    return NULL;
}

void *read_test(void *arg) {
	args *t_args = arg;
	int size = t_args->size;
	char *big_boy = malloc(size);
	int fp = open("/dev/sda5", O_RDONLY | O_SYNC);
	
	pthread_barrier_wait(&barrier);

	pread(fp, big_boy, size, (random() % 145) * 1000000000);

    close(fp);

    free(big_boy);
	return NULL;
}

// void *write_test_static(void *arg) {
//     args *t_args = arg;
//     size_t size = 1000000;
//     char *big_boy = malloc(size);

//     for(int i = 0; i < size; i++) {
//         big_boy[i] = 'A' + (random() % 26);
//     }

//     char file_name[9] = {'g','a','r','b','a','g','e','e'};
//     file_name[7] = t_args->tids;

// 	FILE *fp;
// 	if(t_args->tid%2 == 0) {
//         char *filep = malloc(32*sizeof(char));
//         strcpy(filep, "../testdirectory/");
//         strcat(filep, file_name);
// 		fp = fopen(filep, "w");
//     } else {
// 		fp = fopen(file_name, "w");
// 	}

//     if(fp == NULL) {
//         perror("fopen");
//         free(big_boy);
//         return NULL;
// 	}
// 	if(t_args->tid == 0)
// 		printf("%s\n", "Static size test");
//     struct timeval tval_before, tval_after, tval_result;
//     pthread_barrier_wait(&barrier);
//     /* Timer start */
//     gettimeofday(&tval_before, NULL);
//     for(int i = 0; i < 1000; i++)
//     	fwrite(big_boy, size, 1, fp);

//     fclose(fp);

//     /* Timer end */
//     gettimeofday(&tval_after, NULL);
//     timersub(&tval_after, &tval_before, &tval_result);
    
//     pthread_barrier_wait(&barrier);

//     printf("(%d)Time elapsed: %ld.%06ld\n", t_args->tid, (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

//     fprintf(t_args->res, "%ld.%06ld\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec);

//     free(big_boy);
//     return NULL;
// }