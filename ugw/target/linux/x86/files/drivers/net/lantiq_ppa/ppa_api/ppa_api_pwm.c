/******************************************************************************
**
** FILE NAME    : ppa_api_pwm.c
** PROJECT      : UEIP
** MODULES      : PPA API (Power Management APIs)
**
** DATE         : 16 DEC 2009
** AUTHOR       : Shao Guohua
** DESCRIPTION  : PPA Protocol Stack Power Management API Implementation
** COPYRIGHT    :              Copyright (c) 2009
**                          Lantiq Deutschland GmbH
**                   Am Campeon 3; 85579 Neubiberg, Germany
**
**   For licensing information, see the file 'LICENSE' in the root folder of
**   this software module.
**
** HISTORY
** $Date        $Author              $Comment
** 16 DEC 2009  Shao Guohua          Initiate Version
*******************************************************************************/
/*!
  \defgroup IFX_PPA_API_PWM PPA API Power Management functions
  \ingroup IFX_PPA_API
  \brief IFX PPA API Power Management functions
*/

/*!
 \file ppa_api_pwm.c
 \ingroup IFX_PPA_API
 \brief source file for PPA API Power Management
*/
//#include <linux/kernel.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
#include <linux/autoconf.h>
#else
#include <generated/autoconf.h>
#endif

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <asm/time.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
  #include <asm/ifx/types.h>
  #include <asm/ifx/pmcu.h>
#endif

//PMCU specific Head File
#ifdef CONFIG_LTQ_CPU_FREQ
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 33)
  #include <ltq_lxfreq.h>
  #include <linux/cpufreq.h>
#else
  #include <cpufreq/ltq_cpufreq.h>
  #include <linux/cpufreq.h>
#endif
#endif

//PPA Specific Head File
#include <net/ppa_api.h>
#include <net/ppa_ppe_hal.h>
#include "ppa_api_netif.h"
#include "ppa_api_session.h"
#include "ppa_api_misc.h"
#include "ppe_drv_wrapper.h"
#include "ppa_datapath_wrapper.h"
#include "ppa_api_pwm_logic.h"

/* ============================= */
/* Function declaration          */
/* ============================= */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
static IFX_PMCU_RETURN_t ppa_pwm_state_set(IFX_PMCU_STATE_t pmcuState);
static IFX_PMCU_RETURN_t ppa_pwm_state_get(IFX_PMCU_STATE_t *pmcuState);
static IFX_PMCU_RETURN_t ppa_pwm_feature_switch(
	                               IFX_PMCU_PWR_STATE_ENA_t pmcuPwrStateEna);
static IFX_PMCU_RETURN_t  ppa_pwm_prechange(IFX_PMCU_MODULE_t pmcuModule, 
											IFX_PMCU_STATE_t newState, 
											IFX_PMCU_STATE_t oldState);
static IFX_PMCU_RETURN_t ppa_pwm_postchange(IFX_PMCU_MODULE_t pmcuModule, 
											IFX_PMCU_STATE_t newState, 
											IFX_PMCU_STATE_t oldState);
#else
static int ppa_pwm_feature_switch(int pmcuPwrStateEna);
static int ppa_pwm_state_set(enum ltq_cpufreq_state pmcuState);
static int ppa_pwm_state_get_cgs(enum ltq_cpufreq_state *pmcuState);
static int ppa_pwm_state_get_fss(enum ltq_cpufreq_state *pmcuState);
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
static int  ppa_pwm_prechange(enum ltq_cpufreq_module pmcuModule, 
								enum ltq_cpufreq_state newState, 
								enum ltq_cpufreq_state oldState,
								uint8_t flags);
static int ppa_pwm_postchange(enum ltq_cpufreq_module pmcuModule, 
								enum ltq_cpufreq_state newState, 
								enum ltq_cpufreq_state oldState,
								uint8_t flags);
/* threshold data for D0:D3 */
struct ltq_cpufreq_threshold *th_data = NULL;
#else
static int  ppa_pwm_prechange(enum ltq_cpufreq_module pmcuModule, 
								enum ltq_cpufreq_state newState, 
								enum ltq_cpufreq_state oldState);
static int ppa_pwm_postchange(enum ltq_cpufreq_module pmcuModule, 
								enum ltq_cpufreq_state newState, 
								enum ltq_cpufreq_state oldState);

#endif
#endif




#ifdef CONFIG_LTQ_CPU_FREQ
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
/* Linux CPUFREQ support start */
extern struct list_head ltq_lxfreq_head_mod_list_g;
#endif
static int
ppa_pwm_cpufreq_notifier(struct notifier_block *nb, unsigned long val, 
						 void *data);

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
struct LTQ_LXFREQ_MODSTRUCT ppa_pwm_lxfreq_mod_g = {
	.name							= "PPE clock gating support",
	.pmcuModule						= IFX_PMCU_MODULE_PPE,
	.pmcuModuleNr					= 0,
	.powerFeatureStat				= IFX_PMCU_PWR_STATE_ON,
	.ltq_lxfreq_state_get			= ppa_pwm_state_get,
	.ltq_lxfreq_pwr_feature_switch	= ppa_pwm_feature_switch,
};

static int ppa_pwm_cpufreq_notifier(struct notifier_block *nb, unsigned long val, 
								 void *data);
static struct notifier_block ppa_pwm_cpufreq_notifier_block = {
	.notifier_call  = ppa_pwm_cpufreq_notifier
};
#else
struct ltq_cpufreq_module_info ppa_pwr_feature_fss = {
	.name							= "PPE frequency scaling support",
	.pmcuModule						= LTQ_CPUFREQ_MODULE_PPE,
	.pmcuModuleNr					= 0,
	.powerFeatureStat				= 1,
	.ltq_cpufreq_state_get			= ppa_pwm_state_get_fss,
	.ltq_cpufreq_pwr_feature_switch	= NULL,
};

struct ltq_cpufreq_module_info ppa_pwr_feature_cgs = {
	.name							= "PPE clock gating support",
	.pmcuModule						= LTQ_CPUFREQ_MODULE_PPE,
	.pmcuModuleNr					= 0,
	.powerFeatureStat				= 1,
	.ltq_cpufreq_state_get			= ppa_pwm_state_get_cgs,
	.ltq_cpufreq_pwr_feature_switch	= ppa_pwm_feature_switch,
};
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
/* keep track of frequency transitions */
static int
ppa_pwm_cpufreq_notifier(struct notifier_block *nb, unsigned long val, void *data)
{
	struct cpufreq_freqs *freq = data;
	IFX_PMCU_STATE_t new_State,old_State;
	IFX_PMCU_RETURN_t ret;

	new_State = ltq_lxfreq_get_ps_from_khz(freq->new);
	if(new_State == IFX_PMCU_STATE_INVALID) {
		return NOTIFY_STOP_MASK | (IFX_PMCU_MODULE_PPE<<4);
	}
	old_State = ltq_lxfreq_get_ps_from_khz(freq->old);
	if(old_State == IFX_PMCU_STATE_INVALID) {
		return NOTIFY_STOP_MASK | (IFX_PMCU_MODULE_PPE<<4);
	}
	if (val == CPUFREQ_PRECHANGE){
		ret = ppa_pwm_prechange(IFX_PMCU_MODULE_PPE, new_State, old_State);
		if (ret == IFX_PMCU_RETURN_DENIED) {
			return NOTIFY_STOP_MASK | (IFX_PMCU_MODULE_PPE<<4);
		}
		ret = ppa_pwm_state_set(new_State);
		if (ret == IFX_PMCU_RETURN_DENIED) {
			return NOTIFY_STOP_MASK | (IFX_PMCU_MODULE_PPE<<4);
		}
	} else if (val == CPUFREQ_POSTCHANGE){
		ret = ppa_pwm_postchange(IFX_PMCU_MODULE_PPE, new_State, old_State);
		if (ret == IFX_PMCU_RETURN_DENIED) {
			return NOTIFY_STOP_MASK | (IFX_PMCU_MODULE_PPE<<4);
		}
	}else{
		return NOTIFY_OK | (IFX_PMCU_MODULE_PPE<<4);
	}
	return NOTIFY_OK | (IFX_PMCU_MODULE_PPE<<4);
}
#else
static int
ppa_pwm_cpufreq_notifier(struct notifier_block *nb, unsigned long val, void *data)
{
	struct cpufreq_freqs *freq = data;
	enum ltq_cpufreq_state new_State,old_State;
	int ret;

	new_State = ltq_cpufreq_get_ps_from_khz(freq->new);
	if(new_State == LTQ_CPUFREQ_PS_UNDEF) {
		return NOTIFY_STOP_MASK | (LTQ_CPUFREQ_MODULE_PPE<<4);
	}
	old_State = ltq_cpufreq_get_ps_from_khz(freq->old);
	if(old_State == LTQ_CPUFREQ_PS_UNDEF) {
		return NOTIFY_STOP_MASK | (LTQ_CPUFREQ_MODULE_PPE<<4);
	}
	if (val == CPUFREQ_PRECHANGE){
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
		ret = ppa_pwm_prechange(LTQ_CPUFREQ_MODULE_PPE, new_State, old_State, freq->flags);
#else
		ret = ppa_pwm_prechange(LTQ_CPUFREQ_MODULE_PPE, new_State, old_State);
#endif

		if (ret < 0) {
			return NOTIFY_STOP_MASK | (LTQ_CPUFREQ_MODULE_PPE<<4);
		}
		ret = ppa_pwm_state_set(new_State);
		if (ret < 0) {
			return NOTIFY_STOP_MASK | (LTQ_CPUFREQ_MODULE_PPE<<4);
		}
	} else if (val == CPUFREQ_POSTCHANGE){
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
		ret = ppa_pwm_postchange(LTQ_CPUFREQ_MODULE_PPE, new_State, old_State, freq->flags);
#else
		ret = ppa_pwm_postchange(LTQ_CPUFREQ_MODULE_PPE, new_State, old_State);
#endif
		if (ret < 0) {
			return NOTIFY_STOP_MASK | (LTQ_CPUFREQ_MODULE_PPE<<4);
		}
	}else{
		return NOTIFY_OK | (LTQ_CPUFREQ_MODULE_PPE<<4);
	}
	return NOTIFY_OK | (LTQ_CPUFREQ_MODULE_PPE<<4);
}
static struct notifier_block ppa_pwm_cpufreq_notifier_block = {
        .notifier_call  = ppa_pwm_cpufreq_notifier
};
#endif
#endif
/* Linux CPUFREQ support end */

#ifdef CONFIG_LTQ_PMCU
/* define dependency list;  state D0D3 means don't care */
/* static declaration is necessary to let gcc accept this static initialisation. */
static IFX_PMCU_MODULE_DEP_t depList=
{
    1,
    {
        {IFX_PMCU_MODULE_CPU, 0, IFX_PMCU_STATE_D0, IFX_PMCU_STATE_D0D3, IFX_PMCU_STATE_D0D3, IFX_PMCU_STATE_D0D3}
    }
};
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
static IFX_PMCU_PWR_STATE_ENA_t g_pwr_state_ena = IFX_PMCU_PWR_STATE_ON;
#else
static int g_pwr_state_ena = 1;
#endif
/*!
 \brief the callback function by pmcu to request PPA Power Mangement to change to new state
    \param pmcuState This parameter is a PMCU state.
    \return IFX_PMCU_RETURN_SUCCESS Set Power State successfully
    \return IFX_PMCU_RETURN_ERROR   Failed to set power state.
    \return IFX_PMCU_RETURN_DENIED  Not allowed to operate power state
    \ingroup IFX_PPA_API_PWM
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
static IFX_PMCU_RETURN_t ppa_pwm_state_set(IFX_PMCU_STATE_t pmcuState)
{
    if ( g_pwr_state_ena != IFX_PMCU_PWR_STATE_ON )
        return IFX_PMCU_RETURN_DENIED;

    if( ppa_pwm_set_current_state(pmcuState, 0) == IFX_SUCCESS )
        return IFX_PMCU_RETURN_SUCCESS;
    else
        return IFX_PMCU_RETURN_DENIED;
}
#else
static int ppa_pwm_state_set(enum ltq_cpufreq_state pmcuState)
{
    if ( g_pwr_state_ena != 1 )
        return -1;

    if( ppa_pwm_set_current_state(pmcuState, 0) == PPA_SUCCESS )
        return 0;
    else
        return -1;
}
#endif


/*!
 \brief the callback function by pmcu to get PPA current Power management state
    \param pmcuState Pointer to return power state.
    \return IFX_PMCU_RETURN_SUCCESS Set Power State successfully
    \return IFX_PMCU_RETURN_ERROR   Failed to set power state.
    \return IFX_PMCU_RETURN_DENIED  Not allowed to operate power state
    \ingroup IFX_PPA_API_PWM
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
static IFX_PMCU_RETURN_t ppa_pwm_state_get(IFX_PMCU_STATE_t *pmcuState)
{
    if( pmcuState )
    {
        *pmcuState = ppa_pwm_get_current_status(0);
    }

    return IFX_PMCU_RETURN_SUCCESS;
}
#else
static int ppa_pwm_state_get_cgs(enum ltq_cpufreq_state *pmcuState)
{
    if( pmcuState )
    {
        *pmcuState = ppa_pwm_get_current_status(0);
    }

    return 0;
}
/*!
 \brief the callback function by pmcu to get PPA frequency state
    \param pmcuState Pointer to return power state.
    \ingroup IFX_PPA_API_PWM
 */
static int ppa_pwm_state_get_fss(enum ltq_cpufreq_state *pmcuState)
{
    if( pmcuState )
    {
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
		*pmcuState = ppa_pwm_get_current_status(0);
#else
        /*todo: here you should return the current state of the frequency scaling*/
		*pmcuState = LTQ_CPUFREQ_PS_UNDEF;
#endif
    }

    return 0;
}
#endif


/*!
 \brief callback function by pmcu before a state change
    \param   pmcuModule      Module
    \param   newState        New state
    \param   oldState        Old state
    \return IFX_PMCU_RETURN_SUCCESS Set Power State successfully
    \return IFX_PMCU_RETURN_ERROR   Failed to set power state.
    \return IFX_PMCU_RETURN_DENIED  Not allowed to operate power state
    \ingroup IFX_PPA_API_PWM
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
static IFX_PMCU_RETURN_t  ppa_pwm_prechange(IFX_PMCU_MODULE_t pmcuModule, IFX_PMCU_STATE_t newState, IFX_PMCU_STATE_t oldState)
{
    if ( g_pwr_state_ena != IFX_PMCU_PWR_STATE_ON )
        return IFX_PMCU_RETURN_DENIED;

    if ( ppa_pwm_pre_set_current_state(newState, 0) == IFX_SUCCESS )
        return IFX_PMCU_RETURN_SUCCESS;
    else
        return IFX_PMCU_RETURN_DENIED;
}
#else
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
static int  ppa_pwm_prechange(enum ltq_cpufreq_module pmcuModule, enum ltq_cpufreq_state newState, enum ltq_cpufreq_state oldState, uint8_t flags)
#else
static int  ppa_pwm_prechange(enum ltq_cpufreq_module pmcuModule, enum ltq_cpufreq_state newState, enum ltq_cpufreq_state oldState)
#endif
{
    if ( g_pwr_state_ena != 1 )
        return -1;

    if ( ppa_pwm_pre_set_current_state(newState, 0) == PPA_SUCCESS )
        return 0;
    else
        return -1;
}
#endif

/*!
 \brief callback function by pmcu after a state change
    \param   pmcuModule      Module
    \param   newState        New state
    \param   oldState        Old state
    \return IFX_PMCU_RETURN_SUCCESS Set Power State successfully
    \return IFX_PMCU_RETURN_ERROR   Failed to set power state.
    \ingroup IFX_PPA_API_PWM
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
static IFX_PMCU_RETURN_t ppa_pwm_postchange(IFX_PMCU_MODULE_t pmcuModule, IFX_PMCU_STATE_t newState, IFX_PMCU_STATE_t oldState)
{
    if ( g_pwr_state_ena != IFX_PMCU_PWR_STATE_ON )
        return IFX_PMCU_RETURN_DENIED;

    if ( ppa_pwm_post_set_current_state(newState, 0) == IFX_SUCCESS )
        return IFX_PMCU_RETURN_SUCCESS;
    else
        return IFX_PMCU_RETURN_DENIED;
}
#else
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
static int ppa_pwm_postchange(enum ltq_cpufreq_module pmcuModule, enum ltq_cpufreq_state newState, enum ltq_cpufreq_state oldState, uint8_t flags)
#else
static int ppa_pwm_postchange(enum ltq_cpufreq_module pmcuModule, enum ltq_cpufreq_state newState, enum ltq_cpufreq_state oldState)
#endif
{
    if ( g_pwr_state_ena != 1 )
        return -1;

    if ( ppa_pwm_post_set_current_state(newState, 0) == PPA_SUCCESS )
        return 0;
    else
        return -1;
}
#endif

/*!
 \brief callback function by pmcu to enable/disable power saving feature
    \param   pmcuPwrStateEna        New state.
    \return IFX_PMCU_RETURN_SUCCESS Enable/disable power saving feature successfully.
    \return IFX_PMCU_RETURN_ERROR   Failed to enable/disable power saving feature.
    \ingroup IFX_PPA_API_PWM
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
static IFX_PMCU_RETURN_t ppa_pwm_feature_switch(IFX_PMCU_PWR_STATE_ENA_t pmcuPwrStateEna)
{
    if ( g_pwr_state_ena != IFX_PMCU_PWR_STATE_ON && pmcuPwrStateEna == IFX_PMCU_PWR_STATE_ON )
    {
        ppa_drv_ppe_pwm_change(1, 1);   //  turn on clock gating
        g_pwr_state_ena = pmcuPwrStateEna;
    }
    if ( g_pwr_state_ena != IFX_PMCU_PWR_STATE_OFF && pmcuPwrStateEna == IFX_PMCU_PWR_STATE_OFF )
    {
        ppa_drv_ppe_pwm_change(0, 1);   //  turn off clock gating
        if ( ppa_pwm_get_current_status(0) != IFX_PMCU_STATE_D0
            && ppa_pwm_prechange(IFX_PMCU_MODULE_PPE, IFX_PMCU_STATE_D0, ppa_pwm_get_current_status(0)) == IFX_PMCU_RETURN_SUCCESS
            && ppa_pwm_state_set(IFX_PMCU_STATE_D0) == IFX_PMCU_RETURN_SUCCESS
            && ppa_pwm_postchange(IFX_PMCU_MODULE_PPE, IFX_PMCU_STATE_D0, ppa_pwm_get_current_status(0)) == IFX_PMCU_RETURN_SUCCESS )
        {
        }
        g_pwr_state_ena = pmcuPwrStateEna;
    }
    return IFX_PMCU_RETURN_SUCCESS;
}
#else
static int ppa_pwm_feature_switch(int pmcuPwrStateEna)
{
    if ( g_pwr_state_ena != 1 && pmcuPwrStateEna == 1 )
    {
        ppa_drv_ppe_pwm_change(1, 1);   //  turn on clock gating
        g_pwr_state_ena = pmcuPwrStateEna;
    }
    if ( g_pwr_state_ena != 0 && pmcuPwrStateEna == 0 )
    {
        ppa_drv_ppe_pwm_change(0, 1);   //  turn off clock gating
        if ( ppa_pwm_get_current_status(0) != LTQ_CPUFREQ_PS_D0
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
            && ppa_pwm_prechange(LTQ_CPUFREQ_MODULE_PPE, LTQ_CPUFREQ_PS_D0, ppa_pwm_get_current_status(0), 0) == 0
#else
            && ppa_pwm_prechange(LTQ_CPUFREQ_MODULE_PPE, LTQ_CPUFREQ_PS_D0, ppa_pwm_get_current_status(0)) == 0
#endif
            && ppa_pwm_state_set(LTQ_CPUFREQ_PS_D0) == 0
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
            && ppa_pwm_postchange(LTQ_CPUFREQ_MODULE_PPE, LTQ_CPUFREQ_PS_D0, ppa_pwm_get_current_status(0), 0) == 0 )
#else
            && ppa_pwm_postchange(LTQ_CPUFREQ_MODULE_PPE, LTQ_CPUFREQ_PS_D0, ppa_pwm_get_current_status(0)) == 0 )
#endif
        {
        }
        g_pwr_state_ena = pmcuPwrStateEna;
    }
    return 0;
}
#endif


/*!
      \brief Request D0 power state when any session is added into table.
       \return none
       \ingroup IFX_PPA_API_PWM
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
void ppa_pwm_activate_module(void)
{
#ifdef CONFIG_LTQ_PMCU
	if ( ppa_pwm_get_current_status(0) != IFX_PMCU_STATE_D0 )
        ifx_pmcu_state_req(IFX_PMCU_MODULE_PPE, 0, IFX_PMCU_STATE_D0);
#endif
#ifdef CONFIG_LTQ_CPU_FREQ
	ltq_lxfreq_state_req(IFX_PMCU_MODULE_PPE, 0, IFX_PMCU_STATE_D0);
#endif

}
#else
void ppa_pwm_activate_module(void)
{
#ifdef CONFIG_LTQ_PMCU
	if ( ppa_pwm_get_current_status(0) != LTQ_CPUFREQ_PS_D0 )
        ifx_pmcu_state_req(IFX_PMCU_MODULE_PPE, 0, LTQ_CPUFREQ_PS_D0);
#endif
#ifdef CONFIG_LTQ_CPU_FREQ
#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
	if ( ppa_pwm_get_current_status(0) != LTQ_CPUFREQ_PS_D0 )
        {
                printk("D3->D0\n");
                ltq_cpufreq_state_req(LTQ_CPUFREQ_MODULE_PPE, 0, LTQ_CPUFREQ_PS_D0);
        }
#else
	ltq_cpufreq_state_req(LTQ_CPUFREQ_MODULE_PPE, 0, LTQ_CPUFREQ_PS_D0);
#endif

#endif

}
#endif
/*!
      \brief Request D3 power state when any session is removed from table.
       \return none
       \ingroup IFX_PPA_API_PWM
*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
void ppa_pwm_deactivate_module(void)
{
#ifdef CONFIG_LTQ_PMCU
    if ( g_pwr_state_ena == IFX_PMCU_PWR_STATE_ON && ppa_pwm_get_current_status(0) != IFX_PMCU_STATE_D3 && ppa_pwm_pre_set_current_state(IFX_PMCU_STATE_D3, 0) == IFX_SUCCESS )
        ifx_pmcu_state_req(IFX_PMCU_MODULE_PPE, 0, IFX_PMCU_STATE_D3);
#endif
#ifdef CONFIG_LTQ_CPU_FREQ
		 ltq_lxfreq_state_req(IFX_PMCU_MODULE_PPE, 0, IFX_PMCU_STATE_D3);
#endif
}
#else
void ppa_pwm_deactivate_module(void)
{
#ifdef CONFIG_LTQ_PMCU
    if ( g_pwr_state_ena == 1 && ppa_pwm_get_current_status(0) != LTQ_CPUFREQ_PS_D3 && ppa_pwm_pre_set_current_state(LTQ_CPUFREQ_PS_D3, 0) == PPA_SUCCESS )
        ifx_pmcu_state_req(LTQ_CPUFREQ_MODULE_PPE, 0, LTQ_CPUFREQ_PS_D3);
#endif
#ifdef CONFIG_LTQ_CPU_FREQ
		 ltq_cpufreq_state_req(LTQ_CPUFREQ_MODULE_PPE, 0, LTQ_CPUFREQ_PS_D3);
#endif
}
#endif
/*!
      \brief Init PPA Power management
       \return none
       \ingroup IFX_PPA_API_PWM
*/
void ppa_pwm_init(void)
{
#ifdef CONFIG_LTQ_PMCU
	IFX_PMCU_REGISTER_t pmcuRegister;
    ppa_pwm_logic_init();

    memset(&pmcuRegister, 0, sizeof(pmcuRegister));

    pmcuRegister.pmcuModule     = IFX_PMCU_MODULE_PPE;
    pmcuRegister.pmcuModuleNr   = 0;
    pmcuRegister.pmcuModuleDep  = &depList;

    /* Register callback function for PMCU*/
    pmcuRegister.pre                        = ppa_pwm_prechange;
    pmcuRegister.ifx_pmcu_state_change      = ppa_pwm_state_set;
    pmcuRegister.post                       = ppa_pwm_postchange;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
    pmcuRegister.ifx_pmcu_state_get         = ppa_pwm_state_get;
#else
    pmcuRegister.ifx_pmcu_state_get         = ppa_pwm_state_get_cgs;
#endif
    pmcuRegister.ifx_pmcu_pwr_feature_switch= ppa_pwm_feature_switch;

    if( ifx_pmcu_register ( &pmcuRegister ) != IFX_PMCU_RETURN_SUCCESS )
    {
        ppa_debug(DBG_ENABLE_MASK_ERR, "ppa pwm failed to ifx_pmcu_register\n");
        return ;
    }

#endif

#ifdef CONFIG_LTQ_CPU_FREQ
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)

    if ( cpufreq_register_notifier(&ppa_pwm_cpufreq_notifier_block,CPUFREQ_TRANSITION_NOTIFIER) ) {
        ppa_debug(DBG_ENABLE_MASK_ERR, "Fail in registering ppa pwm to CPUFREQ\n");                
        return;                                                                                    
    }                                                                                              
    list_add_tail(&ppa_pwm_lxfreq_mod_g.list, &ltq_lxfreq_head_mod_list_g);                        
#else
	struct ltq_cpufreq* ppa_pwm_cpufreq_p;
    if ( cpufreq_register_notifier(&ppa_pwm_cpufreq_notifier_block,CPUFREQ_TRANSITION_NOTIFIER) ) {
        ppa_debug(DBG_ENABLE_MASK_ERR, "Fail in registering ppa pwm to CPUFREQ\n");                
        return;                                                                                    
    }                                                                                              
    ppa_pwm_cpufreq_p = ltq_cpufreq_get();

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
	ltq_cpufreq_mod_list(&ppa_pwr_feature_fss.list, LTQ_CPUFREQ_LIST_ADD);
#else
	list_add_tail(&ppa_pwr_feature_fss.list, &ppa_pwm_cpufreq_p->list_head_module);                        
	list_add_tail(&ppa_pwr_feature_cgs.list, &ppa_pwm_cpufreq_p->list_head_module);                        
#endif

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
	th_data = ltq_cpufreq_get_threshold(LTQ_CPUFREQ_MODULE_PPE, 0);
	if (th_data == NULL)
        	ppa_debug(DBG_ENABLE_MASK_ERR, "No PS related threshold values are defined\n");                
#endif


#endif                                                                                                   
#endif

	ppa_debug(DBG_ENABLE_MASK_PWM, "ppa pwm init ok !\n");
}


/*!
      \brief Exit PPA Power management
       \return none
       \ingroup IFX_PPA_API_PWM
*/
void ppa_pwm_exit(void)
{
#ifdef CONFIG_LTQ_PMCU
    IFX_PMCU_REGISTER_t pmcuUnRegister;

    memset (&pmcuUnRegister, 0, sizeof(pmcuUnRegister));
    pmcuUnRegister.pmcuModule   = IFX_PMCU_MODULE_PPE;
    pmcuUnRegister.pmcuModuleNr = 0;
    ifx_pmcu_unregister(&pmcuUnRegister);
#endif

#ifdef CONFIG_LTQ_CPU_FREQ
    if ( cpufreq_unregister_notifier(&ppa_pwm_cpufreq_notifier_block,CPUFREQ_TRANSITION_NOTIFIER) ) {
    ppa_debug(DBG_ENABLE_MASK_ERR, "Fail in unregistering ppa pwm from CPUFREQ\n");                  
    }                                                                                                
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
    list_del(&ppa_pwm_lxfreq_mod_g.list);
#else

#if defined(CONFIG_LTQ_PPA_GRX500) && CONFIG_LTQ_PPA_GRX500
	list_del(&ppa_pwr_feature_fss.list);                                                            
#else
    list_del(&ppa_pwr_feature_cgs.list);                                                            
    list_del(&ppa_pwr_feature_fss.list);                                                            
#endif
#endif                                                            
#endif                                                                                               

	ppa_debug(DBG_ENABLE_MASK_PWM, "ppa pwm exit !\n");
}
