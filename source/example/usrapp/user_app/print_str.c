/*============================ INCLUDES ======================================*/
#define __USER_PRINT_STR_CLASS_IMPLEMENT

#include <stdio.h>
#include "./print_str.h"

/*============================ MACROS ========================================*/
#ifndef this
#   define this  (*ptThis)
#endif

/*========================== EXTERNAL STATEMENT ==============================*/
extern bool output_adapter(uint8_t chByte);
//extern bool serial_out(uint8_t chByte);

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
const i_print_str_t PRINT_STR = {
    .Init    = &print_str_init,
    .Prepare = &print_str_prepare
};

/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/

/*=============================== Print str ==================================*/
implement_vsf_pt(print_str_t) 
{
    vsf_pt_begin();

    class_internal(&(this.use_as____print_str_t), ptTarget, __print_str_t);

    do {
        if(output_adapter(*(ptTarget->chPrintStr))) { 
            ptTarget->chPrintStr++;
        }
    }while(*(ptTarget->chPrintStr) != '\0');
    ptTarget->chPrintStr = 0;
    
    vsf_pt_end();
}

/*============================ Print str init ================================*/

bool print_str_init(print_str_t *ptObj, uint8_t *pchStr)
{   
    do {
        if((NULL == ptObj) && (NULL == pchStr)) {
            break;
        }
        
        class_internal(&(ptObj->param.use_as____print_str_t), ptThis, __print_str_t);
        this.chPrintStr = pchStr;
        init_vsf_pt(print_str_t, ptObj, vsf_priority_0);
        return true;
    } while(0);
    return false;
}

/*=========================== Print str prepare ===============================*/

bool print_str_prepare(vsf_pt(print_str_t) *ptObj, uint8_t *pchStr)
{   
    do {
        if((NULL == ptObj) && (NULL == pchStr)) {
            break;
        }
        
        class_internal(&(ptObj->use_as____print_str_t), ptThis, __print_str_t);
        this.chPrintStr = pchStr;
        return true;
    } while(0);
    return false;
}
