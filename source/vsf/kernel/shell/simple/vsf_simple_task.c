/*****************************************************************************
 *   Copyright(C)2009-2019 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/


/*============================ INCLUDES ======================================*/
#include "kernel/vsf_kernel_cfg.h"


#if VSF_USE_KERNEL_SIMPLE_SHELL == ENABLED && VSF_USE_KERNEL == ENABLED
#include "../../vsf_kernel_common.h"
#include "./vsf_simple.h"
#include "../../task/vsf_thread.h"
/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

SECTION(".text.vsf.kernel.vsf_eda_polling_state_get")
extern bool vsf_eda_polling_state_get(vsf_eda_t *peda);

SECTION(".text.vsf.kernel.vsf_eda_polling_state_set")
extern void vsf_eda_polling_state_set(vsf_eda_t *peda, bool state);

/*============================ IMPLEMENTATION ================================*/

#if VSF_KERNEL_CFG_EDA_SUPPORT_TIMER == ENABLED
SECTION("text.vsf.kernel.__vsf_delay")
vsf_evt_t __vsf_delay(uint_fast32_t tick)
{
    vsf_evt_t result = VSF_EVT_INVALID;
    enum {
        VSF_APP_STATE_SET_TIMER = 0,
        VSF_APP_STATE_WAIT_TIMER_EVT,
    };
    if (0 == tick) {
        return VSF_EVT_TIMER;
    }
    vsf_teda_t *pteda = (vsf_teda_t *)vsf_eda_get_cur();
    VSF_KERNEL_ASSERT(NULL != pteda);

#   if VSF_KERNEL_CFG_SUPPORT_THREAD == ENABLED
    if (vsf_eda_is_stack_owner(&(pteda->use_as__vsf_eda_t))) {
        vsf_thread_delay(tick);
        result = vsf_eda_get_cur_evt();
    } else 
#   endif
    {
        if (vsf_eda_polling_state_get(&(pteda->use_as__vsf_eda_t))) {
            /* VSF_APP_STATE_WAIT_TIMER_EVT */
            result = vsf_eda_get_cur_evt();
            if (result == VSF_EVT_TIMER) {
                vsf_eda_polling_state_set(  &(pteda->use_as__vsf_eda_t),
                                            (bool)VSF_APP_STATE_SET_TIMER);
            }
        } else  {
            /*VSF_APP_STATE_SET_TIMER*/
            vsf_teda_set_timer(tick);
            vsf_eda_polling_state_set(  &(pteda->use_as__vsf_eda_t),
                                        (bool)VSF_APP_STATE_WAIT_TIMER_EVT);
        }
    }
    
    return result;
}

#endif

SECTION("text.vsf.kernel.__vsf_sem_pend")
vsf_sync_reason_t __vsf_sem_pend(vsf_sem_t *psem, int_fast32_t time_out)
{
    vsf_sync_reason_t result = VSF_SYNC_PENDING;
    vsf_err_t err;
    enum {
        VSF_APP_STATE_PENDING = 0,
        VSF_APP_STATE_WAIT_PENDING,
    };
    
    vsf_teda_t *pteda = (vsf_teda_t *)vsf_eda_get_cur();
    VSF_KERNEL_ASSERT(NULL != pteda);
    
    do {
#if VSF_KERNEL_CFG_SUPPORT_THREAD == ENABLED
        if (vsf_eda_is_stack_owner(&(pteda->use_as__vsf_eda_t))) {
            result = vsf_thread_sem_pend(psem, time_out);
        } else 
#endif
        {
            if (vsf_eda_polling_state_get(&(pteda->use_as__vsf_eda_t))) {
                /* VSF_APP_STATE_WAIT_PENDING */
                result = vsf_eda_sync_get_reason(psem, vsf_eda_get_cur_evt());
                if (result != VSF_SYNC_PENDING) {
                    vsf_eda_polling_state_set(  &(pteda->use_as__vsf_eda_t),
                                                (bool)VSF_APP_STATE_PENDING);
                }
            } else {
                /* VSF_APP_STATE_PENDING */
                err = vsf_eda_sem_pend(psem, time_out);
                if (!err) { 
                    result = VSF_SYNC_GET; 
                    break;
                } else if (err < 0) { 
                    result = VSF_SYNC_FAIL; 
                    break;
                } 
                vsf_eda_polling_state_set(  &(pteda->use_as__vsf_eda_t),
                                            (bool)VSF_APP_STATE_WAIT_PENDING);
            }
        }
    } while(false);
    
    return result;
}

SECTION("text.vsf.kernel.vsf_mutex_enter")
vsf_sync_reason_t __vsf_mutex_enter(vsf_mutex_t *pmtx, int_fast32_t time_out)
{
    VSF_KERNEL_ASSERT(NULL != pmtx);
    return __vsf_sem_pend(&(pmtx->use_as__vsf_sync_t), time_out);
}

SECTION("text.vsf.kernel.vsf_yield")
vsf_evt_t __vsf_yield(void)
{    
    vsf_evt_t result = VSF_EVT_YIELD;
    vsf_eda_t *eda = vsf_eda_get_cur();
    VSF_KERNEL_ASSERT(NULL != eda);

    enum {
        VSF_APP_STATE_YIELD = 0,
        VSF_APP_STATE_WAIT_EVENT,
    };

#if VSF_KERNEL_CFG_SUPPORT_THREAD == ENABLED
    if (vsf_eda_is_stack_owner(eda)) {
        vsf_eda_yield();
        vsf_thread_wait();          //!< wait for any evt
    } else 
#endif
    {
        if (vsf_eda_polling_state_get(eda)) {
            /* VSF_APP_STATE_WAIT_EVENT */
            vsf_eda_polling_state_set(eda, VSF_APP_STATE_YIELD);
        } else {
            vsf_eda_yield();
            /* VSF_APP_STATE_YIELD */
            vsf_eda_polling_state_set(eda, VSF_APP_STATE_WAIT_EVENT);
            result = VSF_EVT_NONE;
        }
    }
    return result;
}

#endif
/*EOF*/
