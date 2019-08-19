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
#define __VSF_EDA_CLASS_IMPLEMENT
#include "kernel/vsf_kernel_cfg.h"

#if VSF_USE_KERNEL == ENABLED

#include "./vsf_kernel_common.h"

#include "./vsf_eda.h"
#include "./vsf_evtq.h"

#include "./vsf_os.h"

#ifdef __VSF_OS_CFG_EVTQ_ARRAY

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

SECTION(".text.vsf.kernel.eda")
extern void vsf_eda_on_terminate(vsf_eda_t *pthis);

SECTION(".text.vsf.kernel.__vsf_set_cur_evtq")
extern vsf_evtq_t * __vsf_set_cur_evtq(vsf_evtq_t *evtq);

SECTION(".text.vsf.kernel.eda")
extern void __vsf_dispatch_evt(vsf_eda_t *pthis, vsf_evt_t evt);

extern vsf_evtq_t *__vsf_os_evtq_get(vsf_prio_t priority);
extern vsf_err_t __vsf_os_evtq_activate(vsf_evtq_t *pthis);
extern vsf_err_t __vsf_os_evtq_init(vsf_evtq_t *pthis);

/*============================ IMPLEMENTATION ================================*/

void vsf_evtq_on_eda_init(vsf_eda_t *eda) {}

static bool __vsf_eda_terminate(vsf_eda_t *pthis)
{
    bool terminate;

    VSF_KERNEL_ASSERT(pthis != NULL);

    terminate = !pthis->evt_cnt;
    if (terminate) {
        vsf_eda_on_terminate(pthis);
    }
    return terminate;
}


void vsf_evtq_on_eda_fini(vsf_eda_t *pthis)
{
    if (!__vsf_eda_terminate((vsf_eda_t *)pthis)) {
        pthis->is_to_exit = true;
    }
}

vsf_err_t vsf_evtq_init(vsf_evtq_t *pthis)
{
    VSF_KERNEL_ASSERT(pthis != NULL);
    pthis->cur.eda = NULL;
    pthis->cur.evt = VSF_EVT_INVALID;
    pthis->cur.msg = NULL;
    pthis->head = 0;
    pthis->tail = 0;
    return __vsf_os_evtq_init(pthis);
}

#if VSF_CFG_EVT_MESSAGE_EN == ENABLED
static vsf_err_t __vsf_evtq_post(vsf_eda_t *eda, vsf_evt_t evt, void *msg, bool force)
#else
static vsf_err_t __vsf_evtq_post(vsf_eda_t *eda, uint_fast32_t value, bool force)
#endif
{
    vsf_evtq_t *evtq;
    uint_fast8_t tail, tail_next, mask;
    vsf_protect_t orig;

    VSF_KERNEL_ASSERT(eda != NULL);
    evtq = __vsf_os_evtq_get((vsf_prio_t)eda->cur_priority);
    mask = (1 << evtq->bitsize) - 1;

    orig = vsf_protect_int();
    if (eda->evt_cnt && eda->is_limitted && !force) {
        vsf_unprotect_int(orig);
        return VSF_ERR_FAIL;
    }
    tail = evtq->tail;
    tail_next = (tail + 1) & mask;
    if (tail_next == evtq->head) {
        vsf_unprotect_int(orig);
        VSF_KERNEL_ASSERT(false);
        return VSF_ERR_NOT_ENOUGH_RESOURCES;
    }
    evtq->tail = tail_next;
    evtq->node[tail].eda = eda;
#if VSF_CFG_EVT_MESSAGE_EN == ENABLED
    evtq->node[tail].evt = (vsf_evt_t)value;
    evtq->node[tail].msg = msg;
#else
    evtq->node[tail].value = value;
#endif
    eda->evt_cnt++;
    vsf_unprotect_int(orig);

    return __vsf_os_evtq_activate(evtq);
}

vsf_err_t vsf_evtq_post_evt_ex(vsf_eda_t *pthis, vsf_evt_t evt, bool force)
{
#if VSF_CFG_EVT_MESSAGE_EN == ENABLED
    return __vsf_evtq_post(pthis, evt, NULL, force);
#else
    return __vsf_evtq_post(pthis, (uint32_t)((evt << 1) | 1), force);
#endif
}

vsf_err_t vsf_evtq_post_evt(vsf_eda_t *pthis, vsf_evt_t evt)
{
    return vsf_evtq_post_evt_ex(pthis, evt, false);
}

vsf_err_t vsf_evtq_post_msg(vsf_eda_t *pthis, void *msg)
{
#if VSF_CFG_EVT_MESSAGE_EN == ENABLED
    return __vsf_evtq_post(pthis, VSF_EVT_MESSAGE, msg, false);
#else
    return __vsf_evtq_post(pthis, (uint32_t)msg, false);
#endif
}

vsf_err_t vsf_evtq_poll(vsf_evtq_t *pthis)
{
    vsf_evtq_t *evtq_orig;
    vsf_evt_node_t *node;
    vsf_eda_t *eda;
    uint_fast8_t size;
    vsf_protect_t orig;

    VSF_KERNEL_ASSERT(pthis != NULL);
    size = 1 << pthis->bitsize;

    evtq_orig = __vsf_set_cur_evtq(pthis);
    while (pthis->head != pthis->tail) {
        node = &pthis->node[pthis->head];
        pthis->head = (pthis->head + 1) & (size - 1);
        eda = node->eda;

        if (eda != NULL) {

            if (!eda->is_to_exit) {
                orig = vsf_protect_int();
                    pthis->cur.eda = eda;

#if VSF_CFG_EVT_MESSAGE_EN == ENABLED
                    pthis->evt_cur = node->evt;
                    pthis->msg_cur = node->msg;
#else
                    uint_fast32_t value = node->value;
                    if (value & 1) {
                        pthis->cur.evt = (vsf_evt_t)(value >> 1);
                        pthis->cur.msg = NULL;
                    } else {
                        pthis->cur.evt = VSF_EVT_MESSAGE;
                        pthis->cur.msg = (void *)value;
                    }
#endif
                vsf_unprotect_int(orig);

                __vsf_dispatch_evt(eda, pthis->cur.evt);
            }

            orig = vsf_protect_int();
                pthis->cur.eda = NULL;
                pthis->cur.evt = VSF_EVT_INVALID;
                pthis->cur.msg = NULL;
                eda->evt_cnt--;
            vsf_unprotect_int(orig);
            pthis->cur.eda = NULL;

            if (eda->is_to_exit) {
                __vsf_eda_terminate(eda);
            }
        }
    }
    __vsf_set_cur_evtq(evtq_orig);
    return VSF_ERR_NONE;
}
#endif
#endif
