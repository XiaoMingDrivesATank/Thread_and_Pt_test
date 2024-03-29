/*****************************************************************************
 *   Copyright(C)2009-2019 by VSF Team                                       *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *     http://www.apache.org/licenses/LICENSE-2.0                            *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 ****************************************************************************/

#ifndef __CODE_REGION_H__
#define __CODE_REGION_H__

/*============================ INCLUDES ======================================*/
#include <stdbool.h>
#include <stdint.h>

/*! \brief How To Define and Use your own CODE_REGION
 *!        Example:

    static void __code_region_example_on_enter(void *pobj, void *plocal)
    {
        printf("-------enter-------\r\n");
    }

    static void __code_region_example_on_leave(void *pobj,void *plocal)
    {
        printf("-------leave-------\r\n");
    }

    const static i_code_region_t __example_code_region = {
        .OnEnter = __code_region_example_on_enter,
        .OnLeave = __code_region_example_on_leave,
    };


    void main(void)
    {
        ...
        code_region(&__example_code_region, NULL){
            printf("\tbody\r\n");
        }
        ...
    }

Output:

-------enter-------
        body
-------leave-------


 *! \note How to use code_region()
 *!       Syntax:
 *!             code_region(<Address of i_code_region_t obj>, <Object Address>) {
 *!                 //! put your code here
 *!             }
 *!
 *! \note <Address of i_code_region_t obj>: this can be NULL, if so, 
 *!         DEFAULT_CODE_REGION_NONE will be used.
 *! 
 *! \note <Object Address>: it is the address of the object you want to pass to 
 *!         your OnEnter and OnLeave functions. It can be NULL
 *!
 *! \note A local object will be generated from users' stack, the size is specified
 *!         by i_code_region_t.chLocalSize. The address of this local object will
 *!         be passed to your OnEnter and OnLeave functions. You can use it to
 *!         store some local status.
 *! 
 *! \name   List of Default Code Regions
 *! @{
 *!         DEFAULT_CODE_REGION_ATOM_CODE           //!< interrupt-safe region
 *!         DEFAULT_CODE_REGION_NONE                //!< do nothing
 *! @}
 */


/*============================ MACROS ========================================*/
#ifndef IAR_PATCH_CODE_REGION_LOCAL_SIZE
#   define IAR_PATCH_CODE_REGION_LOCAL_SIZE     4
#endif

/*============================ MACROFIED FUNCTIONS ===========================*/

#if __IS_COMPILER_IAR__
#   define __CODE_REGION(__REGION_ADDR)                                         \
    for(code_region_t *pcode_region = (code_region_t *)(__REGION_ADDR);         \
        NULL != pcode_region;                                                   \
        pcode_region = NULL)                                                    \
        for(uint8_t local[IAR_PATCH_CODE_REGION_LOCAL_SIZE],                    \
                TPASTE2(__code_region_, __LINE__) = 1;                          \
            TPASTE2(__code_region_, __LINE__)-- ?                               \
                (pcode_region->pmethods->OnEnter(  pcode_region->ptarget, local)\
                    ,1)                                                         \
                : 0;                                                            \
            pcode_region->pmethods->OnLeave(pcode_region->ptarget, local))

#   define __CODE_REGION_START(__REGION_ADDR)   __CODE_REGION(__REGION_ADDR) {
#   define __CODE_REGION_END()                  }

#   define __CODE_REGION_SIMPLE(__REGION_ADDR, ...)                             \
    do {if (NULL != (__REGION_ADDR)) {                                          \
        code_region_t *pcode_region = (code_region_t *)(__REGION_ADDR);         \
        uint8_t local[IAR_PATCH_CODE_REGION_LOCAL_SIZE];                        \
        pcode_region->pmethods->OnEnter(pcode_region->ptarget, local);          \
        __VA_ARGS__;                                                            \
        pcode_region->pmethods->OnLeave(pcode_region->ptarget, local);          \
    } } while(0);

#   define __CODE_REGION_SIMPLE_START(__REGION_ADDR, ...)                       \
    do {if (NULL != (__REGION_ADDR)) {                                          \
        code_region_t *pcode_region = (code_region_t *)(__REGION_ADDR);         \
        uint8_t local[IAR_PATCH_CODE_REGION_LOCAL_SIZE];                        \
        pcode_region->pmethods->OnEnter(pcode_region->ptarget, local);          

#   define __CODE_REGION_SIMPLE_END(__REGION_ADDR, ...)                         \
        pcode_region->pmethods->OnLeave(pcode_region->ptarget, local);          \
    } } while(0);

#else
#   define __CODE_REGION(__REGION_ADDR)                                         \
    for(code_region_t *pcode_region = (code_region_t *)(__REGION_ADDR);         \
        NULL != pcode_region;                                                   \
        pcode_region = NULL)                                                    \
        for(uint8_t local[pcode_region->pmethods->local_obj_size],              \
                TPASTE2(__code_region_, __LINE__) = 1;                          \
            TPASTE2(__code_region_, __LINE__)-- ?                               \
                (pcode_region->pmethods->OnEnter(  pcode_region->ptarget, local)\
                    ,1)                                                         \
                : 0;                                                            \
            pcode_region->pmethods->OnLeave(pcode_region->ptarget, local))

#   define __CODE_REGION_START(__REGION_ADDR)   __CODE_REGION(__REGION_ADDR) {
#   define __CODE_REGION_END()                  }


#   define __CODE_REGION_SIMPLE(__REGION_ADDR, ...)                             \
    do {if (NULL != (__REGION_ADDR)) {                                          \
        code_region_t *pcode_region = (code_region_t *)(__REGION_ADDR);         \
        uint8_t local[pcode_region->pmethods->local_obj_size];                  \
        pcode_region->pmethods->OnEnter(pcode_region->ptarget, local);          \
        __VA_ARGS__;                                                            \
        pcode_region->pmethods->OnLeave(pcode_region->ptarget, local);          \
    } }while(0);


#   define __CODE_REGION_SIMPLE_START(__REGION_ADDR, ...)                       \
    do {if (NULL != (__REGION_ADDR)) {                                          \
        code_region_t *pcode_region = (code_region_t *)(__REGION_ADDR);         \
        uint8_t local[pcode_region->pmethods->local_obj_size];                  \
        pcode_region->pmethods->OnEnter(pcode_region->ptarget, local);          

#   define __CODE_REGION_SIMPLE_END(__REGION_ADDR, ...)                         \
        pcode_region->pmethods->OnLeave(pcode_region->ptarget, local);          \
    } } while(0);

#endif
        
#define EXIT_CODE_REGION()                                                      \
            pcode_region->ptMethods->OnLeave(pcode_region->ptarget, local)
#define exit_code_region()  EXIT_CODE_REGION()

#define CODE_REGION(__REGION_ADDR)          __CODE_REGION((__REGION_ADDR))
#define code_region(__REGION_ADDR)          __CODE_REGION((__REGION_ADDR))

#define CODE_REGION_START(__REGION_ADDR)    __CODE_REGION_START((__REGION_ADDR))
#define CODE_REGION_END()                   __CODE_REGION_END()

#define CODE_REGION_SIMPLE(__REGION_ADDR, ...)                                  \
            __CODE_REGION_SIMPLE((__REGION_ADDR), __VA_ARGS__)
#define code_region_simple(__REGION_ADDR, ...)                                  \
            __CODE_REGION_SIMPLE((__REGION_ADDR), __VA_ARGS__)

#define CODE_REGION_SIMPLE_START(__REGION_ADDR)                                 \
            __CODE_REGION_SIMPLE_START((__REGION_ADDR))
#define CODE_REGION_SIMPLE_END()                                                \
            __CODE_REGION_SIMPLE_END()


/*============================ TYPES =========================================*/
typedef struct {
    uint_fast8_t    local_obj_size;
    void (*OnEnter)(void *pobj, void *plocal);
    void (*OnLeave)(void *pobj, void *plocal);
}i_code_region_t;

typedef struct {
    void *ptarget;
    i_code_region_t *pmethods;
} code_region_t;

/*============================ GLOBAL VARIABLES ==============================*/
extern const code_region_t DEFAULT_CODE_REGION_ATOM_CODE;
extern const code_region_t DEFAULT_CODE_REGION_NONE;
/*============================ PROTOTYPES ====================================*/

#endif
