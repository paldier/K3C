/***************************************************************************** *
 *     File Name  : errorhd_test.c                                             *
 *     Project    : UGW                                                        *
 *     Description: Error Handler Daemon test program.		                   *
 *                                                                             *
 ******************************************************************************/
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "errorhd.h"

#define UNUSED(x) (void)(x)
#define LOG(fmt, ...) printf("%s:%d: errorhd_test: " fmt, __func__, __LINE__, ##__VA_ARGS__);

struct thread_info {
	pthread_t thread_id;
	int id;
	int delay;
	int iterations;
};

bool nopadlogs = false;

static void *errorhd_test_thread(void *arg)
{
	struct thread_info tinfo = *(struct thread_info *)arg;
	int ret = 0;
	struct eh_info info = { 0 };

	LOG("START thread %d for %d iterations\n", tinfo.id, tinfo.iterations);
	while (tinfo.iterations > 0) {
		snprintf(info.subsys, sizeof(info.subsys),"PID%ld-ed_test_thread_#%d", (long)getpid(), tinfo.id);
		snprintf(info.headline, sizeof(info.headline), "error handler test thread payload\nI am thread number #%d\nIterations left=%d\n", tinfo.id, tinfo.iterations);
		info.nopadlog = nopadlogs;
		errorhd_trigger(&info);
		tinfo.iterations--;
		sleep(tinfo.delay);
	}

	LOG("EXIT thread %d, iterations=%d\n", tinfo.id, tinfo.iterations);
	return (void *)ret;
}

#define handle_error(msg) \
               do { perror(msg); exit(EXIT_FAILURE); } while (0)
#define handle_error_en(en, msg) \
               do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

int main(int argc, char *argv[])
{
	struct thread_info *tinfo;
	int tnum, opt, s, iterations = 10, num_threads = 1, delay = 1;
	void *res;

	srand(time(NULL));

	while ((opt = getopt(argc, argv, "i:t:nd:")) != -1) {
		switch (opt) {
		case 'i':
			iterations = strtoul(optarg, NULL, 0);
			if(iterations < 0) {
				LOG("Specified number of iterations per thread is outside supported range [ 0 - 1000 ]\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 't':
			num_threads = strtoul(optarg, NULL, 0);
			if(num_threads > 100) {
				LOG("Specified number of threads to run is outside supported range [ 0 - 100 ]\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'd':
			delay = strtoul(optarg, NULL, 0);
			if(delay < 0) {
				LOG("Specified delay in seconds is outside supported range [ 0 - 600 ]\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'n':
			nopadlogs = true;
			break;
		default:
			fprintf(stderr, "Usage: %s [-i iterations -d delay -t num_threads]\n"
					"\t-i <iterations> - number of iterations per thread\n"
					"\t-t <num_threads> - number of threads to run\n"
					"\t-d <delay> - delay in seconds between operations\n"
					"\t-n            - without padlogs\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	tinfo = calloc(num_threads, sizeof(struct thread_info));
	if (tinfo == NULL)
		handle_error("calloc");

	LOG("START [num_threads=%d, iterations=%d, delay=%d]\n", num_threads, iterations, delay);
	for (tnum = 0; tnum < num_threads; tnum++) {
		tinfo[tnum].id = tnum;
		tinfo[tnum].iterations = iterations;
		tinfo[tnum].delay = delay;
		s = pthread_create(&tinfo[tnum].thread_id, NULL, &errorhd_test_thread, &tinfo[tnum]);
		if (s != 0)
			handle_error_en(s, "pthread_create");
	}

	LOG("Now join with each thread, and display its returned value\n");
	for (tnum = 0; tnum < num_threads; tnum++) {
		s = pthread_join(tinfo[tnum].thread_id, &res);
		if (s != 0)
			handle_error_en(s, "pthread_join");

		LOG("Joined with thread %d; returned value was %d\n",
					 tinfo[tnum].id, (int) res);
	}

	exit(EXIT_SUCCESS);
}
