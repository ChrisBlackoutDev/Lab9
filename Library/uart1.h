#ifndef UART1_H_
#define UART1_H_

void uart1_init(void);
int uart1_RxAvail(void);
int uart1_read(void);
int uart1_tryWrite(int ch);

#endif
