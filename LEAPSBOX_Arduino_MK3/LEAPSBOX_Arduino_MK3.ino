/**************************************************************
 * Noabox V1 - Water Quality Monitoring Station
 * File: NoaboxV1_Arduino.ino (MODULAR VERSION)
 *
 * Features:
 * - 5 tombol fisik untuk setting (Mode, OK, Cancel, Up, Down)
 * - LCD 20x4 I2C dengan 3 slide manual
 * - Kontrol pompa otomatis dari EEPROM
 * - Logging ke SD card (per hari)
 * - Kirim data ke ESP32 via level shifter
 * - GPS untuk lokasi
 * - Sensor air: Water Temp, Salinitas, TDS, pH, DO
 * - Sensor udara: DHT22 (Temp & Humidity)
 * - Sensor lain: Ultrasonic
 * 
 * Non-blocking design using millis() throughout
 **************************************************************/

// ================== INCLUDE ALL MODULES ==================
#include "config.h"
#include "sensors_water.h"
#include "sensors_other.h"
#include "button_eeprom.h"
#include "lcd_display.h"
#include "pump_control.h"
#include "sd_logging.h"
#include "rtc_time.h"

// ================== GLOBAL VARIABLES DEFINITION ==================
// Debug flags
const bool GPS_DEBUG = false;
bool debugpar = true;
const bool DEBUG_PH = debugpar;
const bool DEBUG_SALINITAS = debugpar;
const bool DEBUG_TDS = debugpar;
const bool DEBUG_DO = debugpar;
const bool DEBUG_USONIC = debugpar;
const bool DEBUG_WATER_TEMP = debugpar;
const bool DEBUG_DHT22 = debugpar;

// Button pins
const int btnMode = 34;
const int btnOK = 33;
const int btnCancel = 37;
const int btnUp = 35;
const int btnDown = 36;




// Mode system
Mode currentMode = MODE_DISPLAY;

// Setting variables
int settingIndex = 0;
bool isEditing = false;
int editingField = 0;
bool blinkState = false;
unsigned long lastBlinkUpdate = 0;
bool needsRedraw = true;

// Setting indices
const int IDX_POMPA_LAUT = 0;
const int IDX_POMPA_BILAS = 1;
const int IDX_OFFSET_ULTRASONIC = 2;
const int TOTAL_SETTINGS = 3;

// Settings data
Setting settings[3];

// RTC & LCD instances
DS3231 rtc(SDA, SCL);
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Interval waktu
unsigned long previousMillis = 0;
const long interval = 1000;  // 1 detik

// GPS
const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
bool gpsEverFixed = false;
double gpsLat = 0.0, gpsLng = 0.0;
unsigned long lastGpsRead = 0;
const unsigned long GPS_READ_INTERVAL = 500;  // 500ms

// RTC strings
String jam, menit, detik, tanggal, bulan, tahun;
String previousMenit = "";

// Ultrasonic
const int ultrasonicTrigPin = 8;
const int ultrasonicEchoPin = 7;
int distance_cm = 0;
unsigned long lastUltraMs = 0;
const unsigned long ultraIntervalMs = 500;  // 500ms

// LCD slide control
unsigned long prevLcdMillis = 0;
const unsigned long lcdMinInterval = 100;
bool lcdForce = false;
uint8_t currentSlide = 0;
uint8_t prevSlide = 255;

// Sensor instances
OneWire ds(WATER_TEMP_PIN);
DallasTemperature waterTempSensor(&ds);

// DHT22
DHT dht(DHT_PIN, DHT_TYPE);
float g_airTempC = NAN;
float g_airRH = NAN;
unsigned long lastDhtRead = 0;
const unsigned long DHT_READ_INTERVAL = 2000;  // 2 detik (DHT22 butuh minimal 1 detik antar bacaan)

// Salinitas
unsigned long lastSalinityRead = 0;
const unsigned long SALINITY_READ_INTERVAL = 15000;  // 15 detik (karena butuh DMS warmup)
const unsigned long SALINITY_DMS_WARMUP = 10000;     // 10 detik warmup
bool salinityDmsActive = false;
unsigned long salinityDmsStartMs = 0;

// TDS calibration
const float VREF_TDS_MEAS = 5.0f;
const float TDS_FACTOR = 0.5f;
const float CAL_FACTOR = 1.0f;
int tdsBuf[TDS_SCOUNT], tdsBufTmp[TDS_SCOUNT];
int tdsIdx = 0;

// pH calibration (SESUAIKAN DENGAN KALIBRASI ANDA!)
const float PH_M_SLOPE = 19.66f;
const float PH_B_OFFSET = -76.08f;

// DO calibration (SESUAIKAN DENGAN KALIBRASI ANDA!)
const float DO_VZERO_mV = 35.0f;
const float DO_VAIR_mV = 1145.0f;

// Global sensor cache
float g_waterTempC = NAN;
float g_salinityPPT = NAN;
float g_conductivityUS = NAN;
float g_tdsPPM = NAN;
float g_tdsAvgVolt = NAN;
float g_pH = NAN;
float g_do_mgL = NAN;
float g_do_mV = NAN;
float g_vcc = 5.0;
// Pumps
bool pompaLautOn = false;
bool pompaBilasOn = false;
unsigned long pompaLautStartMs = 0;
unsigned long pompaBilasStartMs = 0;
bool overrideLaut = false;
bool overrideBilas = false;
int remainingMeasurements = 0;
int lastTriggerMinuteLaut = -1;   // ← TAMBAH INI
int lastTriggerMinuteBilas = -1;  // ← TAMBAH INI

// SD Card
const int chipSelect = 53;
File dataFile;
bool sdOk = false;
unsigned long lastSdAttemptMs = 0;
const unsigned long sdRetryIntervalMs = 5000;
const int SD_CARD_DETECT_PIN = -1;

// Button debounce
unsigned long lastModeButtonPress = 0;
const unsigned long MODE_BUTTON_DEBOUNCE = 1000;

// ================== SETUP ==================
void setup() {
  Serial.begin(9600);
  Serial2.begin(GPSBaud);  // GPS di Serial2 (pin 16/17)
  Serial3.begin(9600);     // ESP32 di Serial3 (pin 14/15) via level shifter

  // ← TAMBAH 3 BARIS INI

  // Button pins
  pinMode(btnMode, INPUT_PULLUP);
  pinMode(btnOK, INPUT_PULLUP);
  pinMode(btnCancel, INPUT_PULLUP);
  pinMode(btnUp, INPUT_PULLUP);
  pinMode(btnDown, INPUT_PULLUP);

  Serial.println(F("Testing buttons..."));
  Serial.print(F("btnMode (33): "));
  Serial.println(digitalRead(btnMode));
  Serial.print(F("btnOK (34): "));
  Serial.println(digitalRead(btnOK));
  Serial.print(F("btnCancel (35): "));
  Serial.println(digitalRead(btnCancel));
  Serial.print(F("btnUp (36): "));
  Serial.println(digitalRead(btnUp));
  Serial.print(F("btnDown (37): "));
  Serial.println(digitalRead(btnDown));

  // Load settings from EEPROM
  loadSettingsFromEEPROM();

  // Initialize SD card
  Serial.println(F("Initializing SD card..."));
  if (SD_CARD_DETECT_PIN != -1) {
    pinMode(SD_CARD_DETECT_PIN, INPUT_PULLUP);
  }
  tryInitSD(true);

  // Initialize I2C devices
  Wire.begin();
  rtc.begin();
  // Baca RTC untuk inisialisasi lastTriggerMinute
  bacaRTC();
  lastTriggerMinuteLaut = menit.toInt();
  lastTriggerMinuteBilas = menit.toInt();
  Serial.print(F("Init lastTriggerMinute: "));
  Serial.println(menit);
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Noabox V1");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  delay(2000);

  // Initialize sensors
  pinMode(ultrasonicTrigPin, OUTPUT);
  pinMode(ultrasonicEchoPin, INPUT);
  pinMode(SALINITAS_DMS_PIN, OUTPUT);
  digitalWrite(SALINITAS_DMS_PIN, HIGH);  // DMS OFF by default

  waterTempSensor.begin();
  dht.begin();

  // Initialize relay pins
  pinMode(POMPA_AIR_LAUT_PIN, OUTPUT);
  pinMode(POMPA_AIR_BILAS_PIN, OUTPUT);
  digitalWrite(POMPA_AIR_LAUT_PIN, LOW);
  digitalWrite(POMPA_AIR_BILAS_PIN, LOW);

  Serial.println(F("Noabox V1 - Ready!"));
  lcd.clear();
  matikanPompaLaut();
  matikanPompaBilas();
}

// ================== LOOP ==================
void loop() {
  unsigned long currentMillis = millis();

  // ========== MODE BUTTON ==========
  if (digitalRead(btnMode) == LOW) {
    if (currentMillis - lastModeButtonPress >= MODE_BUTTON_DEBOUNCE) {
      lastModeButtonPress = currentMillis;

      if (currentMode == MODE_DISPLAY) {
        currentMode = SETTING;
        resetBackgroundProcesses();
        Serial.println(F("Entered SETTING mode"));
      } else {
        currentMode = MODE_DISPLAY;
        resetBackgroundProcesses();
        Serial.println(F("Returned to DISPLAY mode"));
      }

      delay(500);
    }
  }

  // ========== MODE DISPLAY ==========
  if (currentMode == MODE_DISPLAY) {
    // Baca semua sensor secara non-blocking
    readGPS();
    readUltrasonic();
    readWaterTemp();
    readSalinity();
    readTDS();
    readPH();
    readDO();
    readDHT22();

    // Tombol ganti slide
    if (readButton(btnUp)) {
      lcd.clear();
      currentSlide = (currentSlide + 1) % 3;  // 3 slides untuk noabox
      lcdForce = true;
      delay(200);
    } else if (readButton(btnDown)) {
      lcd.clear();
      currentSlide = (currentSlide - 1 + 3) % 3;
      lcdForce = true;
      delay(200);
    }

    // Per detik
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      bacaRTC();

      prosesPerintahSerialPompa();
      kontrolPompaTerjadwal();

      // Per menit: logging dan kirim ke ESP
      if (menit != previousMenit) {
        String latStr = gpsEverFixed ? String(gpsLat, 6) : "searching";
        String lngStr = gpsEverFixed ? String(gpsLng, 6) : "searching";

        bool canMeasure = pompaLautOn || pompaBilasOn || (remainingMeasurements > 0);

        // Prepare data strings
        String waterTempStr = canMeasure ? String(g_waterTempC, 2) : "TM";
        String salinityStr = canMeasure ? String(g_salinityPPT, 2) : "TM";
        String conductivityStr = canMeasure ? String(g_conductivityUS, 2) : "TM";
        String tdsStr = canMeasure ? String(g_tdsPPM, 0) : "TM";
        String phStr = canMeasure ? String(g_pH, 2) : "TM";
        String doStr = canMeasure ? String(g_do_mgL, 2) : "TM";
        String airTempStr = isnan(g_airTempC) ? "?" : String(g_airTempC, 1);
        String airRhStr = isnan(g_airRH) ? "?" : String(g_airRH, 1);

        // TXT format (human readable)
        String txtLine = String("[") + jam + ":" + menit + ":" + detik + "] "
                         + "GPS:" + latStr + "," + lngStr + " | "
                         + "Dist:" + String(distance_cm) + "cm | "
                         + "WTemp:" + waterTempStr + "C | "
                         + "Sal:" + salinityStr + "ppt | "
                         + "Cond:" + conductivityStr + "uS | "
                         + "TDS:" + tdsStr + "ppm | "
                         + "pH:" + phStr + " | "
                         + "DO:" + doStr + "mgL | "
                         + "ATemp:" + airTempStr + "C | "
                         + "AHum:" + airRhStr + "% | "
                         + "P1:" + (pompaLautOn ? "ON" : "OFF") + " | "
                         + "P2:" + (pompaBilasOn ? "ON" : "OFF");

        if (sdAppendLine(txtLine)) {
          Serial.println(F("Data written to SD card"));
        } else {
          Serial.println(F("SD not ready"));
        }

        // Kirim ke ESP32 via Serial3
        String dataToESP = String("timestamp:") + jam + ":" + menit + ":" + detik
                           + ",gps_lat:" + latStr
                           + ",gps_lng:" + lngStr
                           + ",ultrasonic:" + String(distance_cm)
                           + ",water_temp:" + waterTempStr
                           + ",salinity:" + salinityStr
                           + ",conductivity:" + conductivityStr
                           + ",tds:" + tdsStr
                           + ",pH:" + phStr
                           + ",do:" + doStr
                           + ",air_temp:" + airTempStr
                           + ",air_humidity:" + airRhStr
                           + ",pompa_laut:" + (pompaLautOn ? "ON" : "OFF")
                           + ",pompa_bilas:" + (pompaBilasOn ? "ON" : "OFF");

        Serial3.println(dataToESP);
        Serial.print(F("To ESP32 -> "));
        Serial.println(dataToESP);

        // Update counter pengukuran setelah pompa mati
        if (!pompaLautOn && !pompaBilasOn && remainingMeasurements > 0) {
          remainingMeasurements--;
        }
        if (menit.toInt() != lastTriggerMinuteLaut) {
          lastTriggerMinuteLaut = -1;
        }
        if (menit.toInt() != lastTriggerMinuteBilas) {
          lastTriggerMinuteBilas = -1;
        }
        previousMenit = menit;
        
        lcdForce = true;
      }
    }

    // Update LCD otomatis setiap 1 detik jika ada perubahan
    checkAndUpdateLCD();

    // Force update ketika ganti slide atau ada event tertentu
    if (lcdForce) {
      if (currentMillis - prevLcdMillis >= lcdMinInterval) {
        updateLCD();
        updatePreviousValues();
        lcdForce = false;
      }
    }
  }

  // ========== MODE SETTING ==========
  else if (currentMode == SETTING) {
    handleSettingMode();
  }

  // Small delay to prevent excessive looping (optional)
  // delay(10); // Commented out untuk pure non-blocking
}
