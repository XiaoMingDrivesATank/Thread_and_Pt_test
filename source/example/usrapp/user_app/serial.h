#include "vsf.h"
#include "Device.h"
#include <stdio.h>

extern void uart_config(void);

/*================================= UART ======================================*/
void vsf_uart_init(void)
{
    uart_config();
}

bool vsf_serial_in(uint8_t *pchByte)
{
    if(CMSDK_UART0->STATE & CMSDK_UART_STATE_RXBF_Msk) {
        *pchByte = (uint8_t)(CMSDK_UART0->DATA);
        return true;
    } else {
        return false;
    }
}

bool vsf_serial_out(uint8_t chByte)
{
    if(!(CMSDK_UART0->STATE & CMSDK_UART_STATE_TXBF_Msk)) {
        CMSDK_UART0->DATA = (uint32_t)chByte;
        (*(volatile uint32_t *)0x41000000) = chByte;
        return true;
    } else {
        return false;
    }
}
