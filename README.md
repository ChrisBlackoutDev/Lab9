# ENGS 28 Lab 9 breadboard prototype

Target: NUCLEO-C031C6 with Adafruit Bluefruit LE UART Friend. This is a separate project copy; Lab 8 is untouched. The default build keeps PB5 solenoid drive **off** and shows an allowed unlock request on the on-board LED for 1 second. The breadboard prototype was tested on the connected Nucleo on 2026-11-12 with a separate 300 ms coil build.

## Build

```sh
make all          # integrated prototype, build/Lab9Prototype.bin
make coil         # tested 300 ms solenoid pulse, build/coil/Lab9Prototype.bin
make passthrough  # Day 17 two-way UART exercise, build/Passthrough.bin
cc -std=c11 -Wall -Wextra -Werror -ISrc Src/SafeLogic.c tests/test_safe_logic.c -o /tmp/lab9_safe_logic_test
/tmp/lab9_safe_logic_test
```

The Makefile uses the installed STM32CubeIDE GCC 14.3 toolchain and the existing 12 MHz course clock setup. The ST-LINK previously reported a NUCLEO-C031C6, and the board was flashed with a verified 300 ms coil build before the final source refactor. The refactored binaries have been built but have not yet been flashed. `make all` remains the coil-disabled test build.

## Pins and behavior

The [illustrated breadboard layout](docs/breadboard_layout.png) shows the components and colored jumpers in the style of the lab figures. The [complete circuit schematic](docs/full_breadboard_wiring.png) shows the same electrical connections. The illustrated placement is a guide rather than a literal hole-by-hole template; verify your breadboard's actual row and power-rail breaks before connecting power.

Share the [group I/O pinout](docs/group_pinout.md) for the final-project check-in; it lists every allocated pin, signal direction, and sampling or command behavior.

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

The button is an active-low **pushbutton**, per the student's latest choice. Lab 3 specifies a debouncing capacitor; use the course's nonpolarized 0.15 µF capacitor **between PB10/D4 and GND, in parallel with the button**, as shown on the [complete diagram](docs/full_breadboard_wiring.png). The firmware also debounces the input in software. On a four-leg tactile button, some legs are internally joined; use continuity mode to select a pair that is open when released and closed when pressed. Never put the button directly across 3V3 and GND.

The pot must first enter its low region (ADC 0–1228) to arm one local request. Moving it high (2867–4095) while the button is pressed triggers one request. Holding high does not repeat. The button is debounced for 25 ms. Releasing it cancels an active request. The raw light value is for observation only. `U` requests, `L` cancels, `?` sends status. The phone receives a line every second: `POT`, `DOOR`, `LIGHT`, `SOLENOID`. `SOLENOID=DISABLED` means PB5 is still forced low. The advertised name is `ENGS28-Safe-Kensa` if the startup AT sequence succeeds. The USB console also prints status once per second when no pulse is active.

The pushbutton simulates door position, while the solenoid retracts a latch. The student confirmed the Lab 3 9 V DC adapter and driver wiring: barrel board `VIN` to solenoid red, solenoid black to KSC2073 collector, emitter to shared GND, and D6/PB5 through 150 Ω to the base. The barrel board's `5V` pin is unused for the solenoid. A 300 ms pulse moved the latch in the powered bench tests; `make coil` reproduces that build. The coil's current and long-term duty rating have not been measured. Unplug the 9 V adapter when not actively demonstrating the prototype. The LED is useful feedback; the Lab 9 handout does not establish that it qualifies as the required second actuator for the final graded project.

## Source map

| Source | Origin / change |
|---|---|
| `sysinit.c`, `uart.c`, board headers/startup | Copied from the local Lab 8 class project; `uart.c` is the supplied course UART driver plus the Day 17 `uart2_RxAvail()` addition |
| `Library/ADC.*` | Reused from the tested Lab 8 project; extends the course `ADCPot.c` example to handle both ADC channels, calibration, and bounded waits |
| `Src/SafeLogic.c` | New integration of Lab 3's button debounce, with PB10 replacing conflicting PB4, plus pulse/interlock logic |
| `Src/Lab9Prototype.c` | Lab 3 PB5 output pattern, Lab 4 PB1 ADC, Lab 8 PA0 ADC read pattern, new main loop |
| `Library/uart1.c` | New USART1 driver following the class USART2 driver and Day 17 slides; RTS/CTS added |
| `Src/Passthrough.c` | Day 17 passthrough exercise adapted for nonblocking USART1 transmit |
| `Library/ble.c` | Lab 9 section 4.4 startup sequence; bounded command waits and nonblocking data queue |

Class archive: `/home/kensa/Downloads/courseinfo/ENGS28_course_archive`. The source review checked its supplied `uart.c`, `button.c`, `ADCPot.c`, Day 17 BLE slides, and Lab 9 handout. The archive has no supplied USART1 driver, BLE startup function, or two-channel ADC library; the Day 17 slides and Lab 9 handout direct students to write the USART1 and BLE parts. Lab 9 and Day 17 PDFs are also in the handoff ZIP. The Bluefruit [manufacturer pinout](https://learn.adafruit.com/introducing-the-adafruit-bluefruit-le-uart-friend/pinouts) confirms crossed RTS/CTS and MOD level; its [GAP command documentation](https://learn.adafruit.com/introducing-the-adafruit-bluefruit-le-uart-friend/ble-gap) confirms the name syntax and reset.

## Bench verification

- The ST-LINK identified the NUCLEO-C031C6 and flashed both the coil-disabled build and the separate 300 ms coil build with successful readback verification.
- USB status showed the door button change `NOT_CLOSED → CLOSED → NOT_CLOSED`, pot readings span `0–4095`, and the LDR reading fell from about `2850` to `1280` when covered.
- A Linux BLE central found and connected to `ENGS28-Safe-Kensa`, received status, and got both a rejected unlock with the button released and an accepted unlock with it pressed.
- LD4 lit during a one-second coil-disabled request. With the coil supply unplugged, debugger reads confirmed PB5 high during the 300 ms pulse and low afterward.
- With the 9 V adapter connected, the student observed the solenoid move for a USB `U` command, the local pot low-to-high request, and a BLE `U` command. The powered BLE test also returned `UNLOCK PULSE`; debugger reads showed PB5 high only during the pulse.

`prototype_progress.md` holds the detailed test record and remaining measurements.
