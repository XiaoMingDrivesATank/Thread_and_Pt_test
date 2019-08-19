#include "app_cfg.h"
#include "vsf.h"
#include <stdio.h>

#include "../user_app/serial.h"
#include "../user_app/byte_queue.h"
#include "../user_app/check_str.h"
#include "../user_app/print_str.h"
#include "../user_app/check_and_print.h"
#include "../user_app/check_use_peek.h"

extern void vsf_uart_init(void);
extern bool vsf_serial_in(uint8_t *pchByte);
extern bool vsf_serial_out(uint8_t chByte);

/*=============================== DEFINE =====================================*/
static byte_queue_t s_tFifoIn;
static byte_queue_t s_tFifoOut;
static byte_queue_t s_tDorpByte;

/*============================= UART FOR QUEUE ===============================*/
declare_vsf_task(uart_input_t)
declare_vsf_task(uart_output_t)

def_vsf_task(uart_input_t,
    def_params(
        uint8_t chByte;
    )
);
    
def_vsf_task(uart_output_t,
    def_params(
        uint8_t chByte;
    )
);
    
#define UART_INPUT_RESET_FSM()   do { vsf_task_state = 0;} while(0)
implement_vsf_task(uart_input_t) 
{
    vsf_task_begin();
    
    enum {
        START = 0,
        REV_BYTE,
        EN_QUEUE
    };
    
    switch (vsf_task_state) {
        case START:                       
            vsf_task_state = REV_BYTE;                           
            break;
        
        case REV_BYTE:
            if (vsf_serial_in(&this.chByte)) {
               vsf_task_state = EN_QUEUE;  
            }
            break;
            
        case EN_QUEUE:
            if(BYTE_QUEUE.Enqueue(&s_tFifoIn, this.chByte)) {
                UART_INPUT_RESET_FSM();
            }
            break;
    }
    vsf_task_end();
}

#define UART_OUTPUT_RESET_FSM()   do { vsf_task_state = 0;} while(0)
implement_vsf_task(uart_output_t) 
{
    vsf_task_begin();
    
    enum {
        START = 0,
        DE_QUEUE,
        SEN_BYTE
    };
    
    switch (vsf_task_state) {
        case START:                       
            vsf_task_state = DE_QUEUE;                           
            break;
        
        case DE_QUEUE:
            if (BYTE_QUEUE.Dequeue(&s_tFifoOut, &this.chByte)) {
               vsf_task_state = SEN_BYTE;  
            }
            break;
            
        case SEN_BYTE:
            if(vsf_serial_out(this.chByte)) {
                UART_INPUT_RESET_FSM();
            }
            break;
    }
    vsf_task_end();
}

void uart_to_queue_run(void) 
{
    static NO_INIT uart_input_t  s_tInputTask;
    static NO_INIT uart_output_t s_tOutputTask;
    init_vsf_task(uart_input_t,  &s_tInputTask,  vsf_priority_0);
    init_vsf_task(uart_output_t, &s_tOutputTask, vsf_priority_0);
}

/*============================ OUTPUT ADAPTER  ================================*/
bool output_adapter(uint8_t chByte)  
{
    return BYTE_QUEUE.Enqueue(&s_tFifoOut, chByte);
}

/*============================ INPUT ADAPTER  =================================*/
bool input_adapter(uint8_t *pchByte)
{
    return BYTE_QUEUE.Peek.ReadByte(&s_tFifoIn, pchByte);
}

/*================================= MAIN ======================================*/
int main()
{  
    vsf_stdio_init();
    vsf_uart_init();
    uart_to_queue_run();
    
    //Array init
    static uint8_t s_chInString[1030];
    static uint8_t s_chOutString[1030];
    static uint8_t s_chDropString[256];
    
    static NO_INIT vsf_task(check_apple_t)   s_tApple;
    static NO_INIT vsf_task(check_orange_t)  s_tOrange;
    static NO_INIT vsf_task(check_hello_t)   s_tHello;
    static check_use_peek_t s_tCheckUsePeek;
    
    //Queue init
    BYTE_QUEUE.Init(&s_tFifoIn,   s_chInString,   UBOUND(s_chInString));
    BYTE_QUEUE.Init(&s_tFifoOut,  s_chOutString,  UBOUND(s_chOutString));
    BYTE_QUEUE.Init(&s_tDorpByte, s_chDropString, UBOUND(s_chDropString));
    
    //Check init
    check_task_prepare(&s_tApple, &s_tOrange, &s_tHello);
    
    //Print init  
    print_task_init();
    
    IMPLEMENT_CHECK_AGENT(*ptCheckAgents,
        (vsf_task(check_agent_t) *)&s_tApple,
        (vsf_task(check_agent_t) *)&s_tOrange,
        (vsf_task(check_agent_t) *)&s_tHello
    );
    
    static check_use_peek_cfg_t s_tCheckUsePeekCfg = {
        ptCheckAgents,
        UBOUND(ptCheckAgents),
        &s_tFifoIn,
        &s_tDorpByte,
    };
    
    //Check_use_peek init
    CHECK_USE_PEEK.Init(&s_tCheckUsePeek, &s_tCheckUsePeekCfg);
    
    while(1) {
        vsf_delay_ms(1000);
    }
}