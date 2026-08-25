#ifndef BUTTON_EEPROM_H
#define BUTTON_EEPROM_H

#include "config.h"

// ================== FUNCTION DECLARATIONS ==================
bool readButton(int pin);
void handleSettingMode();
void displaySetting(int index);
void displayWithBlink(int index, int field, bool state);
void handleValueChange(int index, int field, bool increment);
int getMaxField(int index);
void saveSetting(int index);
String getSettingString(int index);
void loadSettingsFromEEPROM();
void resetBackgroundProcesses();

// ================== FUNCTION IMPLEMENTATIONS ==================

// Read button with debounce
bool readButton(int pin) {
  if (digitalRead(pin) == LOW) {
    delay(50);
    if (digitalRead(pin) == LOW) return true;
  }
  return false;
}

// Reset background processes when switching modes
void resetBackgroundProcesses() {
  lcdForce = true;
  needsRedraw = true;
  isEditing = false;
  editingField = 0;
  blinkState = false;
  Serial.println(F("Background processes reset"));
}

// Load settings from EEPROM
void loadSettingsFromEEPROM() {
  for (int i = 0; i < TOTAL_SETTINGS; i++) {
    for (int j = 0; j < 6; j++) {
      int addr = EEPROM_OFFSET + (i * 6 + j) * 2;
      settings[i].data[j] = EEPROM.read(addr) | (EEPROM.read(addr + 1) << 8);
      
      // Handle invalid EEPROM values
      if (settings[i].data[j] == -1 || settings[i].data[j] > 1000) {
        settings[i].data[j] = 0;
      }
    }
  }
  
  // Default durasi pompa minimal 1 menit
  if (settings[IDX_POMPA_LAUT].data[2] == 0) settings[IDX_POMPA_LAUT].data[2] = 1;
  if (settings[IDX_POMPA_BILAS].data[2] == 0) settings[IDX_POMPA_BILAS].data[2] = 1;
  
  Serial.println(F("Settings loaded from EEPROM"));
}

// Save setting to EEPROM
void saveSetting(int index) {
  for (int j = 0; j < 6; j++) {
    int addr = EEPROM_OFFSET + (index * 6 + j) * 2;
    int value = settings[index].data[j];
    EEPROM.update(addr, value & 0xFF);
    EEPROM.update(addr + 1, (value >> 8) & 0xFF);
  }
  Serial.println("Setting " + String(index) + " saved to EEPROM");
}

// Get setting name string
String getSettingString(int index) {
  if (index == IDX_POMPA_LAUT) return "Pompa Laut";
  else if (index == IDX_POMPA_BILAS) return "Pompa Bilas";
  else if (index == IDX_OFFSET_ULTRASONIC) return "Offset Ultrasonic";
  return "";
}

// Get max field count for setting
int getMaxField(int index) {
  if (index == IDX_POMPA_LAUT || index == IDX_POMPA_BILAS) {
    return 6; // durasi menit, durasi detik, repeat hari, repeat jam, waktu jam/menit, waktu menit/count
  } else if (index == IDX_OFFSET_ULTRASONIC) {
    return 2; // meter dan cm (2 field)
  }
  return 1;
}

// Handle value change in setting mode
void handleValueChange(int index, int field, bool increment) {
  int delta = increment ? 1 : -1;

  if (index == IDX_POMPA_LAUT || index == IDX_POMPA_BILAS) {
    int repJam = settings[index].data[3];

    switch (field) {
      case 0: // Durasi menit
        settings[index].data[0] += delta;
        if (settings[index].data[0] > 59) settings[index].data[0] = 0;
        if (settings[index].data[0] < 0) settings[index].data[0] = 59;
        break;
      case 1: // Durasi detik
        settings[index].data[1] += delta;
        if (settings[index].data[1] > 59) settings[index].data[1] = 0;
        if (settings[index].data[1] < 0) settings[index].data[1] = 59;
        break;
      case 2: // Repeat hari (setiap X hari)
        settings[index].data[2] += delta;
        if (settings[index].data[2] > 30) settings[index].data[2] = 1;
        if (settings[index].data[2] < 1) settings[index].data[2] = 30;
        break;
      case 3: // Repeat jam (setiap X jam, 0-24)
        settings[index].data[3] += delta;
        if (settings[index].data[3] > 24) settings[index].data[3] = 0;
        if (settings[index].data[3] < 0) settings[index].data[3] = 24;
        break;
      case 4: // Waktu jam ATAU waktu menit (tergantung repJam)
        settings[index].data[4] += delta;
        if (repJam > 0) {
          // Mode per jam: ini waktu menit (0-59)
          if (settings[index].data[4] > 59) settings[index].data[4] = 0;
          if (settings[index].data[4] < 0) settings[index].data[4] = 59;
        } else {
          // Mode per hari: ini waktu jam (0-23)
          if (settings[index].data[4] > 23) settings[index].data[4] = 0;
          if (settings[index].data[4] < 0) settings[index].data[4] = 23;
        }
        break;
      case 5: // Waktu menit ATAU repeat count (tergantung repJam)
        settings[index].data[5] += delta;
        if (repJam > 0) {
          // Mode per jam: ini repeat count (1-59)
          if (settings[index].data[5] > 59) settings[index].data[5] = 1;
          if (settings[index].data[5] < 1) settings[index].data[5] = 59;
        } else {
          // Mode per hari: ini waktu menit (0-59)
          if (settings[index].data[5] > 59) settings[index].data[5] = 0;
          if (settings[index].data[5] < 0) settings[index].data[5] = 59;
        }
        break;
    }
  } else if (index == IDX_OFFSET_ULTRASONIC) {
    // 2 field: meter dan cm
    if (field == 0) {
      // Field 0: Meter (-5 sampai +5)
      settings[index].data[0] += delta;
      if (settings[index].data[0] > 5) settings[index].data[0] = 5;
      if (settings[index].data[0] < -5) settings[index].data[0] = -5;
    } else if (field == 1) {
      // Field 1: CM (0-99)
      settings[index].data[1] += delta;
      if (settings[index].data[1] > 99) settings[index].data[1] = 0;
      if (settings[index].data[1] < 0) settings[index].data[1] = 99;
    }
  }
}

// Display setting with blink effect
void displayWithBlink(int index, int field, bool state) {
  displaySetting(index);

  if (!state) {
    if (index == IDX_POMPA_LAUT || index == IDX_POMPA_BILAS) {
      int repJam = settings[index].data[3];

      switch (field) {
        case 0: // Durasi menit
          lcd.setCursor(8, 1);
          lcd.print("  ");
          break;
        case 1: // Durasi detik
          lcd.setCursor(12, 1);
          lcd.print("  ");
          break;
        case 2: // Repeat hari
          lcd.setCursor(8, 2);
          lcd.print("  ");
          break;
        case 3: // Repeat jam
          lcd.setCursor(13, 2);
          lcd.print("  ");
          break;
        case 4: // Waktu jam/menit
          if (repJam == 0) {
            lcd.setCursor(7, 3);
            lcd.print("  ");
          } else {
            lcd.setCursor(7, 3);
            lcd.print("  ");
          }
          break;
        case 5: // Waktu menit/count
          if (repJam > 0) {
            lcd.setCursor(7, 3);
            lcd.print("  ");
          } else {
            lcd.setCursor(10, 3);
            lcd.print("  ");
          }
          break;
      }
    } else if (index == IDX_OFFSET_ULTRASONIC) {
      switch (field) {
        case 0: // Meter
          lcd.setCursor(7, 1);
          lcd.print("   ");
          break;
        case 1: // CM
          lcd.setCursor(4, 2);
          lcd.print("   ");
          break;
      }
    }
  }
}

// Display setting screen
void displaySetting(int index) {
  lcd.clear();

  if (index == IDX_POMPA_LAUT || index == IDX_POMPA_BILAS) {
    String pompaNama = (index == IDX_POMPA_LAUT) ? "Pompa Laut" : "Pompa Bilas";
    int durMin = settings[index].data[0];
    int durSec = settings[index].data[1];
    int repHari = settings[index].data[2];
    int repJam = settings[index].data[3];
    int waktuJam = settings[index].data[4];
    int waktuMin = settings[index].data[5];

    lcd.setCursor(0, 0);
    lcd.print("Setting: " + pompaNama);
    lcd.setCursor(0, 1);
    lcd.print("Durasi: " + String(durMin) + "m " + String(durSec) + "s");
    lcd.setCursor(0, 2);
    lcd.print("Setiap: " + String(repHari) + "h, " + String(repJam) + "j");
    lcd.setCursor(0, 3);
    if (repJam > 0) {
      lcd.print("Waktu: " + String(waktuMin) + "m");
    } else {
      String jamStr = (waktuJam < 10) ? "0" + String(waktuJam) : String(waktuJam);
      String minStr = (waktuMin < 10) ? "0" + String(waktuMin) : String(waktuMin);
      lcd.print("Waktu: " + jamStr + ":" + minStr);
    }

  } else if (index == IDX_OFFSET_ULTRASONIC) {
    int meter = settings[index].data[0];
    int cm = settings[index].data[1];

    lcd.setCursor(0, 0);
    lcd.print("Set: Offset U.S.");
    lcd.setCursor(0, 1);
    lcd.print("Meter: " + String(meter));
    lcd.setCursor(0, 2);
    lcd.print("CM: " + String(cm));
  }
}
// Handle setting mode
void handleSettingMode() {
  static int lastSettingIndex = -1;

  if (!isEditing) {
    if (needsRedraw || settingIndex != lastSettingIndex) {
      lcd.clear();
      displaySetting(settingIndex);
      lastSettingIndex = settingIndex;
      needsRedraw = false;
    }

    if (readButton(btnUp)) {
      lcd.clear();
      settingIndex = (settingIndex + 1) % TOTAL_SETTINGS;
      needsRedraw = true;
      delay(200);
    } else if (readButton(btnDown)) {
      lcd.clear();
      settingIndex = (settingIndex - 1 + TOTAL_SETTINGS) % TOTAL_SETTINGS;
      needsRedraw = true;
      delay(200);
    } else if (readButton(btnOK)) {
      isEditing = true;
      editingField = 0;
      blinkState = true;
      lastBlinkUpdate = millis();
      displaySetting(settingIndex);
      Serial.println("Mulai edit setting: " + String(settingIndex));
      delay(200);
    }

  } else {
    unsigned long currentMillis = millis();
    if (currentMillis - lastBlinkUpdate >= 500) {
      lastBlinkUpdate = currentMillis;
      blinkState = !blinkState;
      displayWithBlink(settingIndex, editingField, blinkState);
    }

    if (readButton(btnOK)) {
      int maxField = getMaxField(settingIndex);

      if (editingField < maxField - 1) {
        if (editingField == 3 && (settingIndex == IDX_POMPA_LAUT || settingIndex == IDX_POMPA_BILAS)) {
          int repJam = settings[settingIndex].data[3];
          if (repJam > 0) {
            editingField = 5; // Skip ke repeat count
          } else {
            editingField++;
          }
        } else {
          editingField++;
        }
        blinkState = true;
        displayWithBlink(settingIndex, editingField, blinkState);
        delay(200);
      } else {
        saveSetting(settingIndex);
        isEditing = false;
        needsRedraw = true;
        displaySetting(settingIndex);
        Serial.println(F("Setting saved"));
        delay(200);
      }
    } else if (readButton(btnCancel)) {
      isEditing = false;
      needsRedraw = true;
      displaySetting(settingIndex);
      Serial.println(F("Edit canceled"));
      delay(200);
    } else if (readButton(btnUp)) {
      handleValueChange(settingIndex, editingField, true);
      blinkState = true;
      displayWithBlink(settingIndex, editingField, blinkState);
      lastBlinkUpdate = millis();
      delay(150);
    } else if (readButton(btnDown)) {
      handleValueChange(settingIndex, editingField, false);
      blinkState = true;
      displayWithBlink(settingIndex, editingField, blinkState);
      lastBlinkUpdate = millis();
      delay(150);
    }
  }
}

#endif // BUTTON_EEPROM_H
