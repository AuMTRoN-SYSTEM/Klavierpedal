# AuMTRoN SYSTEM — Build and Deployment

## Hardware

Target:
- RP2350 board

Current board package:
- RPI Pico2w

---

## Development Environment

IDE:
- Arduino IDE 2.3.10

Required libraries:
- Arduino Wire (Wire.h)

---

## Build Procedure

1. Open:
   - `firmware/AuMTRoN_SYSTEM/AuMTRoN_SYSTEM.ino`
2. *DISABLE* serial monitor if active. (Causes problems with firmware upload)
3. Compile/Verify.
4. Upload firmware.

(3 & 4 can be accomplished by CTRL+U or clicking the 2nd arrow icon in IDE)

---

## Upload Procedure

1. Connect RP2350 board via USB.
2. Enter bootloader mode if required.
3. Upload using Arduino IDE.
4. Confirm successful deployment.

---

## Known Issues / Notes

- [2026-08-04] Initial documentation created.
- Avoid pasting very large source files into external AI tools without preserving local copies first.

## Environment Snapshot

Date:
2026-08-04

Known working configuration:

Arduino IDE:
2.3.10

Board:
rpipico2w

Board core:
rp2040

OS:
Win10 x64

Compiler:
pqt-gcc\\5.0.0-9576866