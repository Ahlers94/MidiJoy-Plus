# Hardware Pinout and Mapping Guide (MidiJoy-Plus)

This document outlines the exact hardware trace-out from the Arduino Pro Micro (ATmega32U4) to the physical Male DB9 controller ports. 

---

## 📌 Master Pin Mapping Table

| DB9 Pin | Port 1 (Main Bus) | Port 2 (Extended / Paddles) | Pro Micro Pin | Software Variable Name |
| :--- | :--- | :--- | :--- | :--- |
| **1** | D-Pad Up / Data 0 | Port 2 Up | **2** (Port 1) / **7** (Port 2) | `p1_up` / `p2_up` |
| **2** | D-Pad Down / Data 1 | Port 2 Down / Paddle B | **3** (Port 1) / **8 / A8** (Port 2) | `p1_down` / `p2_down` |
| **3** | D-Pad Left / Data 2 | Port 2 Left | **4** (Port 1) / **9** (Port 2) | `p1_left` / `p2_left` |
| **4** | D-Pad Right / Data 3 | Port 2 Right | **5** (Port 1) / **16** (Port 2) | `p1_right` / `p2_right` |
| **5** | $+5\text{V}$ VCC (Optional) | Paddle A Analog Input | **VCC** (Port 1) / **A0** (Port 2) | `paddleA_pin` |
| **6** | Button B / Audio Gate 0 | Port 2 Fire / Audio Gate 1 | **6** (Port 1) / **14** (Port 2) | `p1_fire` / `p2_fire` |
| **7** | Sega SELECT Line | Sega SELECT Line (Linked) | **7** (Driven by Pro Micro) | `p2_up` (Doubles as Select) |
| **8** | Common Ground (`GND`) | Common Ground (`GND`) | **GND** | *Hardware Ground* |
| **9** | Unused / GND | Unused | *N/A* | *N/A* |

---

## ⚠️ Critical Electrical Verification

### 1. The Sega Select Hookup (Pin 7)
Do **not** tie Pin 7 of the DB9 ports directly to the VCC side of your hardware toggle switch. 
* To decode a Sega Genesis controller, **Pro Micro Pin 7** must act as an active logic output that toggles between $0\text{V}$ and $+5\text{V}$ dynamically in code. 
* Wire DB9 Port 1 Pin 7 and DB9 Port 2 Pin 7 together, and run them straight to **Pin 7** on your Pro Micro. 

### 2. The Mode Switch (Pin 10)
Your hardware toggle switch should be wired like this:
* **Center Pin (Common):** Connected to **Pro Micro Pin 10**.
* **Position 1 (Engaged):** Tied straight to **GND** (pulls Pin 10 `LOW` to activate Controller Mode).
* **Position 2 (Released):** Left floating or tied to VCC (internal pull-up keeps Pin 10 `HIGH` to activate Synth Mode).

### 3. Paddle B Dual-Routing (Pin 2 / A8)
Because Port 2 Pin 2 acts as both a digital direction input (**Down**) for a standard joystick and an analog input for **Paddle B**, it maps to Pro Micro **Pin 8**, which natively exposes the ATmega32U4's **A8** Analog-to-Digital converter channel. The software automatically handles switching this pin's behavior contextually.
