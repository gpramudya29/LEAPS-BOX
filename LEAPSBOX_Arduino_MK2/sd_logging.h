#ifndef SD_LOGGING_H
#define SD_LOGGING_H

#include "config.h"

// ================== FUNCTION DECLARATIONS ==================
void tryInitSD(bool firstTime);
bool sdAppendLine(const String& line);

// ================== FUNCTION IMPLEMENTATIONS ==================

// Inisialisasi SD card
void tryInitSD(bool firstTime) {
  if (SD_CARD_DETECT_PIN != -1) {
    if (digitalRead(SD_CARD_DETECT_PIN) == HIGH) {
      if (sdOk) {
        Serial.println(F("SD card removed"));
        sdOk = false;
      }
      return;
    }
  }

  if (firstTime || !sdOk) {
    if (!SD.begin(chipSelect)) {
      Serial.println(F("SD init failed"));
      sdOk = false;
      return;
    }
    Serial.println(F("SD card initialized"));
    sdOk = true;
    
    // Buat file data_log.txt jika belum ada
    if (!SD.exists("data_log.txt")) {
      File newFile = SD.open("data_log.txt", FILE_WRITE);
      if (newFile) {
        newFile.println("=== NOABOX V1 DATA LOG ===");
        newFile.println("Format: timestamp | gps_lat,gps_lng | ultrasonic_cm | water_temp_C | salinity_ppt | conductivity_uS | tds_ppm | pH | do_mgL | air_temp_C | air_humidity_% | pompa_laut | pompa_bilas");
        newFile.println("========================================");
        newFile.close();
        Serial.println(F("Created data_log.txt"));
      }
    } else {
      Serial.println(F("data_log.txt exists"));
    }
  }
}

// Append satu baris data ke SD card
bool sdAppendLine(const String& line) {
  unsigned long now = millis();

  if (!sdOk) {
    if (now - lastSdAttemptMs >= sdRetryIntervalMs) {
      lastSdAttemptMs = now;
      tryInitSD(false);
    }
    if (!sdOk) return false;
  }

  dataFile = SD.open("data_log.txt", FILE_WRITE);
  if (!dataFile) {
    Serial.println(F("Failed to open data_log.txt"));
    sdOk = false;
    lastSdAttemptMs = now;
    return false;
  }

  dataFile.println(line);
  dataFile.close();
  return true;
}

#endif // SD_LOGGING_H
