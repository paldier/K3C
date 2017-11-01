/******************************************************************************
**
** FILE NAME    : ltq_ppa_cpu_freq.c
** PROJECT      : UEIP
** MODULES      : PPA API (Power Management APIs)
**
** DATE         : 16 OCT 2015
** AUTHOR       : Purnendu Ghosh
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
** 16 OCT 2015  Purnendu Ghosh       Initiate Version
*******************************************************************************/
/*!
  \defgroup IFX_PPA_API_PWM PPA API Power Management functions
  \ingroup IFX_PPA_API
  \brief IFX PPA API Power Management functions
*/

/*!
 \file ltq_ppa_cpu_freq.c
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

//CPU FREQ specific Head File
#include <cpufreq/ltq_cpufreq.h>
#include <linux/cpufreq.h>

//PPA Specific Head File
#include <net/ppa_api.h>
#include <net/ppa_ppe_hal.h>
#include "ppa_api_netif.h"
#include "ppa_api_session.h"
#include "ppa_api_misc.h"
#include "ppe_drv_wrapper.h"
#include "ppa_datapath_wrapper.h"
#include "ppa_api_cpu_freq.h"

/* ============================= */
/* Function declaration          */
/* ============================= */

static int ppa_api_cpufreq_feature_switch(int pmcuPwrStateEna);
static int ppa_api_cpufreq_state_set(enum ltq_cpufreq_state pmcuState);
static int ppa_api_cpufreq_state_get_cgs(enum ltq_cpufreq_state *pmcuState);
static int ppa_api_cpufreq_state_get_fss(enum ltq_cpufreq_state *pmcuState);

static int  ppa_api_cpufreq_prechange(enum ltq_cpufreq_module pmcuModule, 
								enum ltq_cpufreq_state newState, 
								enum ltq_cpufreq_state oldState,
								uint8_t flags);
static int ppa_api_cpufreq_postchange(enum ltq_cpufreq_module pmcuModule, 
								enum ltq_cpufreq_state newState, 
								enum ltq_cpufreq_state oldState,
								uint8_t flags);
/* threshold data for D0:D3 */
struct ltq_cpufreq_threshold *cpufreq_thresh_data = NULL;

static enum ltq_cpufreq_state ppa_cpufreq_stat_curr   = LTQ_CPUFREQ_PS_D0;

static int
ppa_api_cpufreq_notifier(struct notifier_block *nb, unsigned long val, void *data);

struct ltq_cpufreq_module_info ppa_cpufreq_feature_fss = {
	.name							= "PPA frequency scaling support",
	.pmcuModule						= LTQ_CPUFREQ_MODULE_PPE,
	.pmcuModuleNr					= 0,
	.powerFeatureStat				= 1,
	.ltq_cpufreq_state_get			= ppa_api_cpufreq_state_get_fss,
	.ltq_cpufreq_pwr_feature_switch	= NULL,
};



enum ltq_cpufreq_state ppa_api_cpufreq_get_current_status(int32_t flag)
{
	return ppa_cpufreq_stat_curr;
}

int32_t ppa_api_cpufreq_set_current_state(enum ltq_cpufreq_state e_state, int32_t flag)
{
	return PPA_SUCCESS;
}

/**====================================
//currently, we only accept D0( fully on) and D3 (off) states. We take D1 & D2 as D3
//int32_t inline ppa_api_cpufreq_turn_state_on(IFX_PMCU_STATE_t cur_state, IFX_PMCU_STATE_t new_state)
//{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 10, 7)
//    return (cur_state != IFX_PMCU_STATE_D0 && new_state == IFX_PMCU_STATE_D0);
#else
//    return (cur_state != LTQ_CPUFREQ_PS_D0 && new_state == LTQ_CPUFREQ_PS_D0);
#endif
//}
==================================== */

int32_t inline ppa_api_cpufreq_turn_state_off(enum ltq_cpufreq_state cur_state, enum ltq_cpufreq_state new_state)
{
	return (cur_state == LTQ_CPUFREQ_PS_D0 && new_state != LTQ_CPUFREQ_PS_D0);
}

int32_t ppa_api_cpufreq_pre_set_current_state(enum ltq_cpufreq_state e_state, int32_t flag)
{
	if(ppa_api_cpufreq_turn_state_off(ppa_cpufreq_stat_curr, e_state)){
		return (ppa_get_hw_session_cnt() > 0 ? PPA_FAILURE : PPA_SUCCESS);
	}

	return PPA_SUCCESS;
}

int32_t ppa_api_cpufreq_post_set_current_state(enum ltq_cpufreq_state e_state, int32_t flag)
{
	unsigned long sys_flag;

	local_irq_save(sys_flag);
	if ( ppa_cpufreq_stat_curr != e_state )
	{
		ppa_cpufreq_stat_curr = e_state;
	}
	local_irq_restore(sys_flag);

	return PPA_SUCCESS;
}

static int
ppa_api_cpufreq_notifier(struct notifier_block *nb, unsigned long val, void *data)
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

		ret = ppa_api_cpufreq_prechange(LTQ_CPUFREQ_MODULE_PPE, new_State, old_State, freq->flags);

		if (ret < 0) {
			return NOTIFY_STOP_MASK | (LTQ_CPUFREQ_MODULE_PPE<<4);
		}
		ret = ppa_api_cpufreq_state_set(new_State);
		if (ret < 0) {
			return NOTIFY_STOP_MASK | (LTQ_CPUFREQ_MODULE_PPE<<4);
		}
	} else if (val == CPUFREQ_POSTCHANGE){

		ret = ppa_api_cpufreq_postchange(LTQ_CPUFREQ_MODULE_PPE, new_State, old_State, freq->flags);

		if (ret < 0) {
			return NOTIFY_STOP_MASK | (LTQ_CPUFREQ_MODULE_PPE<<4);
		}
	}else{
		return NOTIFY_OK | (LTQ_CPUFREQ_MODULE_PPE<<4);
	}
	return NOTIFY_OK | (LTQ_CPUFREQ_MODULE_PPE<<4);
}

static struct notifier_block ppa_api_cpufreq_notifier_block = {
        .notifier_call  = ppa_api_cpufreq_notifier
};


static int g_pwr_state_ena = 1;

/*!
 \brief the callback function by pmcu to request PPA Power Mangement to change to new state
    \param pmcuState This parameter is a PMCU state.
    \return IFX_PMCU_RETURN_SUCCESS Set Power State successfully
    \return IFX_PMCU_RETURN_ERROR   Failed to set power state.
    \return IFX_PMCU_RETURN_DENIED  Not allowed to operate power state
    \ingroup IFX_PPA_API_PWM
 */
static int ppa_api_cpufreq_state_set(enum ltq_cpufreq_state pmcuState)
{
    if ( g_pwr_state_ena != 1 )
        return -1;

    if( ppa_api_cpufreq_set_current_state(pmcuState, 0) == PPA_SUCCESS )
        return 0;
    else
        return -1;
}


/*!
 \brief the callback function by pmcu to get PPA current Power management state
    \param pmcuState Pointer to return power state.
    \return IFX_PMCU_RETURN_SUCCESS Set Power State successfully
    \return IFX_PMCU_RETURN_ERROR   Failed to set power state.
    \return IFX_PMCU_RETURN_DENIED  Not allowed to operate power state
    \ingroup IFX_PPA_API_PWM
 */
static int ppa_api_cpufreq_state_get_cgs(enum ltq_cpufreq_state *pmcuState)
{
	if( pmcuState )
	{
		*pmcuState = ppa_api_cpufreq_get_current_status(0);
	}

	return 0;
}
/*!
 \brief the callback function by pmcu to get PPA frequency state
    \param pmcuState Pointer to return power state.
    \ingroup IFX_PPA_API_PWM
 */
static int ppa_api_cpufreq_state_get_fss(enum ltq_cpufreq_state *pmcuState)
{
    if( pmcuState )
    {
	*pmcuState = ppa_api_cpufreq_get_current_status(0);
    }

    return 0;
}


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
static int  ppa_api_cpufreq_prechange(enum ltq_cpufreq_module pmcuModule, enum ltq_cpufreq_state newState, enum ltq_cpufreq_state oldState, uint8_t flags)
{
	//printk("CPUFREQ_PRECHANGE \n");
	if ( g_pwr_state_ena != 1 )
		return -1;

	if ( ppa_api_cpufreq_pre_set_current_state(newState, 0) == PPA_SUCCESS )
		return 0;
	else
		return -1;
}

/*!
 \brief callback function by pmcu after a state change
    \param   pmcuModule      Module
    \param   newState        New state
    \param   oldState        Old state
    \return IFX_PMCU_RETURN_SUCCESS Set Power State successfully
    \return IFX_PMCU_RETURN_ERROR   Failed to set power state.
    \ingroup IFX_PPA_API_PWM
 */
static int ppa_api_cpufreq_postchange(enum ltq_cpufreq_module pmcuModule, enum ltq_cpufreq_state newState, enum ltq_cpufreq_state oldState, uint8_t flags)

{
	//printk("CPUFREQ_POSTCHANGE \n");
	if ( g_pwr_state_ena != 1 )
		return -1;

	if ( ppa_api_cpufreq_post_set_current_state(newState, 0) == PPA_SUCCESS )
		return 0;
	else
		return -1;
}


/*!
 \brief callback function by pmcu to enable/disable power saving feature
    \param   pmcuPwrStateEna        New state.
    \return IFX_PMCU_RETURN_SUCCESS Enable/disable power saving feature successfully.
    \return IFX_PMCU_RETURN_ERROR   Failed to enable/disable power saving feature.
    \ingroup IFX_PPA_API_PWM
 */

static int ppa_api_cpufreq_feature_switch(int pmcuPwrStateEna)
{
	if ( g_pwr_state_ena != 1 && pmcuPwrStateEna == 1 )
	{
		ppa_drv_ppe_pwm_change(1, 1);   //  turn on clock gating
		g_pwr_state_ena = pmcuPwrStateEna;
	}
	if ( g_pwr_state_ena != 0 && pmcuPwrStateEna == 0 )
	{
		ppa_drv_ppe_pwm_change(0, 1);   //  turn off clock gating
		if ( ppa_api_cpufreq_get_current_status(0) != LTQ_CPUFREQ_PS_D0

				&& ppa_api_cpufreq_prechange(LTQ_CPUFREQ_MODULE_PPE, LTQ_CPUFREQ_PS_D0, ppa_api_cpufreq_get_current_status(0), 0) == 0
				&& ppa_api_cpufreq_state_set(LTQ_CPUFREQ_PS_D0) == 0
				&& ppa_api_cpufreq_postchange(LTQ_CPUFREQ_MODULE_PPE, LTQ_CPUFREQ_PS_D0, ppa_api_cpufreq_get_current_status(0), 0) == 0 )
		{
		}
		g_pwr_state_ena = pmcuPwrStateEna;
	}
	return 0;
}

/*!
      \brief Request D0 power state when any session is added into table.
       \return none
       \ingroup IFX_PPA_API_PWM
*/

void ppa_api_cpufreq_activate_module(void)
{
	if ( ppa_api_cpufreq_get_current_status(0) != LTQ_CPUFREQ_PS_D0 )
        {
                printk("D3->D0\n");
                ltq_cpufreq_state_req(LTQ_CPUFREQ_MODULE_PPE, 0, LTQ_CPUFREQ_PS_D0);
        }
}

/*!
      \brief Request D3 power state when any session is removed from table.
       \return none
       \ingroup IFX_PPA_API_PWM
*/

void ppa_api_cpufreq_deactivate_module(void)
{
	 ltq_cpufreq_state_req(LTQ_CPUFREQ_MODULE_PPE, 0, LTQ_CPUFREQ_PS_D3);
}

/*!
      \brief Init PPA Power management
       \return none
       \ingroup IFX_PPA_API_PWM
*/
void ppa_api_cpufreq_init(void)
{
	struct ltq_cpufreq* ppa_api_cpufreq_cpufreq_p;
	ppa_api_cpufreq_cpufreq_p = ltq_cpufreq_get();

	if (ppa_api_cpufreq_cpufreq_p == NULL) {
                ppa_debug(DBG_ENABLE_MASK_ERR, "CPUFREQ registration failed.\n");
                return PPA_FAILURE;
        }

	if ( cpufreq_register_notifier(&ppa_api_cpufreq_notifier_block,CPUFREQ_TRANSITION_NOTIFIER) ) {
		ppa_debug(DBG_ENABLE_MASK_ERR, "Fail in registering ppa pwm to CPUFREQ\n");                
		return;                                                                                    
	}                                                                                              
	ltq_cpufreq_mod_list(&ppa_cpufreq_feature_fss.list, LTQ_CPUFREQ_LIST_ADD);

	cpufreq_thresh_data = ltq_cpufreq_get_threshold(LTQ_CPUFREQ_MODULE_PPE, 0);
	if (cpufreq_thresh_data == NULL)
		ppa_debug(DBG_ENABLE_MASK_ERR, "No PS related threshold values are defined\n");                

	ppa_debug(DBG_ENABLE_MASK_PWM, "ppa pwm init ok !\n");
}


/*!
      \brief Exit PPA Power management
       \return none
       \ingroup IFX_PPA_API_PWM
*/
void ppa_api_cpufreq_exit(void)
{
	if ( cpufreq_unregister_notifier(&ppa_api_cpufreq_notifier_block,CPUFREQ_TRANSITION_NOTIFIER) ) {
		ppa_debug(DBG_ENABLE_MASK_ERR, "Fail in unregistering ppa pwm from CPUFREQ\n");                  
	}                                                                                                

	list_del(&ppa_cpufreq_feature_fss.list);                                                            

	ppa_debug(DBG_ENABLE_MASK_PWM, "ppa pwm exit !\n");
}
