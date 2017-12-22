#ifndef __PPA_API_PROC_H__20081103_2009__
#define __PPA_API_PROC_H__20081103_2009__



/*******************************************************************************
**
** FILE NAME    : ppa_api_proc.h
** PROJECT      : PPA
** MODULES      : PPA API (Routing/Bridging Acceleration APIs)
**
** DATE         : 3 NOV 2008
** AUTHOR       : Xu Liang
** DESCRIPTION  : PPA Protocol Stack Hook API Proc Filesystem Functions Header
**                File
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author         $Comment
** 03 NOV 2008  Xu Liang        Initiate Version
*******************************************************************************/
/*! \file ppa_api_proc.h
    \brief This file contains all proc api.
*/

/** \defgroup PPA_PROC_API PPA Proc API
    \brief  PPA Proc API provide a ppa configure command to let user comfigure PPA
            - ppa_api_proc.h: Header file for PPA PROC API
            - ppa_api_proc.c: C Implementation file for PPA API
*/
/* @{ */ 

/*
 * ####################################
 *              Definition
 * ####################################
 */



/*
 * ####################################
 *              Data Type
 * ####################################
 */



/*
 * ####################################
 *             Declaration
 * ####################################
 */

/*
 *  Init/Uninit Functions
 */
/*! \fn ppa_api_proc_file_create
    \brief This function to create ppa proc 
*/
void ppa_api_proc_file_create(void);

/*! \fn ppa_api_proc_file_destroy
    \brief This function to destroy ppa proc 
*/

void ppa_api_proc_file_destroy(void);



/* @} */
#endif  //  __PPA_API_PROC_H__20081103_2009__
