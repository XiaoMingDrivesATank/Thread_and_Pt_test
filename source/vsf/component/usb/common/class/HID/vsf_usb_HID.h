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

#ifndef __VSF_USB_HID_H__
#define __VSF_USB_HID_H__

/*============================ INCLUDES ======================================*/
/*============================ MACROS ========================================*/

#define USB_HID_PROTOCOL_BOOT               0
#define USB_HID_PROTOCOL_REPORT             1

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/

enum usb_HID_description_type_t {
    USB_HID_DT_HID          = 0x21,
    USB_HID_DT_REPORT       = 0x22,
    USB_HID_DT_PHYSICAL     = 0x23,
};
typedef enum usb_HID_description_type_t usb_HID_description_type_t;

enum usb_HID_req_t {
    USB_HIDREQ_GET_REPORT   = 0x01,
    USB_HIDREQ_GET_IDLE     = 0x02,
    USB_HIDREQ_GET_PROTOCOL = 0x03,
    USB_HIDREQ_SET_REPORT   = 0x09,
    USB_HIDREQ_SET_IDLE     = 0x0A,
    USB_HIDREQ_SET_PROTOCOL = 0x0B,
};
typedef enum usb_HID_req_t usb_HID_req_t;

enum usb_HID_report_type_t {
    USB_HID_REPORT_INPUT    = 1,
    USB_HID_REPORT_OUTPUT   = 2,
    USB_HID_REPORT_FEATURE  = 3,
};
typedef enum usb_HID_report_type_t usb_HID_report_type_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

#endif    // __VSF_USB_HID_H__
