/*============================ INCLUDES ======================================*/
#define __CHECK_USE_PEEK_CLASS_IMPLEMENT

#include <stdio.h>
#include "./check_use_peek.h"

typedef enum {
    fsm_rt_Drop = -2,
}check_fsm_rt_t;
                                                                                    
/*============================ MACROS ========================================*/
#ifndef this
#   define this  (*ptThis)
#endif

const i_check_use_peek_t CHECK_USE_PEEK = {
    .Init    = &check_use_peek_init,
    .Prepare = &check_use_peek_prepare
};

implement_vsf_task(check_agent_t) 
{
    vsf_task_begin();
                                                             
    return this.fnHandler(ptFrame, evt);
    
    vsf_task_end();
}

/*============================  check_use_peek ===============================*/
#define CHECK_USE_PEEK_RESET_FSM()   do { vsf_task_state = START;} while(0)
implement_vsf_task(check_use_peek_t) 
{
    vsf_task_begin();
    
    class_internal(&(this.use_as____check_use_peek_t), ptTarget, __check_use_peek_t);
    
    uint8_t   chByte;
    uint8_t   chAdd;
    
    enum {
        START = 0,                 
        CHECK_AGENT,  
        CHECK_STATE,
        SET_WORKING,            
        CHECK_INDEX,       
        CHECK_DROP,        
        CHECK_WORKING      
    };
    
    switch (vsf_task_state) {
        case START:
            ptTarget->chAgentIndex = 0;                   
            ptTarget->bIsRequestDrop = true;
            vsf_task_state = CHECK_AGENT;
            //break;
            
        case CHECK_AGENT:
            ptTarget->tFsmState = vsf_task_call_task(check_agent_t, ptTarget->pptCheckAgents[ptTarget->chAgentIndex]);
            if(ptTarget->tFsmState != fsm_rt_on_going) {
                vsf_task_state = CHECK_STATE;
            }
            break;

        case CHECK_STATE:
            ptTarget->bIsDrop = false;
            if(fsm_rt_cpl == ptTarget->tFsmState) {                                  
                for(chAdd = 0; chAdd < ptTarget->chAgentNumber; chAdd++) {
                    ptTarget->pptCheckAgents[chAdd]->bIsWorking = false;
                }
                BYTE_QUEUE.Peek.GetAllPeeked(ptTarget->ptQueue); 
                CHECK_USE_PEEK_RESET_FSM();     
            } else if(fsm_rt_Drop == ptTarget->tFsmState) {                             
                ptTarget->bIsDrop = true;
                ptTarget->bIsRequestDrop &= ptTarget->bIsDrop;
                vsf_task_state = SET_WORKING;
            }                                                                 
            break;
            
        case SET_WORKING:
            if(ptTarget->bIsDrop != false) {               
                ptTarget->pptCheckAgents[ptTarget->chAgentIndex]->bIsWorking = true;
            }
            vsf_task_state = CHECK_INDEX;
            break;
            
        case CHECK_INDEX:
label_chk_index:
            ptTarget->chAgentIndex++;                                     
            if(ptTarget->chAgentIndex >= ptTarget->chAgentNumber) {                    
                ptTarget->chAgentIndex = 0;
                vsf_task_state = CHECK_DROP;
            } else {         
                vsf_task_state = CHECK_WORKING;
                goto label_chk_working;
            }
            break;
            
        case CHECK_DROP:
            if(ptTarget->bIsRequestDrop) {
                for(chAdd = 0; chAdd < ptTarget->chAgentNumber; chAdd++) {
                    ptTarget->pptCheckAgents[chAdd]->bIsWorking = false;
                }
                BYTE_QUEUE.Dequeue(ptTarget->ptQueue, &chByte); 
                BYTE_QUEUE.Enqueue(ptTarget->ptDropQueue, chByte);
                CHECK_USE_PEEK_RESET_FSM();
            } else {
                ptTarget->bIsRequestDrop = true; 
                vsf_task_state = CHECK_WORKING;
            }
            break;
            
        case CHECK_WORKING:
label_chk_working:
            if(ptTarget->pptCheckAgents[ptTarget->chAgentIndex]->bIsWorking != false) {           
                vsf_task_state = CHECK_INDEX;
                goto label_chk_index;                                 
            } else {
                vsf_task_state = CHECK_AGENT;
                BYTE_QUEUE.Peek.Reset(ptTarget->ptQueue, NULL);
            }
            break;
    }
    vsf_task_end();
}

/*=========================  check_use_peek_init =============================*/
bool check_use_peek_init(check_use_peek_t *ptObj, check_use_peek_cfg_t *ptCfg)
{
    do{
        if((NULL == ptObj) || (NULL == ptCfg)) {
            break;
        }
        
        class_internal(&(ptObj->param.use_as____check_use_peek_t), ptThis, __check_use_peek_t);
        
        ptThis->chState         = 0;
        ptThis->pptCheckAgents  = ptCfg->pptCheckAgents;
        ptThis->chAgentIndex    = 0;
        ptThis->chAgentNumber   = ptCfg->chAgentNumber;
        ptThis->ptQueue         = ptCfg->ptQueue;
        ptThis->ptDropQueue     = ptCfg->ptDropQueue;
        ptThis->bIsRequestDrop  = true;                                        
        
        init_vsf_task(check_use_peek_t, ptObj, vsf_priority_0);                
        return true;
    }while(0);
    return false;
}

/*=========================  check_use_peek_prepare ==========================*/
bool check_use_peek_prepare(vsf_task(check_use_peek_t) *ptObj, check_use_peek_cfg_t *ptCfg)
{
    do{
        if((NULL == ptObj) || (NULL == ptCfg)) {
            break;
        }
        
        class_internal(&(ptObj->use_as____check_use_peek_t), ptThis, __check_use_peek_t);
        
        ptThis->chState         = 0;
        ptThis->pptCheckAgents  = ptCfg->pptCheckAgents;
        ptThis->chAgentIndex    = 0;
        ptThis->chAgentNumber   = ptCfg->chAgentNumber;
        ptThis->ptQueue         = ptCfg->ptQueue;
        ptThis->ptDropQueue     = ptCfg->ptDropQueue;
        ptThis->bIsRequestDrop  = true;
        return true;
    }while(0);
    return false;
}
