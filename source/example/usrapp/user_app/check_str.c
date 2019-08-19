/*============================ INCLUDES ======================================*/
#define __USER_CHECK_STR_CLASS_IMPLEMENT

#include <stdio.h>
#include "./check_str.h"

/*============================ MACROS ========================================*/
#ifndef this
    define this  (*ptThis)
#endif

/*========================== EXTERNAL STATEMENT ==============================*/
extern bool input_adapter(uint8_t *pchByte);

/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
const i_check_str_t CHECK_STR = {
    .Init    = &check_str_init,
    .Prepare = &check_str_prepare
};

/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/
#define CHECK_STR_RESET_FSM()   do { vsf_task_state = START;} while(0)
implement_vsf_task(check_str_t) 
{
    vsf_task_begin();
    
    class_internal(&(this.use_as____check_str_t), ptTarget, __check_str_t);

    //ASSERT(NULL == ptTarget);
    
    enum {
        START = 0,
        RECEIVE_CHAR,
        CHECK_CHAR
    };

    uint8_t chChar;
    
    switch (vsf_task_state) {
        case START:
            vsf_task_state = RECEIVE_CHAR; 
            break;
        
        case RECEIVE_CHAR:     
label_get_byte:            
            if(input_adapter(&(ptTarget->chChar))) {
                vsf_task_state = CHECK_CHAR;                                 //!< tranfer to next state
            } else {
                return fsm_rt_asyn;
            }
            break;
            
        case CHECK_CHAR:
            if(*(ptTarget->pCheckStr) == ptTarget->chChar) {
                ptTarget->pCheckStr++;
                if('\0' == *(ptTarget->pCheckStr)) {
                    CHECK_STR_RESET_FSM();
                    return fsm_rt_cpl;
                } else {
                    vsf_task_state = RECEIVE_CHAR;
                    goto label_get_byte;
                }
            } else {
                CHECK_STR_RESET_FSM();
                return fsm_rt_Drop;
            }
            break;
    }
    vsf_task_end();
}

/*============================ Check str init ================================*/

bool check_str_init(check_str_t *ptObj, uint8_t *pCheckStr)
{   
    do {
        if((NULL == ptObj) || (NULL == pCheckStr)) {
            break;
        }
        
        class_internal(&(ptObj->use_as__vsf_task_t), ptThis, __check_str_t);
        this.pCheckStr = pCheckStr;
        init_vsf_task(check_str_t, ptObj, vsf_priority_0);
        return true;
    }while(0);
    return false;
}

/*=========================== Check str prepare ===============================*/

bool check_str_prepare(vsf_task(check_str_t) *ptObj, uint8_t *pCheckStr)
{   
    do {
        if((NULL == ptObj) || (NULL == pCheckStr)) {
            break;
        }
        
        class_internal(&(ptObj->use_as____check_str_t), ptThis, __check_str_t);
        this.pCheckStr = pCheckStr;
        return true;
    }while(0);
    return false;
}

