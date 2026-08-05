# Klavierpedal — Hardware Blueprint

## Controller

Klavierpedal is built around a Raspberry Pi Pico 2 W microcontroller.

The Pico 2 W is responsible for:

- Sampling the three analogue outputs from the Roland RPU-3 pedals.
- Processing pedal movement and calibration.
- Managing the user interface (LCD, rotary encoder and push switch).
- Generating MIDI messages over USB (planned).
- Hosting future performance features such as presets.

The design is assembled using DuPont point-to-point wiring and a small breadboard power strip.

## Pinout Diagram

                            TOP / USB PORT
                        ┌───────────────────┐
        (LCD Data) YEL ─┤ 1  GP0     VBUS 40├─ GRN (LCD Backlight 5V)
       (LCD Clock) BLU ─┤ 2  GP1     VSYS 39├─ [OPEN]d
     (Encoder GND) YEL ─┤ 3  GND      GND 38├─ PUR (LCD Backlight GND)
     (Encoder CLK) GRN ─┤ 4  GP2   3V3_EN 37├─ [OPEN]
      (Encoder DT) RED ─┤ 5  GP3  3V3_OUT 36├─ ORG (Power Rail 3.3V)
                [OPEN] ─┤ 6  GP4 ADC_VREF 34├─ [OPEN]
                [OPEN] ─┤ 7  GP5     GP28 34├─ YEL (Pedal 3 Signal / Una Corda Soft)
                [OPEN] ─┤ 8  GND      GND 33├─ GRN (Power Rail GND)
                [OPEN] ─┤ 9  GP6     GP27 32├─ YEL (Pedal 2 Signal / Sostenuto Hold)
                [OPEN] ─┤ 10 GP7     GP26 31├─ YEL (Pedal 1 Signal / Damper Sustain)
                [OPEN] ─┤ 11 GP8      RUN 30├─ [OPEN]
                [OPEN] ─┤ 12 GP9     GP22 29├─ [OPEN]
                [OPEN] ─┤ 13 GND      GND 28├─ GRN (Pedal 3 GND / Una Corda Soft)*
                [OPEN] ─┤ 14 GP10    GP21 27├─ [OPEN]
                [OPEN] ─┤ 15 GP11    GP20 26├─ [OPEN]
                [OPEN] ─┤ 16 GP12    GP19 25├─ [OPEN]
                [OPEN] ─┤ 17 GP13    GP18 24├─ [OPEN]
    (Switch GND)   BRN ─┤ 18 GND      GND 23├─ [OPEN]
    xSwitch Line)  ORG ─┤ 19 GP14    GP17 22├─ [OPEN]
                [OPEN] ─┤ 20 GP15    GP16 21├─ [OPEN]
                        └───────────────────┘
                               BOTTOM

##Encoder Wiring##

    Enc. side                   Switch side
	
		  [ ]                         [ ]
		|-----|                     |-----|
		| ||| |                     | | | |
			|||                       	| |
			||-- GRN - CLK > PIN 4    	| -- ORG - LINE > PIN 19
			|--- YEL - GND > PIN 3    	|
			---- RED - DT  > PIN 5    	---- BRN - GND  > PIN 18

##Screen Wiring##

Uses 4 pin header connector.

    GRN - 5v power > PIN 40
		PUR - GND > PIN 38
		BLU - LCD Data > PIN 1
		YEL - LCD Clock > PIN 2

##Hardware Inventory##

		HW-01: Raspberry Pi Pico 2 W (Main Controller).
		HW-02: Waveshare RGB1602 LCD I2C Character Display (Text at 0x3E, RGB at 0x60).
		HW-03: Continuous Rotary Encoder Parameter Dial
		HW-04: Shaft Push-Button Interrupt Switch (Built into encoder center-click).
		HW-05: 6.35mm Jack Socket 1 (Right Pedal / Damper Sustain).
		HW-06: 6.35mm Jack Socket 2 (Middle Pedal / Sostenuto Hold).
		HW-07: 6.35mm Jack Socket 3 (Left Pedal / Una Corda Soft).
		HW-08: Several DuPont wires
		HW-09: Panel-Mount USB Extender (Chassis strain relief link).
		HW-10: Small plastic enclosure.

##Pending Workspace Acquisitions##

		PD-01: Standard Breakaway Male Pin Headers (Logged for future circuit coupling builds)

##Known Hardware Notes##

		There is a short circuit problem with the jacks I selected for the project. They are switched (normally-closed), which is not the type I should have bought. See KNOWN_ISSUES.md. :)
