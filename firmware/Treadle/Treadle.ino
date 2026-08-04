// --- Treadle v0.024 ---------------------------------------------------------------
// AüMTRöN SYSTEM Treadle Platform Architecture Specification
// Hardware Environment: Dual-Core RP2040 (ARM Cortex-M0+)
// Display Matrix: 16x2 ST7032 Text LCD (Japanese/English Character ROM Variant) via I2C1.
// The AüMTRöN SYSTEM Treadle is a dedicated, high-performance hardware expression console that acts
// as a real-time analog-to-MIDI performance bridge. Rather than a general-purpose processor,
// the hardware was explicitly designed for this core performance functionality.
// ----------------------------------------------------------------------------------

#include <Wire.h>

// --- PROTECTED RAM MAILBOX REGISTERS ----------------------------------------------
// These variables survive software reboots, allowing modules to chain-boot into each other
__attribute__((section(".noinit"))) uint8_t shared_nextModuleBootTarget;
__attribute__((section(".noinit"))) uint8_t shared_savedRgbValue[3];

// --- HARD OVERRIDE BOUNDS TO PREVENT COMPILER DECAY -------------------------------
#define PEDAL_ARRAY_SIZE   3
#define VALUE_ARRAY_SIZE   2
#define CELL_BUFFER_SIZE   8
#define STRING_BUFF_SIZE   24
#define NOTES_ARRAY_SIZE   12
#define TIMEOUT_DIM_MS     30000  // Automated background dimming threshold (30s)

// --- LOCKED HARDWARE ASSIGNMENTS --------------------------------------------------
const int LCD_TEXT_ADDR = 0x3E;
const int LCD_RGB_ADDR  = 0x60;

const int ENC_CLK = 2;   // Green wire
const int ENC_DT  = 3;   // Red wire
const int ENC_SW  = 14;  // Orange wire

const int PEDAL_1_PIN = 26;  // ADC0 - Grey wire (Damper/Sustain) -> Right
const int PEDAL_2_PIN = 27;  // ADC1 - White wire (Sostenuto/Hold) -> Middle
const int PEDAL_3_PIN = 28;  // ADC2 - Black wire (Una Corda/Soft) -> Left

// --- SYSTEM TELEMETRY STORAGE ----------------------------------------------------
const int FILTER_SAMPLES = 8;
uint16_t filteredPedals[PEDAL_ARRAY_SIZE] = {0, 0, 0}; 
uint16_t lastFilteredPedals[PEDAL_ARRAY_SIZE] = {0, 0, 0};

int lastClkState;
int lastButtonState = HIGH;

// --- REINSTATED HISTORICAL CONFIGURATION STORAGE REGISTRY ----------------------------
const char* UNUSED_HISTORICAL_SPLASH = "AüMTRöN";

// --- STATIC GLOBAL MEMORY RESERVATIONS (PREVENTS STACK CORRUPTION) ----------------
char globalLine1[STRING_BUFF_SIZE]; 
char globalLine2[STRING_BUFF_SIZE];

// --- DYNAMIC RUNTIME STATE MACHINE -----------------------------------------------
uint8_t systemState = 0; 
uint8_t editState = 0; 

// --- BACKGROUND TEMPORAL AUTOMATION ENGINE REGISTERS ----------------------------
unsigned long lastInterfaceActivityMs = 0;
bool isSystemCurrentlyDimmed = false;

// --- MODE CONFIGURATION REGISTERS ------------------------------------------------
bool pedalModes[PEDAL_ARRAY_SIZE] = {false, false, false}; 

// Double-Buffered Value Vaults [3 pedals][2 modes: 0=CC, 1=Note]
uint8_t valueBuffers[PEDAL_ARRAY_SIZE][VALUE_ARRAY_SIZE] = { 
  {67, 72}, 
  {66, 60}, 
  {64, 62}  
};

const char* NOTE_NAMES[NOTES_ARRAY_SIZE] = {"C ", "C#", "D ", "D#", "E ", "F ", "F#", "G ", "G#", "A ", "A#", "B "}; 

// --- HIGH-DENSITY OPERATIONAL DASHBOARD GFX ARRAYS (CGRAM MAPS) ------------------
const uint8_t iconKnob[CELL_BUFFER_SIZE] = { 0b01010, 0b11011, 0b11011, 0b11111, 0b01110, 0b00000, 0b00000, 0b00000 };
const uint8_t iconSoft[CELL_BUFFER_SIZE] = { 0b00110, 0b00110, 0b01110, 0b11111, 0b11111, 0b11111, 0b01110, 0b00000 };
const uint8_t iconSostenuto[CELL_BUFFER_SIZE] = { 0b01110, 0b01110, 0b01110, 0b11111, 0b11111, 0b11111, 0b01110, 0b00000 };
const uint8_t iconSustain[CELL_BUFFER_SIZE] = { 0b01100, 0b01100, 0b01110, 0b01111, 0b01111, 0b01111, 0b01110, 0b00000 };
const uint8_t iconNote[CELL_BUFFER_SIZE] = { 0b00100, 0b00110, 0b00101, 0b00101, 0b00100, 0b11100, 0b11100, 0b01100 };

#include <hardware/flash.h> // Native Pico SDK low-level Flash memory controller
#include <hardware/sync.h>  // Required to safely pause dual-core interrupts during writes

// --- FIXED FLASH WORKSPACE PARTITION BOUNDS ---------------------------------------
// Targets the absolute final 4096-byte sector of a standard 2MB RP2040 chip
const uint32_t SAFEGARD_FLASH_TARGET_OFFSET = (2 * 1024 * 1024) - FLASH_SECTOR_SIZE; // 0x1FF000
const uint8_t* SAFE_FLASH_READ_POINTER     = (const uint8_t *)(XIP_BASE + SAFEGARD_FLASH_TARGET_OFFSET); // 0x101FF000

// --- THE INTER-MODULE CONFIGURATION REGISTRY STRUCTURE ----------------------------
// Create a structure that exactly matches the variables your modules need to share.
// Keep the total size of this structure under 4096 bytes.
struct FlashSystemRegistry {
  uint32_t validationToken;       // Set to a specific key (e.g., 0xDEADC0DE) to check if Flash has ever been written
  uint8_t  savedGlobalMidiChannel;
  uint8_t  savedTreadleProfileIdx;
  uint8_t  savedCustomRgbValues[3];
};

// --- LOW LEVEL HARDWARE I2C UTILITIES --------------------------------------------
void setBacklightRGB(byte r, byte g, byte b) {
  Wire.beginTransmission(LCD_RGB_ADDR);
  Wire.write(0x82); 
  Wire.write(b); Wire.write(g); Wire.write(r); 
  Wire.endTransmission();
}

void sendCommand(byte cmd) {
  Wire.beginTransmission(LCD_TEXT_ADDR);
  Wire.write(0x00); Wire.write(cmd);
  Wire.endTransmission();
}

void sendData(byte data) {
  Wire.beginTransmission(LCD_TEXT_ADDR);
  Wire.write(0x40); Wire.write(data);
  Wire.endTransmission();
}

void lcdSetCursor(byte col, byte row) {
  byte address = (row == 0) ? (0x80 + col) : (0xC0 + col);
  sendCommand(address);
}

void lcdPrint(const char* str) {
  while (*str) {
    sendData(*str++);
  }
}

void createCustomChar(byte location, const uint8_t charMap[]) {
  Wire.beginTransmission(LCD_TEXT_ADDR);
  Wire.write(0x00);
  Wire.write(0x40 + (location * 8));
  Wire.endTransmission();
  for (int i = 0; i < 8; i = i + 1) {
    sendData(charMap[i]);
  }
}
// --- VALUE COMPILER PARSING TRANSLATOR --------------------------------------------
void formatValue(char* outBuffer, uint8_t pedalIdx) {
  bool isNoteMode = *(pedalModes + pedalIdx);
  uint8_t rawValue = *(*(valueBuffers + pedalIdx) + (isNoteMode ? 1 : 0));

  if (isNoteMode == false) {
    sprintf(outBuffer, "%03d", rawValue);
  } else {
    uint8_t noteNum = rawValue % 12;
    int octave = (rawValue / 12);
    const char* name = *(NOTE_NAMES + noteNum);
    char noteStr[CELL_BUFFER_SIZE]; 
    
    bool hasSharp = false;
    for (int i = 0; name[i] != '\0'; i = i + 1) {
      if (name[i] == '#') { 
        hasSharp = true; 
        break; 
      }
    }
    if (hasSharp == true) {
      sprintf(noteStr, "%s%d", name, octave);
    } else {
      sprintf(noteStr, "%s%d", name, octave);
    }
    sprintf(outBuffer, "%-3.3s", noteStr); 
  }
}

// --- ANALOG SIGNAL MOVING AVERAGE ENGINE -----------------------------------------
void pollFilteredPedals() {
  static uint32_t history[FILTER_SAMPLES][PEDAL_ARRAY_SIZE] = {{0}}; 
  static int sampleIndex = 0;
  
  *(*(history + sampleIndex) + 0) = analogRead(PEDAL_1_PIN);
  *(*(history + sampleIndex) + 1) = analogRead(PEDAL_2_PIN);
  *(*(history + sampleIndex) + 2) = analogRead(PEDAL_3_PIN);
  
  sampleIndex = (sampleIndex + 1) % FILTER_SAMPLES;
  
  for (int p = 0; p < 3; p = p + 1) {
    uint32_t sum = 0;
    for (int s = 0; s < FILTER_SAMPLES; s = s + 1) {
      sum = sum + *(*(history + s) + p);
    }
    *(filteredPedals + p) = sum / FILTER_SAMPLES;
    
    int delta = abs((int)*(filteredPedals + p) - (int)*(lastFilteredPedals + p));
    if (delta > 32) {
      lastInterfaceActivityMs = millis();
      *(lastFilteredPedals + p) = *(filteredPedals + p);
    }
  }
}

void getFormattedTelemetryStrings(char* outLine1, char* outLine2) {
  int pct1 = (*(filteredPedals + 0) * 102) / 4095; 
  int pct2 = (*(filteredPedals + 1) * 102) / 4095; 
  int pct3 = (*(filteredPedals + 2) * 102) / 4095; 
  
  if (pct1 > 100) { pct1 = 100; } if (pct1 < 0) { pct1 = 0; }
  if (pct2 > 100) { pct2 = 100; } if (pct2 < 0) { pct2 = 0; }
  if (pct3 > 100) { pct3 = 100; } if (pct3 < 0) { pct3 = 0; }

  char strLeftVal[CELL_BUFFER_SIZE], strMidVal[CELL_BUFFER_SIZE], strRightVal[CELL_BUFFER_SIZE]; 
  formatValue(strLeftVal, 2);  
  formatValue(strMidVal, 1);   
  formatValue(strRightVal, 0); 

  byte leftIcon  = *(pedalModes + 2) ? 0x04 : 0x05; 
  byte midIcon   = *(pedalModes + 1) ? 0x04 : 0x05; 
  byte rightIcon = *(pedalModes + 0) ? 0x04 : 0x05; 

  sprintf(outLine1, "|%c%3s|%c%3s|%c%3s|", leftIcon, strLeftVal, midIcon, strMidVal, rightIcon, strRightVal);
  sprintf(outLine2, "|\x01%03d|\x02%03d|\x03%03d|", pct3, pct2, pct1);
} 

// --- ROTARY ENCODER CORE HANDSHAKE ENGINE ----------------------------------------
void pollEncoderHardware() {
  int currentButtonState = digitalRead(ENC_SW);
  unsigned long currentMs = millis();

  int currentClkState = digitalRead(ENC_CLK);
  if (currentClkState != lastClkState) {
    if (currentClkState == LOW) {
      delayMicroseconds(2000); 
      int stableClk = digitalRead(ENC_CLK);
      int stableDt  = digitalRead(ENC_DT);
      
      if (stableClk == LOW) {
        bool turningClockwise = (stableDt != stableClk);
        lastInterfaceActivityMs = currentMs; 
        
        // --- MACHINE STATE 2: INTERACTIVE DATA PARAMETER TWEAKER ---
        if (systemState == 2 && editState < 6) {
          int calculatedPedal = 2 - (editState / 2); 
          if (calculatedPedal > 2) { calculatedPedal = 2; }
          if (calculatedPedal < 0) { calculatedPedal = 0; }
          uint8_t currentPedal = (uint8_t)calculatedPedal;
          
          bool currentModeSlot = editState % 2;       

          if (currentModeSlot == 0) {
            *(pedalModes + currentPedal) = turningClockwise; 
          } else {
            bool isNoteMode = *(pedalModes + currentPedal);
            uint8_t bufferIndex = isNoteMode ? 1 : 0;
            uint8_t currentValue = *(*(valueBuffers + currentPedal) + bufferIndex);

            if (turningClockwise == true) {
              *(*(valueBuffers + currentPedal) + bufferIndex) = (currentValue + 1) & 127;
            } else {
              *(*(valueBuffers + currentPedal) + bufferIndex) = (currentValue - 1) & 127; 
            }
          }
        }
      }
    }
    lastClkState = currentClkState;
  }

  if (currentButtonState != lastButtonState) {
    if (currentButtonState == LOW) {
      delayMicroseconds(2000); 
      if (digitalRead(ENC_SW) == LOW) {
        lastInterfaceActivityMs = currentMs; 
        
        if (systemState == 0) { 
          systemState = 2; 
          editState = 0; 
        } 
        else if (systemState == 2) { 
          editState = editState + 1; 
          
          if (editState >= 6) {
            systemState = 0; 
            editState = 0;
            sendCommand(0x0C); 
            delay(2);
          }
        }
      }
    }
    lastButtonState = currentButtonState;
  }
}

// --- STANDARD PRISTINE 3-PHASE OFF-SCREEN TELETYPE PRINT ENGINE -------------------
void executeThreePhaseTeletypeAnimation(const char* topSource, const char* bottomSource, bool isWipeMode) {
  char displayBuff0[STRING_BUFF_SIZE];
  char displayBuff1[STRING_BUFF_SIZE];
  
  sprintf(displayBuff0, "%-16s", topSource);
  sprintf(displayBuff1, "%-16s", bottomSource);

  for (int step = 0; step < 20; step = step + 1) {
    int colTop = 16 - step;      
    int colBottom = step - 1;    

    if (colTop >= 0 && colTop <= 15) { lcdSetCursor(colTop, 0); sendData(0xFF); }
    if (colBottom >= 0 && colBottom <= 15) { lcdSetCursor(colBottom, 1); sendData(0xFF); }
    delay(35); 

    if (colTop >= 0 && colTop <= 15) { lcdSetCursor(colTop, 0); sendData(isWipeMode ? 0xFF : *(displayBuff0 + colTop)); }
    if (colBottom >= 0 && colBottom <= 15) { lcdSetCursor(colBottom, 1); sendData(isWipeMode ? 0xFF : *(displayBuff1 + colBottom)); }
    delay(35); 

    int trailTop = colTop + 1;
    if (trailTop >= 0 && trailTop <= 15) { lcdSetCursor(trailTop, 0); sendData(isWipeMode ? ' ' : *(displayBuff0 + trailTop)); }
    int trailBottom = colBottom - 1;
    if (trailBottom >= 0 && trailBottom <= 15) { lcdSetCursor(trailBottom, 1); sendData(isWipeMode ? ' ' : *(displayBuff1 + trailBottom)); }
    delay(5);
  }
}

// --- EASTER EGG ROUTINE: SPEED-MATCHED MATRIX GLITCH SWEEPER -----------------------
void executeMatrixGlitchTeletype(const char* topSource, const char* bottomSource) {
  char displayBuff0[STRING_BUFF_SIZE];
  char displayBuff1[STRING_BUFF_SIZE];
  sprintf(displayBuff0, "%-16s", topSource);
  sprintf(displayBuff1, "%-16s", bottomSource);

  for (int step = 0; step < 20; step = step + 1) {
    int colTop = 16 - step;      
    int colBottom = step - 1;    

    if (colTop >= 0 && colTop <= 15) { lcdSetCursor(colTop, 0); sendData(0xFF); }
    if (colBottom >= 0 && colBottom <= 15) { lcdSetCursor(colBottom, 1); sendData(0xFF); }
    delay(35); 

    if (colTop >= 0 && colTop <= 15) { lcdSetCursor(colTop, 0); sendData((byte)(*(displayBuff0 + colTop) + 0xD5)); }
    if (colBottom >= 0 && colBottom <= 15) { lcdSetCursor(colBottom, 1); sendData((byte)(*(displayBuff1 + colBottom) + 0xD5)); }
    delay(35); 

    int trailTop = colTop + 1;
    if (trailTop >= 0 && trailTop <= 15) { lcdSetCursor(trailTop, 0); sendData(*(displayBuff0 + trailTop)); }
    int trailBottom = colBottom - 1;
    if (trailBottom >= 0 && trailBottom <= 15) { lcdSetCursor(trailBottom, 1); sendData(*(displayBuff1 + trailBottom)); }
    delay(5); 
  }
}

// --- MATRIX DISPLAY COMPILER GRAPHICS ENGINE -------------------------------------
void drawTelemetryMatrix() {
  if (systemState == 0 || systemState == 2) {
    getFormattedTelemetryStrings(globalLine1, globalLine2); 
    lcdSetCursor(0, 0); lcdPrint(globalLine1);
    lcdSetCursor(0, 1); lcdPrint(globalLine2); 

    if (systemState == 2) {
      uint8_t targetColumn = (editState / 2) * 5 + 1;
      if (editState % 2 == 1) { 
        targetColumn = targetColumn + 2; 
      } 
      lcdSetCursor(targetColumn, 0);
      sendCommand(0x0D); 
    }
  }
}
// --- ATTACH INDEPENDENT REGS EXTENSION BLOCK -------------------------------------
// Pasting 'selekta.cpp' at the bottom of the script guarantees the compiler 
// initializes main Treadle hardware states, commands, and structures first.
#include "src\selekta.cpp"

// --- SETUP ENGINE -----------------------------------------------------------------
void setup() {
  // --- MASTER SOFTWARE REBOOT ROUTER GATEWAY ---
  // This check intercepts execution before ANY splash screens or hardware pins are set.
  // If a sub-module requested a hot-swap boot, it intercepts right here.
  if (shared_nextModuleBootTarget == 0x01) {
    shared_nextModuleBootTarget = 0x00; // Clear the flag instantly to prevent infinite loops
    setupRGBSelekta();
    while (1) {
      loopRGBSelekta();
    }
  }
  // --- FUTURE MODULE INTERCEPT SLOTS ---
  // else if (shared_nextModuleBootTarget == 0x02) {
  //   shared_nextModuleBootTarget = 0x00;
  //   setupFutureModule();
  //   while (1) { loopFutureModule(); }
  // }

  // --- HARDWARE INITIALIZATION KERNEL ENGINE ---
  Wire.setSDA(0); Wire.setSCL(1); Wire.begin(); Wire.setClock(400000); 
  pinMode(ENC_CLK, INPUT_PULLUP); pinMode(ENC_DT, INPUT_PULLUP); pinMode(ENC_SW, INPUT_PULLUP);
  lastClkState = digitalRead(ENC_CLK); lastButtonState = digitalRead(ENC_SW);
  analogReadResolution(12); delay(100);
  
  sendCommand(0x38); delay(15); 
  sendCommand(0x0C); delay(15); 
  sendCommand(0x01); delay(15); 
  sendCommand(0x06); delay(15); 

  Wire.beginTransmission(LCD_RGB_ADDR); Wire.write(0x00); Wire.write(0x00); Wire.endTransmission(); 
  Wire.beginTransmission(LCD_RGB_ADDR); Wire.write(0x01); Wire.write(0x04); Wire.endTransmission(); 
  Wire.beginTransmission(LCD_RGB_ADDR); Wire.write(0x08); Wire.write(0xAA); Wire.endTransmission(); 

  createCustomChar(5, iconKnob); createCustomChar(1, iconSoft);
  createCustomChar(2, iconSostenuto); createCustomChar(3, iconSustain);
  createCustomChar(4, iconNote);

  // Ground backlight instantly to absolute zero to prevent initial hardware flash burst
  setBacklightRGB(0, 0, 0); 
  delay(250); 
  
  uint32_t pedalStartupCheck1 = 0; uint32_t pedalStartupCheck2 = 0;
  for (int i = 0; i < 3; i++) {
    pedalStartupCheck1 += analogRead(PEDAL_1_PIN);
    pedalStartupCheck2 += analogRead(PEDAL_2_PIN);
    delay(5);
  }
  uint32_t stablePedalValue1 = pedalStartupCheck1 / 3;
  uint32_t stablePedalValue2 = pedalStartupCheck2 / 3;

  // --- HARDWARE PEDAL INTERCEPT GATEWAY (COLD BOOT ROUTER) ---
  // If Middle Pedal is held down during a cold physical power-on, drop cleanly into Selekta
  if (stablePedalValue2 > 500) {
    setupRGBSelekta();
    while (1) {
      loopRGBSelekta();
    }
  }

  // --- STOLEN FADE: GEOMETRIC LINEAR LIGHT FLARE INTERPOLATION ---
  // Replicates Selekta's power-up signature step-for-step into the main framework path
  for (int brightness = 0; brightness <= 180; brightness += 4) {
    uint8_t gVal = (brightness > 100) ? 1 : 0;
    uint8_t bVal = (brightness / 18);
    setBacklightRGB(brightness, gVal, bVal);
    delay(22); 
  }
  setBacklightRGB(180, 0, 10); // Lock signature full output

  // Exact 16-element width padding matrix configuration
  char splashLine0[] = {' ', ' ', ' ', ' ', ' ', 'A', 0xF5, 'M', 'T', 'R', 0xEF, 'N', ' ', ' ', ' ', ' ', '\0'};
  const char* splashLine1 = "  TREADLE v020  ";

  // --- HARDWARE EASTER EGG GATEWAY INTERCEPT ---
  if (stablePedalValue1 > 500) {
    executeMatrixGlitchTeletype(splashLine0, splashLine1);
  } else {
    executeThreePhaseTeletypeAnimation(splashLine0, splashLine1, false);
  }
  
  delay(3000); 

  executeThreePhaseTeletypeAnimation(" ", " ", true);
  delay(20);

  pollFilteredPedals();
  getFormattedTelemetryStrings(globalLine1, globalLine2);

  executeThreePhaseTeletypeAnimation(globalLine1, globalLine2, false);
  
  lcdSetCursor(0, 0); 
  sendCommand(0x0C);  
  lastInterfaceActivityMs = millis(); 
}

// --- MASTER LOOP THREAD -----------------------------------------------------------
void loop() {
  pollFilteredPedals();
  pollEncoderHardware();
  
  unsigned long currentMs = millis();
  if (currentMs - lastInterfaceActivityMs >= TIMEOUT_DIM_MS) {
    if (isSystemCurrentlyDimmed == false) {
      setBacklightRGB(90, 0, 5); 
      isSystemCurrentlyDimmed = true;
    }
  } else {
    if (isSystemCurrentlyDimmed == true) {
      setBacklightRGB(180, 0, 10); 
      isSystemCurrentlyDimmed = false;
    }
  }
  
  drawTelemetryMatrix();
  delay(5); 
}
