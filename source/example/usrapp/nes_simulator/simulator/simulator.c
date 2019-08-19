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
#include "./jeg/fce.h"
#include "./jeg/hal.h"
/*============================ MACROS ========================================*/
#ifndef NES_DEFAULT_ROM_NUMBER
#   define NES_DEFAULT_ROM_NUMBER           1
#endif
#define __DEFAULT_ROM(__N)          (uint8_t *)NES_ROM_##__N, NES_ROM_##__N##_Length
#define __NES_DEFAULT_ROM(__N)      __DEFAULT_ROM(__N)
#define NES_DEFAULT_ROM             __NES_DEFAULT_ROM(NES_DEFAULT_ROM_NUMBER)

#define __DEFAULT_ROM_ITEM(__N)     {(uint8_t *)NES_ROM_##__N, &NES_ROM_##__N##_Length,}
#define NES_DEFAULT_ROM_ITEM(__N)      __DEFAULT_ROM_ITEM(__N)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/* Color coding (16-bit):
     15..11 = R4..0 (Red)
     10..5  = G5..0 (Green)
      4..0  = B4..0 (Blue)
*/
typedef union {
    struct {
    
        uint16_t    B   :5;
        uint16_t    G   :6;
        uint16_t    R   :5;
    };
    uint16_t tValue;
}color_t;

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
NO_INIT static color_t s_tColorMap[UBOUND(palette)];

//! \name default roms
//! @{
extern const uint8_t NES_ROM_1[];               //!< city tank
extern const uint32_t NES_ROM_1_Length;
extern const uint8_t NES_ROM_2[];               //!< road fighter
extern const uint32_t NES_ROM_2_Length;
extern const uint8_t NES_ROM_3[];               //!< super mario bro
extern const uint32_t NES_ROM_3_Length;
extern const uint8_t NES_ROM_4[];               //!< Contra(USA)
extern const uint32_t NES_ROM_4_Length;
//! @}

struct {
    uint8_t *pchROM;
    const uint32_t *pwSize;
}c_tDefaultROM[] = {
    NES_DEFAULT_ROM_ITEM(1),
    NES_DEFAULT_ROM_ITEM(2),
    NES_DEFAULT_ROM_ITEM(3),
    NES_DEFAULT_ROM_ITEM(4)
};

/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

WEAK void uvc_app_fill_line(void *line_buf, uint_fast16_t size, bool last_line);



void nes_hal_init(void)
{
    do {
        //! initialise color map
        uint32_t n = 0;
        for (;n < UBOUND(s_tColorMap);n++) {
            pal_t color = palette[n];
            s_tColorMap[n].B = color.b >> 3;
            s_tColorMap[n].G = color.g >> 2;
            s_tColorMap[n].R = color.r >> 3;
        }
    } while(false);
}

void update_frame(frame_t *ptFrame)
{
    //! do nothing
    UNUSED_PARAM(ptFrame);
}

void uvc_app_on_fill_line_cpl(bool frame_cpl)
{
    
}

void nes_draw_pixels(void *ptTag, uint_fast8_t y, uint_fast8_t x, uint_fast8_t chColor)
{
    //frame_t *ptThis = (frame_t *)ptTag;
    static uint_fast8_t s_wOldY = 0;
    static uint16_t s_hwLineBuffer[256];
    
    
    if (s_wOldY != y) {
        //! new line is ready
        uvc_app_fill_line(s_hwLineBuffer, sizeof(s_hwLineBuffer), s_wOldY == 239);
    }
    s_hwLineBuffer[x] = s_tColorMap[chColor].tValue;
    s_wOldY = y;
}

void uvc_app_init(void)
{
    NO_INIT static uint8_t s_chDefaultROMIndex;
    s_chDefaultROMIndex ++;
    s_chDefaultROMIndex %= 3;
    
    //! use default
    //if (fce_load_rom(c_tDefaultROM[s_chDefaultROMIndex].pchROM, (*c_tDefaultROM[s_chDefaultROMIndex].pwSize)) != 0){
    if (fce_load_rom(c_tDefaultROM[2].pchROM, (*c_tDefaultROM[2].pwSize)) != 0){
        ASSERT(false);
    }

    fce_init();
}

void uvc_app_task(void)
{
    fce_task();
}

/* EOF */
