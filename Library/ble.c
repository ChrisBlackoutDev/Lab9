/*
 * Bluefruit command setup and nonblocking data output
 *
 * D10 / PB0: Bluefruit MOD pin; HIGH selects command mode
 * D1 / PB6, D0 / PB7, D5 / PB4, D3 / PB3: USART1 and flow control
 */
#include "ble.h"
#include "uart1.h"
#include "ES28.h"

#define MOD_BITNUM 0U
#define MOD_BITMASK (1U << MOD_BITNUM)
#define GPIO_BSRR_RESET_BIT_OFFSET 16U
#define TX_CAPACITY 192U
#define COMMAND_TIMEOUT_MS 1500U
#define RESPONSE_TIMEOUT_MS 2500U
#define RESPONSE_IDLE_MS 20U
#define COMMAND_STARTUP_MS 300U
#define COMMAND_SETTLE_MS 200U
#define DATA_MODE_SETTLE_MS 200U

static char tx_buffer[TX_CAPACITY] = {0};
static unsigned int tx_head = 0U;
static unsigned int tx_tail = 0U;

static void SetCommandMode(bool command_mode) {
    GPIOB->BSRR = command_mode ? MOD_BITMASK
                             : (MOD_BITMASK << GPIO_BSRR_RESET_BIT_OFFSET);
}

static bool SendCommand(const char *command) {
    uint32_t command_started = milliseconds();
    for (const char *character = command; *character != '\0'; ++character) {
        while (!uart1_tryWrite(*character)) {
            if ((uint32_t)(milliseconds() - command_started) >= COMMAND_TIMEOUT_MS) {
                return false;
            }
        }
    }

    /* The handout specifies response drain until 10-20 ms idle. */
    bool received_response = false;
    bool received_ok = false;
    int previous_character = -1;
    uint32_t last_byte = milliseconds();
    uint32_t response_started = last_byte;
    while ((uint32_t)(milliseconds() - response_started) < RESPONSE_TIMEOUT_MS) {
        if (uart1_RxAvail()) {
            int response_character = uart1_read();
            received_response = true;
            last_byte = milliseconds();
            if (previous_character == 'O' && response_character == 'K') {
                received_ok = true;
            }
            previous_character = response_character;
        } else if (received_response &&
                   (uint32_t)(milliseconds() - last_byte) >= RESPONSE_IDLE_MS) {
            break;
        }
    }
    delay_ms(COMMAND_SETTLE_MS);
    return received_ok;
}

bool BLE_Init(void) {
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    SetCommandMode(true);
    GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE0_Msk)
                 | (GPIO_OUTPUT << GPIO_MODER_MODE0_Pos);
    uart1_init();
    delay_ms(COMMAND_STARTUP_MS);

    bool command_succeeded = SendCommand("AT+FACTORYRESET\r\n");
    if (command_succeeded) {
        command_succeeded = SendCommand("AT+GAPDEVNAME=ENGS28-Safe-Kensa\r\n");
    }
    if (command_succeeded) {
        command_succeeded = SendCommand("ATZ\r\n");
    }
    SetCommandMode(false);
    delay_ms(DATA_MODE_SETTLE_MS);
    return command_succeeded;
}

bool BLE_Queue(const char *message) {
    unsigned int length = 0U;
    while (message[length] != '\0') {
        ++length;
    }
    if (length >= TX_CAPACITY) {
        return false;
    }
    unsigned int queued_length = (tx_head + TX_CAPACITY - tx_tail) % TX_CAPACITY;
    if (length > TX_CAPACITY - 1U - queued_length) {
        /* A disconnected module must not leave a backlog of stale status. */
        tx_head = tx_tail;
    }
    for (unsigned int i = 0; i < length; ++i) {
        tx_buffer[tx_head] = message[i];
        tx_head = (tx_head + 1U) % TX_CAPACITY;
    }
    return true;
}

void BLE_Poll(void) {
    if (tx_tail != tx_head && uart1_tryWrite(tx_buffer[tx_tail])) {
        tx_tail = (tx_tail + 1U) % TX_CAPACITY;
    }
}
