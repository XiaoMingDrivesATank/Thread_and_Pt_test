#include "vsf.h"
#include <stdio.h>

/*============================ TYPES =========================================*/
declare_vsf_thread(user_thread_a_t)

def_vsf_thread(user_thread_a_t, 512,

    features_used(
        mem_sharable( )
        mem_nonsharable( )
    )
    
    );

implement_vsf_thread(user_thread_a_t) 
{
    while(1) {

        printf("Hello World!\r\n");
        vsf_delay_ms(1000);
    }
}

void thread_a_set_up(void) 
{
    //! start the user task a
    static NO_INIT user_thread_a_t __user_task_a;
    init_vsf_thread(user_thread_a_t, &__user_task_a, vsf_prio_0);
}

int main(void)
{
    static_task_instance(
        features_used(
            mem_sharable( )
            mem_nonsharable( )
        )
    )
    
    vsf_stdio_init();
    
    thread_a_set_up();
    
    while(1) {
        vsf_yield();
    }
}