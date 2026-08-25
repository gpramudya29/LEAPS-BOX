#ifndef SENSORS_WATER_H
#define SENSORS_WATER_H

#include "config.h"

// ================== FUNCTION DECLARATIONS ==================
void readWaterTemp();
void readSalinity();
void readTDS();
void readPH();
void readDO();
float DOsaturationAtTemp(float tempC);

// ================== DEBUG FUNCTIONS ==================
void debugWaterTemp(float temp) {
  if (DEBUG_WATER_TEMP) {
    Serial.print(F("[WATER TEMP] "));
    Serial.print(temp, 2);
    Serial.println(F(" C"));
  }
}

void debugSalinity(int adc, float conductivity, float tds, float salinity) {
  if (DEBUG_SALINITAS) {
    Serial.print(F("[SALINITY] ADC="));
    Serial.print(adc);
    Serial.print(F(" | Conductivity="));
    Serial.print(conductivity, 2);
    Serial.print(F(" uS/cm | TDS_sal="));
    Serial.print(tds, 2);
    Serial.print(F(" ppm | Salinity="));
    Serial.print(salinity, 2);
    Serial.println(F(" ppt"));
  }
}

void debugTDS(int median, float vAvg, float vComp, float tdsRaw, float tdsFinal) {
  if (DEBUG_TDS) {
    Serial.print(F("[TDS] Median="));
    Serial.print(median);
    Serial.print(F(" | VAvg="));
    Serial.print(vAvg, 3);
    Serial.print(F(" | VComp="));
    Serial.print(vComp, 3);
    Serial.print(F(" | TDSraw="));
    Serial.print(tdsRaw, 1);
    Serial.print(F(" | TDSfinal="));
    Serial.print(tdsFinal, 1);
    Serial.println(F(" ppm"));
  }
}

void debugPH(float adc, float vPH_mV, float temp, float pH) {
  if (DEBUG_PH) {
    Serial.print(F("[PH] ADC="));
    Serial.print(adc, 1);
    Serial.print(F(" | V="));
    Serial.print(vPH_mV / 1000.0, 3);
    Serial.print(F(" V | Temp="));
    Serial.print(temp, 1);
    Serial.print(F(" C | pH="));
    Serial.println(pH, 2);
  }
}

void debugDO(uint16_t raw, float mV, float temp, float doSat, float doMgL) {
  if (DEBUG_DO) {
    Serial.print(F("[DO] Raw="));
    Serial.print(raw);
    Serial.print(F(" | mV="));
    Serial.print(mV, 1);
    Serial.print(F(" | Temp="));
    Serial.print(temp, 1);
    Serial.print(F(" C | DOsat="));
    Serial.print(doSat, 2);
    Serial.print(F(" | DO="));
    Serial.print(doMgL, 2);
    Serial.println(F(" mg/L"));
  }
}

// ================== HELPER FUNCTIONS ==================

// Hitung DO saturation berdasarkan suhu (mg/L)
float DOsaturationAtTemp(float tempC) {
  if (tempC < 0.0f) tempC = 0.0f;
  if (tempC > 50.0f) tempC = 50.0f;
  return 14.652f - 0.41022f * tempC + 0.007991f * tempC * tempC - 0.000077774f * tempC * tempC * tempC;
}

// ================== SENSOR READING FUNCTIONS ==================

// Baca Water Temperature (DS18B20 OneWire) - Non-blocking
void readWaterTemp() {
  static unsigned long lastRequest = 0;
  static bool waitingForConversion = false;
  const unsigned long CONVERSION_TIME = 800; // 800ms untuk DS18B20
  
  unsigned long now = millis();
  
  if (!waitingForConversion) {
    // Request temperature
    waterTempSensor.requestTemperatures();
    lastRequest = now;
    waitingForConversion = true;
  } else {
    // Check if conversion is complete
    if (now - lastRequest >= CONVERSION_TIME) {
      float temp = waterTempSensor.getTempCByIndex(0);
      
      if (temp != DEVICE_DISCONNECTED_C && temp > -50 && temp < 125) {
        g_waterTempC = temp;
        debugWaterTemp(temp);
      } else {
        g_waterTempC = NAN;
      }
      
      waitingForConversion = false;
    }
  }
}

// Baca Salinitas/Conductivity - Non-blocking dengan DMS control
void readSalinity() {
  unsigned long now = millis();
  
  // State machine untuk DMS control
  if (!salinityDmsActive) {
    // Cek apakah sudah waktunya baca salinity
    if (now - lastSalinityRead >= SALINITY_READ_INTERVAL) {
      // Aktifkan DMS
      digitalWrite(SALINITAS_DMS_PIN, LOW);
      salinityDmsActive = true;
      salinityDmsStartMs = now;
      if (DEBUG_SALINITAS) Serial.println(F("[SALINITY] DMS activated, warming up..."));
    }
  } else {
    // DMS sudah aktif, cek apakah sudah warm-up
    if (now - salinityDmsStartMs >= SALINITY_DMS_WARMUP) {
      // Baca sensor
      int sensorValue = analogRead(SALINITAS_ANALOG_PIN);
      
      // Konversi ke conductivity (uS/cm)
      float conductivity = (0.2142 * sensorValue) + 494.93;
      
      // Konversi ke TDS (ppm) - dari sensor salinitas
      float tds = (0.3417 * sensorValue) + 281.08;
      
      // Konversi ke salinity (ppt)
      float salinity = conductivity * 0.00064;
      
      g_conductivityUS = conductivity;
      g_salinityPPT = salinity;
      
      debugSalinity(sensorValue, conductivity, tds, salinity);
      
      // Matikan DMS
      digitalWrite(SALINITAS_DMS_PIN, HIGH);
      salinityDmsActive = false;
      lastSalinityRead = now;
      
      if (DEBUG_SALINITAS) Serial.println(F("[SALINITY] DMS deactivated"));
    }
  }
}

// Baca TDS - Menggunakan median filter
void readTDS() {
  static unsigned long lastTdsRead = 0;
  const unsigned long TDS_READ_INTERVAL = 1000; // 1 detik
  
  unsigned long now = millis();
  
  if (now - lastTdsRead >= TDS_READ_INTERVAL) {
    lastTdsRead = now;
    
    // Baca sensor
    tdsBuf[tdsIdx++] = analogRead(TDS_PIN);
    if (tdsIdx >= TDS_SCOUNT) tdsIdx = 0;
    
    // Copy dan sort untuk median
    for (int i = 0; i < TDS_SCOUNT; i++) {
      tdsBufTmp[i] = tdsBuf[i];
    }
    for (int i = 0; i < TDS_SCOUNT - 1; i++) {
      for (int j = i + 1; j < TDS_SCOUNT; j++) {
        if (tdsBufTmp[i] > tdsBufTmp[j]) {
          int temp = tdsBufTmp[i];
          tdsBufTmp[i] = tdsBufTmp[j];
          tdsBufTmp[j] = temp;
        }
      }
    }
    int median = tdsBufTmp[TDS_SCOUNT / 2];
    
    // Konversi ke voltage
    float vAvg = (median * VREF_TDS_MEAS) / 1023.0f;
    
    // Kompensasi suhu (gunakan water temp jika ada)
    float tempUsed = (isnan(g_waterTempC) || g_waterTempC <= -999) ? 25.0f : g_waterTempC;
    float vComp = vAvg / (1.0f + 0.02f * (tempUsed - 25.0f));
    
    // Konversi ke TDS menggunakan polynomial
    float tdsRaw = (133.42f * vComp * vComp * vComp - 255.86f * vComp * vComp + 857.39f * vComp) * TDS_FACTOR;
    g_tdsPPM = tdsRaw * CAL_FACTOR;
    g_tdsAvgVolt = vAvg;
    
    debugTDS(median, vAvg, vComp, tdsRaw, g_tdsPPM);
  }
}

// Baca pH - Simple averaging
void readPH() {
  static unsigned long lastPhRead = 0;
  const unsigned long PH_READ_INTERVAL = 1000; // 1 detik
  
  unsigned long now = millis();
  
  if (now - lastPhRead >= PH_READ_INTERVAL) {
    lastPhRead = now;
    
    // Average multiple readings
    long total = 0;
    const int N = 10;
    for (int i = 0; i < N; i++) {
      total += analogRead(PH_PIN);
      delay(2); // Small delay antar reading
    }
    float avg = total / (float)N;
    
    // Konversi ke voltage
    float v = (avg * 5.0) / 1023.0;
    
    // Konversi ke pH menggunakan kalibrasi linear
    float pH = PH_M_SLOPE * v + PH_B_OFFSET;
    
    // Validasi range pH (0-14)
    if (pH < 0) pH = 0;
    if (pH > 14) pH = 14;
    
    g_pH = pH;
    
    debugPH(avg, v * 1000.0, (isnan(g_waterTempC) ? 25.0 : g_waterTempC), pH);
  }
}

// Baca DO (Dissolved Oxygen)
void readDO() {
  static unsigned long lastDoRead = 0;
  const unsigned long DO_READ_INTERVAL = 1000; // 1 detik
  
  unsigned long now = millis();
  
  if (now - lastDoRead >= DO_READ_INTERVAL) {
    lastDoRead = now;
    
    // Average multiple readings
    long doSum = 0;
    const int DO_N = 20;
    for (int i = 0; i < DO_N; i++) {
      doSum += analogRead(DO_PIN);
      delay(2);
    }
    uint16_t doRaw = doSum / DO_N;
    
    // Konversi ke mV
    float do_mV = doRaw * (VREF_MV / ADC_COUNTS);
    g_do_mV = do_mV;
    
    // Gunakan water temp jika ada
    float tempUsed = (isnan(g_waterTempC) || g_waterTempC <= -999) ? 25.0f : g_waterTempC;
    
    // Hitung DO saturation
    float doSat = DOsaturationAtTemp(tempUsed);
    
    // Hitung fraction
    float frac = (do_mV - DO_VZERO_mV) / (DO_VAIR_mV - DO_VZERO_mV);
    if (frac < 0) frac = 0;
    if (frac > 1.2f) frac = 1.2f;
    
    g_do_mgL = frac * doSat;
    
    debugDO(doRaw, do_mV, tempUsed, doSat, g_do_mgL);
  }
}

#endif // SENSORS_WATER_H
