# Klavierpedal — Software Architecture

1. Design Philosophy

Single-purpose firmware.

The firmware is developed using the Arduino-Pico core by Earle Philhower, providing access to the RP2350 hardware through the Arduino framework.

Built from the ground up with minimal external dependencies.

Low latency.

Deterministic behaviour.

No dynamic memory allocation.

Modular components.

Easy to extend.

2. System Components

Pedal Manager
    Reads ADC values

Calibration
    Converts raw ADC to calibrated position

MIDI Engine
    Produces MIDI events

Display Manager
    LCD updates

Input Manager
    Encoder and switch

Settings Manager
    Presets

Application
    Coordinates everything

3. Main Execution Loop

Initialise
    │
    ▼
Read pedals
    │
    ▼
Read encoder
    │
    ▼
Update UI
    │
    ▼
Generate MIDI
    │
    ▼
Repeat

4. Data Flow

ADC

↓

Raw Values

↓

Calibration

↓

Normalised Values

↓

Processing

↓

MIDI Messages

↓

USB

5. Configuration

Calibration.

Mappings.

Sensitivity.

Presets.

6. Future Expansion

envelopes

LFOs

alternate controller modes

scripting (if you ever go there)

