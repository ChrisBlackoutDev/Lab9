/*
 * ENGS 28 Lab 9 breadboard prototype
 *
 * A3 / PB1: potentiometer wiper, ADC channel 18
 * A0 / PA0: light divider, ADC channel 0
 * D4 / PB10: active-low door pushbutton with pull-up
 * D6 / PB5: solenoid transistor drive
 * D13 / PA5: on-board user LED
 * D10 / PB0: Bluefruit command/data mode
 * D1 / PB6, D0 / PB7, D5 / PB4, D3 / PB3: Bluefruit USART1 with flow control
 * PA2 / PA3: ST-LINK USB console on USART2
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "ES28.h"
#include "ADC.h"
#include "ble.h"
#include "uart.h"
#include "uart1.h"
#include "SafeLogic.h"

#define POT_CHANNEL 18U
#define LIGHT_CHANNEL 0U
#define BUTTON_BITNUM 10U
#define BUTTON_BITMASK (1U << BUTTON_BITNUM)
#define SOLENOID_BITNUM 5U
#define SOLENOID_BITMASK (1U << SOLENOID_BITNUM)
#define LED_BITNUM 5U
#define LED_BITMASK (1U << LED_BITNUM)
#define GPIO_BSRR_RESET_BIT_OFFSET 16U
#define SAMPLE_MS 20U
#define STATUS_MS 1000U
#define STATUS_LINE_CAPACITY 96U
#define LED_ONLY_FEEDBACK_MS 1000U

/* Keep this zero until the actual coil rating, supply and driver are verified. */
#ifndef SOLENOID_PULSE_MS
#define SOLENOID_PULSE_MS 0U
#endif

#if SOLENOID_PULSE_MS > 0U
#define PULSE_MS SOLENOID_PULSE_MS
#else
#define PULSE_MS LED_ONLY_FEEDBACK_MS
#endif

static void InitializeGPIO(void) {
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN;
    /* Keep the coil off while PB5 changes from its reset mode to output. */
    GPIOB->BSRR = SOLENOID_BITMASK << GPIO_BSRR_RESET_BIT_OFFSET;
    GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE5_Msk)
                 | (GPIO_OUTPUT << GPIO_MODER_MODE5_Pos);
    GPIOA->BSRR = LED_BITMASK << GPIO_BSRR_RESET_BIT_OFFSET;
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE5_Msk)
                 | (GPIO_OUTPUT << GPIO_MODER_MODE5_Pos);
    /* The door button pulls PB10 low when pressed. */
    GPIOB->MODER &= ~GPIO_MODER_MODE10_Msk;
    GPIOB->PUPDR = (GPIOB->PUPDR & ~GPIO_PUPDR_PUPD10_Msk)
                 | (GPIO_PULLUP << GPIO_PUPDR_PUPD10_Pos);
}

static void ApplyOutputs(const SafeState *state) {
#if SOLENOID_PULSE_MS > 0U
    GPIOB->BSRR = state->pulse_active ? SOLENOID_BITMASK
                                     : (SOLENOID_BITMASK << GPIO_BSRR_RESET_BIT_OFFSET);
#else
    GPIOB->BSRR = SOLENOID_BITMASK << GPIO_BSRR_RESET_BIT_OFFSET;
#endif
    GPIOA->BSRR = state->pulse_active ? LED_BITMASK
                                     : (LED_BITMASK << GPIO_BSRR_RESET_BIT_OFFSET);
}

static int ReadADC(unsigned int channel) {
    adc_setChannel(channel);
    /* Lab 8 sensor check discarded the first reading after channel change. */
    if (adc_getValue() < 0) {
        return -1;
    }
    return adc_getValue();
}

static void SendStatus(const SafeState *state) {
    char line[STATUS_LINE_CAPACITY] = {0};
    (void)snprintf(line, sizeof(line), "POT=%u DOOR=%s LIGHT=%u SOLENOID=%s\r\n",
                   (unsigned int)state->pot,
                   state->door_closed ? "CLOSED" : "NOT_CLOSED",
                   (unsigned int)state->light,
#if SOLENOID_PULSE_MS > 0U
                   state->pulse_active ? "ON" : "OFF"
#else
                   "DISABLED"
#endif
    );
    (void)BLE_Queue(line);
    /* The USB console is useful when there is no phone. Avoid blocking during a pulse. */
    if (!state->pulse_active) {
        printf("%s", line);
    }
}

static void ProcessCommand(SafeState *state, int ch, uint32_t now) {
    if (ch == 'U' || ch == 'u') {
        if ((GPIOB->IDR & BUTTON_BITMASK) != 0U) {
            CancelSafe(state);
            (void)BLE_Queue("UNLOCK REJECTED: BUTTON RELEASED\r\n");
            return;
        }
        if (ProcessSafeCommand(state, (char)ch, now)) {
#if SOLENOID_PULSE_MS > 0U
            (void)BLE_Queue("UNLOCK PULSE\r\n");
#else
            (void)BLE_Queue("UNLOCK SIMULATED; COIL DISABLED\r\n");
#endif
        } else {
            (void)BLE_Queue("UNLOCK REJECTED: BUTTON RELEASED OR BUSY\r\n");
        }
    } else if (ch == 'L' || ch == 'l') {
        (void)ProcessSafeCommand(state, (char)ch, now);
        (void)BLE_Queue("PULSE CANCELLED\r\n");
    } else if (ch == '?') {
        SendStatus(state);
    }
}

int main(void) {
    SafeState state = {0};
    InitializeGPIO();
    InitializeSafe(&state, milliseconds());
    ApplyOutputs(&state);
    uart2_init();
    adc_init();
    bool ble_ready = BLE_Init();
    printf("Lab 9 prototype; BLE init %s; coil %s\r\n",
           ble_ready ? "OK" : "failed/absent",
#if SOLENOID_PULSE_MS > 0U
           "enabled"
#else
           "disabled"
#endif
    );
    uint32_t last_sample = milliseconds();
    uint32_t last_status = last_sample;

    while (1) {
        uint32_t now = milliseconds();
        ExpireSafe(&state, now, PULSE_MS);
        ApplyOutputs(&state);

        if ((uint32_t)(now - last_sample) >= SAMPLE_MS) {
            last_sample = now;
            int pot = ReadADC(POT_CHANNEL);
            int light = ReadADC(LIGHT_CHANNEL);
            if (pot < 0 || light < 0) {
                state.sensor_valid = false;
                CancelSafe(&state);
            } else {
                bool pressed = (GPIOB->IDR & BUTTON_BITMASK) == 0U;
                SampleSafe(&state, now, pressed, (uint16_t)pot,
                            (uint16_t)light, PULSE_MS);
            }
            ApplyOutputs(&state);
        }

        if (uart1_RxAvail()) {
            ProcessCommand(&state, uart1_read(), now);
            ApplyOutputs(&state);
        }
        if (uart2_RxAvail()) {
            ProcessCommand(&state, uart2_read(), now);
            ApplyOutputs(&state);
        }
        if ((uint32_t)(now - last_status) >= STATUS_MS) {
            last_status = now;
            SendStatus(&state);
        }
        BLE_Poll();
    }
}
