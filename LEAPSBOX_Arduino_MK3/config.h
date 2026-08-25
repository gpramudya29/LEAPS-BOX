#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <DS3231.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>
#include <EEPROM.h>
#include <SD.h>
#include <SPI.h>
#include <DHT.h>

// ================== Debug ==================
extern const bool GPS_DEBUG;
extern bool debugpar;
extern const bool DEBUG_PH;
extern const bool DEBUG_SALINITAS;
extern const bool DEBUG_TDS;
extern const bool DEBUG_DO;
extern const bool DEBUG_USONIC;
extern const bool DEBUG_WATER_TEMP;
extern const bool DEBUG_DHT22;

// ================== PIN BUTTON SETTING ==================
extern const int btnMode;
extern const int btnOK;
extern const int btnCancel;
extern const int btnUp;
extern const int btnDown;

// ================== MODE SISTEM ==================
enum Mode { MODE_DISPLAY, SETTING };
extern Mode currentMode;

// ================== SETTING VARIABLES ==================
extern int settingIndex;
extern bool isEditing;
extern int editingField;
extern bool blinkState;
extern unsigned long lastBlinkUpdate;
extern bool needsRedraw;

// ================== MAPPING INDEX SETTING ==================
extern const int IDX_POMPA_LAUT;
extern const int IDX_POMPA_BILAS;
extern const int IDX_OFFSET_ULTRASONIC;
extern const int TOTAL_SETTINGS;

// ================== STRUKTUR DATA SETTING ==================
struct Setting {
  int data[6];
};
extern Setting settings[3];

// ================== RTC & LCD ==================
extern DS3231 rtc;
extern LiquidCrystal_I2C lcd;

// ================== Interval Waktu ==================
extern unsigned long previousMillis;
extern const long interval;

// ================== GPS ==================
extern const uint32_t GPSBaud;
extern TinyGPSPlus gps;
extern bool gpsEverFixed;
extern double gpsLat, gpsLng;
extern unsigned long lastGpsRead;
extern const unsigned long GPS_READ_INTERVAL;

// ================== RTC strings ==================
extern String jam, menit, detik, tanggal, bulan, tahun;
extern String previousMenit;

// ================== Ultrasonic ==================
extern const int ultrasonicTrigPin;
extern const int ultrasonicEchoPin;
extern int distance_cm;
extern unsigned long lastUltraMs;
extern const unsigned long ultraIntervalMs;

// ================== LCD slide control ==================
extern unsigned long prevLcdMillis;
extern const unsigned long lcdMinInterval;
extern bool lcdForce;
extern uint8_t currentSlide;
extern uint8_t prevSlide;

// ================== Sensor pins ==================
#define WATER_TEMP_PIN 2
#define SALINITAS_DMS_PIN 6
#define SALINITAS_ANALOG_PIN A0
#define TDS_PIN A3
#define PH_PIN A1
#define DO_PIN A2
#define DHT_PIN 3

extern OneWire ds;
extern DallasTemperature waterTempSensor;

// ================== DHT22 ==================
#define DHT_TYPE DHT22
extern DHT dht;
extern float g_airTempC;
extern float g_airRH;
extern unsigned long lastDhtRead;
extern const unsigned long DHT_READ_INTERVAL;

// ================== Salinitas ==================
extern unsigned long lastSalinityRead;
extern const unsigned long SALINITY_READ_INTERVAL;
extern const unsigned long SALINITY_DMS_WARMUP;
extern bool salinityDmsActive;
extern unsigned long salinityDmsStartMs;

// ================== TDS calibrated ==================
extern const float VREF_TDS_MEAS;
extern const float TDS_FACTOR;
extern const float CAL_FACTOR;
#define TDS_SCOUNT 30
extern int tdsBuf[TDS_SCOUNT], tdsBufTmp[TDS_SCOUNT];
extern int tdsIdx;

// ================== pH calibration ==================
extern const float PH_M_SLOPE;
extern const float PH_B_OFFSET;

// ================== DO calibration ==================
#define ADC_COUNTS 1024.0
#define VREF_MV 5000.0
extern const float DO_VZERO_mV;
extern const float DO_VAIR_mV;

// ================== Global sensor cache ==================
extern float g_waterTempC;
extern float g_salinityPPT;
extern float g_conductivityUS;
extern float g_tdsPPM;
extern float g_tdsAvgVolt;
extern float g_pH;
extern float g_do_mgL;
extern float g_do_mV;
extern float g_vcc;
// ================== PUMPS ==================
#define POMPA_AIR_LAUT_PIN 39
#define POMPA_AIR_BILAS_PIN 41
extern int lastTriggerMinuteLaut;      // ← TAMBAH INI
extern int lastTriggerMinuteBilas;    // ← TAMBAH INI


extern bool pompaLautOn;
extern bool pompaBilasOn;
extern unsigned long pompaLautStartMs;
extern unsigned long pompaBilasStartMs;
extern bool overrideLaut;
extern bool overrideBilas;
extern int remainingMeasurements;

// ================== SD Card ==================
extern const int chipSelect;
extern File dataFile;
extern bool sdOk;
extern unsigned long lastSdAttemptMs;
extern const unsigned long sdRetryIntervalMs;
extern const int SD_CARD_DETECT_PIN;

// ================== MODE BUTTON DEBOUNCE ==================
extern unsigned long lastModeButtonPress;
extern const unsigned long MODE_BUTTON_DEBOUNCE;

// ================== EEPROM ==================
#define EEPROM_OFFSET 100

#endif // CONFIG_H
