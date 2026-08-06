# SID, Electronics & Audio

## "Electronics is the art of controlling how voltage and current change over time. Components are the tools; waveforms are the language."

- Bias & P-P (peak to peak), is pretty much what I thought - a baseline and a peak to peak operates around it
- SID uses 9/12v for audio and 5v for logic 
- TTL likely works for SID @ 3.3v
- dont bother running it faster than ~1mhz
- use smoothing circuit as per c64/128 diagram -> output voltage ~-2.5/+2.5v
- use 3.3v pico logic to drive a 1Mhz square wave at Phi2 on the sid
- put data and address on the bus before the rising edge
- assert /CS and R//W within a few dozen ns after the edge

----------------------------------------------------------

# ohms law

        V
       ───
      I × R

----------------------------------------------------------
      
# Basic electronic building blocks

## Resistor (R)
Controls the relationship between voltage and current; follows Ohm's Law V=IR, mainly used to limit current, divide voltage, and set bias conditions.

## Capacitor (C)
Stores energy in an electric field; resists sudden voltage changes and creates time-dependent behaviour (filters, smoothing, coupling, timing).

## Inductor (L)
Stores energy in a magnetic field; resists sudden current changes and is used in filters, power supplies, transformers, and magnetic devices.

## Diode
A one-way semiconductor valve; allows current in one direction and introduces nonlinear behaviour used for rectification, protection, LEDs, and signal clipping/distortion.

## Transistor
A controllable semiconductor device; a small signal controls a larger current, allowing it to act as a switch (digital logic) or amplifier (analogue circuits).

# Concepts linked to the components

## Ohm's Law
The basic relationship between voltage, current, and resistance; the foundation for understanding electrical behaviour.

## Time Constant (RC)
The combination of resistance and capacitance determines how quickly a circuit responds; the basis of filters, envelopes, and timing behaviour.

## Linearity
A system where input/output relationships stay proportional and predictable; signals are transformed without creating new frequency content.

## Nonlinearity
A system where the relationship changes depending on signal level; creates harmonics, distortion, saturation, and analogue "character".

## Harmonics
Higher-frequency sine waves related to the fundamental frequency; combinations of harmonics create the shape and tone of complex waveforms.

## Fourier Analysis
The idea that any repeating waveform can be represented as a combination of sine waves with different frequencies, amplitudes, and phases.

## Filter
A circuit that changes the balance of frequencies in a signal; usually built from resistors, capacitors, inductors, and active devices.

## Phase
The timing relationship between waveforms or frequencies; filters can change phase because different frequencies experience different delays.

# Audio connections

## Analogue clipping
A nonlinear circuit limitation (diodes, transistors, tubes, etc.) that reshapes a waveform and creates harmonics.

## Digital clipping
A numerical limit where samples exceed the maximum value and are flattened; usually harsher because the boundary is mathematically absolute.

## Analogue "vibe"
Often the result of controlled nonlinear behaviour, where imperfections in real components create harmonics and dynamic responses.
