# TRONIX 3

# Impedance / buffers

* Impedance (Z): the opposition a circuit presents to AC; measured in ohms (Ω). For now, think of it as the AC counterpart to resistance.

* Loading: a source's output voltage can be affected when the load impedance is too low for the source to drive comfortably.

* Parallel loads: when one signal feeds multiple inputs, their impedances appear in parallel. Two 10 kΩ inputs → 5 kΩ effective load.

* Audio splitting: a passive Y-split isn't inherently wrong. With a low-impedance source and high-impedance inputs, the voltage loss can be tiny.

* Buffer: presents a high impedance to its input and provides a low impedance output, allowing the signal to drive subsequent circuitry without heavily loading the original source.

* Guitar pedal buffer: same fundamental idea — protect the guitar pickup from having to drive cables/pedals directly, while providing a stronger, low-Z version of the signal downstream.

* Guitar input: deliberately high-Z so it doesn't heavily load the pickup and alter its signal/frequency response.

* Power vs signal: when powering circuits in parallel, they all receive the supply voltage; each branch draws the current it requires. You don't divide the voltage/current into equal portions.

A voltage source should not be asked to drive a load heavier than it was designed to drive.