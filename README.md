# ENGS 28 Lab 9 breadboard prototype

Target: NUCLEO-C031C6 with Adafruit Bluefruit LE UART Friend. This is a separate project copy; Lab 8 is untouched. The default build keeps PB5 solenoid drive **off** and shows an allowed unlock request on the on-board LED for 150 ms. It is compiled but has not been flashed or electrically tested.

## Build

```sh
make all          # integrated prototype, build/Lab9Prototype.bin
make passthrough  # Day 17 two-way UART exercise, build/Passthrough.bin
cc -std=c11 -Wall -Wextra -Werror -ISrc Src/SafeLogic.c tests/test_safe_logic.c -o /tmp/lab9_safe_logic_test
/tmp/lab9_safe_logic_test
```

The Makefile uses the installed STM32CubeIDE GCC 14.3 toolchain and the existing 12 MHz course clock setup. Flash only after confirming the actual Nucleo target and USB device. No ST-LINK board was present during preparation.

## Pins and behavior

| Part | Arduino pin | MCU pin | Configuration |
|---|---|---|---|
| Pot wiper | A3 | PB1 | ADC channel 18; outer legs 3V3 and GND |
| Light divider | A0 | PA0 | ADC channel 0; LDR to 3V3, 10 kΩ to GND |
| Door-position pushbutton | D4 | PB10 | Input with internal pull-up; pressed to GND = CLOSED |
| Driver control | D6 | PB5 | Output to transistor base **through resistor**, disabled in default build |
| User LED | D13 | PA5 | Output; lights for accepted unlock request |
| Bluefruit RXI / TXO | D1 / D0 | PB6 / PB7 | USART1 TX AF0 / RX AF0 |
| Bluefruit RTS / CTS | D5 / D3 | PB4 / PB3 | USART1 CTS AF4 / RTS AF4 |
| Bluefruit MOD | D10 | PB0 | Output; HIGH command, LOW data |
| USB console | built in | PA2 / PA3 | USART2 at 9600 baud |

The button is an active-low **pushbutton**, per the student's latest choice. The schematic is [docs/pushbutton.svg](docs/pushbutton.svg). On a four-leg tactile button, some legs are internally joined; use continuity mode to select a pair that is open when released and closed when pressed. Never put the button directly across 3V3 and GND.

The pot must first enter its low region (ADC 0–1228) to arm one local request. Moving it high (2867–4095) while the button is pressed triggers one request. Holding high does not repeat. The button is debounced for 25 ms. Releasing it cancels an active request. The raw light value is for observation only. `U` requests, `L` cancels, `?` sends status. The phone receives a line every second: `POT`, `DOOR`, `LIGHT`, `SOLENOID`. `SOLENOID=DISABLED` means PB5 is still forced low. The advertised name is `ENGS28-Safe-Kensa` if the startup AT sequence succeeds. The USB console also prints status once per second when no pulse is active.

The default code does not claim a physically locked door: the pushbutton simulates door position, while the solenoid only retracts a latch. To enable a real coil pulse later, first verify the exact coil supply and duty rating, KSC2073 pinout, base resistor, built-in diode polarity, external supply, and ground arrangement against the actual parts and Lab 3 figure. Only then build with `make -B BUILD=build/coil COIL_PULSE_MS=<verified_duration_in_ms> all`; use that separate output for flashing. Keep the separate coil supply disconnected during input and BLE tests. The LED is useful feedback; the Lab 9 handout does not establish that it qualifies as the required second actuator for the final graded project.

## Source map

| Source | Origin / change |
|---|---|
| `Library/ADC.*`, `sysinit.c`, `uart.c`, board headers/startup | Copied from local Lab 8 class project; one Day 17 `uart2_RxAvail()` addition |
| `Src/SafeLogic.c` | New integration of Lab 3's button debounce, with PB10 replacing conflicting PB4, plus pulse/interlock logic |
| `Src/Lab9Prototype.c` | Lab 3 PB5 output pattern, Lab 4 PB1 ADC, Lab 8 PA0 ADC read pattern, new main loop |
| `Library/uart1.c` | New USART1 driver following the class USART2 driver and Day 17 slides; RTS/CTS added |
| `Src/Passthrough.c` | Day 17 passthrough exercise adapted for nonblocking USART1 transmit |
| `Library/ble.c` | Lab 9 section 4.4 startup sequence; bounded command waits and nonblocking data queue |

Class archive: `/home/kensa/Downloads/courseinfo/ENGS28_course_archive`; Lab 9 and Day 17 PDFs are also in the handoff ZIP. The Bluefruit [manufacturer pinout](https://learn.adafruit.com/introducing-the-adafruit-bluefruit-le-uart-friend/pinouts) confirms crossed RTS/CTS and MOD level; its [GAP command documentation](https://learn.adafruit.com/introducing-the-adafruit-bluefruit-le-uart-friend/ble-gap) confirms the name syntax and reset.

## Physical checkpoints

1. **USB and external coil supply unplugged:** verify the pushbutton contact pair, then connect one contact to D4/PB10 and the other to GND. Keep all coil power disconnected. See [pushbutton schematic](docs/pushbutton.svg). Inspect actual button legs before power.
2. Add the pot and LDR divider to the 3V3 logic rail following [low-voltage schematic](docs/low_voltage_inputs.png). Check divider node and rail breaks with a meter. Reconnect only USB and observe raw console readings.
3. Wire the Bluefruit per [BLE diagram](docs/ble_wiring.png), then flash the passthrough target and verify both UART directions with terminal/phone. Flash the integrated target only after that baseline.
4. Use `U`, `L`, `?` with coil supply still disconnected and confirm LED and status. Collect real screenshots/readings.
5. Assemble the Lab 3 transistor driver only after part and supply verification using [Lab 3 driver figure](docs/lab3_driver.png). Set a verified pulse value, build, flash, and test one observed coil pulse.

`prototype_progress.md` tracks what has actually been verified. No physical checkpoint has been claimed complete.
