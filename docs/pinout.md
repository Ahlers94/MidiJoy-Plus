# Hardware Pinout and Mapping Guide

This document provides the exact hardware trace-out from the Arduino Pro Micro (ATmega32U4) to the physical Male DB9 controller ports. Because this device functions bi-directionally, the pins alter their electrical configuration dynamically based on the hardware toggle switch on **Pin 10**.

---

## 📌 Master Pin Mapping Table

| Pro Micro Pin | ATmega32U4 Port | DB9 Port Assignment | Controller Mode Function (Input) | Synth Mode Function (Output) |
| :--- | :--- | :--- | :--- | :--- |
| **2** | PD1 | Port 1 - Pin 1 | D-Pad Up | Parallel Data Bit 0 |
| **3** | PD0 | Port 1 - Pin 2 | D-Pad Down | Parallel Data Bit 1 |
| **4** | PD4 | Port 1 - Pin 3 | D-Pad Left | Parallel Data Bit 2 |
| **5** | PC6 | Port 1 - Pin 4 | D-Pad Right | Parallel Data Bit 3 |
| **6** | PD7 | Port 1 - Pin 6 | Button B / Button A | Audio Channel Gate 0 |
| **7** | PE6 | Port 1 & 2 - Pin 7 | Sega SELECT Multiplex Line | Vol / Pitch Latch Toggle |
| **14** | PB3 | Port 2 - Pin 6 | Port 2 Fire Trigger | Audio Channel Gate 1 |
| **16** | PB2 | Port 2 - Pin 4 | Port 2 D-Pad Right | Idle |
| **8** | PB4 | Port 2 - Pin 2 | Port 2 D-Pad Down | Parallel Data Bit 4 |
| **9** | PB5 | Port 2 - Pin 3 | Port 2 D-Pad Left / Sega Start / C | Parallel Data Bit 5 |
| **A0 (D18)** | PF7 | Port 2 - Pin 5 | Paddle A Analog Read | Idle |
| **A8 (D8)** | PF1 | Port 2 - Pin 2 | Paddle B Analog Read (Doubled) | Idle |
| **10** | PB6 | Internal Switch | Mode Selector (Low = Controller, High = Synth) |

---

## 🔌 Hardware Port Explanations

### Port 1 (Main Controller / Main Synth Bus)
Port 1 functions as the primary hub for your standard gameplay inputs (Atari, Master System, and the primary lines of a Sega Genesis pad). When flipped to **Synth Mode**, these lines instantly become an active 4-bit data bus running parallel information alongside an audio channel gate to communicate with the Atari 8-bit computer's sound architecture.

### Port 2 (Extended Controller / Paddle Hub)
Port 2 handles your advanced secondary controllers and dedicated analog inputs. 
* **Sega 3/6-Button Extensions:** Pin 9 (`p2_left`) tracks the multiplexed shift-register updates for Sega **Button C** and the **Start Button**. 
* **Analog Paddles:** **Pin 5** on Port 2 routes directly into the ATmega32U4's internal Analog-to-Digital Converter (**A0**) to continuously monitor the RC charging rate of Paddle A. **Paddle B** is tied to **A8** for seamless analog streaming.

---

## ⚡ Technical Electrical Notes

### 1. Internal Pull-Ups
When **Controller Mode** is engaged, all digital pins are configured as `INPUT_PULLUP`. This pulls the lines high to $+5\text{V}$. When a directional switch or arcade button on a classic gamepad is pressed, it bridges that pin directly to Ground (`GND`), pulling the line `LOW`. The firmware senses this logic inversion (`!button_state`) to register the click.

### 2. The Sega Select Multiplexer
**Pin 7 (PE6)** is a critical pin. 
* In **Controller Mode**, it acts as an active `OUTPUT` used to aggressively toggle the internal multiplexing chips inside Sega Genesis pads to read all 6 extended buttons across multiple fast polling cycles.
* In **Synth Mode**, it drops back to serve as the Volume/Pitch latch control line sending synchronization timing pulses back to the Atari console.

### 3. Mechanical Safety Isolation
Because the device outputs $+5\text{V}$ and $0\text{V}$ data pulses on Pins 2, 3, 4, 5, 6, and 7 during Synth playback, **never connect a gamepad to the DB9 port while a female-to-female MIDIJoy console link is attached.** The physical layout of using a single male port for both functions inherently acts as a safety interlock, preventing the user from accidentally connecting both simultaneously.
