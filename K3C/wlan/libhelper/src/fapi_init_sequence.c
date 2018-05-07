#include "fapi_init_sequence.h"
#include <syslog.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int sequence = 0;

void log_fapi_init_msg(const char* file, const char *func, const int line)
{
	syslog(LOG_DEBUG, "FAPI_INIT: %s [%s:%d] FAPI sequence number: %d Process ID: %d\n", file, func, line, ++sequence, getpid());
	
}

void log_fapi_done_msg(const char* file, const char *func, const int line)
{
		
	syslog(LOG_DEBUG, "FAPI_INIT: %s [%s:%d]\n", file, func, line);
}
