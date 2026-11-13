# Lab 9 paper design draft

This design began with the course Lab 9 handout and was then used for the 2026-11-12 breadboard test. The 9 V adapter, 150 Ω base resistor, and 300 ms pulse moved the latch during that test. Coil current and long-term duty rating remain unmeasured. The final graded project still needs an instructor-approved second actuator.

## Schematics

- [Illustrated breadboard layout](breadboard_layout.png) and [complete breadboard circuit](full_breadboard_wiring.png), including the Lab 2/Lab 3 nonpolarized 0.15 µF capacitor across the door-position pushbutton: PB10/D4 to GND when pressed.
- [Bluefruit UART, flow control, MOD, and power](ble_wiring.png).
- [Original Lab 3 KSC2073 solenoid driver](lab3_driver.png). The current breadboard connections are in the [complete circuit](full_breadboard_wiring.png): 9 V DC adapter and 150 Ω base resistor, with the external positive rail separate from 3V3. Confirm the actual transistor marking and lead orientation before powering the driver.

## USART1 alternate functions

| USART1 function | MCU | AF | Bluefruit |
|---|---:|---:|---|
| TX | PB6/D1 | 0 | RXI |
| RX | PB7/D0 | 0 | TXO |
| CTS | PB4/D5 | 4 | RTS |
| RTS | PB3/D3 | 4 | CTS |

The complete pin list and configuration are in the project [README](../README.md#pins-and-behavior).

## Peripherals and software plan

Peripherals: ADC1 channels 18 and 0, USART1 at 9600 baud with hardware RTS/CTS, USART2 at 9600 baud for the USB console, GPIO for PB10/PB5/PA5/PB0, and the course SysTick millisecond helper. The current version polls UART and ADC; SysTick is the only interrupt. The Lab 9 handout gives one additional point for an interrupt-driven UART driver, which can be added after polling works on hardware.

```text
Boot: PB5 off, LED off; configure GPIO and ADC; initialize USART2 and USART1.
      Put Bluefruit in CMD mode, send factory reset, device name, ATZ;
      wait for bounded responses, then return to UART mode.
Repeat:
      Check pulse timeout and drive PB5 off when elapsed.
      Every 20 ms: read pot and LDR; debounce the pushbutton;
                   cancel on release; arm pot when low; request once when high.
      If a BLE or USB character is waiting: process U, L, or ?.
      Every 1 s: queue current raw sensor and output status for BLE;
                 print status to USB console outside a pulse.
      Send one queued BLE character when USART1 can accept it.
```

The default firmware uses an LED-only request indication and holds PB5 low. `make coil` enables the 300 ms pulse used in the earlier breadboard test. The pushbutton only simulates a closed door; no latch position is sensed.
