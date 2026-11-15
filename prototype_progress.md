# Prototype progress

## 2026-11-15 toggle update

- Replaced the door-button interlock and 300 ms request pulse with independent toggle requests: each debounced button press, each low-to-high pot turn, and each BLE/USB `U` changes the requested state. `L` forces lock; `?` reports status. The light reading remains telemetry only.
- Built and passed the host toggle/debounce test (`make test`) and the MCU build (`make all`). Flashed the **coil-disabled** image to the connected NUCLEO-C031C6; STM32CubeProgrammer verified the write and reset the board.
- USB status reported `REQUEST=LOCK/UNLOCK` and `SOLENOID=DISABLED`. Two USB `U` commands changed `LOCK → UNLOCK → LOCK` while the button was released. The student confirmed LD4 alternated when pressing the button and turning the pot.
- External 9 V power remained unplugged. The installed coil's exact model and continuous-duty rating are unknown, and a visually similar online product does not identify it. Do not enable a sustained coil output or connect 9 V for this firmware until the actual part and rating are verified.

## 2026-11-12 pulse prototype

- Board: ST-LINK identified NUCLEO-C031C6 / STM32C03x, 32 KB flash, 3.24 V target voltage. The ST-LINK USB virtual COM port was available during the test.
- Source and build: `make -B all passthrough`, `make coil`, and the host `SafeLogic` test passed. The default `make all` binary leaves PB5 low. `make coil` builds a separate 300 ms pulse binary.
- Firmware: ST-LINK flash and readback verification passed for both default and 300 ms builds. The 300 ms binary used in this test had SHA-256 `b188a709eff42d819db899d553d86c98cbda2a9d686d82c9eff0a3e16db74125`. The later source refactor has been built but has not yet been flashed or retested on the board.
- Previous board flash: backed up before programming; the backup is outside this Git repository.
- Pushbutton: live USB status changed `NOT_CLOSED → CLOSED → NOT_CLOSED` during a hand press and release. The 0.15 µF capacitor's individual value was not measured.
- Potentiometer: live ADC readings covered the full `0–4095` range. The student observed one solenoid actuation when moving low to high while holding the button.
- LDR: covering it changed the ADC reading from about `2850` to `1280`; uncovering restored the higher reading.
- LED: the student saw LD4 light with the one-second LED-only feedback. During that request, the debugger read GPIOA PA5 high and GPIOB PB5 low.
- BLE: the module replied `OK` to startup commands. Linux scanning found `ENGS28-Safe-Kensa` and connected to its Nordic UART service. GATT notifications carried status; a wireless `U` with the button released returned `UNLOCK REJECTED: BUTTON RELEASED`, and with the button held returned `UNLOCK SIMULATED; COIL DISABLED` in the default build.
- Driver dry-run: with the 9 V adapter unplugged, the 300 ms build made PA5 and PB5 high during a USB `U` request and low afterward, as read by ST-LINK.
- Powered solenoid: the student confirmed the adapter label says 9 V DC and confirmed the Lab 3 VIN/red, black/collector, emitter/shared-GND, and 150 Ω base-resistor wiring. A USB `U` request, local pot low-to-high request, and wireless `U` request each produced observed latch motion. During powered USB and BLE tests, ST-LINK read PA5/PB5 high during the command pulse and both low afterward. BLE returned `UNLOCK PULSE` in the powered test.
- Limits: the supply voltage under load, solenoid current, resistor voltage drop, exact coil part/duty rating, and wiring were not independently measured or visually inspected. The 300 ms pulse is bench verified for motion, not a long-term duty certification. No box or physical door is present; the pushbutton simulates door position.
- Lab 9 submission work still pending: real screenshots/photos, final graded-project actuator decision, demo, and reflection. The breadboard prototype functions in the tested configuration.
