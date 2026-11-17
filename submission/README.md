# Lab 9 submission assembly

`Lab9_written_D1_D2.pdf` contains the completed USART1 pin table, schematic, pin/configuration list, and software plan. Fill in the student and collaborator names before uploading. The editable source is `Lab9_written_D1_D2.html`.

The source for Deliverable 3 is `Library/uart1.c`, `Library/uart1.h`, the nonblocking `uart2_RxAvail()` addition in `Library/uart.c`/`uart.h`, and `Src/Passthrough.c`. The source for Deliverable 5 is `Library/ble.c`/`ble.h`, called by `Src/Lab9Prototype.c`. The full source snapshot from commit `780d3e1` is in `Lab9_source.zip` and at the repository's `main` branch.

Evidence still needed from the actual setup:

1. Deliverable 3: WaveForms screenshot showing a CoolTerm character entering USART2 RX and appearing on USART1 TX (D1/PB6).
2. Deliverable 4: terminal screenshot showing `AT+FACTORYRESET`, `AT+GAPDEVNAME=ENGS28-Safe-Kensa`, and `ATZ`, each with the module's response.
3. Deliverable 5: Adafruit app screenshot showing a connection to `ENGS28-Safe-Kensa`, with the name visible.
4. Deliverable 6: in-class checkoff. The archived Canvas assignment says to leave the checklist blank in the uploaded document.
5. Deliverable 7: each student writes their own reflection about challenges, labor division, and personal contribution.

Do not substitute generated or reconstructed screenshots for measurements from the actual board and software.

## Capture order

1. With the working Lab 9 firmware running, connect in the Adafruit Bluefruit Connect app. Capture the connected device with `ENGS28-Safe-Kensa` visible. The UART page can also show periodic status; `?` requests status without toggling the solenoid.
2. For the USART1 screenshot, disconnect the 9 V adapter before changing firmware. Build `make passthrough` and flash `build/Passthrough.bin`. Connect AD2 ground to Nucleo GND and a WaveForms logic input to D1/PB6 (USART1 TX). In WaveForms, decode UART at 9600 baud, 8 data bits, no parity, 1 stop bit. Send a single character from CoolTerm through the ST-LINK USB serial port at 9600 baud. Capture WaveForms showing that character on PB6, ideally with CoolTerm visible too.
3. Leave passthrough running and move the Bluefruit switch to CMD. In CoolTerm, send `AT+FACTORYRESET`, `AT+GAPDEVNAME=ENGS28-Safe-Kensa`, and `ATZ` one at a time with the line ending needed by the module. Wait for each actual response and capture the commands and responses together. Enable local echo if CoolTerm otherwise hides what you typed.
4. Restore the integrated hold build (`make hold`, then flash `build/hold/Lab9Prototype.bin`) and return the Bluefruit switch to UART. Confirm the system still works before the class demo.
