#ifndef __CHECK_STR_H__
#define __CHECK_STR_H__

/*============================ INCLUDES ======================================*/
#include <stdint.h>
#include <stdbool.h>
#include "vsf.h"

/*! \NOTE: Make sure #include "plooc_class.h" is close to the class
 *!        definition and there is NO ANY OTHER module-interface-header file 
 *!        included in this file
 */
#define __PLOOC_CLASS_USE_STRICT_TEMPLATE__
   
#if     defined(__USER_CHECK_STR_CLASS_IMPLEMENT)
#   define __PLOOC_CLASS_IMPLEMENT
#elif   defined(__USER_CHECK_STR_CLASS_INHERIT)
#   define __PLOOC_CLASS_INHERIT
#endif   

#include "utilities/ooc_class.h"

/*================================ TYPES =====================================*/
typedef enum {
    fsm_rt_Drop = -2,
}check_fsm_rt_t;

declare_class(__check_str_t)

declare_vsf_task(check_str_t)

def_class(__check_str_t, /*!< no inherit or implement */
    /*! no public_member */  ,/*! don't forget the comma here */
    private_member(
        uint8_t chChar;
        uint8_t *pCheckStr;
    )
)
end_def_class(__check_str_t)

def_vsf_task(check_str_t,
    def_params(
        implement(__check_str_t)
    ));

def_interface(i_check_str_t)
    bool (*Init)    (check_str_t *ptObj, uint8_t *pCheckStr);
    bool (*Prepare) (vsf_task(check_str_t) *ptObj, uint8_t *pCheckStr);
end_def_interface(i_user_check_str_t) /*do not remove this for forward compatibility */
//! @}

/*============================ GLOBAL VARIABLES ==============================*/
extern const i_check_str_t CHECK_STR;

extern bool check_str_init(check_str_t *ptObj, uint8_t *pCheckStr);
extern bool check_str_prepare(vsf_task(check_str_t) *ptObj, uint8_t *pCheckStr);

#endif
