/********************************************************************************
 
  	Copyright (c) 2015
	Lantiq Beteiligungs-GmbH & Co. KG
        Lilienthalstrasse 15, 85579 Neubiberg, Germany 
	For licensing information, see the file 'LICENSE' in the root folder of
        this software module.
 
********************************************************************************/

/*  ***************************************************************************** 
 *         File Name    : help_defs.h                                          	*
 *         Description  : Common Library contains functions, defines,		*
 *			  structs, enums used across modules like CAL, CAPI,	*
 *			  CSD, Servd, and Management Entities			*
 *  *****************************************************************************/


/*! \file help_defs.h
\brief File contains the common definitions, macros and data
    structures common across all the modules in the software
*/
/** \addtogroup LIBHELP */
/* @{ */

#ifndef _HELP_DEFS_H
#define _HELP_DEFS_H

#include <stdint.h>
#include <syslog.h>

#include "help_enums.h"

#ifndef OUT
/*!
	\brief 
*/
#define OUT /*!< Macro for OUT null */
#endif 

#ifndef INOUT
/*! 
	\brief 
*/
#define INOUT /*!< Macro INOUT null */
#endif 

#ifndef IN 
/*! 
	\brief 
*/
#define IN /*!< Macro for IN null */
#endif

#define UNUSED_VAR(arg)     (void)(arg)     /*!< Macro to define variable  unused in the function  */ 

#define UNUSED_ARG	__attribute__ ((unused))	/*!< Macro to define function argument unused in the function  */

#define OBJLIST(x)     (IS_SOPT_ID(x) ||  IS_SOPT_OBJVALUE(x) || \
			 IS_SOPT_LEAFNODE(x) || IS_SOPT_OBJNAME(x) || IS_SOPT_DEFVAL(x))     /*!< Macro find objlist type check */


#define OBJATTRLIST(x) IS_SOPT_OBJATTR(x)	/*!< Macro to find objattrlist type check */

#define OWNER_MGMT(x) IS_OWN_WEB(x) ||  IS_OWN_TR69(x) || IS_OWN_CLI(x)  /*!< Macro to check owner  */

/*! 
	\brief 
*/
#define BOOT_CHK_PARAM "Device.X_INTEL_COM_BootChk"


/*! 
	\brief 
*/
#define IS_SOPT_ORGOWN(bit) (bit & SOPT_ORGOWN) 	/*!< Macro to check orginal owner bit set */


/*! 
	\brief 
*/
#define IS_SOPT_ID(bit) (bit & SOPT_ID) 	/*!< Macro to check suboper id bit set */

/*! 
	\brief 
*/
#define IS_SOPT_OBJNAME(bit) (bit & SOPT_OBJNAME) 	/*!< Macro to check suboper subtree bit set */

/*! 
	\brief 
*/
#define IS_SOPT_OBJVALUE(bit) (bit & SOPT_OBJVALUE) 	/*!< Macro to check suboper complete bit set */

/*! 
	\brief 
*/
#define IS_SOPT_OBJATTR(bit) (bit & SOPT_OBJATTR) 	/*!< Macro to check suboper attrval bit set */

/*! 
	\brief 
*/
#define IS_SOPT_LEAFNODE(bit) (bit & SOPT_LEAFNODE) 	/*!< Macro to check suboper leaf node bit set */

/*! 
	\brief 
*/
#define IS_SOPT_DEFVAL(bit) (bit & SOPT_DEFVAL) 	/*!< Macro to check suboper defult val bit set */

/*! 
	\brief 
*/
#define IS_SOPT_OBJACSATTR(bit) (bit & SOPT_OBJACSATTR) 	/*!< Macro to check suboper ACS attrval bit set */

/*!
        \brief 
*/
#define IS_ATTR_ACTIVE_NOTIFY_SET(bit) (bit & ATTR_ACTIVENOTI)   /*!< Macro for access control attribute */


/*! 
	\brief 
*/
#define IS_OWN_TR69(bit) (bit & OWN_TR69) 	/*!< Macro to check tr69 owner bit set */

/*! 
	\brief 
*/
#define IS_OWN_WEB(bit) (bit & OWN_WEB) 	/*!< Macro to check web owner bit set */

/*! 
	\brief 
*/
#define IS_OWN_SERVD(bit) (bit & OWN_SERVD)		/*!< Macro to check servd owner bit set */
 
/*! 
	\brief 
*/
#define IS_OWN_CLI(bit) (bit & OWN_CLI) 	/*!< Macro to check cli owner bit set */

/*! 
	\brief 
*/
#define IS_OWN_OTHER(bit) (bit & OWN_OTHER) /*!< Macro to check other owner bit set */

/*! 
	\brief 
*/
#define IS_OWN_POLLD(bit) (bit & OWN_POLLD)		/*!< Macro to check polld owner bit set */
 

/*! 
        \brief 
*/
#define MAX_LEN_OBJNAME 256 /*!< Object Name string length */

/*!
        \brief
*/
#define MAX_LEN_ID 6 /*!< Object ID string length */

/*!
        \brief 
*/
#define MAX_LEN_PARAM_NAME 256 /*!< Parameter Name string length */

/*!
        \brief 
*/
#define MAX_LEN_PARAM_VALUE 128 /*!< Parameter Value string length */

/*!
        \brief 
*/
#define MAX_LEN_VALID_VALUE 1024 /*!< Parameter valid value string length */

/*! 
	\brief 
*/
#define NO_ARG_VALUE  0 /*!< Macro for NO_ARG_VALUE null */

/*!
        \brief 
*/
#define MAX_LEN_PROFILENAME 64 /*!< Profile name string length */

/*!
        \brief 
*/
#define MAX_LEN_WEBNAME 128 /*!< Webname string length */

/*!
        \brief 
*/
#define MAX_LEN_PARAM_TYPE 20  /*!< Parameter Type string length */

/*! 
	\brief 
*/
typedef struct list_head ListHead;  /*!< List head structure typedef */

/*! 
	\brief 
*/
#define char8_t int8_t   /*!< Character typedef */

/*!
        \brief 
*/
#define IS_OBJLIST(x) OBJLIST(x)  /*!< Macro to check objlist structure used */

/*!
        \brief 
*/
#define IS_ATTR_PASSIVE_NOTIFY_SET(bit) (bit & ATTR_PASSIVENOTI)	/*!< Macro for access control attribute */

/*! \def NOT_SET
  \brief 
  */
#define NOT_SET 2 /*!< Flag Not Set*/

/*! \def SET
  \brief 
  */
#define SET 3   /*!< Flag Set*/



#endif  //#ifndef _HELP_DEFS_H

/* @} */
