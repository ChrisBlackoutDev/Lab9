/* Day 17 BLE passthrough exercise, kept separate from the prototype main(). */
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
