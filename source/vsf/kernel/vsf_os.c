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
#include "./vsf_kernel_cfg.h"

#if VSF_USE_KERNEL == ENABLED

#include "./vsf_kernel_common.h"
#include "./vsf_eda.h"
#include "./vsf_evtq.h"
#include "./vsf_os.h"
#include "./task/vsf_task.h"
#include "service/vsf_service.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

struct __vsf_os_t {
#if __VSF_KERNEL_CFG_EVTQ_EN == ENABLED
#   if defined(__VSF_OS_CFG_EVTQ_LIST)
    vsf_pool(vsf_evt_node_pool) node_pool;
#   endif
#endif
    const vsf_kernel_resource_t *res_ptr;
};

typedef struct __vsf_os_t __vsf_os_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
static NO_INIT __vsf_os_t __vsf_os;
/*============================ PROTOTYPES ====================================*/
SECTION(".text.vsf.kernel.eda")
#if __VSF_KERNEL_CFG_EDA_FRAME_POOL == ENABLED
void vsf_kernel_init(   vsf_pool_block(vsf_eda_frame_pool) *frame_buf_ptr,
                        uint_fast16_t count);
#else
void vsf_kernel_init(void);
#endif

SECTION(".text.vsf.kernel.teda")
extern vsf_err_t vsf_timer_init(void);

#if __VSF_KERNEL_CFG_EVTQ_EN == ENABLED
SECTION(".text.vsf.kernel.__vsf_set_cur_evtq")
extern vsf_evtq_t *__vsf_set_cur_evtq(vsf_evtq_t *pnew);
extern vsf_err_t vsf_evtq_poll(vsf_evtq_t *pthis);
#endif

extern const vsf_kernel_resource_t * vsf_kernel_get_resource_on_init(void);

/*============================ IMPLEMENTATION ================================*/

#ifdef __VSF_OS_CFG_EVTQ_LIST
implement_vsf_pool( vsf_evt_node_pool, vsf_evt_node_t)
#endif

static void vsf_kernel_os_init(void)
{
    memset(&__vsf_os, 0, sizeof(__vsf_os));

    __vsf_os.res_ptr = vsf_kernel_get_resource_on_init();
    VSF_KERNEL_ASSERT(NULL != __vsf_os.res_ptr);

#if __VSF_KERNEL_CFG_EDA_FRAME_POOL == ENABLED
    vsf_kernel_init(__vsf_os.res_ptr->frame_stack.frame_buf_ptr, 
                    __vsf_os.res_ptr->frame_stack.frame_cnt);
#else
    vsf_kernel_init();
#endif

#if __VSF_KERNEL_CFG_EVTQ_EN == ENABLED
#   ifdef __VSF_OS_CFG_EVTQ_LIST
    do {                        
        VSF_POOL_PREPARE(vsf_evt_node_pool, (&__vsf_os.node_pool),
            .pTarget = &__vsf_os,
            .ptRegion = (code_region_t *)&DEFAULT_CODE_REGION_ATOM_CODE,
        );

        if  (   (NULL == __vsf_os.res_ptr->evt_queue.nodes_buf_ptr)
            ||  (0 == __vsf_os.res_ptr->evt_queue.node_cnt)) {
            break;
        }
        VSF_POOL_ADD_BUFFER(vsf_evt_node_pool,
                            (&__vsf_os.node_pool),               
                            __vsf_os.res_ptr->evt_queue.nodes_buf_ptr,                                  
                            __vsf_os.res_ptr->evt_queue.node_cnt 
                                * sizeof(vsf_pool_block(vsf_evt_node_pool)));  
                            
    } while(0);
#   endif
#endif
}

#if __VSF_KERNEL_CFG_EVTQ_EN == ENABLED
static void __vsf_os_evtq_swi_handler(void *p)
{
    vsf_evtq_t *pcur, *pold;

    VSF_KERNEL_ASSERT(p != NULL);

    pcur = (vsf_evtq_t *)p;
    pold = __vsf_set_cur_evtq(pcur);
    vsf_evtq_poll(pcur);
    __vsf_set_cur_evtq(pold);
}

vsf_evtq_t *__vsf_os_evtq_get(vsf_prio_t priority)
{
    if ((priority >= 0) && (priority < __vsf_os.res_ptr->evt_queue.queue_cnt)) {
        return &__vsf_os.res_ptr->evt_queue.queue_array[priority];
    }
    return NULL;
}

vsf_prio_t __vsf_os_evtq_get_prio(vsf_evtq_t *pthis)
{
    uint_fast8_t index = pthis - __vsf_os.res_ptr->evt_queue.queue_array;
    VSF_KERNEL_ASSERT(      (pthis != NULL) 
                        && (index < __vsf_os.res_ptr->evt_queue.queue_cnt));

    return (vsf_prio_t)index;
}

vsf_err_t __vsf_os_evtq_set_priority(vsf_evtq_t *pthis, vsf_prio_t priority)
{
    uint_fast8_t index = pthis - __vsf_os.res_ptr->evt_queue.queue_array;
    VSF_KERNEL_ASSERT((     pthis != NULL) 
                        &&  (index < __vsf_os.res_ptr->evt_queue.queue_cnt));

#if VSF_OS_CFG_MAIN_EVTQ_EN == ENABLED
    if (vsf_prio_0 == priority) {
        vsf_kernel_err_report(VSF_KERNEL_ERR_INVALID_USAGE);
    }
#endif
#ifdef __VSF_OS_SWI_PRIORITY_BEGIN
    if (priority >= __VSF_OS_SWI_PRIORITY_BEGIN) {
        return vsf_swi_init(
                index, 
                __vsf_os.res_ptr->arch.os_swi_priorities_ptr[priority],
                &__vsf_os_evtq_swi_handler, pthis);
    }
#endif
    return VSF_ERR_FAIL;
}

vsf_err_t __vsf_os_evtq_init(vsf_evtq_t *pthis)
{
    uint_fast8_t index = pthis - __vsf_os.res_ptr->evt_queue.queue_array;
    VSF_KERNEL_ASSERT(      (pthis != NULL) 
                        &&  (index < __vsf_os.res_ptr->evt_queue.queue_cnt));

#ifdef __VSF_OS_SWI_PRIORITY_BEGIN
    if (index >= __VSF_OS_SWI_PRIORITY_BEGIN) {
        __vsf_os_evtq_set_priority(pthis, (vsf_prio_t)index);
    }
#endif
    return VSF_ERR_NONE;
}

vsf_err_t __vsf_os_evtq_activate(vsf_evtq_t *pthis)
{
    uint_fast8_t index = pthis - __vsf_os.res_ptr->evt_queue.queue_array;
    VSF_KERNEL_ASSERT(      (pthis != NULL) 
                        &&  (index < __vsf_os.res_ptr->evt_queue.queue_cnt));

#ifdef __VSF_OS_SWI_PRIORITY_BEGIN
    if (index >= __VSF_OS_SWI_PRIORITY_BEGIN) {
        vsf_swi_trigger(index);
    }
#endif
    return VSF_ERR_NONE;
}

#ifdef __VSF_OS_CFG_EVTQ_LIST
vsf_evt_node_t *__vsf_os_alloc_evt_node(void)
{
    return VSF_POOL_ALLOC(vsf_evt_node_pool, &__vsf_os.node_pool);
}

void __vsf_os_free_evt_node(vsf_evt_node_t *pnode)
{
    VSF_POOL_FREE(vsf_evt_node_pool, &__vsf_os.node_pool, pnode);
}
#endif
#endif

#if __VSF_KERNEL_CFG_EVTQ_EN == ENABLED
vsf_sched_lock_status_t vsf_sched_lock(void)
{
    return vsf_set_base_priority(
        __vsf_os.res_ptr->arch.os_swi_priorities_ptr[
                __vsf_os.res_ptr->arch.swi_priority_cnt - 1]);
        
}

void vsf_sched_unlock(vsf_sched_lock_status_t origlevel)
{
    vsf_set_base_priority(origlevel);
}
#endif

static void __vsf_code_region_sched_on_enter(void *pobj, void *plocal)
{
    vsf_sched_lock_status_t *pstate = (vsf_sched_lock_status_t *)plocal;
    VSF_KERNEL_ASSERT(NULL != plocal);
    (*pstate) = vsf_sched_lock();
}

static void __vsf_code_region_sched_on_leave(void *pobj,void *plocal)
{
    vsf_sched_lock_status_t *pstate = (vsf_sched_lock_status_t *)plocal;
    VSF_KERNEL_ASSERT(NULL != plocal);
    vsf_sched_unlock(*pstate);   
}

static const i_code_region_t __vsf_i_code_region_sched_safe = {
    .local_obj_size =   sizeof(vsf_sched_lock_status_t),
    .OnEnter =          &__vsf_code_region_sched_on_enter,
    .OnLeave =          &__vsf_code_region_sched_on_leave,
};

const code_region_t VSF_SCHED_SAFE_CODE_REGION = {
    .pmethods = (i_code_region_t *)&__vsf_i_code_region_sched_safe,
};


WEAK void vsf_plug_in_on_kernel_idle(void)
{
    vsf_arch_sleep(0);
}

WEAK void vsf_plug_in_for_kernel_diagnosis(void)
{
    //! doing nothing here
}

WEAK void __post_vsf_kernel_init(void)
{
}

void vsf_kernel_os_run_priority(vsf_prio_t priority)
{
#if __VSF_KERNEL_CFG_EVTQ_EN == ENABLED
    __vsf_os_evtq_swi_handler(&__vsf_os.res_ptr->evt_queue.queue_array[priority]);
#endif
}

void vsf_kernel_os_start(void)
{
    vsf_service_init();
    vsf_kernel_os_init();

#if __VSF_KERNEL_CFG_EVTQ_EN == ENABLED
    vsf_evtq_t *pevtq = &__vsf_os.res_ptr->evt_queue.queue_array[0];
#   ifdef __VSF_OS_CFG_EVTQ_ARRAY
    uint_fast16_t node_size = (__vsf_os.res_ptr->evt_queue.node_bit_sz);
#   endif
    for (uint_fast8_t i = 0; 
        i < __vsf_os.res_ptr->evt_queue.queue_cnt; 
        i++, pevtq++) {
#   ifdef __VSF_OS_CFG_EVTQ_ARRAY
        uint_fast16_t temp = 1 << node_size;
        vsf_evt_node_t *node = 
            (vsf_evt_node_t *)__vsf_os.res_ptr->evt_queue.nodes + i * temp;
        memset( node,0, sizeof(vsf_evt_node_t) * temp );
        pevtq->node = node;
        pevtq->bitsize = node_size; 
#   endif
        vsf_evtq_init(pevtq);
    }
    __vsf_set_cur_evtq(NULL);
#endif

#if VSF_KERNEL_CFG_EDA_SUPPORT_TIMER
    vsf_timer_init();
#endif
    vsf_hal_advance_init();
    __post_vsf_kernel_init();
}

#if !__IS_COMPILER_IAR__
__attribute__((constructor(255)))
#endif

void __vsf_main_entry(void)
{
    vsf_hal_init();
    vsf_kernel_os_start();

    while (1) {
#if VSF_OS_CFG_MAIN_EVTQ_EN == ENABLED
        vsf_kernel_os_run_priority(0);
#endif
        vsf_plug_in_for_kernel_diagnosis(); //!< customised kernel diagnosis
        vsf_plug_in_on_kernel_idle();       //!< user defined idle task 
    }
}


#endif


