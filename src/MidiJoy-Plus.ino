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

// Dedicated Paddle Analog Inputs tailored to your trace-out
const int paddleA_pin = A0;  // Jumper from DB9 Port 2 Pin 5
const int paddleB_pin = A8;  // Native tracking from DB9 Port 2 Pin 2 -> D8

// --- DEFINITIVE OPTION B FIX ---
// We initialize with a clean, fully explicit constructor:
// Parameters: ID, Type, Button Count, Hat Count, X, Y, Z, Rx, Ry, Rz, Rudder, Throttle, Accelerator, Brake, Steering
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, JOYSTICK_TYPE_GAMEPAD,
  8, 1,                  // 8 Buttons, 1 Hat Switch
  true, true, false,     // X and Y Axis enabled for paddles, No Z Axis
  false, false, false,   // No Rx, Ry, Rz
  false, false, false,   // No Rudder, Throttle, Accelerator
  false, false);         // No Brake, No Steering

// State Tracking for Controller Debouncing
bool lastUp = HIGH; bool lastDown = HIGH; bool lastLeft = HIGH; bool lastRight = HIGH; bool lastFire = HIGH;
int lastPaddleA = -1; int lastPaddleB = -1;
bool lastModeState = true; // High tracks Synth, Low tracks Controller

// Extra tracking states for Genesis specific buttons (MIDI Mapping)
bool lastA = HIGH; bool lastC = HIGH; bool lastStart = HIGH;

void setup() {
  pinMode(modeSwitchPin, INPUT_PULLUP);
  
  // Set explicit axis ranges to match our old mapping values (-127 to 127)
  Joystick.setXAxisRange(-127, 127);
  Joystick.setYAxisRange(-127, 127);
  
  // CRITICAL: Passing 'false' here inside begin() is the official library 
  // method to turn off Auto-Reporting and activate manual .sendReport()!
  Joystick.begin(false); 
  
  setSynthMode();
}

void setSynthMode() {
  pinMode(p1_up, OUTPUT);    pinMode(p1_down, OUTPUT);
  pinMode(p1_left, OUTPUT);   pinMode(p1_right, OUTPUT);
  pinMode(p1_fire, OUTPUT);
  
  pinMode(p2_up, OUTPUT);    pinMode(p2_down, OUTPUT);
  pinMode(p2_left, OUTPUT);   pinMode(p2_right, OUTPUT);
  pinMode(p2_fire, OUTPUT);

  allNotesOff();
}

void allNotesOff() {
  digitalWrite(p1_up, HIGH);    digitalWrite(p1_down, HIGH);
  digitalWrite(p1_left, HIGH);   digitalWrite(p1_right, HIGH);
  digitalWrite(p1_fire, HIGH);
  
  digitalWrite(p2_up, HIGH);    digitalWrite(p2_down, HIGH);
  digitalWrite(p2_left, HIGH);   digitalWrite(p2_right, HIGH);
  digitalWrite(p2_fire, HIGH);
}

void setControllerMode() {
  // Configure Port 1 for standard input reading
  pinMode(p1_up, INPUT_PULLUP);    pinMode(p1_down, INPUT_PULLUP);
  pinMode(p1_left, INPUT_PULLUP);   pinMode(p1_right, INPUT_PULLUP);
  pinMode(p1_fire, INPUT_PULLUP);
  
  // Port 2 lines configured for inputs
  pinMode(p2_down, INPUT_PULLUP);   pinMode(p2_left, INPUT_PULLUP);
  pinMode(p2_right, INPUT_PULLUP);  pinMode(p2_fire, INPUT_PULLUP);
  
  // Pin 7 (p2_up) hooks to DB9 Pin 7 via your power gate switch
  pinMode(p2_up, OUTPUT);
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

// --- ENGINES 2 & 3: HYBRID CONTROLLER ENGINE WITH SEGA MULTIPLEXING ---
void handleInputs() {
  // Variables to hold decoded Sega states
  bool g_up = HIGH, g_down = HIGH, g_left = HIGH, g_right = HIGH;
  bool button_A = HIGH, button_B = HIGH, button_C = HIGH, button_Start = HIGH;
  bool button_X = HIGH, button_Y = HIGH, button_Z = HIGH, button_Mode = HIGH;

  // --- STEP 1: CYCLE SELECT HIGH ($5V) ---
  digitalWrite(p2_up, HIGH);
  delayMicroseconds(20); 
  
  g_up    = digitalRead(p1_up);
  g_down  = digitalRead(p1_down);
  g_left  = digitalRead(p1_left);
  g_right = digitalRead(p1_right);
  button_B = digitalRead(p1_fire); 
  button_C = digitalRead(p2_left); 

  // --- STEP 2: CYCLE SELECT LOW ($0V) ---
  digitalWrite(p2_up, LOW);
  delayMicroseconds(20);
  
  button_A     = digitalRead(p1_fire); 
  button_Start = digitalRead(p2_left); 

  // --- STEP 3: HIGH/LOW PULSE SPRINT TO DISCOVER 6-BUTTON EXTENSIONS ---
  digitalWrite(p2_up, HIGH); delayMicroseconds(10);
  digitalWrite(p2_up, LOW);  delayMicroseconds(10);
  digitalWrite(p2_up, HIGH); delayMicroseconds(10);
  
  if (digitalRead(p1_up) == LOW && digitalRead(p1_down) == LOW) {
    button_Z    = digitalRead(p1_left);
    button_Y    = digitalRead(p1_right);
    button_X    = digitalRead(p1_fire);
    button_Mode = digitalRead(p2_left);
  }
  
  digitalWrite(p2_up, HIGH);

  // --- HID GAMEPAD MAPPING ---
  Joystick.setButton(0, !button_A);
  Joystick.setButton(1, !button_B);
  Joystick.setButton(2, !button_C);
  Joystick.setButton(3, !button_Start);
  Joystick.setButton(4, !button_X);
  Joystick.setButton(5, !button_Y);
  Joystick.setButton(6, !button_Z);
  Joystick.setButton(7, !button_Mode);
  
  int angle = -1;
  if (g_up == LOW) {
    if (g_right == LOW) angle = 45;
    else if (g_left == LOW) angle = 315;
    else angle = 0;
  } else if (g_down == LOW) {
    if (g_right == LOW) angle = 135;
    else if (g_left == LOW) angle = 225;
    else angle = 180;
  } else if (g_right == LOW) angle = 90;
  else if (g_left == LOW) angle = 270;
  
  Joystick.setHatSwitch(0, angle);

  // Read Analog Paddles 
  int rawA = analogRead(paddleA_pin);
  int rawB = analogRead(paddleB_pin);
  
  // Set axes directly; scale mappings are handled safely by the library now
  Joystick.setXAxis(map(rawA, 0, 1023, -127, 127));
  Joystick.setYAxis(map(rawB, 0, 1023, -127, 127));
  
  Joystick.sendState();

  // --- USB MIDI MAPPING ---
  if (g_up == LOW && lastUp == HIGH)       { sendMidi(0x90, 60, 127); lastUp = LOW; }
  else if (g_up == HIGH && lastUp == LOW)  { sendMidi(0x80, 60, 0);   lastUp = HIGH; }
  
  if (button_B == LOW && lastFire == HIGH)      { sendMidi(0x90, 64, 127); lastFire = LOW; }
  else if (button_B == HIGH && lastFire == LOW) { sendMidi(0x80, 64, 0);   lastFire = HIGH; }

  if (button_A == LOW && lastA == HIGH)       { sendMidi(0x90, 65, 127); lastA = LOW; }
  else if (button_A == HIGH && lastA == LOW)  { sendMidi(0x80, 65, 0);   lastA = HIGH; }

  if (button_C == LOW && lastC == HIGH)       { sendMidi(0x90, 67, 127); lastC = LOW; }
  else if (button_C == HIGH && lastC == LOW)  { sendMidi(0x80, 67, 0);   lastC = HIGH; }

  if (button_Start == LOW && lastStart == HIGH)       { sendMidi(0x90, 72, 127); lastStart = LOW; }
  else if (button_Start == HIGH && lastStart == LOW)  { sendMidi(0x80, 72, 0);   lastStart = HIGH; }

  int midiCC_A = map(rawA, 0, 1023, 0, 127);
  int midiCC_B = map(rawB, 0, 1023, 0, 127);
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
    // --- SYNTH MODE ENGINE ---
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
    // --- CONTROLLER MODE ENGINE ---
    if (lastModeState != isControllerMode) { 
      setControllerMode(); 
      lastModeState = isControllerMode; 
    }
    
    handleInputs();
    delay(10); 
  }
}
