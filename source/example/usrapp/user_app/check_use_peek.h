#ifndef __CHECK_USE_PEEK_H__
#define __CHECK_USE_PEEK_H__

/*============================ INCLUDES ======================================*/
#include <stdint.h>
#include <stdbool.h>
#include "vsf.h"
#include "./byte_queue.h"

/*! \NOTE: Make sure #include "plooc_class.h" is close to the class
 *!        definition and there is NO ANY OTHER module-interface-header file 
 *!        included in this file
 */
#define __PLOOC_CLASS_USE_STRICT_TEMPLATE__
   
#if     defined(__CHECK_USE_PEEK_CLASS_IMPLEMENT)
#   define __PLOOC_CLASS_IMPLEMENT
#elif   defined(__CHECK_USE_PEEK_CLASS_INHERIT)
#   define __PLOOC_CLASS_INHERIT
#endif   

#include "utilities/ooc_class.h"

/*============================ MACROS ========================================*/
    
#define IMPLEMENT_CHECK_AGENT(__NAME,...)                                       \
        static vsf_task(check_agent_t) __NAME[] = {__VA_ARGS__}

/*================================ TYPES =====================================*/
declare_vsf_task(check_agent_t)

declare_class(__check_use_peek_t)
declare_vsf_task(check_use_peek_t)
            
def_vsf_task(check_agent_t,
    def_params(
        vsf_fsm_entry_t  fnHandler;     
        bool             bIsWorking;
    ));
            
def_class(__check_use_peek_t, /*!< no inherit or implement */
    /*! no public_member */  ,/*! don't forget the comma here */
    private_member(
        bool            bIsDrop;
        bool            bIsRequestDrop;  
        fsm_rt_t        tFsmState;
        uint8_t         chState;            
        uint8_t         chAgentIndex;       
        uint8_t         chAgentNumber;      
        vsf_task(check_agent_t)   **pptCheckAgents;     
        byte_queue_t    *ptQueue;           
        byte_queue_t    *ptDropQueue;       
    )
)
end_def_class(__check_use_peek_t)

def_vsf_task(check_use_peek_t,
    def_params(
        implement(__check_use_peek_t)
    ));

typedef struct {
    vsf_task(check_agent_t)  **pptCheckAgents;      
    uint8_t        chAgentNumber;       
    byte_queue_t   *ptQueue;            
    byte_queue_t   *ptDropQueue;        
}check_use_peek_cfg_t;

def_interface(i_check_use_peek_t)
    bool     (*Init)    (check_use_peek_t *ptThis, check_use_peek_cfg_t *ptCfg);
    bool     (*Prepare) (vsf_task(check_use_peek_t) *ptThis, check_use_peek_cfg_t *ptCfg);
end_def_interface(i_check_use_peek_t) /*do not remove this for forward compatibility */

/*============================ GLOBAL VARIABLES ==============================*/
extern const i_check_use_peek_t CHECK_USE_PEEK;

extern bool check_use_peek_init(check_use_peek_t *ptObj, check_use_peek_cfg_t *ptCfg);
extern bool check_use_peek_prepare(vsf_task(check_use_peek_t) *ptObj, check_use_peek_cfg_t *ptCfg);
#endif
