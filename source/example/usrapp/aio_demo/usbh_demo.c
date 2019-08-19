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

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

struct usbh_demo_const_t {
    struct {
        vsf_ohci_param_t ohci_param;
    } usbh;
};
typedef struct usbh_demo_const_t usbh_demo_const_t;

struct usbh_demo_t {
    struct {
        vsf_usbh_t host;
#if VSF_USE_USB_HOST_HUB == ENABLED
        vsf_usbh_class_t hub;
#endif
#if VSF_USE_USB_HOST_ECM == ENABLED
        vsf_usbh_class_t ecm;
#endif
#if VSF_USE_USB_HOST_BTHCI == ENABLED
        vsf_usbh_class_t bthci;
#endif
#if VSF_USE_USB_HOST_HID == ENABLED
        vsf_usbh_class_t hid;
#endif
#if VSF_USE_USB_HOST_DS4 == ENABLED
        vsf_usbh_class_t ds4;
#endif
    } usbh;
};
typedef struct usbh_demo_t usbh_demo_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

static const usbh_demo_const_t usbh_demo_const = {
    .usbh.ohci_param        = {
        .op                 = &VSF_USB_HC0_IP,
        .priority           = vsf_arch_prio_0,
    },
};

static usbh_demo_t usbh_demo = {
    .usbh                       = {
        .host.drv               = &vsf_ohci_drv,
        .host.param             = (void *)&usbh_demo_const.usbh.ohci_param,

#if VSF_USE_USB_HOST_HUB == ENABLED
        .hub.drv                = &vsf_usbh_hub_drv,
#endif
#if VSF_USE_USB_HOST_ECM == ENABLED
        .ecm.drv                = &vsf_usbh_ecm_drv,
#endif
#if VSF_USE_USB_HOST_BTHCI == ENABLED
        .bthci.drv              = &vsf_usbh_bthci_drv,
#endif
#if VSF_USE_USB_HOST_HID == ENABLED
        .hid.drv                = &vsf_usbh_hid_drv,
#endif
#if VSF_USE_USB_HOST_DS4 == ENABLED
        .ds4.drv                = &vsf_usbh_ds4_drv,
#endif
    },
};

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

void usbh_demo_start(void)
{
    vsf_ohci_init();
    vsf_usbh_init(&usbh_demo.usbh.host);
#if VSF_USE_USB_HOST_HUB == ENABLED
    vsf_usbh_register_class(&usbh_demo.usbh.host, &usbh_demo.usbh.hub);
#endif
#if VSF_USE_USB_HOST_ECM == ENABLED
    vsf_usbh_register_class(&usbh_demo.usbh.host, &usbh_demo.usbh.ecm);
#endif
#if VSF_USE_USB_HOST_BTHCI == ENABLED
    vsf_usbh_register_class(&usbh_demo.usbh.host, &usbh_demo.usbh.bthci);
#endif
#if VSF_USE_USB_HOST_HID == ENABLED
    vsf_usbh_register_class(&usbh_demo.usbh.host, &usbh_demo.usbh.hid);
#endif
#if VSF_USE_USB_HOST_DS4 == ENABLED
    vsf_usbh_register_class(&usbh_demo.usbh.host, &usbh_demo.usbh.ds4);
#endif
}
