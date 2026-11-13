/*
 * USART1 polling driver based on the course USART2 driver and Day 17 slides
 *
 * D1 / PB6: USART1 TX to Bluefruit RXI
 * D0 / PB7: USART1 RX from Bluefruit TXO
 * D5 / PB4: USART1 CTS from Bluefruit RTS
 * D3 / PB3: USART1 RTS to Bluefruit CTS
 */
#include "uart1.h"
#include "ES28.h"

#define UART_BAUD_RATE 9600U
#define APB_CLK 12000000U
#define UART_DATA_MASK 0xFFU

void uart1_init(void) {
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;

    /* PB6 TX and PB7 RX use AF0; PB4 CTS and PB3 RTS use AF4. */
    const uint32_t pins = GPIO_MODER_MODE3_Msk | GPIO_MODER_MODE4_Msk
                        | GPIO_MODER_MODE6_Msk | GPIO_MODER_MODE7_Msk;
    GPIOB->MODER = (GPIOB->MODER & ~pins)
                 | (GPIO_ALTERNATE << GPIO_MODER_MODE3_Pos)
                 | (GPIO_ALTERNATE << GPIO_MODER_MODE4_Pos)
                 | (GPIO_ALTERNATE << GPIO_MODER_MODE6_Pos)
                 | (GPIO_ALTERNATE << GPIO_MODER_MODE7_Pos);
    GPIOB->AFR[0] &= ~(GPIO_AFRL_AFSEL3_Msk | GPIO_AFRL_AFSEL4_Msk
                      | GPIO_AFRL_AFSEL6_Msk | GPIO_AFRL_AFSEL7_Msk);
    GPIOB->AFR[0] |= (GPIO_AF4 << GPIO_AFRL_AFSEL3_Pos)
                   | (GPIO_AF4 << GPIO_AFRL_AFSEL4_Pos);

    RCC->APBENR2 |= RCC_APBENR2_USART1EN;
    USART1->CR1 = 0U;
    USART1->BRR = (APB_CLK + UART_BAUD_RATE / 2U) / UART_BAUD_RATE;
    USART1->CR3 = USART_CR3_RTSE | USART_CR3_CTSE;
    USART1->CR1 = USART_CR1_RE | USART_CR1_TE | USART_CR1_UE;
}

int uart1_RxAvail(void) {
    if ((USART1->ISR & USART_ISR_ORE) != 0U) {
        USART1->ICR = USART_ICR_ORECF;
    }
    return (USART1->ISR & USART_ISR_RXNE_RXFNE) != 0U;
}

int uart1_read(void) {
    return (int)(USART1->RDR & UART_DATA_MASK);
}

/* CTS may stay high if the module is missing: never wait here. */
int uart1_tryWrite(int ch) {
    if ((USART1->ISR & USART_ISR_TXE_TXFNF) == 0U) {
        return 0;
    }
    USART1->TDR = (uint32_t)ch & UART_DATA_MASK;
    return 1;
}
