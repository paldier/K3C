#ifndef __IFX_PPA_API_CPU_FREQ_H__20151101_1952__
#define __IFX_PPA_API_CPU_FREQ_H__20151101_1952__
/*******************************************************************************
**
** FILE NAME    : ppa_api_cpu_freq.h
** PROJECT      : PPA
** MODULES      : PPA API ( Power Management APIs)
**
** DATE         : 1 Nov 2015
** AUTHOR       : Shao Guohua
** DESCRIPTION  : PPA Protocol Stack Hook API Power Management
**                File
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date                $Author         $Comment
** 16 Dec 2009  Shao Guohua             Initiate Version
*******************************************************************************/

/*! \file ppa_api_cpu_freq.h
    \brief This file contains es.
           provide PPA power management API.
*/

/** \addtogroup  ppa_api_cpufreq_API */
/*@{*/



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

/*!
      \brief Request D0 power state when any session is added into table.
       \return none
       \ingroup IFX_PPA_API_PWM
*/
extern void ppa_api_cpufreq_activate_module(void);

/*!
      \brief Request D3 power state when any session is removed from table.
       \return none
       \ingroup IFX_PPA_API_PWM
*/
extern void ppa_api_cpufreq_deactivate_module(void);

/*!
   \brief Initialize ppa pwm
   \return
*/
extern void ppa_api_cpufreq_init(void);

/*!
   \brief exit ppa pwm
   \return
*/
extern void ppa_api_cpufreq_exit(void);

/* @} */


#endif  //  __IFX_PPA_API_CPU_FREQ_H__20151101_1952__
