/* RGB SELEKTA
   -----------
   The complete RGB SELEKTA! BO! 
   Choose and test LCD screen backlight colours for the Waveshare LCD1602 RGB module!
   Complete with "Pro level" FACTORY PRESETS! (OOF!)
   Choose and set colours AND LOOK AT THEM! (FREELY! EYES NOT INCLUDED!)
   You can store/delete up 255 volatile presets! (NO WAY!) TODO: Check preset doesnt exist (duplicates)
   Has uptime clock screensaver, and help scroller! (WELL, THATS INSANE!)
   DEFINABLE screensaver timeout range: off, 5-255 seconds! (TOO KIND!)
   Classy boot up/transitions and hyper-precise UI design/kinetics! (ART!)
   CUSTOM LCD chars! (WAUW!)
   Control via single rotary encoder (SIMPLEZ!)
   TODO: PINOUT TO FOLLOW *HERE*
   TODO: Maybe write the presets to flash... protected for multi use pico...
   TODO: Scroll effect flags.... stops on first two lines in boot text.... then continues
   
   INSTRUX:
   Apply power to boot. In the main screen you can short press (SP) the encoder to jump between
   RGB edit fields and presets. Cursor shows selected field. Turn encoder to alter value.
   Long pressing (LP) on an RGB field will store a new preset. LP on the preset field allows deletion.
   By default, after 30 seconds the screen saver will start. In screen saver mode, you may turn or click
   the encoder (SP) to exit to the main screen. A LP will enter the timeout config screen. Turn encoder
   and click to return to the screensaver. 
*/

// v01D-SS_CONFIG_FINAL

// --- INCLUDES ---------------------------------------------------------------------
#include <Wire.h>

// ----------------------------

// --- DEFINES & CONFIGURATION CONSTANTS --------------------------------------------
const char VERSION_STAMP[] = "1DF"; 



// --- GLOBAL VARIABLES & ENGINE STATES ---------------------------------------------

uint32_t buttonPressTime = 0; 
bool longPressExecuted = false;

uint8_t activeFieldIndex = 1;       
bool deleteConfirmSelectionY = false; 

uint8_t rgbValues[3] = {128, 0, 0}; 

struct ColorPreset {
  char name[13]; 
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

ColorPreset presets[16] = {
  {"A\xF5MTR\xEFN RED ", 180, 0, 10}, 
  {"RED         ", 255, 0, 0},
  {"GREEN       ", 0, 255, 0},
  {"BLUE        ", 0, 0, 255},
  {"CYAN        ", 0, 255, 255},
  {"YELLOW      ", 255, 255, 0},
  {"TEAL GREEN  ", 0, 128, 128},
  {"DUCK YELLOW ", 240, 230, 140}
};

uint8_t totalActivePresets = 8;  
int8_t currentPresetIndex = 0;   
uint8_t customSlotCounter = 1;   

uint32_t lastActivityTime = 0;
bool screensaverActive = false;
uint32_t lastScrollTick = 0;
uint16_t scrollOffset = 0;
uint32_t screensaverTimeoutMs = 30000; 

const char scrollerText[] = "                      *A\xF5MTR\xEFN SYSTEM*  RGB SELEKTA v1                   [SP/LP] = Short/Long press   [TURN] to alter selected value   [SP] to cycle fields   [LP] on RGB value stores new preset, or on preset to delete   [TURN/SP] to exit screensaver   [LP] on clock to adjust screensaver timeout                  (c)2026 A\xF5MTR\xEFN SYSTEM                  ";
const uint16_t scrollerLength = sizeof(scrollerText) - 1;

const uint8_t customArrowRight[8] = {
  0b10000,  
  0b11000,  
  0b11100,  
  0b11110,  
  0b11100,  
  0b11000,  
  0b10000,  
  0b00000
};

const uint8_t customArrowLeft[8] = {
  0b00001,  
  0b00011,  
  0b00111,  
  0b01111,  
  0b00111,  
  0b00011,  
  0b00001,  
  0b00000
};

// --- ANIMATION & REFRESH ENGINES --------------------------------------------------
void animateTeletypeClear(int speedMs) {
  for (int col = 0; col < 16; col++) {
    lcdSetCursor(col, 0); sendData(' ');
    lcdSetCursor(col, 1); sendData(' ');
    delay(speedMs);
  }
}

void animateDualLinePrint(const char* topRowText, const char* bottomRowText, int speedMs) {
  const byte cursorBlock = 0xFF; 
  
  for (int step = 0; step < 16; step++) {
    sendCommand(0x01); delay(2); 
    
    for (int i = 15; i > 15 - step; i--) {
      lcdSetCursor(i, 0);
      sendData(topRowText[i]);
    }
    lcdSetCursor(15 - step, 0);
    sendData(cursorBlock);

    for (int i = 0; i < step; i++) {
      lcdSetCursor(i, 1);
      sendData(bottomRowText[i]);
    }
    lcdSetCursor(step, 1);
    sendData(cursorBlock);
    
    delay(speedMs);
  }
  
  sendCommand(0x01); delay(2);
  lcdSetCursor(0, 0);
  for (int i = 0; i < 16; i++) sendData(topRowText[i]);
  lcdSetCursor(0, 1);
  for (int i = 0; i < 16; i++) sendData(bottomRowText[i]);
}

void executeGraphicalPowerOnSequence() {
  setBacklightRGB(0, 0, 0);
  sendCommand(0x01); 
  delay(500); 
  
  for (int brightness = 0; brightness <= 180; brightness += 4) {
    uint8_t gVal = (brightness > 100) ? 1 : 0;
    uint8_t bVal = (brightness / 18);
    setBacklightRGB(brightness, gVal, bVal);
    delay(22); 
  }
  setBacklightRGB(180, 0, 10); 

  const char splashTop[]    = "*A\xF5MTR\xEFN System*"; 
  const char splashBottom[] = " RGB SELEKTA v1 "; 
  animateDualLinePrint(splashTop, splashBottom, 60); 
  delay(3000); 

  sendCommand(0x01); delay(2); 
  delay(500);        
}

void loadActivePresetValues() {
  rgbValues[0] = presets[currentPresetIndex].r;
  rgbValues[1] = presets[currentPresetIndex].g;
  rgbValues[2] = presets[currentPresetIndex].b;
  setBacklightRGB(rgbValues[0], rgbValues[1], rgbValues[2]);
}

void updateDisplayMatrix() {
  char line1[24]; 
  char line2[24];
  bool useHardwareBlink = false;
  uint32_t rightNow = millis();

  static uint32_t lastBlinkToggleTime = 0;
  static bool blinkPhase = true;

  if (rightNow - lastBlinkToggleTime >= 400) {
    blinkPhase = !blinkPhase;
    lastBlinkToggleTime = rightNow;
  }

  sendCommand(0x0C); 

  if (activeFieldIndex == 5) {
    char leftC = blinkPhase ? 0x04 : ' ';
    char rightC = blinkPhase ? 0x05 : ' ';
    uint32_t activeSecsValue = screensaverTimeoutMs / 1000;
    if (activeSecsValue == 0) {
      sprintf(line1, " %cscrnsaver OFF%c ", leftC, rightC);
    } else {
      sprintf(line1, " %cscrnsaver %02ds%c ", leftC, activeSecsValue, rightC);
    }
    sprintf(line2, "  [CLICK ACCPT] ");
    lcdSetCursor(0, 0); lcdPrint(line1);
    lcdSetCursor(0, 1); lcdPrint(line2);
    return;
  }

  if (screensaverActive) {
    uint32_t totalSeconds = rightNow / 1000;
    uint32_t hours = totalSeconds / 3600;
    uint32_t minutes = (totalSeconds % 3600) / 60;
    uint32_t seconds = totalSeconds % 60;

    char leftS = blinkPhase ? 0x04 : ' ';
    char rightS = blinkPhase ? 0x05 : ' ';

    sprintf(line1, "  %c%02dh%02dm%02ds%c  ", leftS, hours, minutes, seconds, rightS);
    lcdSetCursor(0, 0); lcdPrint(line1);
    
    lcdSetCursor(0, 1);
    for (int i = 0; i < 16; i++) {
      uint16_t dynamicIdx = (scrollOffset + i) % scrollerLength;
      sendData(scrollerText[dynamicIdx]);
    }
    return;
  }

  if (activeFieldIndex == 0) {
    char leftP = blinkPhase ? 0x04 : ' ';
    char rightP = blinkPhase ? 0x05 : ' ';
    sprintf(line1, "%c%02X%c%s", leftP, currentPresetIndex, rightP, presets[currentPresetIndex].name);
  } else {
    sprintf(line1, " %02X %s", currentPresetIndex, presets[currentPresetIndex].name);
  }

  sprintf(line2, "| %02X | %02X | %02X |", rgbValues[0], rgbValues[1], rgbValues[2]);

  char leftGlyph = blinkPhase ? 0x04 : ' ';
  char rightGlyph = blinkPhase ? 0x05 : ' ';

  if (activeFieldIndex == 1) { 
    line2[1] = leftGlyph; 
    line2[4] = rightGlyph; 
  }
  else if (activeFieldIndex == 2) { 
    line2[6] = leftGlyph; 
    line2[9] = rightGlyph; 
  }
  else if (activeFieldIndex == 3) { 
    line2[11] = leftGlyph; 
    line2[14] = rightGlyph; 
  }

  if (activeFieldIndex == 4) {
    sprintf(line1, "Delete %02X (Y/N)?", currentPresetIndex);
    useHardwareBlink = true;
  }

  lcdSetCursor(0, 0); lcdPrint(line1);
  lcdSetCursor(0, 1); lcdPrint(line2);

  if (useHardwareBlink) {
    if (activeFieldIndex == 4) {
      if (deleteConfirmSelectionY) {
        lcdSetCursor(11, 0); 
      } else {
        lcdSetCursor(13, 0); 
      }
    }
    sendCommand(0x0D); 
  } else {
    sendCommand(0x0C); 
  }
}

// ----------------------------

// --- WORKSPACE STORAGE INTERFACE --------------------------------------------------
void handleLongPressSave() {
  if (totalActivePresets >= 16) return; 

  uint8_t saveIndex = totalActivePresets;
  sprintf(presets[saveIndex].name, "CUSTOM %02d   ", customSlotCounter);
  presets[saveIndex].r = rgbValues[0];
  presets[saveIndex].g = rgbValues[1];
  presets[saveIndex].b = rgbValues[2];

  totalActivePresets++;
  customSlotCounter++;
  currentPresetIndex = saveIndex; 

  setBacklightRGB(0, 0, 0); delay(100);
  setBacklightRGB(rgbValues[0], rgbValues[1], rgbValues[2]);
  updateDisplayMatrix();
}

void executePresetDeletion() {
  if (totalActivePresets <= 1) {
    activeFieldIndex = 0;
    updateDisplayMatrix();
    return;
  }

  for (uint8_t i = currentPresetIndex; i < totalActivePresets - 1; i++) {
    presets[i] = presets[i + 1];
  }
  
  totalActivePresets--;
  
  if (currentPresetIndex >= totalActivePresets) {
    currentPresetIndex = totalActivePresets - 1;
  }
  
  setBacklightRGB(60, 0, 0); delay(150); 
  
  loadActivePresetValues();
  activeFieldIndex = 0; 
  updateDisplayMatrix();
}

void enterScreensaverMode() {
  screensaverActive = true;
  scrollOffset = 0; 
  
  animateTeletypeClear(20);
  
  for (int step = 3; step >= 1; step--) {
    uint8_t curR = (rgbValues[0] * step) / 4;
    uint8_t curG = (rgbValues[1] * step) / 4;
    uint8_t curB = (rgbValues[2] * step) / 4;
    setBacklightRGB(curR, curG, curB);
    delay(25);
  }
  setBacklightRGB(rgbValues[0] >> 2, rgbValues[1] >> 2, rgbValues[2] >> 2);
  updateDisplayMatrix();
}

void checkActivityWakeup() {
  if (screensaverActive) {
    screensaverActive = false;
    
    animateTeletypeClear(15);
    
    for (int step = 1; step <= 4; step++) {
      uint8_t curR = (rgbValues[0] * step) / 4;
      uint8_t curG = (rgbValues[1] * step) / 4;
      uint8_t curB = (rgbValues[2] * step) / 4;
      setBacklightRGB(curR, curG, curB);
      delay(25);
    }
    setBacklightRGB(rgbValues[0], rgbValues[1], rgbValues[2]);
    updateDisplayMatrix();
  }
  lastActivityTime = millis();
}
// --- SETUP FUNCTION ---------------------------------------------------------------
void setupRGBSelekta() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); 
  
  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  lastClkState = digitalRead(ENC_CLK);
  pinMode(ENC_SW, INPUT_PULLUP);

  Wire.setSDA(0); Wire.setSCL(1);
  Wire.begin(); Wire.setClock(400000); 
  delay(50); 
  
  sendCommand(0x38); delay(2); 
  sendCommand(0x0C); delay(2); 
  sendCommand(0x01); delay(2); 
  sendCommand(0x06); delay(2); 

  Wire.beginTransmission(LCD_RGB_ADDR); Wire.write(0x00); Wire.write(0x00); Wire.endTransmission(); 
  Wire.beginTransmission(LCD_RGB_ADDR); Wire.write(0x01); Wire.write(0x04); Wire.endTransmission(); 
  Wire.beginTransmission(LCD_RGB_ADDR); Wire.write(0x08); Wire.write(0xAA); Wire.endTransmission(); 

  createCustomChar(4, customArrowRight);
  createCustomChar(5, customArrowLeft);

  executeGraphicalPowerOnSequence();
  
  loadActivePresetValues();
  lastActivityTime = millis();
  updateDisplayMatrix();
}

// ----------------------------

// --- LOOP FUNCTION ----------------------------------------------------------------
void loopRGBSelekta() {
  uint32_t currentTicks = millis();

  if (!screensaverActive && activeFieldIndex < 5 && screensaverTimeoutMs > 0 && (currentTicks - lastActivityTime >= screensaverTimeoutMs)) {
    enterScreensaverMode();
  }

  if (screensaverActive && (currentTicks - lastScrollTick >= 200)) {
    scrollOffset = (scrollOffset + 1) % scrollerLength;
    lastScrollTick = currentTicks;
    updateDisplayMatrix();
  }

  static uint32_t lastFlashRefreshTime = 0;
  if (currentTicks - lastFlashRefreshTime >= 50) {
    updateDisplayMatrix();
    lastFlashRefreshTime = currentTicks;
  }

  int currentClkState = digitalRead(ENC_CLK);
  if (currentClkState != lastClkState) {
    if (currentClkState == LOW) {
      delayMicroseconds(2000); 
      int stableClk = digitalRead(ENC_CLK);
      int stableDt  = digitalRead(ENC_DT);
      
      if (stableClk == LOW) {
        bool turningClockwise = (stableDt != stableClk);

        if (screensaverActive) {
          checkActivityWakeup(); 
        } 
        else if (activeFieldIndex == 5) {
          uint32_t currentSecs = screensaverTimeoutMs / 1000;
          if (turningClockwise) {
            if (currentSecs == 0) currentSecs = 5;
            else if (currentSecs >= 255) currentSecs = 0;
            else currentSecs++;
          } else {
            if (currentSecs == 0) currentSecs = 255;
            else if (currentSecs <= 5) currentSecs = 0;
            else currentSecs--;
          }
          screensaverTimeoutMs = currentSecs * 1000;
          updateDisplayMatrix();
        }
        else {
          lastActivityTime = currentTicks; 
          if (activeFieldIndex < 4) {
            if (activeFieldIndex == 0) {
              if (turningClockwise) {
                currentPresetIndex = (currentPresetIndex + 1) % totalActivePresets;
              } else {
                currentPresetIndex = (currentPresetIndex - 1 + totalActivePresets) % totalActivePresets;
              }
              loadActivePresetValues(); 
            } 
            else {
              uint8_t targetColorChannel = activeFieldIndex - 1;
              if (turningClockwise) {
                if (rgbValues[targetColorChannel] < 255) rgbValues[targetColorChannel]++;
              } else {
                if (rgbValues[targetColorChannel] > 0) rgbValues[targetColorChannel]--;
              }
              setBacklightRGB(rgbValues[0], rgbValues[1], rgbValues[2]);
            }
            updateDisplayMatrix();
          } 
          else {
            deleteConfirmSelectionY = !deleteConfirmSelectionY;
            updateDisplayMatrix();
          }
        }
      }
    }
    lastClkState = currentClkState;
  }

  int currentButtonState = digitalRead(ENC_SW);
  if (currentButtonState == LOW) {
    if (lastButtonState == HIGH) {
      buttonPressTime = currentTicks; 
      longPressExecuted = false;
    } 
    else if (!longPressExecuted) {
      uint32_t holdDuration = currentTicks - buttonPressTime;
      
      // FIX OUT: Active long press intercept bypass structural check inside screensaver
      if (screensaverActive && holdDuration >= 1600) {
        screensaverActive = false;
        activeFieldIndex = 5; 
        animateTeletypeClear(20);
        updateDisplayMatrix();
        longPressExecuted = true;
      }
      else if (!screensaverActive && holdDuration >= 1600 && activeFieldIndex < 4) {
        if (activeFieldIndex > 0) {
          handleLongPressSave();
          longPressExecuted = true;
        } 
        else if (activeFieldIndex == 0) {
          deleteConfirmSelectionY = false; 
          activeFieldIndex = 4;           
          updateDisplayMatrix();
          longPressExecuted = true;
        }
      }
    }
  } 
  else {
    if (lastButtonState == LOW && !longPressExecuted && !screensaverActive) {
      uint32_t holdDuration = currentTicks - buttonPressTime;
      if (holdDuration >= 20) {
        lastActivityTime = currentTicks; 
        
        if (activeFieldIndex == 5) {
          screensaverActive = true;
          activeFieldIndex = 1; 
          animateTeletypeClear(20);
          updateDisplayMatrix();
        }
        else if (!screensaverActive) {
          if (activeFieldIndex < 4) {
            activeFieldIndex = (activeFieldIndex + 1) % 4;
          } 
          else {
            if (deleteConfirmSelectionY) {
              executePresetDeletion(); 
            } else {
              activeFieldIndex = 0;
            }
          }
          updateDisplayMatrix();
        }
      }
    }
  }
  lastButtonState = currentButtonState;
}

// v01D-SS_CONFIG_FINAL
