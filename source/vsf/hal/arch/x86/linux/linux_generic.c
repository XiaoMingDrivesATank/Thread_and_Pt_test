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

#include "hal/arch/vsf_arch_abstraction.h"
#include "hal/driver/driver.h"
#include "utilities/template/vsf_list.h"

#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <semaphore.h>
#include <sched.h>

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

struct linux_irq_thread_t {
    vsf_dlist_node_t thread_node;
    pthread_t thread;
    vsf_arch_prio_t priority;

    vsf_irq_handler_t *handler;
    void *param;
};
typedef struct linux_irq_thread_t linux_irq_thread_t;

struct linux_swi_ctx_t {
    implement(linux_irq_thread_t);

    sem_t sem;
    bool inited;
};
typedef struct linux_swi_ctx_t linux_swi_ctx_t;

struct __vsf_arch_irq_mask_result_t {
    bool try_prempt;
};
typedef struct __vsf_arch_irq_mask_result_t __vsf_arch_irq_mask_result_t;

struct __vsf_x86_t {
    linux_swi_ctx_t swi[VSF_ARCH_SWI_NUM];

    struct {
        vsf_systimer_cnt_t tick;
        vsf_systimer_cnt_t unit;
        vsf_systimer_cnt_t max_tick_per_round;
        uint32_t           tick_freq;
    } systimer;

    pthread_mutex_t mutex;
    sem_t wakeup_sem;
    vsf_dlist_t irq_thread_list;

    uint8_t prio_base;
    vsf_gint_state_t gint_state;
};
typedef struct __vsf_x86_t __vsf_x86_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

NO_INIT static __vsf_x86_t __vsf_x86;

/*============================ PROTOTYPES ====================================*/
extern void vsf_systimer_evthandler(vsf_systimer_cnt_t tick);

/*============================ IMPLEMENTATION ================================*/

void __vsf_arch_irq_start(void)
{
}

void __vsf_arch_irq_end(void)
{
    sem_post(&__vsf_x86.wakeup_sem);
}

__vsf_arch_irq_mask_result_t __vsf_arch_irq_mask(uint_fast8_t base)
{
    // TODO
}

void __vsf_arch_try_prempt(vsf_arch_prio_t priority)
{
    // TODO
}

void __vsf_arch_post_irq_mask(__vsf_arch_irq_mask_result_t result)
{
    // TODO
}

void __vsf_arch_irq_enable(linux_irq_thread_t *irq_thread,
        void * (*entry) (void*), vsf_arch_prio_t priority)
{
    pthread_attr_t attr;
    struct sched_param sp;
    pthread_attr_init(&attr);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
    irq_thread->priority = priority;
    // sched_get_priority_min(SCHED_FIFO) priority is reserved for main
    // so swi priority start from sched_get_priority_min(SCHED_FIFO) + 1
    sp.sched_priority = sched_get_priority_min(SCHED_FIFO) + 1 + priority;
    pthread_attr_setschedparam(&attr, &sp);

    pthread_mutex_lock(&__vsf_x86.mutex);
        vsf_dlist_init_node(linux_irq_thread_t, thread_node, irq_thread);
        vsf_dlist_add_to_head(linux_irq_thread_t, thread_node, &__vsf_x86.irq_thread_list, irq_thread);
        pthread_create(&irq_thread->thread, &attr, entry, irq_thread);
    pthread_mutex_unlock(&__vsf_x86.mutex);
    __vsf_arch_try_prempt(irq_thread->priority);
}

WEAK bool on_arch_systimer_tick_evt(vsf_systimer_cnt_t tick)
{
    UNUSED_PARAM(tick);
    return true;
}

WEAK void vsf_systimer_set_idle(void)
{
}

ROOT void vsf_systimer_irq(void)
{
    vsf_systimer_cnt_t tick = __vsf_x86.systimer.tick;
    if (on_arch_systimer_tick_evt(tick)) {
        vsf_systimer_evthandler(tick);
    }
}

WEAK uint_fast32_t vsf_arch_req___systimer_resolution___from_usr(void)
{
    return 1000000ul;
}

/*! \brief initialise SysTick to generate a system timer
 *! \param frequency the target tick frequency in Hz
 *! \return initialization result in vsf_err_t
 */
WEAK vsf_err_t vsf_systimer_init(void)
{
    //! calculate the cycle count of 1 tick
    uint_fast32_t tick_res = vsf_arch_req___systimer_resolution___from_usr();
//    __vsf_x86.systimer.unit = vsf_arch_req___systimer_freq___from_usr() / tick_res;
	__vsf_x86.systimer.unit = 1;
    __vsf_x86.systimer.tick_freq = tick_res;
//    __vsf_x86.systimer.max_tick_per_round = (0x01000000ul / __vsf_x86.systimer.unit) - 1;

    return VSF_ERR_NONE;
}

WEAK vsf_err_t vsf_systimer_start(void)
{
    return VSF_ERR_NONE;
}


WEAK vsf_systimer_cnt_t vsf_systimer_get(void)
{
    vsf_systimer_cnt_t ticks = 0;
    return ticks;
}

WEAK bool vsf_systimer_set(vsf_systimer_cnt_t due)
{
    bool result = false;
    vsf_systimer_cnt_t max_tick_per_round = __vsf_x86.systimer.max_tick_per_round;

    return result;
}

WEAK bool vsf_systimer_is_due(vsf_systimer_cnt_t due)
{
    return (__vsf_x86.systimer.tick >= due);
}



WEAK vsf_systimer_cnt_t vsf_systimer_us_to_tick(uint_fast32_t time_us)
{
    return ((uint64_t)((uint64_t)time_us
        * (uint64_t)__vsf_x86.systimer.tick_freq)
        / 1000000ul);
}

WEAK vsf_systimer_cnt_t vsf_systimer_ms_to_tick(uint_fast32_t time_ms)
{
    return ((uint64_t)((uint64_t)time_ms
        * (uint64_t)__vsf_x86.systimer.tick_freq)
        / 1000ul);
}

WEAK uint_fast32_t vsf_systimer_tick_to_us(vsf_systimer_cnt_t tick)
{
    return tick * 1000000ul / __vsf_x86.systimer.tick_freq;
}

WEAK uint_fast32_t vsf_systimer_tick_to_ms(vsf_systimer_cnt_t tick)
{
    return vsf_systimer_tick_to_us(tick) / 1000;
}







static void * vsf_arch_swi_thread(void *arg)
{
    linux_swi_ctx_t *ctx = arg;
    while (1) {
        if (!sem_wait(&ctx->sem)) {
            __vsf_arch_irq_start();
                if (ctx->handler != NULL) {
                    ctx->handler(ctx->param);
                }
            __vsf_arch_irq_end();
        }
    }
    return NULL;
}

/*! \brief initialise a software interrupt
 *! \param idx the index of the software interrupt
 *! \return initialization result in vsf_err_t
 */
vsf_err_t vsf_arch_swi_init(uint_fast8_t idx, vsf_arch_prio_t priority,
    vsf_swi_handler_t *handler, void *param)
{
    if (idx < dimof(__vsf_x86.swi)) {
        linux_swi_ctx_t *ctx = &__vsf_x86.swi[idx];
        VSF_HAL_ASSERT(!ctx->inited);

        ctx->inited = true;
        ctx->handler = handler;
        ctx->param = param;
        sem_init(&ctx->sem, 0, 0);
        __vsf_arch_irq_enable(&ctx->use_as__linux_irq_thread_t, vsf_arch_swi_thread, priority);

        return VSF_ERR_NONE;
    }
    VSF_HAL_ASSERT(false);
    return VSF_ERR_INVALID_PARAMETER;
}

/*! \brief trigger a software interrupt
 *! \param idx the index of the software interrupt
 */
void vsf_arch_swi_trigger(uint_fast8_t idx)
{
    if (idx < dimof(__vsf_x86.swi)) {
        sem_post(&__vsf_x86.swi[idx].sem);
        __vsf_arch_try_prempt(__vsf_x86.swi[idx].priority);
        return;
    }
    VSF_HAL_ASSERT(false);
}

vsf_arch_prio_t vsf_set_base_priority(vsf_arch_prio_t priority)
{
    vsf_arch_prio_t orig = __vsf_x86.prio_base;
    pthread_mutex_lock(&__vsf_x86.mutex);
        __vsf_arch_irq_mask(priority);
        __vsf_x86.prio_base = priority;
    pthread_mutex_unlock(&__vsf_x86.mutex);
    return orig;
}




/*! \note initialize architecture specific service
 *  \param none
 *  \retval true initialization succeeded.
 *  \retval false initialization failed
 */
bool vsf_arch_init(void)
{
    // set main thread to use SCHED_FIFO scheduler with lowest priority
    struct sched_param sp;
    sp.sched_priority = sched_get_priority_min(SCHED_FIFO);
    if (sched_setscheduler(getpid(), SCHED_FIFO, &sp) == -1) {
        return false;
    }

    memset(&__vsf_x86, 0, sizeof(__vsf_x86));
    pthread_mutex_init(&__vsf_x86.mutex, NULL);
    sem_init(&__vsf_x86.wakeup_sem, 0, 0);
    vsf_systimer_init();
    return true;
}


vsf_gint_state_t vsf_get_interrupt(void)
{
    return __vsf_x86.gint_state;
}

void vsf_set_interrupt(vsf_gint_state_t level)
{
    pthread_mutex_lock(&__vsf_x86.mutex);
        __vsf_arch_irq_mask(level);
        __vsf_x86.gint_state = level;
    pthread_mutex_unlock(&__vsf_x86.mutex);
}

vsf_gint_state_t vsf_disable_interrupt(void)
{
    vsf_gint_state_t orig = __vsf_x86.gint_state;
    pthread_mutex_lock(&__vsf_x86.mutex);
        __vsf_arch_irq_mask(vsf_arch_prio_0);
        __vsf_x86.gint_state = false;
    pthread_mutex_unlock(&__vsf_x86.mutex);
    return orig;
}

void vsf_enable_interrupt(void)
{
    pthread_mutex_lock(&__vsf_x86.mutex);
        __vsf_arch_irq_mask(__vsf_x86.prio_base);
        __vsf_x86.gint_state = true;
    pthread_mutex_unlock(&__vsf_x86.mutex);
}

void vsf_arch_sleep(uint32_t mode)
{
    while (1) {
        if (!sem_wait(&__vsf_x86.wakeup_sem)) {
            return;
        }
    }
}

/* EOF */
