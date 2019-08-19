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

#ifndef __VSF_INPUT_DS4_H__
#define __VSF_INPUT_DS4_H__

/*============================ INCLUDES ======================================*/

#include "../../vsf_input_cfg.h"

#if VSF_INPUT_CFG_DS4_EN == ENABLED

#include "component/usb/common/class/HID/vsf_usb_ds4.h"
#include "../../vsf_input_get_type.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

enum {
    VSF_INPUT_TYPE_DS4 = VSF_INPUT_USER_TYPE,
};

struct vsf_input_ds4u_t {
    vsf_usb_ds4_gamepad_in_report_t data;
    vsf_input_timestamp_t timestamp;
};
typedef struct vsf_input_ds4u_t vsf_input_ds4u_t;

/*============================ GLOBAL VARIABLES ==============================*/

extern const vsf_input_item_info_t vsf_ds4u_gamepad_item_info[GAMEPAD_ID_USER];
extern const vsf_sensor_item_info_t vsf_ds4u_sensor_item_info[6];

/*============================ PROTOTYPES ====================================*/

extern void vsf_ds4u_process_input(vsf_input_ds4u_t *dev, vsf_usb_ds4_gamepad_in_report_t *data);
extern void vsf_ds4u_new_dev(vsf_input_ds4u_t *dev);
extern void vsf_ds4u_free_dev(vsf_input_ds4u_t *dev);

#endif      // VSF_INPUT_CFG_DS4_EN
#endif      // __VSF_INPUT_DS4_H__
