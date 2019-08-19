/*============================ INCLUDES ======================================*/
#define __CHECK_AND_PRINT_CLASS_IMPLEMENT

#include <stdio.h>
#include "./check_and_print.h"

                                                                                           

/*============================ MACROS ========================================*/
#ifndef this
    define this  (*ptThis)
#endif

static NO_INIT vsf_sem_t s_tTaskASem;
static NO_INIT vsf_sem_t s_tTaskBSem;
static NO_INIT vsf_sem_t s_tTaskCSem;

const i_check_and_print_t CHECK_AND_PRINT = {
    .PrintInit    = &print_task_init,
    .CheckPrepare = &check_task_prepare
};
                                                                                           
/*============================ CHECK TASK  ===================================*/

#define CHECK_APPLE_RESET_FSM()   do { vsf_task_state = 0;} while(0)
implement_vsf_task(check_apple_t) 
{
    vsf_task_begin();
    
    class_internal(&(this.use_as____check_apple_t), ptTarget, __check_apple_t);
    
    enum {
        START = 0,
        CHECK,
        CHECK_STATE,
        SET_SEM
    };
    
    switch (vsf_task_state) {                                                                      
        case START:                       
            CHECK_STR.Prepare(&ptTarget->tCheck, (uint8_t *)"apple");
            vsf_task_state = CHECK;                           
            break;
        
        case CHECK:
            ptTarget->tFsmState = vsf_task_call_task(check_str_t, &ptTarget->tCheck);
            if((ptTarget->tFsmState != fsm_rt_on_going) && (ptTarget->tFsmState != fsm_rt_asyn)) {
                vsf_task_state = CHECK_STATE;
            }
            break;
        
        case CHECK_STATE:
            if(fsm_rt_cpl == ptTarget->tFsmState) {
                vsf_task_state = SET_SEM; 
            } else if(fsm_rt_Drop == ptTarget->tFsmState) {                   
                CHECK_APPLE_RESET_FSM();
                return fsm_rt_Drop;
            }
            break;
            
        case SET_SEM:                                                                    
            vsf_sem_post(ptTarget->pSem);                    
            CHECK_APPLE_RESET_FSM();
            return fsm_rt_cpl;
    }
    vsf_task_end();
}

#define CHECK_ORANGE_RESET_FSM()   do { vsf_task_state = 0;} while(0)
implement_vsf_task(check_orange_t) 
{
    vsf_task_begin();
    
    class_internal(&(this.use_as____check_orange_t), ptTarget, __check_orange_t);
    
    enum {
        START = 0,
        CHECK_STATE,
        CHECK,
        SET_SEM
    };
    
    switch (vsf_task_state) {
        case START:                       
            CHECK_STR.Prepare(&ptTarget->tCheck, (uint8_t *)"orange");
            vsf_task_state = CHECK;                           
            break;
        
        case CHECK:
            ptTarget->tFsmState = vsf_task_call_task(check_str_t, &ptTarget->tCheck);
            if((ptTarget->tFsmState != fsm_rt_on_going) && (ptTarget->tFsmState != fsm_rt_asyn)) {
                vsf_task_state = CHECK_STATE;
            }
            break;
        
        case CHECK_STATE:
            if(fsm_rt_cpl == ptTarget->tFsmState) {
               vsf_task_state = SET_SEM; 
            } else if(fsm_rt_Drop == ptTarget->tFsmState) {                   
                CHECK_ORANGE_RESET_FSM();
                return fsm_rt_Drop;
            }
            break;
            
        case SET_SEM:                                                                    
            vsf_sem_post(ptTarget->pSem);                    
            CHECK_ORANGE_RESET_FSM();
            return fsm_rt_cpl;
    }
    vsf_task_end();
}

#define CHECK_HELLO_RESET_FSM()   do { vsf_task_state = 0;} while(0)
implement_vsf_task(check_hello_t) 
{
    vsf_task_begin();
    
    class_internal(&(this.use_as____check_hello_t), ptTarget, __check_hello_t);
    
    enum {
        START = 0,
        CHECK_STATE,
        CHECK,
        SET_SEM
    };
    
    switch (vsf_task_state) {
        case START:                       
            CHECK_STR.Prepare(&ptTarget->tCheck, (uint8_t *)"hello");
            vsf_task_state = CHECK;                           
            break;
        
        case CHECK:
            ptTarget->tFsmState = vsf_task_call_task(check_str_t, &ptTarget->tCheck);
            if((ptTarget->tFsmState != fsm_rt_on_going) && (ptTarget->tFsmState != fsm_rt_asyn)) {
                vsf_task_state = CHECK_STATE;
            }
            break;
        
        case CHECK_STATE:
            if(fsm_rt_cpl == ptTarget->tFsmState) {
               vsf_task_state = SET_SEM; 
            } else if(fsm_rt_Drop == ptTarget->tFsmState) {                   
                CHECK_HELLO_RESET_FSM();
                return fsm_rt_Drop;
            }
            break;
            
        case SET_SEM:                                                                    
            vsf_sem_post(ptTarget->pSem);                    
            CHECK_HELLO_RESET_FSM();
            return fsm_rt_cpl;
    }
    vsf_task_end();
}

bool check_task_prepare(vsf_task(check_apple_t) *ptAppleObj, vsf_task(check_orange_t) *ptOrangeObj, 
                        vsf_task(check_hello_t) *ptHelloObj)
{
    do {
        if((NULL == ptAppleObj) || (NULL == ptOrangeObj) || (NULL == ptHelloObj)) {
            break;
        }
        
        do {
            vsf_sem_init(&s_tTaskASem, 0);
            class_internal(&(ptAppleObj->use_as____check_apple_t), ptThis, __check_apple_t);
            this.fnHandler = (vsf_fsm_entry_t)&vsf_task_func(check_apple_t);
            this.pSem = &s_tTaskASem;
            this.bIsWorking = false;
        } while(0);
        
        do {
            vsf_sem_init(&s_tTaskBSem, 0);
            class_internal(&(ptOrangeObj->use_as____check_orange_t), ptThis, __check_orange_t);
            this.fnHandler = (vsf_fsm_entry_t)&vsf_task_func(check_orange_t);
            this.pSem = &s_tTaskBSem;
            this.bIsWorking = false;
        } while(0);
        
        do {
            vsf_sem_init(&s_tTaskCSem, 0);
            class_internal(&(ptHelloObj->use_as____check_hello_t), ptThis, __check_hello_t);
            this.fnHandler = (vsf_fsm_entry_t)&vsf_task_func(check_hello_t);
            this.pSem = &s_tTaskCSem;
            this.bIsWorking = false;
        } while(0);
        
        return true;
    } while(0);
    return false;
}

/*============================ PRINT TASK  ===================================*/
#define TASK_A_RESET_FSM()   do { vsf_task_state = 0;} while(0)
implement_vsf_task(task_a_t) 
{
    vsf_task_begin();
    
    enum {
        START = 0,
        WAIT_SEM,
        PRINT
    };
    
    switch (vsf_task_state) {
        case START:                       
            vsf_task_state = WAIT_SEM;                           
            break;
        
        case WAIT_SEM:
             vsf_task_wait_until(vsf_sem_pend(this.pSem));
             PRINT_STR.Prepare(&this.tPrint, (uint8_t *)"apple\n\r");
             vsf_task_state = PRINT; 
            break;
            
        case PRINT:
            vsf_task_call_pt(print_str_t, &ptTarget->tPrint);
                
            TASK_A_RESET_FSM();
            break;
    }
    vsf_task_end();
}

#define TASK_B_RESET_FSM()   do { vsf_task_state = 0;} while(0)
implement_vsf_task(task_b_t) 
{
    vsf_task_begin();
    
    enum {
        START = 0,
        WAIT_SEM,
        PRINT
    };

    switch (vsf_task_state) {
        case START:                       
            vsf_task_state = WAIT_SEM;                           
            break;
        
        case WAIT_SEM:
             vsf_task_wait_until(vsf_sem_pend(this.pSem));
             PRINT_STR.Prepare(&this.tPrint, (uint8_t *)"orange\n\r");
             vsf_task_state = PRINT;  
            break;
            
        case PRINT:
//            if(fsm_rt_cpl == vsf_task_call_task(print_str_t, &this.tPrint)) {
//                TASK_B_RESET_FSM();
//            } else {
//                break;
//            }
            break;
    }
    vsf_task_end();
}

#define TASK_C_RESET_FSM()   do { vsf_task_state = 0;} while(0)
implement_vsf_task(task_c_t) 
{
    vsf_task_begin();
    
    enum {
        START = 0,
        WAIT_SEM,
        PRINT
    };
    
    switch (vsf_task_state) {
        case START:                       
            vsf_task_state = WAIT_SEM;                           
            break;
        
        case WAIT_SEM:
             vsf_task_wait_until(vsf_sem_pend(this.pSem));
             PRINT_STR.Prepare(&this.tPrint, (uint8_t *)"world\n\r");
             vsf_task_state = PRINT;  
            break;
            
        case PRINT:
//            if(fsm_rt_cpl == vsf_task_call_task(print_str_t, &this.tPrint)) {
//                TASK_C_RESET_FSM();
//            } else {
//                break;
//            }
            break;
    }
    vsf_task_end();
}

void print_task_init(void)
{
    static NO_INIT task_a_t  s_tTaskA;
    static NO_INIT task_b_t  s_tTaskB;
    static NO_INIT task_c_t  s_tTaskC;
    
    vsf_sem_init(&s_tTaskASem, 0);
    vsf_sem_init(&s_tTaskBSem, 0);
    vsf_sem_init(&s_tTaskCSem, 0);
    s_tTaskA.param.pSem = &s_tTaskASem;
    s_tTaskB.param.pSem = &s_tTaskBSem;
    s_tTaskC.param.pSem = &s_tTaskCSem;
    
    init_vsf_task(task_a_t, &s_tTaskA, vsf_priority_0);
    init_vsf_task(task_b_t, &s_tTaskB, vsf_priority_0);
    init_vsf_task(task_c_t, &s_tTaskC, vsf_priority_0);
}

