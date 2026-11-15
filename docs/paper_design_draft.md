# Lab 9 paper design draft

This design began with the course Lab 9 handout and was then used for the 2026-11-12 breadboard test. The 9 V adapter, 150 Ω base resistor, and 300 ms pulse moved the latch during that test. The 2026-11-15 update uses independent button, pot, and BLE toggle requests with the coil output disabled. Coil current and long-term duty rating remain unmeasured. The final graded project still needs an instructor-approved second actuator.

## Schematics

- [Illustrated breadboard layout](breadboard_layout.png) and [complete breadboard circuit](full_breadboard_wiring.png), including the Lab 2/Lab 3 nonpolarized 0.15 µF capacitor across the toggle pushbutton: PB10/D4 to GND when pressed.
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
      Every 20 ms: read pot and LDR; debounce the pushbutton;
                   toggle on a new button press or a low-to-high pot turn.
      If a BLE or USB character is waiting: U toggles, L requests lock,
                 and ? returns status.
      Every 1 s: queue raw sensor, request, and output status for BLE;
                 print status to USB console.
      Send one queued BLE character when USART1 can accept it.
```

The default firmware uses a persistent LED indication and holds PB5 low. The pushbutton is a toggle input; no door or latch position is sensed. An indefinitely energized coil build needs the actual coil's continuous-duty rating before it can be flashed or powered.
