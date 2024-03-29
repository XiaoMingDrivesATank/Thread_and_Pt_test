#include "ppu.h"
#include "nes.h"

#include ".\common.h"
#include "jeg_cfg.h"

//! \name PPU Control Register bit mask
//! @{
#define PPUCTRL_NAMETABLE                   (3<<0)
#define PPUCTRL_INCREMENT                   (1<<2)
#define PPUCTRL_SPRITE_TABLE                (1<<3)
#define PPUCTRL_BACKGROUND_TABLE            (1<<4)
#define PPUCTRL_SPRITE_SIZE                 (1<<5)
#define PPUCTRL_MASTER_SLAVE                (1<<6)
#define PPUCTRL_NMI                         (1<<7)
//! @}

//! \name PPU Masking Register Bit Mask
//! @{
#define PPUMASK_GRAYSCALE                   (1<<0)
#define PPUMASK_SHOW_LEFT_BACKGROUND        (1<<1)
#define PPUMASK_SHOW_LEFT_SPRITES           (1<<2)
#define PPUMASK_SHOW_BACKGROUND             (1<<3)
#define PPUMASK_SHOW_SPRITES                (1<<4)
#define PPUMASK_RED_TINT                    (1<<5)
#define PPUMASK_GREEN_TINT                  (1<<6)
#define PPUMASK_BLUE_TINT                   (1<<7)
//! @}

//! \name PPU status register bit mask
//! @{
#define PPUSTATUS_SPRITE_OVERFLOW           (1<<5)
#define PPUSTATUS_SPRITE_ZERO_HIT           (1<<6)
#define PPUSTATUS_VBLANK                    (1<<7)
//! @}

extern 
void ppu_bus_write( void *ptTarget, 
                    uint_fast16_t hwAddress, 
                    uint_fast8_t chData);
extern 
uint_fast8_t ppu_bus_read (void *ptTarget, uint_fast16_t hwAddress);

extern 
void ppu_update_cpu_stall_cycle(void *ptTarget, int_fast32_t nCycles);

extern 
int_fast32_t ppu_get_cpu_cycle_count(void *ptTarget);

extern 
uint_fast8_t ppu_get_cartridge_mirror_type(void *ptTarget);

extern 
void ppu_trigger_cpu_nmi(void *ptTarget);

extern 
void ppu_draw_pixel(void *, uint_fast8_t , uint_fast8_t , uint_fast8_t );

extern 
uint8_t *ppu_dma_get_source_address(void *ref, uint_fast16_t hwAddress);

#if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE == ENABLED
bool ppu_init(ppu_t *ppu, ppu_cfg_t *ptCFG) 
{
    bool bResult = false;
    do {
        if (NULL == ppu || NULL == ptCFG) {
            break;
        } else if (     (NULL == ptCFG->ptTarget)) {
            break;
        }
        
        ppu->ptTarget        = ptCFG->ptTarget;
    #if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE != ENABLED
        ppu->video_frame_data   = NULL;
    #endif
        ppu_reset(ppu);

        bResult = true;
    } while(false);
    return bResult;
}
#else
void ppu_init(ppu_t *ppu, nes_t *nes, ppu_read_func_t read, ppu_write_func_t write) 
{
    ppu->nes                = nes;
    ppu->read               = read;
    ppu->write              = write;
    ppu->video_frame_data   = NULL;
    ppu_reset(ppu);
}

void ppu_setup_video(ppu_t *ppu, uint8_t *video_frame_data) 
{
    ppu->video_frame_data = video_frame_data;
    memset(ppu->video_frame_data, 0, 256*240);
}
#endif


void ppu_reset(ppu_t *ppu) 
{
    ppu->last_cycle_number  = 0;
    ppu->cycle              = 340;
    ppu->scanline           = 240;
    ppu->ppuctrl            = 0;
    ppu->ppustatus          = 0;
    ppu->t                  = 0;

    memset(&(ppu->tNameAttributeTable[0]), 0, sizeof(name_attribute_table_t));
    memset(&(ppu->tNameAttributeTable[1]), 0, sizeof(name_attribute_table_t));
    
#if JEG_USE_4_PHYSICAL_NAME_ATTRIBUTE_TABLES == ENABLED
    memset(&(ppu->tNameAttributeTable[2]), 0, sizeof(name_attribute_table_t));
    memset(&(ppu->tNameAttributeTable[3]), 0, sizeof(name_attribute_table_t));
#endif

#if JEG_USE_SPRITE_BUFFER == ENABLED
    memset(&(ppu->tModifiedSpriteTable), 0, sizeof(sprite_table_t));
    memset(&(ppu->wSpriteBuffer), 0, sizeof(ppu->wSpriteBuffer));
#endif

    ppu->ppumask            = 0;
    ppu->oam_address        = 0;
    ppu->register_data      = 0;
    ppu->name_table_byte    = 0;
    
#if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE == DISABLED
    if (NULL != ppu->video_frame_data) {
        memset(ppu->video_frame_data, 0, 256*240);
    }
#endif
}

uint_fast8_t ppu_read(ppu_t *ppu, uint_fast16_t hwAddress) 
{
    int value, buffered;

    switch (hwAddress & 0x07) {
        case 2:
            ppu_update(ppu);
            value=      (ppu->register_data & 0x1F)
                    |   (ppu->ppustatus & (     PPUSTATUS_VBLANK
                                            |   PPUSTATUS_SPRITE_ZERO_HIT
                                            |   PPUSTATUS_SPRITE_OVERFLOW));
                                            
            ppu->ppustatus &= ~PPUSTATUS_VBLANK; // disable vblank flag
            ppu->w=0;
            break;
            
        case 4:
            value=ppu->tSpriteTable.chBuffer[ppu->oam_address];
            break;
            
        case 7:
            value = ppu_bus_read(ppu->ptTarget, ppu->v);
            if ((ppu->v & 0x3FFF) < 0x3F00) {
                buffered = ppu->buffered_data;
                ppu->buffered_data = value;
                value = buffered;
            } else {
                ppu->buffered_data = ppu_bus_read(ppu->ptTarget, ppu->v - 0x1000);
            }
            ppu->v += ((ppu->ppuctrl&PPUCTRL_INCREMENT) == 0) ? 1 : 32;
            break;
            
        default:
            value = ppu->register_data;
            break;
    }
    return value;
}

void ppu_dma_access(ppu_t *ppu, uint_fast8_t chData)
{
    uint_fast16_t address_temp = chData << 8;

    
#if JEG_USE_DMA_MEMORY_COPY_ACCELERATION == ENABLED
    assert(0 == ppu->oam_address);
    
    uint8_t *pchSrc = ppu_dma_get_source_address(ppu->ptTarget, address_temp);
    
#   if JEG_USE_OPTIMIZED_SPRITE_PROCESSING == ENABLED
    uint8_t *pchCheck = pchSrc;
#       if JEG_USE_SPRITE_BUFFER == ENABLED
    uint8_t *pchOAM = ppu->tModifiedSpriteTable.chBuffer;
#       else
    uint8_t *pchOAM = ppu->tSpriteTable.chBuffer;
#endif
    uint_fast8_t n = 64;
    do {
        if (*pchCheck != *pchOAM) {
            ppu->bOAMUpdated = true;
            break;
        }
        pchCheck += 4;
        pchOAM += 4;
    } while(--n);
#   endif
    
#   if JEG_USE_SPRITE_BUFFER == ENABLED
    memcpy(&(ppu->tModifiedSpriteTable.chBuffer[0]), pchSrc, 256);
    ppu->bRequestRefreshSpriteBuffer = true;
#   else
    memcpy(&(ppu->tSpriteTable.chBuffer[0]), pchSrc, 256);
#   endif
    
#else
    for(uint_fast16_t i=0; i<256; i++) {
        uint_fast8_t v = ppu->nes->cpu.read(ppu->nes, address_temp++);
        
#   if JEG_USE_SPRITE_BUFFER == ENABLED
        ppu->tModifiedSpriteTable.chBuffer[(ppu->oam_address + i) & 0xFF] = v;
#   else
#       if JEG_USE_OPTIMIZED_SPRITE_PROCESSING == ENABLED
        if (!(i & 0x03)) {
            if (ppu->tSpriteTable.chBuffer[(ppu->oam_address + i) & 0xFF] != v) {
                ppu->bOAMUpdated = true;
            }
        }
#       endif
        ppu->tSpriteTable.chBuffer[(ppu->oam_address + i) & 0xFF] = v;
#   endif
    }
#   if JEG_USE_SPRITE_BUFFER == ENABLED
    ppu->bRequestRefreshSpriteBuffer = true;
#   endif
#endif

    ppu_update_cpu_stall_cycle(ppu->ptTarget, 513);
    //ppu->nes->cpu.stall_cycles += 513;
    if (ppu_get_cpu_cycle_count(ppu->ptTarget) & 0x01) {
        ppu_update_cpu_stall_cycle(ppu->ptTarget, 1);
    }
}



void ppu_write(ppu_t *ppu, uint_fast16_t hwAddress, uint_fast8_t chData) 
{
    ppu->register_data = chData;

    switch (hwAddress & 7) {
        case 0:
            ppu->ppuctrl=chData;
            ppu->t = (ppu->t & 0xF3FF) | ((chData & 0x03) <<10 );               //! select name/attribute tables
            break;
        case 1:
            ppu->ppumask=chData;
            break;
        case 3:
            ppu->oam_address=chData;
            break;
        case 4:
        
        
        
        #if JEG_USE_SPRITE_BUFFER == ENABLED
        
        #   if JEG_USE_OPTIMIZED_SPRITE_PROCESSING == ENABLED
            if (!(ppu->oam_address & 0x03)) {
                uint_fast8_t chOld = ppu->tModifiedSpriteTable.chBuffer[ppu->oam_address];
                if (chOld != chData) {
                    ppu->bOAMUpdated = true;
                }
            }
        #   endif
            ppu->bRequestRefreshSpriteBuffer = true;
            ppu->tModifiedSpriteTable.chBuffer[ppu->oam_address++] = chData;
        #else
        
        #   if JEG_USE_OPTIMIZED_SPRITE_PROCESSING == ENABLED
            if (!(ppu->oam_address & 0x03)) {
                uint_fast8_t chOld = ppu->tSpriteTable.chBuffer[ppu->oam_address];
                if (chOld != chData) {
                    ppu->bOAMUpdated = true;
                }
            }
        #   endif
            ppu->tSpriteTable.chBuffer[ppu->oam_address++] = chData;
        #endif
            break;
        case 5:
            ppu_update(ppu);
            if (0 == ppu->w) {
                ppu->t = ( ppu->t & 0xFFE0 ) | ( chData>>3 );
                ppu->x = chData & 0x07;
                ppu->w = 1;
            } else {
                ppu->t = (ppu->t & 0x8FFF) | ((chData&0x07)<<12);
                ppu->t = (ppu->t & 0xFC1F) | ((chData&0xF8)<<2);
                ppu->w = 0;
            }
            break;
        case 6:
            if (0 == ppu->w) {
                ppu->t = (ppu->t&0x80FF) | ((chData&0x3F)<<8);
                ppu->w = 1;
            } else {
                ppu->t = (ppu->t&0xFF00) | chData;                              
                ppu->v = ppu->t;
                ppu->w = 0;
            }
            break;
        case 7:
            ppu_bus_write(ppu->ptTarget, ppu->v, chData);
            ppu->v += (0 == (ppu->ppuctrl & PPUCTRL_INCREMENT)) ? 1:32;
            break;
    }

}



static uint32_t fetch_sprite_pattern(ppu_t *ppu, sprite_t *ptSpriteInfo, uint_fast16_t hwRow) 
{
    uint_fast8_t tile = ptSpriteInfo->chIndex;
    uint_fast8_t chAttributes =  ptSpriteInfo->Attributes.chValue;
    uint_fast8_t table;
    uint_fast16_t hwAddress;

    if (ppu->ppuctrl & PPUCTRL_SPRITE_SIZE) {
        if ((chAttributes & 0x80)) {
            hwRow = 15 - hwRow;
        }
        table = tile & 0x01;
        tile &= 0xFE;
        
        if (hwRow > 7) {
            tile++;
            hwRow -= 8;
        }
    } else {
        if ((chAttributes & 0x80)) {
            hwRow = 7 - hwRow;
        }
        table = (ppu->ppuctrl & PPUCTRL_SPRITE_TABLE ? 1 : 0 ) ;
    }
    
    hwAddress = 0x1000 * table + tile * 16 + hwRow;
    
    uint_fast8_t low_tile_byte = ppu_bus_read(ppu->ptTarget, hwAddress);
    uint_fast8_t high_tile_byte = ppu_bus_read(ppu->ptTarget, hwAddress + 8);
    uint32_t data=0;
  
    uint_fast8_t p1, p2;
    if (chAttributes & 0x40) {
        uint_fast8_t n = 8;
        do {
            p1= (low_tile_byte & 0x01);
            p2= (high_tile_byte & 0x01) << 1;
            low_tile_byte >>= 1;
            high_tile_byte >>= 1;
            
            data |= (((chAttributes & 3) << 2) | p1 | p2) << (4 * (8-n));
        } while(--n);
    } else {
        uint_fast8_t n = 8;
        do {
            p1 = (low_tile_byte & 0x80) >> 7;
            p2 = (high_tile_byte & 0x80) >> 6;
            low_tile_byte <<= 1;
            high_tile_byte <<= 1;

            data |= (((chAttributes & 3) << 2) | p1 | p2) << (4 * (8-n));
        } while(--n);
    }

    return data;
}

#if JEG_USE_SPRITE_BUFFER == ENABLED
static void update_sprite_buffer(ppu_t *ptPPU)
{
    if (!ptPPU->bRequestRefreshSpriteBuffer) {
        return ;
    }
    ptPPU->bRequestRefreshSpriteBuffer = false;
    
    sprite_t *ptOriginal = ptPPU->tSpriteTable.SpriteInfo;
    sprite_t *ptNew = ptPPU->tModifiedSpriteTable.SpriteInfo;
    uint_fast8_t chSpriteHeight = ptPPU->ppuctrl & PPUCTRL_SPRITE_SIZE ? 16 : 8;
    
    for (uint_fast8_t n = 0;n < UBOUND(ptPPU->tSpriteTable.SpriteInfo); n++) {
        if (ptOriginal->wValue != ptNew->wValue) {
            //! updated sprite, update the sprite buffer
            
            if (ptNew->chY < 240) {
                uint_fast8_t chLineCount = chSpriteHeight;

                uint32_t *pwDes = ptPPU->wSpriteBuffer[n];
                uint_fast8_t chRow = 0;
                do {
                    uint_fast32_t wPattern = fetch_sprite_pattern(ptPPU, ptNew, chRow++);
                    *pwDes++ = wPattern;
                } while(--chLineCount);
            } 
            
            ptOriginal->wValue = ptNew->wValue;
        }
        ptOriginal++;
        ptNew++;
    } 
}

#endif

#if JEG_USE_OPTIMIZED_SPRITE_PROCESSING == ENABLED
static void sort_sprite_order_list(ppu_t *ptPPU)
{
    //! a very simple & stupid sorting algorithm 
    uint_fast8_t chIndex = 0, n;

    if (!ptPPU->bOAMUpdated) {
        return ;
    }
    ptPPU->bOAMUpdated = false;
    
    //! initialise the list
    for (;chIndex < 64; chIndex++) {
        ptPPU->SpriteYOrderList.List[chIndex].chIndex = chIndex;
        ptPPU->SpriteYOrderList.List[chIndex].chY = ptPPU->tSpriteTable.SpriteInfo[chIndex].chY;
    }
    
    //! sort the list
    for (chIndex = 0; chIndex < 64; chIndex++) {
        uint_fast16_t hwMin = 0xFFFF;
        uint_fast16_t hwMinIndex = chIndex;
        for (n = chIndex; n < 64; n++) {        
            if (ptPPU->SpriteYOrderList.List[n].chY < hwMin) {
                hwMinIndex = n;
                hwMin = ptPPU->SpriteYOrderList.List[n].chY;
            } 
        }
        
        if (hwMin >= 240) {
            //! no need to do it.
            break;
        } else if (chIndex == hwMinIndex) {
            continue;
        }  
        
        //! swap
        
        do {
            uint_fast8_t chTempIndex = ptPPU->SpriteYOrderList.List[hwMinIndex].chIndex;
            
            ptPPU->SpriteYOrderList.List[hwMinIndex].chIndex = ptPPU->SpriteYOrderList.List[chIndex].chIndex; 
            ptPPU->SpriteYOrderList.List[hwMinIndex].chY = ptPPU->SpriteYOrderList.List[chIndex].chY;
            
            ptPPU->SpriteYOrderList.List[chIndex].chIndex = chTempIndex;
            ptPPU->SpriteYOrderList.List[chIndex].chY = hwMin;
        } while(false);
    }
    
    ptPPU->SpriteYOrderList.chVisibleCount = chIndex;
    ptPPU->SpriteYOrderList.chCurrent = 0;
}
#endif

static inline uint_fast8_t fetch_sprite_info_on_specified_line(ppu_t *ptPPU, uint_fast32_t nScanLine)
{
    uint_fast8_t chCount = 0;
    uint_fast8_t chSpriteSize = ((ptPPU->ppuctrl & PPUCTRL_SPRITE_SIZE) ? 16 : 8);

#if JEG_USE_OPTIMIZED_SPRITE_PROCESSING == ENABLED

    for(int_fast32_t j = ptPPU->SpriteYOrderList.chCurrent; j < ptPPU->SpriteYOrderList.chVisibleCount; j++) {
        uint_fast8_t chIndex = ptPPU->SpriteYOrderList.List[j].chIndex;
        
        int_fast32_t row = nScanLine - ptPPU->SpriteYOrderList.List[j].chY;
        if (row < 0) {
            continue;
        } else if (row >= chSpriteSize) {
            //! don't check previous sprites
            ptPPU->SpriteYOrderList.chCurrent++;
            continue;
        }
        
        if (chCount < JEG_MAX_ALLOWED_SPRITES_ON_SINGLE_SCANLINE) {
#   if JEG_USE_SPRITE_BUFFER == ENABLED
            ptPPU->sprite_patterns[chCount] = ptPPU->wSpriteBuffer[chIndex][row];           //!< Debug: why this doesn't work for Road Fighter?
            //ptPPU->sprite_patterns[chCount]   = fetch_sprite_pattern(ptPPU, ptPPU->tSpriteTable.SpriteInfo + chIndex, row);
#   else
            ptPPU->sprite_patterns[chCount]   = fetch_sprite_pattern(ptPPU, ptPPU->tSpriteTable.SpriteInfo + chIndex, row);
#   endif
            ptPPU->sprite_positions[chCount]  = ptPPU->tSpriteTable.SpriteInfo[chIndex].chPosition; 
            ptPPU->sprite_priorities[chCount] = ptPPU->tSpriteTable.SpriteInfo[chIndex].Attributes.Priority;
            ptPPU->sprite_indicies[chCount]   = chIndex;
            chCount++;
        } else {
            break;
        }
    }
#else
    // evaluate sprite
    for(int_fast32_t j = 0; j < 64; j++) {
        int_fast32_t row = ptPPU->scanline-ptPPU->tSpriteTable.SpriteInfo[j].chY;
        if (    (row < 0)
            ||  (row >= chSpriteSize)) {
            continue;
        }
        if (chCount < JEG_MAX_ALLOWED_SPRITES_ON_SINGLE_SCANLINE) {
            ptPPU->sprite_patterns[chCount]   = fetch_sprite_pattern(ptPPU, ptPPU->tSpriteTable.SpriteInfo + j, row);
            ptPPU->sprite_positions[chCount]  = ptPPU->tSpriteTable.SpriteInfo[j].chPosition;
            ptPPU->sprite_priorities[chCount] = ptPPU->tSpriteTable.SpriteInfo[j].Attributes.Priority;
            ptPPU->sprite_indicies[chCount]   = j;
            chCount++;
        }
    }
#endif

    if (chCount > 8) {
        ptPPU->ppustatus |= PPUSTATUS_SPRITE_OVERFLOW;
    }
    return chCount;
}

#if JEG_USE_BACKGROUND_BUFFERING == ENABLED
static void update_background(ppu_t *ptPPU)
{
    uint_fast8_t n = UBOUND(ptPPU->tNameAttributeTable);
    name_attribute_table_t *ptTable = ptPPU->tNameAttributeTable;
    
    do {
        uint_fast16_t hwAddress = 0;
        if (ptTable->bRequestRefresh) {
            ptTable->bRequestRefresh = false;
            
            for (uint_fast8_t chY = 0; chY < 30; chY++) {
            
                uint_fast32_t wLineMask = ptTable->wDirtyMatrix[chY];
                if (0 == wLineMask) {
                    hwAddress += 32;
                    continue;
                }

                ptTable->wDirtyMatrix[chY] = 0;
            
                for (uint_fast8_t chX = 0; chX < 32; chX++) {
                    
                    
                    if (!(wLineMask & 0x01)) {
                        wLineMask >>= 1;
                        hwAddress++;
                        continue;
                    }
                    wLineMask >>= 1;
                    
                    //!< fetch name table byte
                    uint_fast8_t name_table_byte   = ptTable->chNameTable[chY][chX];
                    
                    //!< fetch attribute table byte
                    uint_fast8_t attribute_table_byte = ptTable->AttributeTable[chY>>2][chX>>2].chValue;
                    
                    attribute_table_byte = (   (   attribute_table_byte >> (   ( (hwAddress >> 4) & 4) 
                                                                            |   (  hwAddress &2)) 
                                                ) & 3 
                          ) << 2;
                    
                    for (uint_fast8_t chYOffsite = 0; chYOffsite < 8; chYOffsite++) {
                        //!< fetch low tile byte
                        uint_fast8_t low_tile_byte = ppu_bus_read ( 
                                    ptPPU->ptTarget,    
                                    0x1000*((ptPPU->ppuctrl & PPUCTRL_BACKGROUND_TABLE) ? 1 : 0)
                                +   name_table_byte*16
                                +   chYOffsite
                            );

                        
                        //!< fetch high tile byte
                        uint_fast8_t high_tile_byte = ppu_bus_read(
                                    ptPPU->ptTarget, 
                                    0x1000 * ((ptPPU->ppuctrl & PPUCTRL_BACKGROUND_TABLE) ? 1 : 0)
                                +   name_table_byte*16
                                +   chYOffsite + 8
                            );

                        //! \note the orders of 8 pixels are changed in order to use 32bit copy optimisation.
                        //! @{
                        compact_dual_pixels_t *ptLine = &(ptTable->chBackgroundBuffer[chY*8 + chYOffsite][chX * 4]);
                        //!< store tile data
                        
                        uint_fast8_t n = 4;
                        do {
                        #if false
                            ptLine->Low =     attribute_table_byte
                                        |   ((low_tile_byte  & 0x80) >> 7)
                                        |   ((high_tile_byte & 0x80) >> 6);
                                    
                            low_tile_byte <<= 1;
                            high_tile_byte <<= 1;
                            
                            ptLine->High =     attribute_table_byte
                                        |   ((low_tile_byte  & 0x80) >> 7)
                                        |   ((high_tile_byte & 0x80) >> 6);
                                    
                            low_tile_byte <<= 1;
                            high_tile_byte <<= 1;
                        #else
                            ptLine->Low =     attribute_table_byte
                                        |   ((low_tile_byte  & 0x01))
                                        |   ((high_tile_byte & 0x01) << 1);
                                    
                            low_tile_byte >>= 1;
                            high_tile_byte >>= 1;
                            
                            ptLine->High =     attribute_table_byte
                                        |   ((low_tile_byte  & 0x01))
                                        |   ((high_tile_byte & 0x01) << 1);
                                    
                            low_tile_byte >>= 1;
                            high_tile_byte >>= 1;
                        #endif
                        
                            ptLine++;
                        } while(--n);
                        //! @}
                    }
                    
                    hwAddress++;
                }
            }
        }
        ptTable++;
        
    } while(--n);
}
#endif

static void fetch_background_tile_info(ppu_t *ptPPU)
{
    bool bReadInfo = true;
#if JEG_USE_BACKGROUND_BUFFERING != ENABLED

    uint_fast32_t data = 0;
    ptPPU->tile_data <<= 4;
    
    uint_fast8_t chTableIndex = find_name_attribute_table_index(
                                        ptPPU->nes->cartridge.chMirror, 
                                        ptPPU->v & 0x0FFF);
    name_attribute_table_t *ptTable = &(ptPPU->tNameAttributeTable[chTableIndex]);
    
    uint_fast16_t hwAddress = ptPPU->v & 0x3FF;
    
    switch (ptPPU->cycle & 0x07) {
        case 1:                                                     //!< fetch name table byte
            ptPPU->name_table_byte 
                = ptTable->chBuffer[hwAddress];                     //!< ptPPU->read(ptPPU->nes, 0x2000 | (ptPPU->v&0x0FFF) );
            break;
            
        case 3:                                                     //!< fetch attribute table byte
            ptPPU->attribute_table_byte = ptTable->AttributeTable[ptPPU->tVAddress.YScroll>>2][ptPPU->tVAddress.XScroll>>2].chValue;
            ptPPU->attribute_table_byte = 
                  (   (   ptPPU->attribute_table_byte >> (    ( (ptPPU->v>>4) & 4) 
                                                          |   (  ptPPU->v&2)) 
                      ) & 3 
                  ) << 2;
            break;
            
        case 5:                                                     //!< fetch low tile byte
            ptPPU->low_tile_byte = ptPPU->read ( 
                        ptPPU->nes,    
                        0x1000*((ptPPU->ppuctrl & PPUCTRL_BACKGROUND_TABLE) ? 1 : 0)
                    +   ptPPU->name_table_byte*16
                    +   ptPPU->tVAddress.TileYOffsite
                );
            break;
            
        case 7:                                                     //!< fetch high tile byte
            ptPPU->high_tile_byte = ptPPU->read(
                        ptPPU->nes, 
                        0x1000 * ((ptPPU->ppuctrl & PPUCTRL_BACKGROUND_TABLE) ? 1 : 0)
                    +   ptPPU->name_table_byte*16
                    +   ptPPU->tVAddress.TileYOffsite + 8
                );
            break;
            
        case 0:                                                     //!< store tile data
            for(int_fast32_t j = 0; j<8; j++) {
                data <<= 4;
                data |=     ptPPU->attribute_table_byte
                        |   ((ptPPU->low_tile_byte  & 0x80) >> 7)
                        |   ((ptPPU->high_tile_byte & 0x80) >> 6);
                        
                ptPPU->low_tile_byte <<= 1;
                ptPPU->high_tile_byte <<= 1;
            }
            ptPPU->tile_data |= data;
            break;
    }
    
#else
    ptPPU->tile_data <<= 4;
    if (!(ptPPU->cycle & 0x07)) {
    
        //uint_fast32_t data = 0;
        uint_fast8_t chTableIndex = 
        find_name_attribute_table_index(ppu_get_cartridge_mirror_type(ptPPU->ptTarget), (ptPPU->v&0x0FFF));
        
        name_attribute_table_t *ptTable = &(ptPPU->tNameAttributeTable[chTableIndex]);
        
        if (bReadInfo) {
            uint_fast8_t chY = (ptPPU->tVAddress.YScroll * 8) + ptPPU->tVAddress.TileYOffsite;
        #if JEG_USE_INVERSE_Y_SCANLINE == ENABLED
            compact_dual_pixels_t *ptLine = &(ptTable->chBackgroundBuffer[239 - chY][ptPPU->tVAddress.XScroll * 4]);
        #else
            compact_dual_pixels_t *ptLine = &(ptTable->chBackgroundBuffer[chY][ptPPU->tVAddress.XScroll * 4]);
        #endif
            /*! \note the orders of 8 pixels are changed in order to use 32bit copy optimisation. */
            ptPPU->tile_data |= *((uint32_t *)ptLine);
        }
    }
#endif
}


static void ppu_mix_background_and_foreground(ppu_t *ptPPU)
{
    //! render pixel
    uint_fast8_t background = 0, i = 0, sprite = 0;

    //! get sprite pixel color
    if (ptPPU->ppumask & PPUMASK_SHOW_SPRITES) {

        for(uint_fast8_t j = 0; j < ptPPU->sprite_count; j++) {
            int_fast16_t offset =   (ptPPU->cycle - 1) 
                                  - (int_fast16_t)ptPPU->sprite_positions[j];
                                  
            if ( offset < 0 || offset > 7) {
                continue;
            }
            
            //int_fast32_t color = (ptPPU->sprite_patterns[j] >> ((7 - offset) * 4)) & 0x0F;
            int_fast32_t color = (ptPPU->sprite_patterns[j] >> ((offset) * 4)) & 0x0F;
            if (!(color & 0x03)) {
                continue;
            }
            
            i = j;
            sprite = color;
            break;
        }
    }
    
    uint_fast8_t s = (sprite & 0x03), color = 0;

    //! get background pixel color
    if ((ptPPU->ppumask&PPUMASK_SHOW_BACKGROUND) != 0) {
        background = (ptPPU->tile_data >> (32 + ((7-ptPPU->x) * 4)) ) & 0x0F;
    }

    if ((ptPPU->cycle - 1) < 8) {
        if ((ptPPU->ppumask & PPUMASK_SHOW_LEFT_BACKGROUND) == 0) {
            background = 0;
        }
        if ((ptPPU->ppumask & PPUMASK_SHOW_LEFT_SPRITES) == 0) {
            sprite = 0;
        }
    }

    uint_fast8_t b = (background & 0x03);
    
    if (!b && s) {
        color = sprite | 0x10;
    } else if (b && !s) {
        color = background;
    } else if (b && s) {

        if (    (ptPPU->sprite_indicies[i] == 0) 
            &&  ((ptPPU->cycle - 1) < 255)) {
            ptPPU->ppustatus |= PPUSTATUS_SPRITE_ZERO_HIT;
        }
    
        if (ptPPU->sprite_priorities[i] == 0) {
            color = sprite | 0x10;
        } else {
            color = background;
        }

    }
    
    if ( color >= 16 && !(color & 0x03)) {
        color -= 16;
    }
#if JEG_USE_EXTERNAL_DRAW_PIXEL_INTERFACE == ENABLED
    ppu_draw_pixel( ptPPU->ptTarget,
    //ptPPU->fnDrawPixel(   ptPPU->ptTag, 
                        ptPPU->scanline,                              //!< Y
                        ptPPU->cycle-1,                               //!< X
                        ptPPU->palette[color]);                       //!< 8bit color
#else
    ptPPU->video_frame_data[ptPPU->scanline * 256 + ptPPU->cycle - 1] 
            = ptPPU->palette[color];

#endif
}


#define RENDERING_ENABLED       (ppu->ppumask & (   PPUMASK_SHOW_BACKGROUND     \
                                                |   PPUMASK_SHOW_SPRITES))
#define PRE_LINE                (261 == ppu->scanline)
#define VISIBLE_LINE            (ppu->scanline < 240)
#define RENDER_LINE             (PRE_LINE || VISIBLE_LINE)
#define PRE_FETCH_CYCLE         (ppu->cycle >= 321 && ppu->cycle <= 336)
#define VISIBLE_CYCLE           (ppu->cycle >= 1 && ppu->cycle <= 256)
#define FETCH_CYCLE             (PRE_FETCH_CYCLE || VISIBLE_CYCLE)



uint_fast32_t ppu_update(ppu_t *ppu) 
{
    //! tick
    int_fast32_t nCPUCycles = ppu_get_cpu_cycle_count(ppu->ptTarget);
    int_fast32_t cycles = (nCPUCycles - ppu->last_cycle_number) * 3;
    ppu->last_cycle_number = nCPUCycles;


#if JEG_USE_BACKGROUND_BUFFERING == ENABLED
    update_background(ppu);
#endif
#if JEG_USE_SPRITE_BUFFER == ENABLED
    update_sprite_buffer(ppu);
#endif
#if JEG_USE_OPTIMIZED_SPRITE_PROCESSING == ENABLED
    //! initialise sprite Y order sort list
    sort_sprite_order_list(ppu);
#endif


    while(cycles--) {
        ppu->cycle++;                                                           //!< go to next pixel
        
        if (ppu->cycle > 340) {                                                 //!< if scanline is rendered go to next scanline
            ppu->cycle = 0;
            ppu->scanline++;
            
            if (ppu->scanline > 261) {                                          //!< if frame is finished go to next frame
                ppu->scanline = 0;
                ppu->f^=1;
            }
        }

        //! render
        if (RENDERING_ENABLED) {

            //! background logic
            if (VISIBLE_LINE && VISIBLE_CYCLE) {
                ppu_mix_background_and_foreground(ppu);
            }
            
        
            if (RENDER_LINE && FETCH_CYCLE) {
                //! fetch background tile information with ppu->v 
                fetch_background_tile_info(ppu);
            }
        
            
            if (   PRE_LINE 
                && ppu->cycle >= 280 
                && ppu->cycle <= 304) {
                
                /* equivalent logic
                ppu->tVAddress.YToggleBit = ppu->tTempVAddress.YToggleBit;
                ppu->tVAddress.YScroll = ppu->tTempVAddress.YScroll;
                ppu->tVAddress.TileYOffsite = ppu->tTempVAddress.TileYOffsite;
                */
                ppu->v = (ppu->v & 0x841F) | (ppu->t & 0x7BE0);                 //!< ppu copy y
            }
            
            if (RENDER_LINE) {
                /*
                         (0,0)     (256,0)     (511,0)
                           +-----------+-----------+
                           |           |           |
                           |           |           |
                           |   $2000   |   $2400   |
                           |           |           |
                           |           |           |
                    (0,240)+-----------+-----------+(511,240)
                           |           |           |
                           |           |           |
                           |   $2800   |   $2C00   |
                           |           |           |
                           |           |           |
                           +-----------+-----------+
                         (0,479)   (256,479)   (511,479)
                         
                     The start location of the display window is determined 
                     by (X,Y) 
                        where X = (t.XScroll | t.XToggleBit) << 3 + ppu->x
                              Y = (t.YScroll | t.YToggleBit) << 3 + ppu->tVAddress.TileYOffsite
                */
            
                if (    FETCH_CYCLE 
                    &&  ((ppu->cycle & 0x07) == 0) ) {
                        
                    if (ppu->tVAddress.XScroll == 31) {
                        ppu->tVAddress.XToggleBit ^= 1;                         //! switch to another name table horizontally 
                    } 
                    ppu->tVAddress.XScroll++;
                }
                
                if (256 == ppu->cycle) {
                
                    if (ppu->tVAddress.TileYOffsite == 7) {
                        if (ppu->tVAddress.YScroll == 29) {
                            ppu->tVAddress.YToggleBit ^= 1;                     //! switch to another name table vertically 
                            ppu->tVAddress.YScroll = 0;
                        } else {
                            ppu->tVAddress.YScroll++;
                        }
                    }   
                    ppu->tVAddress.TileYOffsite++;

                } else if (ppu->cycle == 257) {
                    /* equivalent logic
                    ppu->tVAddress.XScroll = ppu->tTempVAddress.XScroll;
                    ppu->tVAddress.XToggleBit = ppu->tTempVAddress.XToggleBit;
                    */
                    ppu->v = (ppu->v & 0xFBE0) | (ppu->t & 0x41F);              //!< copy x
                    
                }
            }

            //! sprite logic
            if (257 == ppu->cycle) {
                if (VISIBLE_LINE) {
                    /*! fetch all the sprite informations on current scanline */
                    ppu->sprite_count = fetch_sprite_info_on_specified_line(ppu, ppu->scanline);
                } else if (240 == ppu->scanline) {
                    //! reset sprite Y order list counter
                    ppu->SpriteYOrderList.chCurrent = 0;
                } else {
                    ppu->sprite_count = 0;
                }
            }
        }

        if (241 == ppu->scanline && 1 == ppu->cycle) {
            ppu->ppustatus |= PPUSTATUS_VBLANK;
            
        #if JEG_USE_FRAME_SYNC_UP_FLAG  == ENABLED
            ppu->bFrameReady = true;
        #endif
        
            if (ppu->ppuctrl & PPUCTRL_NMI) {
                ppu_trigger_cpu_nmi(ppu->ptTarget);
                //cpu6502_trigger_interrupt(&ppu->nes->cpu, INTERRUPT_NMI);
            }
        }

        if (    (260 == ppu->scanline) 
            &&  (329 == ppu->cycle)) {
            ppu->ppustatus &= ~(    PPUSTATUS_VBLANK
                                |   PPUSTATUS_SPRITE_ZERO_HIT
                                |   PPUSTATUS_SPRITE_OVERFLOW);
        }
    }

    return (341*262-((ppu->scanline+21)%262)*341-ppu->cycle)/3+1;
}

#if JEG_USE_FRAME_SYNC_UP_FLAG  == ENABLED
bool ppu_is_frame_ready(ppu_t *ptPPU)
{
    bool bResult = ptPPU->bFrameReady;
    ptPPU->bFrameReady = false;
    return bResult;
}
#endif
