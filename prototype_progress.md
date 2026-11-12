# Prototype progress — 2026-11-12

- Board/project: intended NUCLEO-C031C6; `/home/kensa/Documents/ES28F26/workspace/Lab9_Prototype`.
- Build: `make all passthrough`; both targets compiled. Host SafeLogic test passed.
- Last flashed/hardware-verified firmware: none. No Nucleo/ST-LINK USB device or `/dev/serial/by-id` device detected during preparation.
- Power/wiring state: not observed. Coil supply must remain disconnected for first tests.
- Parts/ratings unresolved: actual button legs, transistor/resistor and coil identity/duty, adapter voltage and polarity, ground wiring. No pulse length approved.
- Stage A: source/build baseline prepared; hardware not inspected. Stages B–F: firmware prepared, physical checks pending.
- Lab 9 deliverables: AF table and draft source prepared; screenshots, actual UART/BLE results, demo, and reflection pending.
- Input measurements, coil motion, photos and screenshots: none.
- Last tested behavior: compile and host logic test only. Default PB5 output remains off by construction.
- Next action: with all power unplugged, identify a normally open pushbutton contact pair and connect it between PB10/D4 and GND. Send a clear photo or continuity confirmation before powering the circuit.
