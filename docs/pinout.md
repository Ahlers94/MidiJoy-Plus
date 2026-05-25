# Hardware Pinout and Mapping Guide (MidiJoy-Plus)

This document provides the definitive physical wire trace-out from the Arduino Pro Micro (ATmega32U4) to the dual Male DB9 controller ports, matching the configuration verified in `MidiJoy-Plus.ino`.

---

## 📌 Master Pin Mapping Table

| DB9 Pin # | Port 1 Function (Main Bus) | Port 2 Function (Extended / Paddles) | Pro Micro Pin | Software Variable Name |
| :--- | :--- | :--- | :--- | :--- |
| **Pin 1** | Port 1 D-Pad Up | Port 2 D-Pad Up | **2** (Port 1) / **7** (Port 2) | `p1_up` / `p2_up` |
| **Pin 2** | Port 1 D-Pad Down | Port 2 D-Pad Down / Paddle B | **3** (Port 1) / **8 / A8** (Port 2) | `p1_down` / `p2_down` |
| **Pin 3** | Port 1 D-Pad Left | Port 2 D-Pad Left | **4** (Port 1) / **9** (Port 2) | `p1_left` / `p2_left` |
| **Pin 4** | Port 1 D-Pad Right | Port 2 D-Pad Right | **5** (Port 1) / **16** (Port 2) | `p1_right` / `p2_right` |
| **Pin 5** | Unused / Floating | Paddle A Analog Input | *N/A* (Port 1) / **A0** (Port 2) | `paddleA_pin` |
| **Pin 6** | Port 1 Fire (Button B) | Port 2 Fire / Audio Gate 1 | **6** (Port 1) / **14** (Port 2) | `p1_fire` / `p2_fire` |
| **Pin 7** | Power Gate / Sync Line | Power Gate / Sync Line | **Hardware Switch / D7** | `p2_up` (Shared Software Logic) |
| **Pin 8** | Common Ground (`GND`) | Common Ground (`GND`) | **GND** | *Hardware Ground* |

---

## ⚡ 4-Pole Switch Mechanical Routing Logic

The hardware utilizes a physical 4-pole toggle switch acting as two independent mechanical routing gates. This layout isolates the software engines and protects connected peripherals on a hardware level.

### Pole A: Software Engine Trigger (`D10`)
* **Switch Engaged (Controller Mode):** Connects Pro Micro Pin **10** directly to Common Ground (`GND`). The firmware registers this `LOW` state and jumps into the Gamepad/USB-MIDI polling loop.
* **Switch Released (Synth Mode):** Breaks the ground connection. The internal ATmega pull-up snaps Pin 10 back to `HIGH`, jumping the firmware into the parallel MIDI-to-Atari synth processing engine.

### Pole B: Power Gate Interlock (DB9 Pins 7)
* **Switch Engaged (Controller Mode):** Mechanically closes the circuit to flood **Pin 7** on both DB9 ports directly with $+5\text{V}$ VCC. This provides steady, continuous hardware logic power to drive the internal circuitry of connected Sega Master System or Genesis controllers.
* **Switch Released (Synth Mode):** Completely breaks the $+5\text{V}$ VCC line from the DB9 ports. Pin 7 on the ports disconnects from system power and is driven purely contextually by parallel data/sync timing states.

---

## 🔌 Core Hardware Layer Implementations

### 1. Dual-Purpose Analog Mapping (Port 2 Pin 2)
Port 2 Pin 2 is tied straight to Pro Micro **Pin 8**, taking advantage of the ATmega32U4's hardware-multiplexed **A8** Analog-to-Digital Converter channel. When a standard digital joystick is plugged in, it reads as a digital "Down" direction (`INPUT_PULLUP`). When analog Paddles are connected, the firmware actively samples the continuous voltage curve to map precise USB MIDI CC values.

### 2. Mechanical Safety Interlock
Because the Pro Micro reconfigures its digital pins into high-current `OUTPUT` lines to flash parallel bit-bursts into the Atari 8-bit computer during Synth Mode playback, **never have a controller plugged into the box when a female-to-female DB9 console link is attached.** The physical sharing of a single male DB9 connector per channel enforces a bulletproof mechanical interlock, eliminating the risk of electrical bus contention.
