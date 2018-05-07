/********************************************************************************
 
  	Copyright (c) 2015
	Lantiq Beteiligungs-GmbH & Co. KG
        Lilienthalstrasse 15, 85579 Neubiberg, Germany 
	For licensing information, see the file 'LICENSE' in the root folder of
        this software module.
 
********************************************************************************/

/*  ***************************************************************************** 
 *         File Name    : help_logging.h	                                *       	
 *         Description  : header file for logging framework 			*
 *  *****************************************************************************/


#ifndef HELP_LOGGING_H
#define HELP_LOGGING_H
#define DBG_TIMESTAMP 1   /*!< Macro for debug timestamp */ 
#include<stdio.h>
#include<stdint.h>
#include<stdarg.h>
#include<syslog.h>
#ifdef DBG_TIMESTAMP
#include <sys/time.h>
#include <time.h>
#endif

/*! \file help_logging.h
 \brief File contains macros and enums for logging debug messages
*/

/** \addtogroup SYSFRAMEWORK_LOG */
/* @{ */


#define AFTERX_TYPE(x) un ## x ## LogType	/*!< x type  */ 
#define XAFTERX_TYPE(x) AFTERX_TYPE(x)		/*!<  x after type */ 
#define AFTERX_LEVEL(x) un ## x ## LogLevel  /*!< x level  */ 
#define XAFTERX_LEVEL(x) AFTERX_LEVEL(x)	/*!< x after level  */ 

#define log_level(NAME) XAFTERX_LEVEL(NAME)   /*!< log level  */ 
#define log_type(NAME) XAFTERX_TYPE(NAME)		/*!< log type  */ 

#ifndef LOGGING_ID
#error "Please define LOGGING_ID"
#else
#define LOGTYPE log_type(LOGGING_ID) 	/*!< log type  */ 
#define LOGLEVEL log_level(LOGGING_ID) 	/*!< log level  */ 
#endif

#define COLOR_NRM  "\x1B[0m"
#define COLOR_RED  "\x1B[31m"
#define COLOR_GRN  "\x1B[32m"
#define COLOR_YEL  "\x1B[33m"
#define COLOR_BLU  "\x1B[34m"
#define COLOR_MAG  "\x1B[35m"
#define COLOR_CYN  "\x1B[36m"
#define COLOR_WHT  "\x1B[37m"
#define COLOR_ORA  "\x1B[38;5;214m"

//#define SYS_LOG_EMERG     LOG_EMERG 
//#define SYS_LOG_ALERT     LOG_ALERT
#define SYS_LOG_CRIT      LOG_CRIT 				/*!< critical level   */ 
#define SYS_LOG_ERR       LOG_ERR				/*!< error level   */ 
//#define SYS_LOG_WARNING   LOG_WARNING 
//#define SYS_LOG_NOTICE    LOG_NOTICE 
#define SYS_LOG_INFO      LOG_INFO 				/*!< info level   */ 
#define SYS_LOG_DEBUG     LOG_DEBUG 			/*!< debug level   */ 

#define SYS_LOG_TYPE_NONE               0				/*!< log type  */ 
#define SYS_LOG_TYPE_FILE             (1 << 0)			/*!< log type file  */ 
#define SYS_LOG_TYPE_CONSOLE          (1 << 1)			/*!< log type console    */ 
#define SYS_LOG_TYPE_FILE_AND_CONSOLE (SYS_LOG_TYPE_FILE | SYS_LOG_TYPE_CONSOLE)   	/*!< log type both console and file    */

extern uint16_t LOGLEVEL;   /*!<  log level  */ 
extern uint16_t LOGTYPE;	/*!<  log type  */ 
static void LOGF_LOG_PRINT(const char *color_code, const char *logtype, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

static void LOGF_LOG_PRINT(const char *color_code, const char *logtype, const char *fmt, ...) {
		char cAppendStr[256]={0};
		va_list args;
#ifdef DBG_TIMESTAMP
		struct timeval tv;
		struct tm * timeinfo;
		time_t nowtime;
		char tmbuf[50]="\0";
		gettimeofday(&tv, NULL);
		nowtime = tv.tv_sec;
		timeinfo = localtime(&nowtime);
		if(timeinfo != NULL)
				strftime(tmbuf,50,"%d-%m-%Y %T", timeinfo);  
#endif
#ifdef DBG_TIMESTAMP
		snprintf(cAppendStr, sizeof(cAppendStr), "%s<%s> [%s] ",color_code, tmbuf, logtype);
#else
		snprintf(cAppendStr, sizeof(cAppendStr), "%s[%s] ",color_code, logtype);
#endif
		do{					
				printf("%s", cAppendStr);
				va_start(args, fmt);
				vprintf(fmt, args);
				va_end(args);
				printf("%s",COLOR_NRM);
				fflush(stdout);
		}while(0);				
}

#define LOGF_OPEN(sl_name) openlog(sl_name, LOG_CONS | LOG_PID | LOG_NDELAY, LOG_USER)  /*!< LOG OPEN */ 
#define LOGF_CLOSE()  closelog()		/*!< LOG CLOSE */ 

static inline void LOGF_MASK(int log_level) {
#ifdef PROC_LEVEL
		if ( log_level >= SYS_LOG_EMERG && log_level <= SYS_LOG_DEBUG)
				setlogmask(LOG_UPTO(log_level));
		else
				setlogmask(LOG_UPTO(SYS_LOG_INFO));
#else
		LOGLEVEL = log_level;
#endif
}

static inline void LOGF_TYPE(int log_type) {
		LOGTYPE = log_type;
}

/* PACKAGE_ID will be provided by relevant package make file.  */
#define LOGF_LOG_ERROR(args...)	      PRINTF(ERR, ##args);   /*!< Macro to define error level  */ 
#define LOGF_LOG_EMERG(args...)       PRINTF(EMERG, ##args)   /*!< Macro to define emergency level  */ 
#define LOGF_LOG_ALERT(args...)       PRINTF(ALERT, ##args)    /*!< Macro to define alert level  */ 
#define LOGF_LOG_CRITICAL(args...)    PRINTF(CRIT, ##args)		/*!< Macro to define critical level  */ 
#define LOGF_LOG_WARNING(args...)	    PRINTF(WARNING, ##args)		/*!< Macro to define warnning  level  */ 
#define LOGF_LOG_NOTICE(args...)	    PRINTF(NOTICE, ##args)		/*!< Macro to define notice level  */ 
#define LOGF_LOG_INFO(args...)		    PRINTF(INFO, ##args)		/*!< Macro to define info level  */ 
#define LOGF_LOG_DEBUG(args...)       PRINTF(DEBUG, ##args)			/*!< Macro to define debug level  */ 
#define LOGF_LOG(LEVEL, fmt, args...) SYSLOG_##LEVEL(PACKAGE_ID"{%s, %d}:"fmt, __func__, __LINE__, ##args) /*!< PRINTF Macro */ 

#define PRINTF(LEVEL, fmt, args...)   SYSLOG_##LEVEL(PACKAGE_ID"{%s, %d}:"fmt, __func__, __LINE__, ##args) /*!< PRINTF Macro */ 

#ifdef SYS_LOG_EMERG  // level 0
#define SYSLOG_EMERG(fmt, args...) \
		do{					\
				if( SYS_LOG_EMERG > LOGLEVEL) \
				break;  \
				if( LOGTYPE & SYS_LOG_TYPE_FILE) \
				syslog(LOG_EMERG, fmt, ##args); \
				if( LOGTYPE & SYS_LOG_TYPE_CONSOLE) \
				LOGF_LOG_PRINT(COLOR_RED, "EMERG", fmt, ##args); \
		}while(0);
#else
#define SYSLOG_EMERG(fmt, args...) { }  /*!< Emergency log level  */ 
#endif

#ifdef SYS_LOG_ALERT // level 1
#define SYSLOG_ALERT(fmt, args...) \
		do{					\
				if( SYS_LOG_ALERT > LOGLEVEL) \
				break;  \
				if( LOGTYPE & SYS_LOG_TYPE_FILE) \
				syslog(LOG_ALERT, fmt, ##args); \
				if( LOGTYPE & SYS_LOG_TYPE_CONSOLE) \
				LOGF_LOG_PRINT(COLOR_RED, "ALERT", fmt, ##args); \
		}while(0);
#else
#define SYSLOG_ALERT(fmt, args...) { }   /*!< Alert log level  */ 
#endif

#ifdef SYS_LOG_CRIT // level 2
#define SYSLOG_CRIT(fmt, args...) \
		do{					\
				if( SYS_LOG_CRIT > LOGLEVEL) \
				break;  \
				if( LOGTYPE & SYS_LOG_TYPE_FILE) \
				syslog(LOG_CRIT, fmt, ##args); \
				if( LOGTYPE & SYS_LOG_TYPE_CONSOLE) \
				LOGF_LOG_PRINT(COLOR_RED, "CRITICAL", fmt, ##args); \
		}while(0);			/*!< Critical log level  */ 
#else
#define SYSLOG_CRIT(fmt, args...) { }			/*!< Critical log level  */ 
#endif

#ifdef SYS_LOG_ERR  // level 3
#define SYSLOG_ERR(fmt, args...) \
		do{					\
				if( SYS_LOG_ERR > LOGLEVEL) \
				break;  \
				if( LOGTYPE & SYS_LOG_TYPE_FILE) \
				syslog(LOG_ERR, fmt, ##args); \
				if( LOGTYPE & SYS_LOG_TYPE_CONSOLE) \
				LOGF_LOG_PRINT(COLOR_ORA, "ERROR" , fmt, ##args); \
		}while(0);			/*!< Error log level  */ 
#else
#define SYSLOG_ERR(fmt, args...) { }		/*!< Error log level  */ 
#endif

#ifdef SYS_LOG_WARNING // level 4
#define SYSLOG_WARNING(fmt, args...) \
		do{					\
				if( SYS_LOG_WARNING > LOGLEVEL) \
				break;  \
				if( LOGTYPE & SYS_LOG_TYPE_FILE) \
				syslog(LOG_WARNING, fmt, ##args); \
				if( LOGTYPE & SYS_LOG_TYPE_CONSOLE) \
				LOGF_LOG_PRINT(COLOR_NRM, "WARNING" , fmt, ##args); \
		}while(0);			/*!< Warning log level  */ 
#else
#define SYSLOG_WARNING(fmt, args...) { }			/*!< Warning log level  */ 
#endif

#ifdef SYS_LOG_NOTICE // level 5
#define SYSLOG_NOTICE(fmt, args...) \
		do{					\
				if( SYS_LOG_NOTICE > LOGLEVEL) \
				break;  \
				if( LOGTYPE & SYS_LOG_TYPE_FILE) \
				syslog(LOG_NOTICE, fmt, ##args); \
				if( LOGTYPE & SYS_LOG_TYPE_CONSOLE) \
				LOGF_LOG_PRINT(COLOR_NRM, "NOTICE", fmt, ##args); \
		}while(0);				/*!< Notice log level  */ 
#else
#define SYSLOG_NOTICE(fmt, args...) { }			/*!< Notice log level  */ 
#endif

#ifdef SYS_LOG_INFO // level 6
#define SYSLOG_INFO(fmt, args...) \
		do{					\
				if( SYS_LOG_INFO > LOGLEVEL) \
				break;  \
				if( LOGTYPE & SYS_LOG_TYPE_FILE) \
				syslog(LOG_INFO, fmt, ##args); \
				if( LOGTYPE & SYS_LOG_TYPE_CONSOLE) \
				LOGF_LOG_PRINT(COLOR_NRM, "INFO", fmt, ##args); \
		}while(0);		/*!< Information log level  */ 
#else
#define SYSLOG_INFO(fmt, args...) { }		/*!< Information log level  */ 
#endif

#ifdef SYS_LOG_DEBUG // level 7
#define SYSLOG_DEBUG(fmt, args...) \
		do{					\
				if( SYS_LOG_DEBUG > LOGLEVEL) \
				break;  \
				if(LOGTYPE & SYS_LOG_TYPE_FILE) \
				syslog(LOG_DEBUG, fmt, ##args); \
				if( LOGTYPE & SYS_LOG_TYPE_CONSOLE) \
				LOGF_LOG_PRINT(COLOR_NRM, "DEBUG", fmt, ##args); \
		}while(0);			/*!< Debug log level  */ 
#else
#define SYSLOG_DEBUG(fmt, args...) { }		/*!< Debug log level  */ 
#endif

/* @} */

#endif //_LTQ_DEBUG_H
