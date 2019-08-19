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

#ifndef __VSF_USBD_HID_H__
#define __VSF_USBD_HID_H__

/*============================ INCLUDES ======================================*/

#include "component/usb/vsf_usb_cfg.h"

#if VSF_USE_USB_DEVICE == ENABLED

#include "../../../common/class/HID/vsf_usb_HID.h"

#if     defined(VSF_USBD_HID_IMPLEMENT)
#   define __PLOOC_CLASS_IMPLEMENT
#   undef VSF_USBD_HID_IMPLEMENT
#elif   defined(VSF_USBD_HID_INHERIT)
#   define __PLOOC_CLASS_INHERIT
#   undef VSF_USBD_HID_INHERIT
#endif
#include "utilities/ooc_class.h"

/*============================ MACROS ========================================*/

#define VSF_USBD_DESC_HID_REPORT(__ptr, __size)                                 \
    {USB_HID_DT_REPORT, 0, 0, (__size), (uint8_t*)(__ptr)}

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

declare_simple_class(vsf_usbd_HID_t)
declare_simple_class(vsf_usbd_HID_report_t)

def_simple_class(vsf_usbd_HID_report_t) {

    public_member(
        usb_HID_report_type_t type;
        vsf_mem_t mem;
        uint8_t id;
        uint8_t idle;
    )

    protected_member(
        bool changed;
        uint8_t idle_cnt;
    )
};

enum vsf_usbd_HID_output_state_t {
    HID_OUTPUT_STATE_WAIT,
    HID_OUTPUT_STATE_RECEIVING,
};
typedef enum vsf_usbd_HID_output_state_t vsf_usbd_HID_output_state_t;

def_simple_class(vsf_usbd_HID_t) {

    public_member(
        uint8_t ep_out;
        uint8_t ep_in;
        uint8_t num_of_report;

        uint8_t has_report_id   : 1;
        uint8_t notify_eda      : 1;

        vsf_usbd_HID_report_t *reports;

        vsf_usbd_desc_t *desc;

        union {
            vsf_err_t (*on_report)(vsf_usbd_HID_t *hid, vsf_usbd_HID_report_t *report);
            vsf_eda_t *eda;
        };
    )

    private_member(
        uint8_t protocol;
        uint8_t cur_report;
        uint8_t cur_OUT_id;
        uint8_t cur_IN_id;

        vsf_usbd_HID_output_state_t output_state;
        bool busy;
        uint16_t pos_out;

        vsf_usbd_trans_t transact_in;
        vsf_usbd_trans_t transact_out;
        vsf_teda_t teda;

        vsf_usbd_dev_t *dev;
        vsf_usbd_ifs_t *ifs;
    )
};

/*============================ GLOBAL VARIABLES ==============================*/

extern const vsf_usbd_class_op_t vsf_usbd_HID;

/*============================ PROTOTYPES ====================================*/

extern bool vsf_usbh_HID_IN_report_can_update(vsf_usbd_HID_report_t *report);
extern vsf_err_t vsf_usbd_HID_IN_report_changed(vsf_usbd_HID_t *hid, vsf_usbd_HID_report_t *report);

#endif      // VSF_USE_USB_DEVICE
#endif      // __VSF_USBD_HID_H__
