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
/*============================ INCLUDES ======================================*/
//#include "app_cfg.h"
#include "vsf.h"
#include <stdio.h>
#include "../user_app/serial.h"

/*============================ MACROS ========================================*/
#define SET     true
#define RESET   false
#define AUTO    true  
#define MANUAL  false   
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
declare_vsf_thread(user_thread_a_t)

def_vsf_thread(user_thread_a_t, 512,

    features_used(
        mem_sharable( )
        mem_nonsharable( )
    )
    
    def_params(
        vsf_trig_t *ptrig;
        vsf_sem_t  *psem;
    ));

declare_vsf_thread(user_thread_b_t)

def_vsf_thread(user_thread_b_t, 512,

    features_used(
        mem_sharable( )
        mem_nonsharable( )
    )
    
    def_params(
        vsf_trig_t *ptrig;
        vsf_sem_t  *psem;
    ));

declare_vsf_thread(user_thread_c_t)

def_vsf_thread(user_thread_c_t, 512,

    features_used(
        mem_sharable( )
        mem_nonsharable( )
    )
    
    def_params(
        vsf_trig_t *ptrig;
        vsf_sem_t  *psem;
    ));

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
static NO_INIT vsf_sem_t user_sem_a;
static NO_INIT vsf_sem_t user_sem_b;
static NO_INIT vsf_sem_t user_sem_c;

static NO_INIT vsf_mutex_t mutex_a;

static NO_INIT vsf_trig_t trig_a;
static NO_INIT vsf_trig_t trig_b;
static NO_INIT vsf_trig_t trig_c;
/*============================ PROTOTYPES ====================================*/
/*============================ IMPLEMENTATION ================================*/


void vsf_kernel_thread_simple_demo(void)
{    
    //! initialise semaphore
    vsf_sem_init(&user_sem_a, 0); 
    vsf_sem_init(&user_sem_b, 0); 
    vsf_sem_init(&user_sem_c, 0); 
    
    vsf_mutex_init(&mutex_a);
    
    vsf_trig_init(&trig_a, RESET, MANUAL);
    vsf_trig_init(&trig_b, RESET, MANUAL);
    vsf_trig_init(&trig_c, RESET, MANUAL);
    
    //! start the user task a
    {
        static NO_INIT user_thread_a_t __user_task_a;
        __user_task_a.param.ptrig = &trig_a;
        __user_task_a.param.psem = &user_sem_a;
        init_vsf_thread(user_thread_a_t, &__user_task_a, vsf_prio_0);
    }
    
    //! start the user task b
    {
        static NO_INIT user_thread_b_t __user_task_b;
        __user_task_b.param.ptrig = &trig_b;
        __user_task_b.param.psem = &user_sem_b;
        init_vsf_thread(user_thread_b_t, &__user_task_b, vsf_prio_0);
    }
    
    //! start the user task c
    {
        static NO_INIT user_thread_c_t __user_task_c;
        __user_task_c.param.ptrig = &trig_c;
        __user_task_c.param.psem = &user_sem_c;
        init_vsf_thread(user_thread_c_t, &__user_task_c, vsf_prio_0);
    }
    
}

/*============================================================================*/
void print_str(char *str)
{    
    //vsf_mutex_enter(&mutex_a, -1);
    
    while(*str != '\0') {
        while(!vsf_serial_out(*str));
        str++;
    }
    
    //vsf_mutex_leave(&mutex_a);
}

/*============================================================================*/
implement_vsf_thread(user_thread_a_t) 
{
    while(1) {
        vsf_sem_pend(this.psem);
        print_str("apple\r\n");
        vsf_trig_set(this.ptrig);
        vsf_yield();
    }
}

implement_vsf_thread(user_thread_b_t) 
{
    while(1) {
        vsf_sem_pend(this.psem);
        print_str("orange\r\n");
        vsf_trig_set(this.ptrig);
        vsf_yield();
    }
}

implement_vsf_thread(user_thread_c_t) 
{
    while(1) {
        vsf_sem_pend(this.psem);
        print_str("hello\r\n");
        vsf_trig_set(this.ptrig);
        vsf_yield();
    }
}

#if VSF_PROJ_CFG_USE_CUBE != ENABLED
int main(void)
{
    static_task_instance(
        features_used(
            mem_sharable( )
            mem_nonsharable( )
        )
    )
    
    vsf_stdio_init();
    
    vsf_kernel_thread_simple_demo();
    
    vsf_sem_post(&user_sem_a);
    vsf_sem_post(&user_sem_b);
    vsf_sem_post(&user_sem_c);
    
#if VSF_KERNEL_CFG_SUPPORT_THREAD == ENABLED
    while(1) { 
        vsf_trig_wait(&trig_a);
        vsf_trig_wait(&trig_b);
        vsf_trig_wait(&trig_c);
        
        vsf_trig_reset(&trig_a);
        vsf_trig_reset(&trig_b);
        vsf_trig_reset(&trig_c);
        
        vsf_sem_post(&user_sem_a);
        vsf_sem_post(&user_sem_b);
        vsf_sem_post(&user_sem_c);
        
        print_str("abc is run\r\n");

        vsf_yield();
    }
#else
    return 0;
#endif
}

#endif