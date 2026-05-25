# MidiJoy-Plus
Turn an Arduino Pro Micro into a bi-directional retro toolkit: an advanced Sega/Atari controller-to-USB MIDI/HID adapter, or a multi-channel USB MIDI parallel synthesizer driver for classic Atari 8-bit computers.

A self-contained, bi-directional hardware interface that bridges vintage DB9 controllers, modern USB MIDI/HID, and classic Atari 8-bit computers using a single Arduino Pro Micro (ATmega32U4).

Based on and inspired by the original [MidiJoy by fredlcore](https://github.com/fredlcore/MidiJoy).

## 🚀 Features

This project utilizes a physical toggle switch to completely alter the hardware and software behavior of the microcontroller, creating two entirely isolated operational modes:

### 1. Synth Mode (Switch Released)
* **Function:** Acts as a USB MIDI-to-Parallel converter.
* **Operation:** Receives streaming class-compliant USB MIDI note and volume messages from a DAW or PC, processes them, and outputs a multiplexed parallel stream through a female-to-female DB9 cable directly into an Atari 8-bit computer to drive its custom internal sound chips.

### 2. Controller & MIDI Mode (Switch Depressed)
* **Legacy HID Parsing:** Actively decodes 1-button Atari joysticks, passive 2-button Master System pads, active 3/6-button Sega Genesis controllers, and continuous analog Paddles.
* **Dual USB Layer Output:** Concurrently maps all retro hardware button states into standard USB HID Gamepad inputs *and* unique USB MIDI note/CC streams for modern DAW automation.

---

## 🛠️ The Hardware Safety Hack

To keep the design completely self-contained and avoid complex, expensive internal multiplexing chip stacks, this project uses a standard **Male DB9 Port** on the enclosure combined with a physical cable-swapping methodology. 

Because a Genesis controller and the female-to-female Atari console cable cannot physically occupy the DB9 socket at the same time, it creates a bulletproof **mechanical safety interlock**. This completely eliminates the risk of the Pro Micro accidentally blasting output voltages into a passive controller or fighting a device for line control if the mode switch is toggled.

---

## 📌 Hardware Tracing (Pro Micro to DB9)

| Pro Micro Pin | DB9 Pin Function (Controller Mode) | DB9 Pin Function (Synth Mode) |
|---------------|-----------------------------------|-------------------------------|
| **Pin 2**     | Port 1 Up                         | Parallel Data bit 0           |
| **Pin 3**     | Port 1 Down                       | Parallel Data bit 1           |
| **Pin 4**     | Port 1 Left                       | Parallel Data bit 2           |
| **Pin 5**     | Port 1 Right                      | Parallel Data bit 3           |
| **Pin 6**     | Port 1 Fire (Sega Button B/A)     | Parallel Audio Channel Gate 0 |
| **Pin 7**     | Port 2 Up (Sega SELECT line)      | Parallel Vol / Pitch Latch    |
| **Pin 14**    | Port 2 Fire                       | Parallel Audio Channel Gate 1 |
| **A0**        | Paddle A Analog Read              | Idle                          |
| **A8 (D8)**   | Paddle B Analog Read              | Idle                          |

---

## 📦 Dependencies & Credits

This project stands on the shoulders of some incredible open-source work:
* **MidiJoy Engine Concepts:** Inspired by [fredlcore's MidiJoy](https://github.com/fredlcore/MidiJoy).
* **USB HID Gamepad Stack:** Powered by the [MHeironimus Arduino Joystick Library](https://github.com/MHeironimus/ArduinoJoystickLibrary) (utilizing the v1.0 `sendState()` architecture).
* **USB MIDI Core:** Managed via the standard Arduino `MIDIUSB` library.

## 📝 License
This project is open-source and available under the [MIT License](LICENSE).
