# Lab 9 smart-safe breadboard prototype: I/O pinout

Board: **NUCLEO-C031C6**. The breadboard pushbutton is a toggle input, not a door or latch-position sensor. The 2026-11-12 hardware tests used an earlier 300 ms pulse build. The last flashed hold build has a sustained output that follows the requested state. The latest source refactor has not yet been flashed.

## Pins already assigned

Directions below are from the **Nucleo's** point of view. `D*` and `A*` are the labels on its Arduino-style headers.

| Nucleo header | MCU pin | Direction / peripheral | Connected part and purpose |
| --- | --- | --- | --- |
| A3 | PB1 | Analog input, ADC1 channel 18 | Potentiometer wiper. The other two pot terminals go to 3V3 and GND. A low-to-high turn toggles the lock request once. |
| D4 | PB10 | Digital input with internal pull-up | Pushbutton to GND. Each debounced press toggles the request. A 0.15 µF capacitor is across PB10 and GND. |
| D6 | PB5 | Digital output | Solenoid driver control through a 150 Ω resistor to the KSC2073 base. The hold build drives PB5 HIGH for unlock and LOW for lock; the coil is never connected directly to PB5. |
| D13 | PA5 | Digital output | On-board LD4 user LED; HIGH while unlock is requested. No external LED wire is needed. |
| D1 | PB6 | USART1 TX, AF0 | Nucleo transmit to Bluefruit **RXI**. |
| D0 | PB7 | USART1 RX, AF0 | Nucleo receive from Bluefruit **TXO**. |
| D5 | PB4 | USART1 CTS, AF4 | Flow-control input from Bluefruit **RTS**. This pin is not available for the old Lab 3 button wiring. |
| D3 | PB3 | USART1 RTS, AF4 | Flow-control output to Bluefruit **CTS**. |
| D10 | PB0 | Digital output | Bluefruit **MOD**: HIGH for AT-command mode during startup; LOW for UART data mode. |
| Built-in USB serial | PA2 / PA3 | USART2 TX / RX | ST-LINK virtual COM port for the USB terminal; no breadboard jumpers. |

## Power and driver connections

- Nucleo **3V3** supplies the pot and Bluefruit **VIN**. Nucleo GND and Bluefruit GND share the logic ground. **A0/PA0 is unused**; disconnect the photoresistor and its 10 kΩ divider resistor.
- The Lab 3 **9 V DC** wall adapter plugs into the barrel-jack board. Its pin labeled **VIN** goes to the solenoid's **red** wire. Its **GND** joins Nucleo GND and the KSC2073 emitter. The jack board's **5V** pin is unused for the solenoid.
- The solenoid's **black** wire goes to the KSC2073 collector. Nucleo D6/PB5 goes through **150 Ω** to the KSC2073 base. Keep the 9 V positive rail separate from Nucleo 3V3 and all MCU pins.

## How the firmware uses the I/O

- The main loop polls the pot ADC and button every **20 ms**. The button uses a **25 ms** software debounce in addition to its capacitor. Each press toggles; release does nothing.
- The pot must first read low (ADC **0–1228**) to arm. Moving it high (ADC **2867–4095**) toggles once. Holding it high does not repeat.
- Bluefruit uses USART1 at **9600 baud** with RTS/CTS flow control. The USB console uses USART2 at **9600 baud**. Both UARTs are polled in the main loop. Wireless or USB `U` toggles; `L` forces a lock request; `?` requests status.
- A **1 ms SysTick interrupt** supplies timing. Status (`POT`, `BUTTON`, `REQUEST`, `SOLENOID`) is sent over BLE every **1 second**. No ADC, button, or UART interrupt is used in this version.
- `make hold` builds the sustained-output version. `REQUEST=UNLOCK` turns LD4 and PB5 on until the next toggle or `L`; `SOLENOID=ON/OFF` reports the commanded output, not a latch-position measurement. `make all` builds a separate coil-disabled version for unpowered bench checks.

Source: [`Src/Lab9Prototype.c`](../Src/Lab9Prototype.c), [`Src/SafeLogic.c`](../Src/SafeLogic.c), [`Library/uart1.c`](../Library/uart1.c), and the [bench test record](../prototype_progress.md).
