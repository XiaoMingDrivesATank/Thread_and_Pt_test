#ifndef __PRINT_STR_H__
#define __PRINT_STR_H__

/*============================ INCLUDES ======================================*/
#include <stdint.h>
#include <stdbool.h>
#include "vsf.h"

/*! \NOTE: Make sure #include "plooc_class.h" is close to the class
 *!        definition and there is NO ANY OTHER module-interface-header file 
 *!        included in this file
 */
#define __PLOOC_CLASS_USE_STRICT_TEMPLATE__
   
#if     defined(__USER_PRINT_STR_CLASS_IMPLEMENT)
#   define __PLOOC_CLASS_IMPLEMENT
#elif   defined(__USER_PRINT_STR_CLASS_INHERIT)
#   define __PLOOC_CLASS_INHERIT
#endif   

#include "utilities/ooc_class.h"

/*================================ TYPES =====================================*/
declare_class(__print_str_t)

declare_vsf_pt(print_str_t)

def_class(__print_str_t, /*!< no inherit or implement */
    /*! no public_member */  ,/*! don't forget the comma here */
    private_member(
        uint8_t *chPrintStr;
    )
)
end_def_class(__print_str_t)

def_vsf_pt(print_str_t,
    def_params(
        implement(__print_str_t)
    ));

def_interface(i_print_str_t)
    bool (*Init)    (print_str_t *ptObj, uint8_t *pchStr);
    bool (*Prepare) (vsf_pt(print_str_t) *ptObj, uint8_t *pchStr);
end_def_interface(i_print_str_t) /*do not remove this for forward compatibility */
//! @}

/*============================ GLOBAL VARIABLES ==============================*/
extern const i_print_str_t PRINT_STR;

extern bool print_str_init(print_str_t *ptObj, uint8_t *pchStr);
extern bool print_str_prepare(vsf_pt(print_str_t) *ptObj, uint8_t *pchStr);

#endif
