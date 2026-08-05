# Klavierpedal — Software Architecture

## Design Philosophy

The overriding function of the software has a single purpose (convert RPU-3 to MIDI).

The firmware is developed using the Arduino-Pico core by Earle Philhower, providing access to the RP2350 hardware through the Arduino framework.

- Built from the ground up with minimal external dependencies.
- Low latency.
- Deterministic behaviour.
- No dynamic memory allocation.
- Modular components.
- Easy to extend.

However, the firmware has a secondary function. It can host other "Modules"; self contained programs that can utilise the hardware platform. For instance: I can hold the encoder down, or the middle pedal, to boot another module. This way the main Klavierpedal part of the firmware stays indepdent, and everything is built at once.

## System Components

- Pedal Manager - Reads ADC values
- Calibration - Converts raw ADC to calibrated position
- MIDI Engine - Produces MIDI events
- Display Manager - LCD updates
- Input Manager - Encoder and switch
- Settings Manager - Presets
- Application - Coordinates everything

## Main Execution Loop

    Initialise
         ↓
    Read pedals
         ↓
    Read encoder
         ↓
     Update UI
         ↓
    Generate MIDI
         ↓
      Repeat
  
## Data Flow

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

## Configuration

- Mappings.
- Sensitivity.
- Presets.

## Future Expansion

- Presets
- Alternate controller modes
