/***************************************************************************
 *   Copyright(C)2009-2018 by Gorgon Meducer<Embedded_zhuoran@hotmail.com> *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __LINUX_GENERIC_H__
#define __LINUX_GENERIC_H__

/*============================ INCLUDES ======================================*/
#include "hal/vsf_hal_cfg.h"

/*============================ MACROS ========================================*/

#define __LITTLE_ENDIAN                 1
#define __BYTE_ORDER                    __LITTLE_ENDIAN

#define VSF_ARCH_PRI_NUM                8

// software interrupt provided by arch
#define VSF_ARCH_SWI_NUM                6

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

typedef uint64_t vsf_systimer_cnt_t;

#define __VSF_ARCH_PRI(__N, __BIT)                                              \
            VSF_ARCH_PRIO_##__N = (__N),                                        \
            vsf_arch_prio_##__N = (__N),

enum vsf_arch_prio_t {
    VSF_ARCH_PRIO_IVALID = -1,
    vsf_arch_prio_ivalid = -1,
    MREPEAT(VSF_ARCH_PRI_NUM, __VSF_ARCH_PRI, VSF_ARCH_PRI_BIT)
};
typedef enum vsf_arch_prio_t vsf_arch_prio_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

static ALWAYS_INLINE void vsf_arch_set_stack(uint32_t stack)
{
    VSF_HAL_ASSERT(false);
}

static ALWAYS_INLINE void vsf_arch_set_pc(uint32_t pc)
{
    VSF_HAL_ASSERT(false);
}

static ALWAYS_INLINE uint32_t vsf_arch_get_lr(void)
{
    VSF_HAL_ASSERT(false);
    return 0;
}

#endif
/* EOF */
