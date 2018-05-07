#ifndef FAPI_INIT_SEQUENCE_H
#define FAPI_INIT_SEQUENCE_H

#include <sys/time.h>
#include <syslog.h>
#include <unistd.h>

/**
*	This file is checked for LOGGING the FAPI init messages
*/
#define FAPI_LOG_FILE "/opt/lantiq/etc/fapi_debug"

typedef struct {
	struct timeval xTv1;
	struct timeval xTv2;
} __xFAPITime;

void log_fapi_init_msg(const char* file, const char *func, const int line);
void log_fapi_done_msg(const char* file, const char *func, const int line);

/** 
*	FAPI time profiling related MACROS
*/
#define TIME_INIT(__ix) __xFAPITime __ix
#define TIME_START(__ix) gettimeofday(&__ix.xTv1, NULL)
#define TIME_STOP(__ix) gettimeofday(&__ix.xTv2, NULL)

#define LOG_FAPI_INIT() log_fapi_init_msg(__FILE__, __func__, __LINE__)
#define LOG_FAPI_DONE() log_fapi_done_msg(__FILE__, __func__, __LINE__)
#define LOG_FAPI_ARGS(args...) syslog(LOG_DEBUG, "FAPI_INIT: "args)

#define FAPI_PRINT_DIFFTIME(fapi_name, __ix) syslog(LOG_DEBUG, "FAPI_INIT: %s initialization completed in %f seconds\n", \
	fapi_name ,(double) (__ix.xTv2.tv_usec - __ix.xTv1.tv_usec) / 1000000 + (double) (__ix.xTv2.tv_sec - __ix.xTv1.tv_sec))

#define LOG_FAPI_CALLFLOW_OPEN(arg) 			\
	        TIME_INIT(xTime); 			\
		memset(&xTime, 0, sizeof(__xFAPITime));	\
		if (access(FAPI_LOG_FILE, F_OK) == 0) { \
        	LOG_FAPI_INIT(); 			\
        	TIME_START(xTime); 			\
	        LOG_FAPI_ARGS(arg); 			\
		}

#define LOG_FAPI_CALLFLOW_CLOSE() 			\
		if (access(FAPI_LOG_FILE, F_OK) == 0) { \
        	TIME_STOP(xTime); 			\
	        FAPI_PRINT_DIFFTIME(__func__, xTime); 	\
        	LOG_FAPI_DONE(); 			\
		}

#endif  /*FAPI_INIT_SEQUENCE_H*/
