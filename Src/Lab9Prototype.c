/* ENGS 28 Lab 9 breadboard prototype. Class ADC, SysTick, and UART2 helpers. */
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
#define BUTTON_PIN (1U << 10)
#define SOLENOID_PIN (1U << 5)
#define LED_PIN (1U << 5)
#define SAMPLE_MS 20U
#define STATUS_MS 1000U
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

static void gpio_init(void) {
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN;
    /* Lab 3: force PB5 low before making it an output. */
    GPIOB->BSRR = SOLENOID_PIN << 16;
    GPIOB->MODER = (GPIOB->MODER & ~GPIO_MODER_MODE5_Msk)
                 | (GPIO_OUTPUT << GPIO_MODER_MODE5_Pos);
    GPIOA->BSRR = LED_PIN << 16;
    GPIOA->MODER = (GPIOA->MODER & ~GPIO_MODER_MODE5_Msk)
                 | (GPIO_OUTPUT << GPIO_MODER_MODE5_Pos);
    /* Pushbutton: one side to PB10/D4, other side to ground. */
    GPIOB->MODER &= ~GPIO_MODER_MODE10_Msk;
    GPIOB->PUPDR = (GPIOB->PUPDR & ~GPIO_PUPDR_PUPD10_Msk)
                 | (1U << GPIO_PUPDR_PUPD10_Pos);
}

static void apply_outputs(const SafeState *state) {
#if SOLENOID_PULSE_MS > 0U
    GPIOB->BSRR = state->pulse_active ? SOLENOID_PIN : (SOLENOID_PIN << 16);
#else
    GPIOB->BSRR = SOLENOID_PIN << 16;
#endif
    GPIOA->BSRR = state->pulse_active ? LED_PIN : (LED_PIN << 16);
}

static int read_adc(unsigned int channel) {
    adc_setChannel(channel);
    /* Lab 8 sensor check discarded the first reading after channel change. */
    if (adc_getValue() < 0) {
        return -1;
    }
    return adc_getValue();
}

static void send_status(const SafeState *state) {
    char line[96];
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
    (void)ble_queue(line);
    /* The USB console is useful when there is no phone. Avoid blocking during a pulse. */
    if (!state->pulse_active) {
        printf("%s", line);
    }
}

static void process_command(SafeState *state, int ch, uint32_t now) {
    if (ch == 'U' || ch == 'u') {
        if ((GPIOB->IDR & BUTTON_PIN) != 0U) {
            safe_cancel(state);
            (void)ble_queue("UNLOCK REJECTED: BUTTON RELEASED\r\n");
            return;
        }
        if (safe_command(state, (char)ch, now)) {
#if SOLENOID_PULSE_MS > 0U
            (void)ble_queue("UNLOCK PULSE\r\n");
#else
            (void)ble_queue("UNLOCK SIMULATED; COIL DISABLED\r\n");
#endif
        } else {
            (void)ble_queue("UNLOCK REJECTED: BUTTON RELEASED OR BUSY\r\n");
        }
    } else if (ch == 'L' || ch == 'l') {
        (void)safe_command(state, (char)ch, now);
        (void)ble_queue("PULSE CANCELLED\r\n");
    } else if (ch == '?') {
        send_status(state);
    }
}

int main(void) {
    SafeState state;
    gpio_init();
    safe_init(&state, milliseconds());
    apply_outputs(&state);
    uart2_init();
    adc_init();
    bool ble_ready = ble_init();
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
        safe_expire(&state, now, PULSE_MS);
        apply_outputs(&state);

        if ((uint32_t)(now - last_sample) >= SAMPLE_MS) {
            last_sample = now;
            int pot = read_adc(POT_CHANNEL);
            int light = read_adc(LIGHT_CHANNEL);
            if (pot < 0 || light < 0) {
                state.sensor_valid = false;
                safe_cancel(&state);
            } else {
                bool pressed = (GPIOB->IDR & BUTTON_PIN) == 0U;
                safe_sample(&state, now, pressed, (uint16_t)pot,
                            (uint16_t)light, PULSE_MS);
            }
            apply_outputs(&state);
        }

        if (uart1_RxAvail()) {
            process_command(&state, uart1_read(), now);
            apply_outputs(&state);
        }
        if (uart2_RxAvail()) {
            process_command(&state, uart2_read(), now);
            apply_outputs(&state);
        }
        if ((uint32_t)(now - last_status) >= STATUS_MS) {
            last_status = now;
            send_status(&state);
        }
        ble_poll();
    }
}
