/***************************************************************************** *
 *     File Name  : errorhd.h                                                  *
 *     Project    : UGW                                                        *
 *     Description: Error Handler Daemon main program and core.                *
 *                                                                             *
 ******************************************************************************/
#pragma once
#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/file.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#define MAX_DATA_SIZE         64U
#define MAX_SUBSYS_SIZE     32U
#define MAX_HEADLINE_SIZE  256U

#define ERRORHD_NAMED_PIPE            "/opt/errorhd_pipe"
#define ERRORHD_NAMED_PIPE_LOCK "/opt/errorhd_pipe_lock"
#define ERRORHD_STATE_FILE		"/tmp/errorhd_is_running"
#define ERRORHD_UNBLOCK			0
#define ERRORHD_BLOCK			   1

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) max(b,a)

struct eh_info {
	char subsys[MAX_SUBSYS_SIZE];
	char headline[MAX_HEADLINE_SIZE];
	char data[3][MAX_DATA_SIZE];
	bool nopadlog;
};

int errorhd_spawn(char *pcBuf, int nBlockingFlag, int* pnChildExitStatus);

static inline bool errorhd_is_running(void)
{
	/* If the file exist (return 0), errorhd is running */
	if (access(ERRORHD_STATE_FILE, F_OK)) {
		return false;
	} else {
		return true;
	}
}

static inline int errorhd_lock(void)
{
	int fd, ret = 0;

	errno = 0;
	fd = open(ERRORHD_NAMED_PIPE_LOCK, O_CREAT | O_WRONLY);
	if (fd < 0) {
		fprintf(stderr, "errorhd_lock: PID %ld open %s failed with error [%s]\n", (long)getpid(), ERRORHD_NAMED_PIPE_LOCK, strerror(errno));
		return -1;
	}

	if ((ret = flock(fd, LOCK_EX)) == -1) {
		fprintf(stderr, "errorhd_lock: PID %ld flock %d failed with error [%s]\n", (long)getpid(), fd, strerror(errno));
		close(fd);
		return -1;
	}

	return fd;
}

static inline int errorhd_unlock(int fd)
{
	int ret = 0;

	errno = 0;
	if ((ret = flock(fd, LOCK_UN)) == -1)
		fprintf(stderr, "errorhd_unlock: PID %ld flock %d failed with error [%s]\n", (long)getpid(), fd, strerror(errno));

	close(fd);
	return ret;
}

/**
 * errorhd_trigger - trigger error handler
 * Not thread safe! If called from threads, protect call with appropriate locking.
 *
 * @param struct eh_info *info
 * @return
 */
static inline int
errorhd_trigger(struct eh_info *info)
{
	FILE *f;
	int lock_fd;
	size_t len = 0;

	/* If errorhd is not running - return */
	if (!errorhd_is_running()) {
		return 0;
	}

	if (access(ERRORHD_NAMED_PIPE, F_OK)) {
		perror("errorhd_trigger: access failed\n");
		return -1;
	}

	if ((lock_fd = errorhd_lock()) == -1)
		return -1;

	f = fopen(ERRORHD_NAMED_PIPE, "wb");
	if (f == NULL) {
		perror("errorhd_trigger: fopen failed\n");
		errorhd_unlock(lock_fd);
		return -1;
	}

	len = fwrite(info, sizeof(struct eh_info), 1, f);
	fclose(f);
	errorhd_unlock(lock_fd);

	if (len != 1) {
		perror("errorhd_trigger: fwrite failed\n");
	}

	return 0;
}
