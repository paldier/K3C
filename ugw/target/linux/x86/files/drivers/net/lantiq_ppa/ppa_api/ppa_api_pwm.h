#ifndef __IFX_PPA_API_PWM_H__20091216_1952__
#define __IFX_PPA_API_PWM_H__20091216_1952__
/*******************************************************************************
**
** FILE NAME    : ppa_api_pwm.h
** PROJECT      : PPA
** MODULES      : PPA API ( Power Management APIs)
**
** DATE         : 16 DEC 2009
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

/*! \file ppa_api_pwm.h
    \brief This file contains es.
           provide PPA power management API.
*/

/** \addtogroup  PPA_PWM_API */
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

#if defined(CONFIG_LTQ_CPU_FREQ) || defined(CONFIG_LTQ_PMCU) || defined(CONFIG_LTQ_PMCU_MODULE)

/*!
      \brief Request D0 power state when any session is added into table.
       \return none
       \ingroup IFX_PPA_API_PWM
*/
extern void ppa_pwm_activate_module(void);

/*!
      \brief Request D3 power state when any session is removed from table.
       \return none
       \ingroup IFX_PPA_API_PWM
*/
extern void ppa_pwm_deactivate_module(void);

/*!
   \brief Initialize ppa pwm
   \return
*/
extern void ppa_pwm_init(void);

/*!
   \brief exit ppa pwm
   \return
*/
extern void ppa_pwm_exit(void);

#else

static inline void ppa_pwm_activate_module(void) {}
static inline void ppa_pwm_deactivate_module(void) {}
static inline void ppa_pwm_init(void) {}
static inline void ppa_pwm_exit(void) {}

#endif

/* @} */


#endif  //  __IFX_PPA_API_PWM_H__20091216_1952__
