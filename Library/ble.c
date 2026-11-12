/* Bluefruit MOD/startup sequence from Lab 9 sections 4.3-4.4. */
#include "ble.h"
#include "uart1.h"
#include "ES28.h"

#define MOD_PIN (1U << 0)
#define TX_CAPACITY 192U
#define COMMAND_TIMEOUT_MS 1500U
#define RESPONSE_TIMEOUT_MS 2500U
#define RESPONSE_IDLE_MS 20U

static char tx_buffer[TX_CAPACITY];
static unsigned int tx_head;
static unsigned int tx_tail;

static void set_command_mode(bool command_mode) {
    GPIOB->BSRR = command_mode ? MOD_PIN : (MOD_PIN << 16);
}

static bool send_command(const char *command) {
    uint32_t started = milliseconds();
    for (const char *p = command; *p != '\0'; ++p) {
        while (!uart1_tryWrite(*p)) {
            if ((uint32_t)(milliseconds() - started) >= COMMAND_TIMEOUT_MS) {
                return false;
            }
        }
    }

    /* The handout specifies response drain until 10-20 ms idle. */
    bool received = false;
    bool saw_ok = false;
    int previous = -1;
    uint32_t last_byte = milliseconds();
    started = last_byte;
    while ((uint32_t)(milliseconds() - started) < RESPONSE_TIMEOUT_MS) {
        if (uart1_RxAvail()) {
            int ch = uart1_read();
            received = true;
            last_byte = milliseconds();
            if (previous == 'O' && ch == 'K') {
                saw_ok = true;
            }
            previous = ch;
        } else if (received && (uint32_t)(milliseconds() - last_byte) >= RESPONSE_IDLE_MS) {
            break;
        }
    }
    delay_ms(200U);
    return saw_ok;
}

bool ble_init(void) {
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    set_command_mode(true);
    GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE0_Msk)
                 | (GPIO_OUTPUT << GPIO_MODER_MODE0_Pos);
    uart1_init();
    delay_ms(300U);

    bool okay = send_command("AT+FACTORYRESET\r\n");
    if (okay) {
        okay = send_command("AT+GAPDEVNAME=ENGS28-Safe-Kensa\r\n");
    }
    if (okay) {
        okay = send_command("ATZ\r\n");
    }
    set_command_mode(false);
    delay_ms(200U);
    return okay;
}

bool ble_queue(const char *message) {
    unsigned int length = 0U;
    while (message[length] != '\0') {
        ++length;
    }
    if (length >= TX_CAPACITY) {
        return false;
    }
    unsigned int used = (tx_head + TX_CAPACITY - tx_tail) % TX_CAPACITY;
    if (length > TX_CAPACITY - 1U - used) {
        /* A disconnected module must not leave a backlog of stale status. */
        tx_head = tx_tail;
    }
    for (unsigned int i = 0; i < length; ++i) {
        tx_buffer[tx_head] = message[i];
        tx_head = (tx_head + 1U) % TX_CAPACITY;
    }
    return true;
}

void ble_poll(void) {
    if (tx_tail != tx_head && uart1_tryWrite(tx_buffer[tx_tail])) {
        tx_tail = (tx_tail + 1U) % TX_CAPACITY;
    }
}
