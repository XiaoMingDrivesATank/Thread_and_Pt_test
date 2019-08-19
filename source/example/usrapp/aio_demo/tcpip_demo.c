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

#include "vsf.h"

#include "component/3rd-party/lwip/1.4.1/port/lwip_netdrv_adapter.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

struct tcpip_demo_t {
    struct {
        bool inited;
        struct netif netif;
    } tcpip;
};
typedef struct tcpip_demo_t tcpip_demo_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static tcpip_demo_t tcpip_demo;

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

void vsf_pnp_on_netdrv_disconnect(vsf_netdrv_t *netdrv)
{
    if (tcpip_demo.tcpip.inited && (netdrv == lwip_netif_get_netdrv(&tcpip_demo.tcpip.netif))) {
        dhcp_stop(&tcpip_demo.tcpip.netif);
    }
}

void vsf_pnp_on_netdrv_connect(vsf_netdrv_t *netdrv)
{
    lwip_netif_set_netdrv(&tcpip_demo.tcpip.netif, netdrv);
}

void vsf_pnp_on_netdrv_connected(vsf_netdrv_t *netdrv)
{
    if (tcpip_demo.tcpip.inited && (netdrv == lwip_netif_get_netdrv(&tcpip_demo.tcpip.netif))) {
        dhcp_start(&tcpip_demo.tcpip.netif);
    }
}

void vsf_pnp_on_netdrv_new(vsf_netdrv_t *netdrv)
{
    vsf_protect_t origlevel = vsf_protect_scheduler();
        if (tcpip_demo.tcpip.inited) {
            vsf_unprotect_scheduler(origlevel);
            return;
        }
        tcpip_demo.tcpip.inited = true;
    vsf_unprotect_scheduler(origlevel);
}

void vsf_pnp_on_netdrv_del(vsf_netdrv_t *netdrv)
{
    vsf_protect_t origlevel = vsf_protect_scheduler();
        tcpip_demo.tcpip.inited = false;
    vsf_unprotect_scheduler(origlevel);
}

void tcpip_demo_start(void)
{
    tcpip_init(NULL, NULL);
}
