/*
 * uart.h
 *
 *
 */

#ifndef UART_H_
#define UART_H_

int uart2_read();
int uart2_RxAvail(void);
void uart2_init(void);
void uart2_write(int ch);

#endif /* UART_H_ */
