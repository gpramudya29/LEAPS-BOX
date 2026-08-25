#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include "config.h"

// ================== PREVIOUS VALUES TRACKING ==================
struct SensorValues {
  String tanggal_prev = "";
  String bulan_prev = "";
  String tahun_prev = "";
  String jam_prev = "";
  String menit_prev = "";
  String detik_prev = "";
  double gpsLat_prev = -999.0;
  double gpsLng_prev = -999.0;
  bool gpsEverFixed_prev = false;
  int distance_cm_prev = -1;

  float g_waterTempC_prev = -999.0;
  float g_salinityPPT_prev = -999.0;
  float g_tdsPPM_prev = -999.0;
  float g_pH_prev = -999.0;
  float g_do_mgL_prev = -999.0;

  float g_airTempC_prev = -999.0;
  float g_airRH_prev = -999.0;

  uint8_t currentSlide_prev = 255;
  bool pompaLautOn_prev = false;
  bool pompaBilasOn_prev = false;
};

SensorValues prevValues;
unsigned long lastLcdCheckMs = 0;
const unsigned long LCD_CHECK_INTERVAL = 1000;  // Cek setiap 1 detik

// ================== FUNCTION DECLARATIONS ==================
void updateLCD();
void clearRestOfLine(int startCol, int row);
bool hasValueChanged();
void updatePreviousValues();
void checkAndUpdateLCD();

// ================== HELPER FUNCTIONS ==================

// Clear sisa baris di LCD
void clearRestOfLine(int startCol, int row) {
  lcd.setCursor(startCol, row);
  int charsToClean = 20 - startCol; // LCD 20x4
  for (int i = 0; i < charsToClean; i++) {
    lcd.print(" ");
  }
}

// Cek apakah ada nilai sensor yang berubah
bool hasValueChanged() {
  bool changed = false;

  // Cek perubahan slide
  if (currentSlide != prevValues.currentSlide_prev) {
    changed = true;
  }

  // Cek perubahan berdasarkan slide yang aktif
  if (currentSlide == 0) {
    // Slide 0: Tanggal, Waktu, GPS, Ultrasonic
    if (tanggal != prevValues.tanggal_prev || 
        bulan != prevValues.bulan_prev || 
        tahun != prevValues.tahun_prev || 
        jam != prevValues.jam_prev || 
        menit != prevValues.menit_prev || 
        detik != prevValues.detik_prev || 
        gpsLat != prevValues.gpsLat_prev || 
        gpsLng != prevValues.gpsLng_prev || 
        gpsEverFixed != prevValues.gpsEverFixed_prev || 
        distance_cm != prevValues.distance_cm_prev) {
      changed = true;
    }
  } else if (currentSlide == 1) {
    // Slide 1: Sensor Air
    bool canMeasure = pompaLautOn || pompaBilasOn || (remainingMeasurements > 0);
    bool canDisplay = canMeasure || DEBUG_WATER_TEMP || DEBUG_SALINITAS || DEBUG_TDS || DEBUG_PH || DEBUG_DO;
    
    if (canDisplay != prevValues.pompaLautOn_prev ||
        g_waterTempC != prevValues.g_waterTempC_prev || 
        g_salinityPPT != prevValues.g_salinityPPT_prev || 
        g_tdsPPM != prevValues.g_tdsPPM_prev || 
        g_pH != prevValues.g_pH_prev || 
        g_do_mgL != prevValues.g_do_mgL_prev) {
      changed = true;
    }
  } else if (currentSlide == 2) {
    // Slide 2: Sensor Udara
    if (g_airTempC != prevValues.g_airTempC_prev || 
        g_airRH != prevValues.g_airRH_prev ||
        pompaLautOn != prevValues.pompaLautOn_prev ||
        pompaBilasOn != prevValues.pompaBilasOn_prev) {
      changed = true;
    }
  }

  return changed;
}

// Update nilai sebelumnya
void updatePreviousValues() {
  prevValues.currentSlide_prev = currentSlide;

  if (currentSlide == 0) {
    prevValues.tanggal_prev = tanggal;
    prevValues.bulan_prev = bulan;
    prevValues.tahun_prev = tahun;
    prevValues.jam_prev = jam;
    prevValues.menit_prev = menit;
    prevValues.detik_prev = detik;
    prevValues.gpsLat_prev = gpsLat;
    prevValues.gpsLng_prev = gpsLng;
    prevValues.gpsEverFixed_prev = gpsEverFixed;
    prevValues.distance_cm_prev = distance_cm;
  } else if (currentSlide == 1) {
    prevValues.g_waterTempC_prev = g_waterTempC;
    prevValues.g_salinityPPT_prev = g_salinityPPT;
    prevValues.g_tdsPPM_prev = g_tdsPPM;
    prevValues.g_pH_prev = g_pH;
    prevValues.g_do_mgL_prev = g_do_mgL;
    prevValues.pompaLautOn_prev = pompaLautOn;
  } else if (currentSlide == 2) {
    prevValues.g_airTempC_prev = g_airTempC;
    prevValues.g_airRH_prev = g_airRH;
    prevValues.pompaLautOn_prev = pompaLautOn;
    prevValues.pompaBilasOn_prev = pompaBilasOn;
  }
}

// Cek dan update LCD setiap 1 detik
void checkAndUpdateLCD() {
  unsigned long currentMs = millis();

  if (currentMs - lastLcdCheckMs >= LCD_CHECK_INTERVAL) {
    lastLcdCheckMs = currentMs;

    if (hasValueChanged()) {
      updateLCD();
      updatePreviousValues();
    }
  }
}

// ================== UPDATE LCD FUNCTION ==================

void updateLCD() {
  prevLcdMillis = millis();

  if (currentSlide != prevSlide) {
    lcd.clear();
    prevSlide = currentSlide;
  }

  bool canMeasure = pompaLautOn || pompaBilasOn || (remainingMeasurements > 0);

  // ==================== SLIDE 0: Tanggal, Waktu, GPS, Ultrasonic ====================
  if (currentSlide == 0) {
    // Baris 0: Tanggal
    lcd.setCursor(0, 0);
    lcd.print("TGL: ");
    lcd.print(tanggal);
    lcd.print("/");
    lcd.print(bulan);
    lcd.print("/");
    lcd.print(tahun);
    int cursorPos = 5 + tanggal.length() + 1 + bulan.length() + 1 + tahun.length();
    clearRestOfLine(cursorPos, 0);

    // Baris 1: Waktu (update tiap detik)
    lcd.setCursor(0, 1);
    lcd.print("Waktu: ");
    lcd.print(jam);
    lcd.print(":");
    lcd.print(menit);
    lcd.print(":");
    lcd.print(detik);
    cursorPos = 7 + jam.length() + 1 + menit.length() + 1 + detik.length();
    clearRestOfLine(cursorPos, 1);

    // Baris 2: GPS
    lcd.setCursor(0, 2);
    if (gpsEverFixed) {
      lcd.print("GPS: ");
      String latStr = String(gpsLat, 4);
      String lngStr = String(gpsLng, 4);
      lcd.print(latStr);
      lcd.print(",");
      lcd.print(lngStr);
      cursorPos = 5 + latStr.length() + 1 + lngStr.length();
      clearRestOfLine(cursorPos, 2);
    } else {
      lcd.print("GPS: mencari sinyal ");
    }

    // Baris 3: Ultrasonic
    lcd.setCursor(0, 3);
    lcd.print("Ultrasonic: ");
    lcd.print(distance_cm);
    lcd.print("cm");
    cursorPos = 12 + String(distance_cm).length() + 2;
    clearRestOfLine(cursorPos, 3);
  }

  // ==================== SLIDE 1: Sensor Air ====================
  else if (currentSlide == 1) {
    bool canDisplay = canMeasure || DEBUG_WATER_TEMP || DEBUG_SALINITAS || DEBUG_TDS || DEBUG_PH || DEBUG_DO;

    // Baris 0: Suhu Air
    lcd.setCursor(0, 0);
    if (canDisplay) {
      lcd.print("Suhu Air: ");
      String tempStr = isnan(g_waterTempC) ? "?" : String(g_waterTempC, 1);
      lcd.print(tempStr);
      lcd.print("C");
      int cursorPos = 10 + tempStr.length() + 1;
      clearRestOfLine(cursorPos, 0);
    } else {
      lcd.print("Suhu Air: TM        ");
    }

    // Baris 1: Salinitas, TDS
    lcd.setCursor(0, 1);
    if (canDisplay) {
      String salStr = isnan(g_salinityPPT) ? "?" : String(g_salinityPPT, 2);
      String tdsStr = isnan(g_tdsPPM) ? "?" : String(g_tdsPPM, 0);
      lcd.print("Sal:");
      lcd.print(salStr);
      lcd.print(" TDS:");
      lcd.print(tdsStr);
      int cursorPos = 4 + salStr.length() + 5 + tdsStr.length();
      clearRestOfLine(cursorPos, 1);
    } else {
      lcd.print("Sal:TM TDS:TM       ");
    }

    // Baris 2: pH, DO (baris pertama)
    lcd.setCursor(0, 2);
    if (canDisplay) {
      String phStr = isnan(g_pH) ? "?" : String(g_pH, 1);
      lcd.print("pH: ");
      lcd.print(phStr);
      int cursorPos = 4 + phStr.length();
      clearRestOfLine(cursorPos, 2);
    } else {
      lcd.print("pH: TM              ");
    }

    // Baris 3: DO (baris kedua)
    lcd.setCursor(0, 3);
    if (canDisplay) {
      String doStr = isnan(g_do_mgL) ? "?" : String(g_do_mgL, 2);
      lcd.print("DO: ");
      lcd.print(doStr);
      lcd.print(" mg/L");
      int cursorPos = 4 + doStr.length() + 5;
      clearRestOfLine(cursorPos, 3);
    } else {
      lcd.print("DO: TM              ");
    }
  }

  // ==================== SLIDE 2: Sensor Udara & Status Pompa ====================
  else if (currentSlide == 2) {
    // Baris 0: Suhu Udara
    lcd.setCursor(0, 0);
    lcd.print("Suhu Udara: ");
    String tempStr = isnan(g_airTempC) ? "?" : String(g_airTempC, 1);
    lcd.print(tempStr);
    lcd.print("C");
    int cursorPos = 12 + tempStr.length() + 1;
    clearRestOfLine(cursorPos, 0);

    // Baris 1: Kelembaban Udara
    lcd.setCursor(0, 1);
    lcd.print("Kelembaban: ");
    String rhStr = isnan(g_airRH) ? "?" : String(g_airRH, 1);
    lcd.print(rhStr);
    lcd.print("%");
    cursorPos = 12 + rhStr.length() + 1;
    clearRestOfLine(cursorPos, 1);

    // Baris 2: Pompa Laut
    lcd.setCursor(0, 2);
    lcd.print("Pompa Laut: ");
    lcd.print(pompaLautOn ? "ON " : "OFF");
    cursorPos = 12 + 3;
    clearRestOfLine(cursorPos, 2);

    // Baris 3: Pompa Bilas
    lcd.setCursor(0, 3);
    lcd.print("Pompa Bilas: ");
    lcd.print(pompaBilasOn ? "ON " : "OFF");
    cursorPos = 13 + 3;
    clearRestOfLine(cursorPos, 3);
  }
}

#endif // LCD_DISPLAY_H
