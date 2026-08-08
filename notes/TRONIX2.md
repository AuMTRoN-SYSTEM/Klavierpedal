# TRONIX 2

# Power — Practical Mental Model

## AC vs DC

* **AC (Alternating Current/Voltage):** polarity changes direction. A typical sine wave goes positive → zero → negative → zero → positive.
* **DC (Direct Current/Voltage):** polarity does not reverse. It does **not** have to be perfectly flat.
* So: **AC can be thought of as changing sign; DC stays on one side of zero.**
* A rectified waveform can therefore be DC even though it still rises and falls.
* AC does not have to be sinusoidal, and DC does not have to be perfectly flat.

## Turning 240 V AC into useful DC

A traditional linear power supply can be thought of as a sequence of jobs:

**240 V AC → Transformer → Rectifier → Capacitor → Regulator → DC supply**

### 1. Transformer

* Changes the AC voltage/current relationship.
* Can provide galvanic isolation from the mains.
* Example: **240 V AC → 9 V AC** or **240 V AC → 12 V AC**.
* Lower voltage means proportionally greater available current (within the transformer's power rating).

### 2. Rectifier

* Converts AC into **unidirectional/pulsating DC**.
* A bridge rectifier uses four diodes.
* During each half-cycle, a different pair of diodes conducts, but current through the load always flows in the same direction.
* Effectively, the negative halves of the sine wave are **flipped upward**.
* With 50 Hz mains: full-wave rectification produces **100 Hz pulses**.
* The source is still 50 Hz AC; the rectifier output simply has two positive pulses per cycle.

### 3. Capacitor / Reservoir Capacitor

* Stores electrical energy.
* Charges near the peaks of the rectified waveform.
* Discharges into the load as the waveform falls.
* Therefore it **fills in the valleys** between rectifier peaks.
* This is essentially a filtering/smoothing action.
* The result is **smoothed DC with ripple**, not perfectly flat DC.
* Bigger capacitor → generally less voltage drop/ripple between peaks for a given load.

### 4. Regulator

* Holds the output voltage at a defined level despite reasonable changes in input and load.
* Conceptually similar to a compressor in the sense that it **actively maintains a target level**, although the mechanism and purpose are different.
* Example: a 7805 can regulate a suitable higher DC input to approximately **5 V DC**.
* A linear regulator dissipates excess voltage as heat.

## Voltage, Current and Power

* The regulator establishes the **voltage**.
* The load determines how much **current** it draws.
* **Power = Voltage × Current.**
* A **5 V, 2 A** supply does NOT force 2 A into the circuit.
* It means the supply can maintain 5 V while being capable of supplying up to 2 A if the load requires it.
* Mental model: **the circuit is a sponge; the power supply is not a forceful pump.**

## Audio connection

* The DC supply is **not the audio signal**.
* DC provides the energy and operating conditions from which analogue circuitry works.
* The circuitry uses that energy to generate/manipulate the changing audio waveform.
* So:

  * **DC power = energy source**
  * **Audio waveform = signal**
* A higher analogue supply can provide analogue circuitry with greater **voltage headroom**, but the supply itself is not a carrier or part of the audio signal.

## SID example

The SID has separate digital and analogue requirements.

* The SID's digital circuitry uses a **5 V-class supply** (exact details depend on the SID model).
* Its analogue section also needs its appropriate **DC supply** — historically around **9 V or 12 V depending on the SID/model and implementation**.
* That higher analogue supply is **not an AC audio carrier**.
* The analogue circuitry simply needs that electrical environment — power and voltage headroom — to operate.
* The circuitry then uses that energy to generate and process the actual audio waveform.

## Logic voltage vs supply voltage

A chip's **supply voltage** and the voltage levels used for **logic communication** are related but are not automatically the same thing.

A 5 V-powered chip can potentially communicate with a 3.3 V processor, provided the logic thresholds and input/output voltage limits are compatible.

In particular:

* **3.3 V → 5 V device:** check whether 3.3 V is recognised as a valid HIGH.
* **5 V device → 3.3 V device:** ensure the 3.3 V device's input is not exposed to an excessive voltage; level shifting may be required.

TTL/CMOS thresholds and the exact SID electrical specifications matter when actually designing the interface.

## The overall mental model

**Transformer:** “Change the voltage.”

**Rectifier:** “Make the current one-way.”

**Capacitor:** “Store energy and smooth out the gaps.”

**Regulator:** “Hold the voltage at the level we want.”

**Load:** “Draw the current it needs.”

**Power:** “How much energy is actually being transferred per second.”

And the key practical idea:

> **A power supply establishes the electrical conditions; the circuit uses those conditions to do its work.**
