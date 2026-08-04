# Treadle — System Overview

          Roland RPU-3
         (3 Analogue Pedals)
                 │
                 ▼
        Raspberry Pi Pico 2 W
      ┌────────────────────────┐
      │ Pedal Processing       │
      │ Calibration            │
      │ Mapping                │
      │ Performance Logic      │
      └────────────────────────┘
          ▲              │
          │              ▼
  LCD + Encoder      USB MIDI Device
      (UI)           (Host Computer)