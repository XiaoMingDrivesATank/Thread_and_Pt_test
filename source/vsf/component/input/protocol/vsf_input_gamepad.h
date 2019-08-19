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

#ifndef __VSF_INPUT_GAMEPAD_H__
#define __VSF_INPUT_GAMEPAD_H__

/*============================ INCLUDES ======================================*/
#include "../vsf_input_cfg.h"

/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/

#define VSF_GAMEPAD_DEF_ITEM_INFO(__name, __bitoffset, __bitlen, __is_signed)   \
            [TPASTE2(GAMEPAD_ID_, __name)] = VSF_INPUT_ITEM(                    \
                TPASTE2(GAMEPAD_ID_, __name),                                   \
                (__bitoffset), (__bitlen), (__is_signed))

/*============================ TYPES =========================================*/

//                         VIRTUAL_GAMEPAD
//          LT                                      RT
//          LB                                      RB
//                MENU_LEFT   MENU_MAIN   MENU_LEFT
//          L_UP               SPECIAL              R_UP
//  L_LEFT  LS/(LX,LY)  L_RIGHT             R_LEFT  RS/(RX,RY)  R_RIGHT
//          L_DOWN                                  R_DOWN

enum vsf_gamepad_id_t {
    GAMEPAD_ID_DUMMY = 0,
    GAMEPAD_ID_L_UP,
    GAMEPAD_ID_L_DOWN,
    GAMEPAD_ID_L_LEFT,
    GAMEPAD_ID_L_RIGHT,
    GAMEPAD_ID_R_UP,
    GAMEPAD_ID_R_DOWN,
    GAMEPAD_ID_R_LEFT,
    GAMEPAD_ID_R_RIGHT,
    GAMEPAD_ID_LB,
    GAMEPAD_ID_RB,
    GAMEPAD_ID_LS,
    GAMEPAD_ID_RS,
    GAMEPAD_ID_MENU_LEFT,
    GAMEPAD_ID_MENU_RIGHT,
    GAMEPAD_ID_MENU_MAIN,
    GAMEPAD_ID_SPECIAL,
    GAMEPAD_ID_LX,
    GAMEPAD_ID_LY,
    GAMEPAD_ID_RX,
    GAMEPAD_ID_RY,
    GAMEPAD_ID_LT,
    GAMEPAD_ID_RT,
    GAMEPAD_ID_DPAD,

    GAMEPAD_ID_NUM,
    GAMEPAD_ID_USER = GAMEPAD_ID_NUM,
};
typedef enum vsf_gamepad_id_t vsf_gamepad_id_t;

struct vsf_gamepad_evt_t {
    implement(vsf_input_evt_t)
    uint8_t bitlen      : 7;
    uint8_t is_signed   : 1;
};
typedef struct vsf_gamepad_evt_t vsf_gamepad_evt_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

#endif
/* EOF */
