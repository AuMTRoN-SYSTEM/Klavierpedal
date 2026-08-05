# Klavierpedal — Build and Deployment

## Hardware
 
- Target: RP2350 board
- Current board package: RPI Pico2w

## Development Environment

- IDE: Arduino IDE 2.3.10
- Required libraries: None currently or planned

## Build and Upload Procedure

- Open `firmware/Klavierpedal/Klavierpedal.ino`.
- Connect RP2350 board via USB.
- *DISABLE* serial monitor if active. (Causes problems with firmware upload)
- Compile/Verify.
- Upload firmware via Arduino IDE. (One-step process with Compile step, by default)
- Confirm successful deployment.

## Environment Snapshot

Known working configuration:

- Date: 2026-08-04
- Arduino IDE: 2.3.10
- Board: rpipico2w
- Board core: rp2040 (Earl Philhower's arduino-pico)
- OS: Win10 x64
- Compiler: pqt-gcc\\5.0.0-9576866
