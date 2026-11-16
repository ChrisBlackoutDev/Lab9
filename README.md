# ENGS 28 Lab 9 breadboard prototype

Target: NUCLEO-C031C6 with an Adafruit Bluefruit LE UART Friend. The button, a low-to-high pot turn, or a BLE/USB `U` command toggles a shared lock request. LD4 stays lit while unlock is requested. The **last flashed hold build** drives PB5 HIGH throughout the unlock state and LOW throughout the lock state; the latest source refactor has not yet been flashed. This is a breadboard prototype, with no door or latch-position sensor.

## Build and test

Install GNU Make, an Arm GNU bare-metal toolchain (`arm-none-eabi-gcc`, `arm-none-eabi-size`, and `arm-none-eabi-objcopy`), and a host C compiler for `make test`. The firmware uses the course's 12 MHz STM32C031 clock setup and does not require STM32 HAL.

The Makefile finds the STM32CubeIDE toolchain at the path used for the original bench build. On another computer, put `arm-none-eabi-*` on `PATH` and run `make TOOLCHAIN=`, or set `TOOLCHAIN=/path/to/tools/bin`. The `TOOLCHAIN` setting can also be provided as an environment variable.

```sh
make all          # integrated prototype, build/Lab9Prototype.bin
make hold         # sustained solenoid toggle, build/hold/Lab9Prototype.bin
make passthrough  # Day 17 two-way UART exercise, build/Passthrough.bin
make test         # host-side toggle and debounce logic test
```

To flash a binary, connect the Nucleo's ST-LINK USB port and use STM32CubeProgrammer. For example, with its CLI on `PATH`:

```sh
STM32_Programmer_CLI -c port=SWD -w build/hold/Lab9Prototype.bin 0x08000000 -v -rst
```

The default `make all` image keeps PB5 low for unpowered bench checks. The `make hold` image uses the sustained PB5 output pattern from the supplied friend project: HIGH while unlock is requested, LOW while lock is requested. The student confirmed the same solenoid, 9 V adapter, and KSC2073 driver were used for the friend's TA-approved sustained-on test. The coil's model and electrical ratings have not been independently measured. The [bench log](prototype_progress.md) records the earlier 300 ms pulse test separately.

## Pins and behavior

The [illustrated breadboard layout](docs/breadboard_layout.png) shows the components and colored jumpers in the style of the lab figures. The [complete circuit schematic](docs/full_breadboard_wiring.png) shows the same electrical connections. The illustrated placement is a guide rather than a literal hole-by-hole template; verify your breadboard's actual row and power-rail breaks before connecting power.

Share the [group I/O pinout](docs/group_pinout.md) for the final-project check-in; it lists every allocated pin, signal direction, and sampling or command behavior.

| Part | Arduino pin | MCU pin | Configuration |
|---|---|---|---|
| Pot wiper | A3 | PB1 | ADC channel 18; outer legs 3V3 and GND |
| Toggle pushbutton | D4 | PB10 | Input with internal pull-up; each debounced press toggles the request |
| Driver control | D6 | PB5 | Output to transistor base **through resistor**; HIGH for unlock in the hold build, LOW for lock |
| User LED | D13 | PA5 | Output; stays lit while unlock is requested |
| Bluefruit RXI / TXO | D1 / D0 | PB6 / PB7 | USART1 TX AF0 / RX AF0 |
| Bluefruit RTS / CTS | D5 / D3 | PB4 / PB3 | USART1 CTS AF4 / RTS AF4 |
| Bluefruit MOD | D10 | PB0 | Output; HIGH command, LOW data |
| USB console | built in | PA2 / PA3 | USART2 at 9600 baud |

The button is an active-low **pushbutton**. Lab 3 specifies a debouncing capacitor; use the course's nonpolarized 0.15 µF capacitor **between PB10/D4 and GND, in parallel with the button**, as shown on the [complete diagram](docs/full_breadboard_wiring.png). The firmware also debounces the input in software. On a four-leg tactile button, some legs are internally joined; use continuity mode to select a pair that is open when released and closed when pressed. Never put the button directly across 3V3 and GND.

Each debounced button press toggles the request; releasing the button does nothing. The pot must first enter its low region (ADC 0–1228) and then move high (2867–4095) to toggle once. Holding it high does not repeat. BLE/USB `U` also toggles; `L` forces a lock request; `?` sends status. The phone receives `POT`, `BUTTON`, `REQUEST`, and `SOLENOID` each second. In the hold build, `SOLENOID=ON/OFF` reports the commanded output state, not a measured latch position. A0/PA0 is unused; disconnect the photoresistor and its 10 kΩ divider resistor. The advertised name is `ENGS28-Safe-Kensa` if the startup AT sequence succeeds.

The tested Lab 3 driver wiring used a 9 V DC adapter: barrel board `VIN` to solenoid red, solenoid black to KSC2073 collector, emitter to shared GND, and D6/PB5 through 150 Ω to the base. The barrel board's `5V` pin is unused for the solenoid. A 300 ms pulse moved the latch in the earlier powered bench tests. The hold build has been flashed and checked over USB with the 9 V adapter unplugged; powered sustained operation on this board has not yet been observed. The LED is useful feedback; the Lab 9 handout does not establish that it qualifies as the required second actuator for the final graded project.

## Source map

| Source | Origin / change |
|---|---|
| `sysinit.c`, `uart.c`, board headers/startup | Copied from the local Lab 8 class project; `uart.c` is the supplied course UART driver plus the Day 17 slide's `uart2_RxAvail()` addition |
| `Library/ADC.*` | Course `ADCPot.c` single-channel setup adapted from PA0 to PB1/channel 18; tested Lab 8 calibration and bounded waits retained |
| `Src/SafeLogic.c` | Day 4 press-edge toggle pattern and Lab 3 button wiring, with project-specific debounce, pot gesture, and shared request state |
| `Src/Lab9Prototype.c` | Lab 3 PB5 solenoid output and Day 7 ADC reading pattern, combined with the Lab 9 main loop; the hold build sustains PB5 HIGH until a lock request, matching the supplied friend's `CodingChallenge.c` output behavior |
| `Library/uart1.c` | New USART1 driver following the class USART2 driver and Day 17 slides; RTS/CTS added |
| `Src/Passthrough.c` | Day 17 passthrough exercise adapted for nonblocking USART1 transmit |
| `Library/ble.c` | Lab 9 section 4.4 startup sequence; bounded command waits and nonblocking data queue |

The course source review checked the supplied `uart.c`, `button.c`, `ADCPot.c`, Day 4 button slides, Day 17 BLE slides, Lab 3 driver, and Lab 9 handout. The handouts and slides are not bundled here. They contain no supplied USART1 driver, BLE startup function, or pot/button/BLE lock state machine; the Day 17 slides and Lab 9 handout direct students to write the USART1 and BLE parts. The Bluefruit [manufacturer pinout](https://learn.adafruit.com/introducing-the-adafruit-bluefruit-le-uart-friend/pinouts) confirms crossed RTS/CTS and MOD level; its [GAP command documentation](https://learn.adafruit.com/introducing-the-adafruit-bluefruit-le-uart-friend/ble-gap) confirms the name syntax and reset.

The `course/chipheaders` support files and ST-generated startup/linker files retain their upstream copyright and license notices. No project-wide license is specified for the class-derived code.

## Bench verification

The following checks used the earlier 300 ms pulse firmware on 2026-11-12:

- The ST-LINK identified the NUCLEO-C031C6 and flashed both the coil-disabled build and the separate 300 ms coil build with successful readback verification.
- USB status showed the door button change `NOT_CLOSED → CLOSED → NOT_CLOSED`, pot readings span `0–4095`, and the LDR reading fell from about `2850` to `1280` when covered.
- A Linux BLE central found and connected to `ENGS28-Safe-Kensa`, received status, and got both a rejected unlock with the button released and an accepted unlock with it pressed.
- LD4 lit during a one-second coil-disabled request. With the coil supply unplugged, debugger reads confirmed PB5 high during the 300 ms pulse and low afterward.
- With the 9 V adapter connected, the student observed the solenoid move for a USB `U` command, the local pot low-to-high request, and a BLE `U` command. The powered BLE test also returned `UNLOCK PULSE`; debugger reads showed PB5 high only during the pulse.

On 2026-11-15, the coil-disabled toggle firmware passed `make test` and `make all`, flashed to the NUCLEO-C031C6 with readback verification, and reported `SOLENOID=DISABLED` over USB. Two USB `U` commands changed `REQUEST=LOCK → UNLOCK → LOCK` while the button was released. The student confirmed LD4 alternated with button and pot input. The 9 V adapter remained unplugged.

Later on 2026-11-15, the hold build passed `make test` and `make hold`, was flashed with readback verification, and reported `LOCK/OFF → UNLOCK/ON → LOCK/OFF` for USB `L`, `U`, `L`. Debugger reads confirmed PB5 HIGH then LOW. The 9 V adapter remained unplugged, so physical latch motion with this build is pending.

After the photoresistor was removed from the design, `make test`, `make hold`, and `make all` passed. The updated hold image was flashed with readback verification. USB status reported `POT`, `BUTTON`, `REQUEST`, and `SOLENOID`, with no `LIGHT` field, while the board remained locked/off.

The later course-code and style refactor passed the host test and both firmware builds. The Nucleo was disconnected, so this refactored image has not yet been flashed or checked on hardware. See the [bench log](prototype_progress.md) for the exact changes.

`prototype_progress.md` holds the detailed test record and remaining measurements.
