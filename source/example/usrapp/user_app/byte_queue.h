#ifndef __BYTE_QUEUE_H__
#define __BYTE_QUEUE_H__

/*============================ INCLUDES ======================================*/
#include <stdint.h>
#include <stdbool.h>
#include "vsf.h"
#include "../plooc/plooc.h"
#include "./customize/t_queue.h"

/*! \NOTE: Make sure #include "plooc_class.h" is close to the class
 *!        definition and there is NO ANY OTHER module-interface-header file 
 *!        included in this file
 */
#define __PLOOC_CLASS_USE_STRICT_TEMPLATE__
   
#if     defined(__BYTE_QUEUE_CLASS_IMPLEMENT)
#   define __PLOOC_CLASS_IMPLEMENT
#elif   defined(__BYTE_QUEUE_CLASS_INHERIT)
#   define __PLOOC_CLASS_INHERIT
#endif   

#include "utilities/ooc_class.h"

/*================================ TYPES =====================================*/
//Queue class
T_EXTERN_QUEUE(byte, def, uint8_t, uint16_t);

//PEEK context class
T_EXTERN_CONTEXT(def, uint16_t);

def_interface(i_byte_queue_t)
    void (*Init)     (byte_queue_t *ptThis, uint8_t *pchBuffer, uint16_t hwSize);
    bool (*Enqueue)  (byte_queue_t *ptThis, uint8_t chObj);
    bool (*Dequeue)  (byte_queue_t *ptThis, uint8_t *pchAddr);
    struct {
        bool (*Reset)         (byte_queue_t *ptThis, peek_context_t *ptContext);
        bool (*Resume)        (byte_queue_t *ptThis, peek_context_t *ptContext);
        bool (*ReadByte)      (byte_queue_t *ptThis, uint8_t *pchAddr);
        bool (*GetAllPeeked)  (byte_queue_t *ptThis);
    }Peek;
end_def_interface(i_byte_queue_t) /*do not remove this for forward compatibility */
//! @}

/*============================ GLOBAL VARIABLES ==============================*/
extern const i_byte_queue_t BYTE_QUEUE;
    
#endif
