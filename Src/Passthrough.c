/*
 * Day 17 USART passthrough exercise
 *
 * D1 / PB6, D0 / PB7, D5 / PB4, D3 / PB3: USART1 and flow control
 * PA2 / PA3: ST-LINK USB console on USART2
 */
#include <stdio.h>
#include "uart.h"
#include "uart1.h"

int main(void) {
    int pending = -1;
    uart1_init();
    uart2_init();
    printf("UART1/USART2 passthrough at 9600 baud\r\n");
    while (1) {
        if (pending < 0 && uart2_RxAvail()) {
            pending = uart2_read();
        }
        if (pending >= 0 && uart1_tryWrite(pending)) {
            pending = -1;
        }
        if (uart1_RxAvail()) {
            uart2_write(uart1_read());
        }
    }
}
