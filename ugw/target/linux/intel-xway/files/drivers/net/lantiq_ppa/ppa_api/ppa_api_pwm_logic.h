#ifndef __IFX_PPA_API_PWM_LOGIC_H__20101014_1952__
#define __IFX_PPA_API_PWM_LOGIC_H__20101014_1952__
/*******************************************************************************
**
** FILE NAME    : ifx_ppa_api_pwm_logic.h
** PROJECT      : PPA
** MODULES      : PPA API ( Power Management logic APIs)
**
** DATE         : 14 Jan 2010
** AUTHOR       : Shao Guohua
** DESCRIPTION  : PPA Power Management Logic API
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
** 14 Jan 2010  Shao Guohua             Initiate Version
*******************************************************************************/
/*! \file ifx_ppa_api_pwm_logic.h
    \brief This file contains es.
           provide PPA power management API.
*/

/** \addtogroup PPA_PWM_API PPA Power Management API
    \brief  PPA Power Management  API provide PPA Power Management and IOCTL API
            The API is defined in the following two source files
            - ifx_ppa_api_pwm.h: Header file for PPA API
            - ifx_ppa_api_pwm_logic.h: Header file for PPA API
            - ifx_ppa_api_pwm.c: C Implementation file for PPA Power management API
            - ifx_ppa_api_pwm_logic.c: C impelementation file for Powr management Logic and interface with PPE driver
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
/*!
   \brief init ppa pwm logic
   \return IFX_SUCESS or IFX_FAILURE
   \ingroup IFX_PPA_API_PWM
*/
extern int32_t ppa_pwm_logic_init(void);

/*!
   \brief get current ppa pwm state
   \param[in] flag future usage only
   \return current status: from IFX_PMCU_STATE_D0 to IFX_PMCU_STATE_D3
   \ingroup IFX_PPA_API_PWM
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
extern IFX_PMCU_STATE_t ppa_pwm_get_current_status(int32_t flag);
#else
extern enum ltq_cpufreq_state ppa_pwm_get_current_status(int32_t flag);
#endif

/*!
   \brief set current ppa pwm state
   \param[in] e_state new state to set. Its value range: from IFX_PMCU_STATE_D0 to IFX_PMCU_STATE_D3
   \param[in] flag future usage only
   \return current status: IFX_SUCESS or IFX_FAILURE
   \ingroup IFX_PPA_API_PWM
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
extern int32_t ppa_pwm_set_current_state(IFX_PMCU_STATE_t e_state, int32_t flag);
#else
extern int32_t ppa_pwm_set_current_state(enum ltq_cpufreq_state e_state, int32_t flag);
#endif

/*!
   \brief before setting current ppa pwm state
   \param[in] e_state new state to set. Its value range: from IFX_PMCU_STATE_D0 to IFX_PMCU_STATE_D3
   \param[in] flag future usage only
   \return current status: IFX_SUCESS or IFX_FAILURE
   \ingroup IFX_PPA_API_PWM
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
extern int32_t ppa_pwm_pre_set_current_state(IFX_PMCU_STATE_t e_state, int32_t flag);
#else
extern int32_t ppa_pwm_pre_set_current_state(enum ltq_cpufreq_state e_state, int32_t flag);
#endif

/*!
   \brief after setting current ppa pwm state
   \param[in] e_state new state to set. Its value range: from IFX_PMCU_STATE_D0 to IFX_PMCU_STATE_D3
   \param[in] flag future usage only
   \return current status: IFX_SUCESS or IFX_FAILURE
   \ingroup IFX_PPA_API_PWM
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
extern int32_t ppa_pwm_post_set_current_state(IFX_PMCU_STATE_t e_state, int32_t flag);
#else
extern int32_t ppa_pwm_post_set_current_state(enum ltq_cpufreq_state e_state, int32_t flag);
#endif
/* @} */


#endif  //  __IFX_PPA_API_PWM_H__20091216_1952__
