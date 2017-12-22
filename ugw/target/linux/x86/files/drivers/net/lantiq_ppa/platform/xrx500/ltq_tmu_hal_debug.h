/******************************************************************************

                               Copyright (c) 2012
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
#ifndef _tmu_hal_debug_h
#define _tmu_hal_debug_h

#if defined(_DEBUG)
/** enable debug printouts */
#  define INCLUDE_DEBUG_SUPPORT
#endif
void tmu_hal_proc_destroy(void);
int tmu_hal_proc_create(void);



#define TMU_DEBUG_ERR (1<< 0)
#define TMU_DEBUG_TRACE (1<<1)
#define TMU_DEBUG_HIGH (1<<2)
#define TMU_ENABLE_ALL_DEBUG (~0)
#define TMU_RAW_DEBUG(level, format, arg...)  \
do{ \
	if (g_tmu_dbg & level) \
	{ \
		printk(format, ##arg);\
	} \
}while(0)

#define TMU_HAL_DEBUG_MSG(level, format, arg...) TMU_RAW_DEBUG(level, format, ##arg)


struct tmu_dgb_info
{
    char *cmd;
    char *des;
    uint32_t flag;
};

static struct tmu_dgb_info dbg_enable_mask_str[] = {
    {"err",      "error level",                     TMU_DEBUG_ERR },
    {"trace",      "debug level",                     TMU_DEBUG_TRACE },
    {"high",      "high level",                     TMU_DEBUG_HIGH },
    /*the last one */
    {"all",       "enable all debug",                TMU_ENABLE_ALL_DEBUG}
};



/** TMU HAL Debug Levels */
enum tmu_hal_debug_level {
	/** Message */
	TMU_HAL_DBG_MSG,
	/** Warning */
	TMU_HAL_DBG_WRN,
	/** Error */
	TMU_HAL_DBG_ERR,
	/** Off */
	TMU_HAL_DBG_OFF
};

/** Debug message prefix */
#  define DEBUG_PREFIX        "[tmu hal]"

#if defined(WIN32)
#  define TMU_HAL_CRLF  "\r\n"
#else
#  define TMU_HAL_CRLF  "\n"
#endif 

//#define INCLUDE_DEBUG_SUPPORT

#ifdef INCLUDE_DEBUG_SUPPORT
extern enum tmu_hal_debug_level tmu_hal_debug_lvl;

int tmu_hal_debug_print_err(const char *format, ...);
int tmu_hal_debug_print_wrn(const char *format, ...);
int tmu_hal_debug_print_msg(const char *format, ...);

#  define DEBUG_ENABLE_ERR
#  define DEBUG_ENABLE_WRN
#  define DEBUG_ENABLE_MSG

#     ifdef DEBUG_ENABLE_ERR
#        define TMU_HAL_DEBUG_ERR   tmu_hal_debug_print_err
#     endif			/* DEBUG_ENABLE_ERR */
#     ifdef DEBUG_ENABLE_WRN
#        define TMU_HAL_DEBUG_WRN   tmu_hal_debug_print_wrn
#     endif			/* DEBUG_ENABLE_WRN */
#     ifdef DEBUG_ENABLE_MSG
#        define TMU_HAL_DEBUG_MSG   tmu_hal_debug_print_msg
#     endif			/* DEBUG_ENABLE_MSG */

#endif				/* INCLUDE_DEBUG_SUPPORT */

#ifndef STATIC
#if 1
#define STATIC static
#else
#define STATIC /**/
#endif
#endif

#ifndef TMU_HAL_DEBUG_ERR
#  if defined(__GNUC__)
#     define TMU_HAL_DEBUG_ERR(fmt, args...)   while(0){}
#  else
#     define TMU_HAL_DEBUG_ERR   {}
#  endif
#endif

#ifndef TMU_HAL_DEBUG_WRN
#  if defined(__GNUC__)
#     define TMU_HAL_DEBUG_WRN(fmt, args...)   while(0){}
#  else
#     define TMU_HAL_DEBUG_WRN   {}
#  endif
#endif

#ifndef TMU_HAL_DEBUG_MSG
#  if defined(__GNUC__)
#     define TMU_HAL_DEBUG_MSG(fmt, args...)   while(0){}
#  else
#     define TMU_HAL_DEBUG_MSG   printk
#  endif
#endif


#endif
