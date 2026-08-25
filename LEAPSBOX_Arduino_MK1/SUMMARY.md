# NOABOX V1 - PROGRAM STRUCTURE SUMMARY

## 📂 File Structure

```
NoaboxV1_Arduino/
├── NoaboxV1_Arduino.ino    # Main program file
├── config.h                # Pin definitions & global variables
├── button_eeprom.h         # Button handling & EEPROM storage
├── lcd_display.h           # LCD 3-slide display system
├── pump_control.h          # Automatic pump scheduling
├── rtc_time.h              # RTC time reading
├── sd_logging.h            # SD card logging (per-day files)
├── sensors_water.h         # Water quality sensors
├── sensors_other.h         # Other sensors (ultrasonic, DHT11, GPS)
└── README.md               # Documentation
```

---

## 🧩 Module Descriptions

### 1. **NoaboxV1_Arduino.ino** (Main File)
**Purpose**: Orchestrates all modules, main loop logic

**Key Functions:**
- `setup()`: Initialize all hardware and load settings
- `loop()`: Main non-blocking loop
  - Mode switching (Display ↔ Setting)
  - Sensor reading coordination
  - Data logging every minute
  - LCD update management

**Flow:**
```
Setup → Load EEPROM → Init Hardware → Loop
  ├─ MODE_DISPLAY: Read sensors, update LCD, log data
  └─ MODE_SETTING: Handle user input, save settings
```

---

### 2. **config.h** (Configuration)
**Purpose**: Central configuration, pin definitions, global variables

**Contains:**
- Library includes
- Pin assignments (all sensors, buttons, SD, LCD, etc)
- Debug flags
- Global sensor variables
- Calibration constants
- EEPROM offset definition

**Important Sections:**
```cpp
// Debug flags
extern const bool DEBUG_PH;
extern const bool DEBUG_SALINITAS;
// ... etc

// Pin definitions
#define WATER_TEMP_PIN 2
#define SALINITAS_DMS_PIN 6
// ... etc

// Calibration
extern const float PH_M_SLOPE;
extern const float PH_B_OFFSET;
// ... etc
```

---

### 3. **button_eeprom.h** (Button & Settings)
**Purpose**: Handle button input and save/load settings from EEPROM

**Key Functions:**
- `readButton(pin)`: Debounced button reading
- `handleSettingMode()`: Main setting menu logic
- `displaySetting(index)`: Show current setting on LCD
- `loadSettingsFromEEPROM()`: Load saved settings on startup
- `saveSetting(index)`: Save settings to EEPROM

**Settings Structure:**
```
Setting[0] = Pompa Laut (6 fields)
Setting[1] = Pompa Bilas (6 fields)
Setting[2] = Offset Ultrasonic (1 field)
```

**EEPROM Layout:**
```
Address 100-111: Pompa Laut settings (6 int × 2 bytes)
Address 112-123: Pompa Bilas settings (6 int × 2 bytes)
Address 124-125: Ultrasonic offset (1 int × 2 bytes)
```

---

### 4. **lcd_display.h** (LCD Display)
**Purpose**: Manage 3-slide LCD display with change detection

**Key Functions:**
- `updateLCD()`: Main LCD update function
- `checkAndUpdateLCD()`: Auto-update every 1 second if changed
- `hasValueChanged()`: Detect sensor value changes
- `clearRestOfLine()`: Clear LCD artifacts

**3 Slides:**
```
Slide 0: Tanggal, Waktu, GPS, Ultrasonic
Slide 1: Water sensors (Temp, Sal, TDS, pH, DO)
Slide 2: Air sensors (Temp, RH) + Pump status
```

**Optimization:**
- Only updates LCD when values actually change
- Tracks previous values per slide
- Prevents flickering

---

### 5. **pump_control.h** (Pump Automation)
**Purpose**: Automatic pump control based on schedule from EEPROM

**Key Functions:**
- `kontrolPompaTerjadwal()`: Check schedule and auto control pumps
- `nyalakanPompa(index, durasi)`: Turn on pump for duration
- `matikanPompa(index)`: Turn off pump
- `prosesPerintahSerialPompa()`: Manual control via Serial commands

**Logic:**
1. Check current time vs schedule
2. Turn pump ON if match
3. Turn pump OFF after duration expires
4. Keep measuring sensors for 2 minutes after pump off

---

### 6. **rtc_time.h** (RTC Time)
**Purpose**: Read time from DS3231 RTC module

**Key Function:**
- `bacaRTC()`: Read and format time strings

**Output Strings:**
```cpp
jam     = "14"  (or "09")
menit   = "09"  (or "05")
detik   = "12"  (or "03")
tanggal = "06"  (or "01")
bulan   = "11"  (or "01")
tahun   = "2025"
```

---

### 7. **sd_logging.h** (SD Card Logging)
**Purpose**: Log data to SD card with per-day file system

**Key Functions:**
- `getLogFileName()`: Generate filename "dd-mm-yy_Data_Log.csv"
- `tryInitSD()`: Initialize SD card
- `sdAppendLine(line)`: Append CSV line to current day's file

**Features:**
- Auto create new file each day
- CSV format with header
- Retry mechanism if SD card fails

**File Format:**
```csv
timestamp,gps_lat,gps_lng,ultrasonic_cm,water_temp_C,salinity_ppt,...
14:09:00,-6.123456,106.567890,405,25.5,2.34,...
```

---

### 8. **sensors_water.h** (Water Quality Sensors)
**Purpose**: Read all water quality sensors (non-blocking)

**Sensors:**
1. **Water Temperature (DS18B20)**
   - `readWaterTemp()`: OneWire, 800ms conversion time
   
2. **Salinitas/Conductivity**
   - `readSalinity()`: With DMS control (10s warmup)
   - State machine: OFF → ON → Warmup → Read → OFF
   
3. **TDS (Total Dissolved Solids)**
   - `readTDS()`: Median filter, temperature compensation
   
4. **pH**
   - `readPH()`: Linear calibration (m * V + b)
   
5. **DO (Dissolved Oxygen)**
   - `readDO()`: 2-point calibration, temp compensation

**All Non-Blocking:**
- Use millis() for timing
- No delay() in sensor functions
- State machines for complex sequences (DMS)

---

### 9. **sensors_other.h** (Other Sensors)
**Purpose**: Read ultrasonic, DHT11, and GPS (non-blocking)

**Sensors:**
1. **Ultrasonic (HC-SR04)**
   - `readUltrasonic()`: Trigger → Wait → Read echo
   - Apply offset from EEPROM
   
2. **DHT11 (Temp & Humidity)**
   - `readDHT11()`: Read every 2 seconds (DHT11 limit)
   
3. **GPS**
   - `readGPS()`: Parse NMEA from Serial2
   - Update location when valid fix

**All Non-Blocking:**
- Use millis() for timing
- No delay() in sensor functions

---

## 🔄 Main Loop Flow

```
1. Check MODE button → Switch modes if pressed

2. MODE_DISPLAY:
   ├─ Read all sensors (non-blocking)
   ├─ Check UP/DOWN buttons → Change slide
   ├─ Every 1 second:
   │  ├─ Read RTC
   │  ├─ Check pump schedule
   │  └─ Every 1 minute:
   │     ├─ Log to SD card (CSV)
   │     └─ Send to ESP32 (Serial3)
   └─ Update LCD if values changed

3. MODE_SETTING:
   └─ Handle setting menu navigation
```

---

## ⏱️ Timing Strategy

### Non-Blocking Design
All sensors use **millis()** for timing, never **delay()**

**Example Pattern:**
```cpp
static unsigned long lastRead = 0;
const unsigned long READ_INTERVAL = 1000;

if (millis() - lastRead >= READ_INTERVAL) {
  lastRead = millis();
  // Do sensor reading
}
```

### Reading Intervals
```
GPS:        500ms   (fast for location tracking)
Ultrasonic: 500ms   (fast for distance monitoring)
Water Temp: 800ms   (DS18B20 conversion time)
TDS:        1000ms  (median filter update)
pH:         1000ms  (averaging)
DO:         1000ms  (averaging)
DHT11:      2000ms  (sensor limitation)
Salinitas:  15000ms (includes 10s DMS warmup)
```

### State Machines
Complex sequences use state machines:

**Salinitas DMS Control:**
```
State: IDLE → Wait 15s → 
State: WARMUP → DMS ON, wait 10s → 
State: READ → Read sensor → 
State: IDLE → DMS OFF
```

---

## 💾 Data Flow

### Sensor → Global Variables
```cpp
g_waterTempC, g_salinityPPT, g_tdsPPM, g_pH, g_do_mgL
g_airTempC, g_airRH
gpsLat, gpsLng, distance_cm
```

### Every Minute → Outputs
```
Global Variables → CSV String → SD Card (append)
Global Variables → Data String → ESP32 (Serial3)
```

### LCD Display
```
Global Variables → hasValueChanged() → updateLCD()
```

---

## 🔧 Customization Points

### Add New Sensor
1. Add pin definition in `config.h`
2. Add global variable for value in `config.h`
3. Create read function in appropriate `.h` file
4. Call read function in main loop
5. Add to LCD display in `lcd_display.h`
6. Add to logging string in main loop

### Change Pump Schedule
1. Press MODE button
2. Navigate to pump setting
3. Edit fields using UP/DOWN and OK
4. Values saved to EEPROM automatically

### Adjust Calibration
1. Edit constants in `config.h`
2. Re-upload program
3. Test with known standards

---

## 🐛 Debug Strategy

### Enable Debug Mode
```cpp
bool debugpar = true;  // In main .ino file
```

### What Gets Printed
- Sensor raw values
- Conversion calculations
- Timing information
- Error messages

### Serial Monitor Output Example
```
[WATER TEMP] 25.5 C
[TDS] Median=512 | VAvg=2.501 | TDSfinal=450.2 ppm
[PH] ADC=612.5 | V=3.001 V | pH=7.82
[DO] Raw=423 | mV=1123.4 | DO=8.45 mg/L
[GPS] Lat=-6.123456 | Lng=106.567890 | Fixed=YES
```

---

## ✅ Best Practices

1. **Never use delay() in loop()** - Always use millis()
2. **Check sensor validity** - Use isnan() before displaying
3. **Pump measurement logic** - Only measure when pump ON or 2min after
4. **SD card handling** - Always check return values
5. **Serial buffer** - Clear buffer if switching serial ports
6. **EEPROM wear** - Only write on save, not every loop
7. **LCD updates** - Only update when values change
8. **Power considerations** - Manage DMS on-time, relay switching

---

## 📊 Memory Usage Estimate

```
Program:  ~25KB / 256KB (10%)
RAM:      ~3KB / 8KB (37%)
EEPROM:   ~100 bytes / 4KB (2.5%)
```

Plenty of room for expansion!

---

## 🎯 Key Takeaways

1. **Modular Design**: Each `.h` file is independent and focused
2. **Non-Blocking**: Everything uses millis(), system never freezes
3. **Configurable**: Settings stored in EEPROM, easy to change
4. **Robust**: Error handling, retry mechanisms, debug mode
5. **Scalable**: Easy to add sensors or features
6. **Professional**: CSV logging, serial communication, RTC timestamping

---

**Created**: November 2025  
**Version**: 1.0  
**Architecture**: Event-driven, non-blocking, modular
