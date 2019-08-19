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

#include "../../vsf_input_cfg.h"

#if VSF_INPUT_CFG_DS4_EN == ENABLED

#include "vsf.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/

const vsf_input_item_info_t vsf_ds4u_gamepad_item_info[GAMEPAD_ID_NUM] = {
    VSF_GAMEPAD_DEF_ITEM_INFO(  R_UP,           47, 1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  R_DOWN,         45, 1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  R_LEFT,         44, 1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  R_RIGHT,        46, 1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  LB,             48, 1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  RB,             49, 1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  LS,             54, 1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  RS,             55, 1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  MENU_LEFT,      52, 1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  MENU_RIGHT,     53, 1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  MENU_MAIN,      56, 1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  SPECIAL,        57, 1,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  LX,             8,  8,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  LY,             16, 8,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  RX,             24, 8,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  RY,             32, 8,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  LT,             64, 8,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  RT,             72, 8,  false),
    VSF_GAMEPAD_DEF_ITEM_INFO(  DPAD,           40, 4,  false),
};

const vsf_sensor_item_info_t vsf_ds4u_sensor_item_info[6] = {
    VSF_SENSOR_DEF_ITEM_INFO(   SENSOR_ID_GYRO, SENSOR_SUBID_PITCH, 16),
    VSF_SENSOR_DEF_ITEM_INFO(   SENSOR_ID_GYRO, SENSOR_SUBID_YAW,   16),
    VSF_SENSOR_DEF_ITEM_INFO(   SENSOR_ID_GYRO, SENSOR_SUBID_ROLL,  16),
    VSF_SENSOR_DEF_ITEM_INFO(   SENSOR_ID_ACC,  SENSOR_SUBID_Y,     16),
    VSF_SENSOR_DEF_ITEM_INFO(   SENSOR_ID_ACC,  SENSOR_SUBID_Z,     16),
    VSF_SENSOR_DEF_ITEM_INFO(   SENSOR_ID_ACC,  SENSOR_SUBID_X,     16),
};

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

WEAK void vsf_ds4u_on_new_dev(vsf_input_ds4u_t *dev)
{
    vsf_input_on_new_dev(VSF_INPUT_TYPE_DS4, dev);
}

WEAK void vsf_ds4u_on_free_dev(vsf_input_ds4u_t *dev)
{
    vsf_input_on_free_dev(VSF_INPUT_TYPE_DS4, dev);
}

WEAK void vsf_ds4u_on_report_input(vsf_gamepad_evt_t *gamepad_evt)
{
    vsf_input_on_evt(VSF_INPUT_TYPE_DS4, &gamepad_evt->use_as__vsf_input_evt_t);
}

WEAK void vsf_ds4u_on_sensor(vsf_sensor_evt_t *sensor_evt)
{
    vsf_input_on_sensor(sensor_evt);
}

void vsf_ds4u_new_dev(vsf_input_ds4u_t *dev)
{
    memset(&dev->data, 0, sizeof(dev->data));
    vsf_ds4u_on_new_dev(dev);
}

void vsf_ds4u_free_dev(vsf_input_ds4u_t *dev)
{
    vsf_ds4u_on_free_dev(dev);
}

void vsf_ds4u_process_input(vsf_input_ds4u_t *dev, vsf_usb_ds4_gamepad_in_report_t *data)
{
    union {
        struct {
            vsf_gamepad_evt_t evt;
            vsf_input_item_info_t *info;
            vsf_input_parser_t parser;
            bool event_sent;
        } gamepad;
        struct {
            vsf_sensor_evt_t evt;
        } sensor;
    };

    gamepad.evt.duration        = vsf_input_update_timestamp(&dev->timestamp);
    gamepad.evt.dev             = dev;

    gamepad.event_sent          = false;
    gamepad.parser.info         = (vsf_input_item_info_t *)vsf_ds4u_gamepad_item_info;
    gamepad.parser.num          = dimof(vsf_ds4u_gamepad_item_info);
    do {
        gamepad.info = vsf_input_parse(&gamepad.parser, (uint8_t *)&dev->data, (uint8_t *)data);
        if (gamepad.info != NULL) {
            gamepad.evt.id = gamepad.info->id;
            gamepad.evt.bitlen = gamepad.info->bitlen;
            gamepad.evt.is_signed = gamepad.info->is_signed;
            gamepad.evt.pre = gamepad.parser.pre;
            gamepad.evt.cur = gamepad.parser.cur;
            vsf_ds4u_on_report_input(&gamepad.evt);
            gamepad.event_sent = true;
        }
    } while (gamepad.info != NULL);
    if (gamepad.event_sent) {
        gamepad.evt.id = GAMEPAD_ID_DUMMY;
        vsf_ds4u_on_report_input(&gamepad.evt);
    }

    // sensor
    sensor.evt.desc.item_info   = (vsf_sensor_item_info_t *)vsf_ds4u_sensor_item_info;
    sensor.evt.desc.item_num    = dimof(vsf_ds4u_sensor_item_info);
    sensor.evt.data             = (uint8_t *)&data->gyro_pitch;
    vsf_ds4u_on_sensor(&sensor.evt);

    dev->data = *data;
}

#endif      // VSF_INPUT_CFG_DS4_EN
