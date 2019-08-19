#ifndef __CHECK_AND_PRINT_H__
#define __CHECK_AND_PRINT_H__

/*============================ INCLUDES ======================================*/
#include <stdint.h>
#include <stdbool.h>
#include "vsf.h"
#include "./check_str.h"
#include "./print_str.h"

/*! \NOTE: Make sure #include "plooc_class.h" is close to the class
 *!        definition and there is NO ANY OTHER module-interface-header file 
 *!        included in this file
 */
#define __PLOOC_CLASS_USE_STRICT_TEMPLATE__
   
#if     defined(__CHECK_AND_PRINT_CLASS_IMPLEMENT)
#   define __PLOOC_CLASS_IMPLEMENT
#elif   defined(__CHECK_AND_PRINT_CLASS_INHERIT)
#   define __PLOOC_CLASS_INHERIT
#endif   

#include "utilities/ooc_class.h"

/*===============================================================*/
declare_class(__check_apple_t)
declare_class(__check_orange_t)
declare_class(__check_hello_t)

declare_vsf_task(check_agent__t)
declare_vsf_task(check_apple_t)
declare_vsf_task(check_orange_t)
declare_vsf_task(check_hello_t)

declare_vsf_task(task_a_t)
declare_vsf_task(task_b_t)
declare_vsf_task(task_c_t)

/*===============================================================*/
def_vsf_task(check_agent__t,
    def_params(
        vsf_fsm_entry_t  fnHandler;     
        bool             bIsWorking;
    ));

def_class(__check_apple_t, 
     which(implement(vsf_task(check_agent__t)))
    /*! no public_member */  ,/*! don't forget the comma here */
    private_member(
        fsm_rt_t  tFsmState;
        vsf_sem_t *pSem;
        vsf_task(check_str_t) tCheck;      
    )
)
end_def_class(__check_apple_t)

def_vsf_task(check_apple_t,
    def_params(
        implement(__check_apple_t)
    )
);

/*===============================================================*/
def_class(__check_orange_t, 
     which(implement(vsf_task(check_agent__t)))
    /*! no public_member */  ,/*! don't forget the comma here */
    private_member(
        fsm_rt_t  tFsmState;
        vsf_sem_t *pSem;
        vsf_task(check_str_t) tCheck;      
    )
)
end_def_class(__check_orange_t)

def_vsf_task(check_orange_t,
    def_params(
        implement(__check_orange_t)
    )
);

/*===============================================================*/
def_class(__check_hello_t, 
     which(implement(vsf_task(check_agent__t)))
    /*! no public_member */  ,/*! don't forget the comma here */
    private_member(
        fsm_rt_t  tFsmState; 
        vsf_sem_t *pSem;
        vsf_task(check_str_t) tCheck;      
    )
)
end_def_class(__check_hello_t)

def_vsf_task(check_hello_t,
    def_params(
        implement(__check_hello_t)
    )
);

/*===============================================================*/
def_vsf_task(task_a_t,
    def_params(
        vsf_sem_t *pSem;
        vsf_pt(print_str_t) tPrint;
    )
);

def_vsf_task(task_b_t,
    def_params(
        vsf_sem_t *pSem;
        vsf_pt(print_str_t) tPrint;
    )
);

def_vsf_task(task_c_t,
    def_params(
        vsf_sem_t *pSem;
        vsf_pt(print_str_t) tPrint;
    )
);

def_interface(i_check_and_print_t)
    void (*PrintInit)    (void);
    bool (*CheckPrepare) (vsf_task(check_apple_t) *ptAppleObj, vsf_task(check_orange_t) *ptOrangeObj, vsf_task(check_hello_t) *ptHelloObj);
end_def_interface(i_check_and_print_t) /*do not remove this for forward compatibility */
//! @}

extern const i_check_and_print_t CHECK_AND_PRINT;
extern void  print_task_init(void);
extern bool  check_task_prepare(vsf_task(check_apple_t) *ptAppleObj, vsf_task(check_orange_t) *ptOrangeObj, vsf_task(check_hello_t) *ptHelloObj);
#endif
