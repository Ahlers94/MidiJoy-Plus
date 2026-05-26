#include <MIDIUSB.h>
#include <Joystick.h>

// Labeled Mode Switch Pin on the Pro Micro
const int modeSwitchPin = 10; 

// Port 1 Connections
const int p1_up    = 2;
const int p1_down  = 3;
const int p1_left  = 4;
const int p1_right = 5;
const int p1_fire  = 6;

// Port 2 Connections
const int p2_up    = 7; // Actively doubles as Port 1 & 2 Sega SELECT line in Controller Mode!
const int p2_down  = 8;  
const int p2_left  = 9;  
const int p2_right = 16; 
const int p2_fire  = 14; 

// Dedicated Paddle Analog Inputs
const int paddleA_pin = A0;  // Jumper from DB9 Port 2 Pin 5
const int paddleB_pin = A8;  // Native tracking from DB9 Port 2 Pin 2 -> D8

// Joystick Configuration
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
  8, 1,                     // 8 Buttons, 1 Hat Switch
  true, true, false,        // X and Y Axis enabled for paddles
  false, false, false,      // No Rx, Ry, Rz
  false, false, false,      // No Rudder, Throttle, Accelerator
  false, false);            // No Brake, No Steering

// State Tracking
bool lastUp = HIGH; bool lastDown = HIGH; bool lastLeft = HIGH; bool lastRight = HIGH; bool lastFire = HIGH;
int lastPaddleA = -1; int lastPaddleB = -1;
bool lastModeState = true; 
bool lastA = HIGH; bool lastC = HIGH; bool lastStart = HIGH;

void setup() {
  pinMode(modeSwitchPin, INPUT_PULLUP);
  
  Joystick.setXAxisRange(-127, 127);
  Joystick.setYAxisRange(-127, 127);
  Joystick.begin(false); 
  
  setSynthMode();
}

void setSynthMode() {
  int pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 14, 16};
  for (int i = 0; i < 10; i++) {
    pinMode(pins[i], OUTPUT);
    digitalWrite(pins[i], HIGH);
  }
}

void allNotesOff() {
  int pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 14, 16};
  for (int i = 0; i < 10; i++) {
    digitalWrite(pins[i], HIGH);
  }
}

void setControllerMode() {
  int inputs[] = {2, 3, 4, 5, 6, 8, 9, 14, 16};
  for (int i = 0; i < 9; i++) {
    pinMode(inputs[i], INPUT_PULLUP);
  }
  pinMode(p2_up, OUTPUT);
  digitalWrite(p2_up, HIGH);
}

// --- ENGINE 1: MIDIJOY PARALLEL MULTIPLEXER ---
void sendToAtari(uint8_t channel, uint8_t data, bool isVolume) {
  digitalWrite(p1_fire, (channel & 0x01) ? LOW : HIGH);
  digitalWrite(p2_fire, (channel & 0x02) ? LOW : HIGH);

  if (isVolume) {
    digitalWrite(p2_up, LOW); 
    digitalWrite(p1_up,    (data & 0x01) ? LOW : HIGH);
    digitalWrite(p1_down,  (data & 0x02) ? LOW : HIGH);
    digitalWrite(p1_left,  (data & 0x04) ? LOW : HIGH);
    digitalWrite(p1_right, (data & 0x08) ? LOW : HIGH);
  } else {
    digitalWrite(p2_up, HIGH);
    digitalWrite(p1_up,    (data & 0x01) ? LOW : HIGH);
    digitalWrite(p1_down,  (data & 0x02) ? LOW : HIGH);
    digitalWrite(p1_left,  (data & 0x04) ? LOW : HIGH);
    digitalWrite(p1_right, (data & 0x08) ? LOW : HIGH);
    digitalWrite(p2_down,  (data & 0x10) ? LOW : HIGH);
    digitalWrite(p2_left,  (data & 0x20) ? LOW : HIGH);
    digitalWrite(p2_right, (data & 0x40) ? LOW : HIGH);
  }
  delayMicroseconds(500); 
  allNotesOff();
}

void parseMidi(uint8_t status, uint8_t byte1, uint8_t byte2) {
  uint8_t command = status & 0xF0;
  uint8_t channel = status & 0x0F;
  if (channel > 3) return; 

  if (command == 0x90 && byte2 > 0) {
    sendToAtari(channel, byte1, false);
    sendToAtari(channel, byte2 >> 3, true); 
  } 
  else if (command == 0x80 || (command == 0x90 && byte2 == 0)) {
    sendToAtari(channel, 0, true); 
  }
}

// --- ENGINES 2 & 3: HYBRID CONTROLLER ENGINE ---
void handleInputs() {
  // 1. Read Standard Buttons (Select HIGH)
  digitalWrite(p2_up, HIGH);
  delayMicroseconds(50); 
  
  bool g_up    = !digitalRead(p1_up);
  bool g_down  = !digitalRead(p1_down);
  bool g_left  = !digitalRead(p1_left);
  bool g_right = !digitalRead(p1_right);
  bool btnB    = !digitalRead(p1_fire); 
  bool btnC    = !digitalRead(p2_left); 

  // 2. Read Select/A (Select LOW)
  digitalWrite(p2_up, LOW);
  delayMicroseconds(50);
  
  bool btnA    = !digitalRead(p1_fire); 
  bool btnStart = !digitalRead(p2_left); 

  // --- HID GAMEPAD MAPPING ---
  Joystick.setButton(0, btnA);
  Joystick.setButton(1, btnB);
  Joystick.setButton(2, btnC);
  Joystick.setButton(3, btnStart);
  
  // D-Pad Hat Mapping
  int angle = -1;
  if (g_up && g_right) angle = 45;
  else if (g_down && g_right) angle = 135;
  else if (g_down && g_left) angle = 225;
  else if (g_up && g_left) angle = 315;
  else if (g_up) angle = 0;
  else if (g_right) angle = 90;
  else if (g_down) angle = 180;
  else if (g_left) angle = 270;
  Joystick.setHatSwitch(0, angle);

  // Analog Paddles 
  Joystick.setXAxis(map(analogRead(paddleA_pin), 0, 1023, -127, 127));
  Joystick.setYAxis(map(analogRead(paddleB_pin), 0, 1023, -127, 127));
  
  Joystick.sendState();
  digitalWrite(p2_up, HIGH); // Reset Select

  // --- USB MIDI MAPPING ---
  if (g_up && lastUp == HIGH)       { sendMidi(0x90, 60, 127); lastUp = LOW; }
  else if (!g_up && lastUp == LOW)  { sendMidi(0x80, 60, 0);   lastUp = HIGH; }
  
  if (btnB && lastFire == HIGH)     { sendMidi(0x90, 64, 127); lastFire = LOW; }
  else if (!btnB && lastFire == LOW){ sendMidi(0x80, 64, 0);   lastFire = HIGH; }

  if (btnA && lastA == HIGH)        { sendMidi(0x90, 65, 127); lastA = LOW; }
  else if (!btnA && lastA == LOW)   { sendMidi(0x80, 65, 0);   lastA = HIGH; }

  if (btnC && lastC == HIGH)        { sendMidi(0x90, 67, 127); lastC = LOW; }
  else if (!btnC && lastC == LOW)   { sendMidi(0x80, 67, 0);   lastC = HIGH; }

  if (btnStart && lastStart == HIGH) { sendMidi(0x90, 72, 127); lastStart = LOW; }
  else if (!btnStart && lastStart == LOW) { sendMidi(0x80, 72, 0); lastStart = HIGH; }

  int midiCC_A = map(analogRead(paddleA_pin), 0, 1023, 0, 127);
  int midiCC_B = map(analogRead(paddleB_pin), 0, 1023, 0, 127);
  if (abs(midiCC_A - lastPaddleA) > 1) { sendMidi(0xB0, 10, midiCC_A); lastPaddleA = midiCC_A; }
  if (abs(midiCC_B - lastPaddleB) > 1) { sendMidi(0xB0, 11, midiCC_B); lastPaddleB = midiCC_B; }
}

void sendMidi(uint8_t cmd, uint8_t data1, uint8_t data2) {
  midiEventPacket_t midiMsg = {cmd >> 4, cmd, data1, data2};
  MidiUSB.sendMIDI(midiMsg);
  MidiUSB.flush();
}

void loop() {
  bool isControllerMode = (digitalRead(modeSwitchPin) == LOW);

  if (!isControllerMode) {
    if (lastModeState != isControllerMode) { 
      setSynthMode(); 
      lastModeState = isControllerMode; 
    }
    midiEventPacket_t rx;
    do {
      rx = MidiUSB.read();
      if (rx.header != 0) { parseMidi(rx.byte1, rx.byte2, rx.byte3); }
    } while (rx.header != 0);
  } else {
    if (lastModeState != isControllerMode) { 
      setControllerMode(); 
      lastModeState = isControllerMode; 
    }
    handleInputs();
    delay(10); 
  }
}
