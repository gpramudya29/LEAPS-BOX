# LEAPSBOX - Water Quality Monitoring Station

Sistem monitoring kualitas air berbasis Arduino Mega 2560 dengan multiple sensors dan data logging.

## 📋 FEATURES

### Hardware
- **Microcontroller**: Arduino Mega 2560
- **Display**: LCD 20x4 I2C dengan 3 slide informasi
- **Input**: 5 tombol navigasi (Mode, OK, Back, Up, Down)
- **Storage**: MicroSD Card (logging per hari)
- **Communication**: 
  - GPS Module (location tracking)
  - ESP32 via Level Shifter (data transmission)
- **Relay Control**: 2 pompa (air laut & bilas)

### Sensors
**Water Quality Sensors:**
1. DS18B20 - Water Temperature (OneWire)
2. Conductivity Sensor - Salinitas/Conductivity (dengan DMS control)
3. TDS Sensor - Total Dissolved Solids
4. pH Sensor - pH level
5. DO Sensor - Dissolved Oxygen

**Environmental Sensors:**
6. DHT11 - Air Temperature & Humidity
7. HC-SR04 - Ultrasonic Distance

**Location:**
8. GPS Module - Latitude & Longitude

### Software Features
- ✅ Non-blocking design (menggunakan millis() di semua sensor)
- ✅ Auto pump control berdasarkan schedule (EEPROM)
- ✅ Data logging ke SD card (format CSV per hari)
- ✅ Real-time data transmission ke ESP32
- ✅ 3-slide LCD display dengan navigasi manual
- ✅ Settings menu untuk konfigurasi pompa dan offset ultrasonic
- ✅ Debug mode untuk troubleshooting

---

## 🔌 WIRING

Lihat file **LEAPSBOXV1_Wiring_Table.md** untuk wiring lengkap.

### Pin Summary
- **Digital**: 2, 3, 6, 7, 8, 14, 15, 16, 17, 20, 21, 33-37, 39, 41, 50-53
- **Analog**: A0, A1, A2, A3
- **Total**: 26 pins

---

## 📦 REQUIRED LIBRARIES

Install library berikut via Arduino IDE Library Manager:

```
1. Wire (built-in)
2. SPI (built-in)
3. EEPROM (built-in)
4. OneWire by Paul Stoffregen
5. DallasTemperature by Miles Burton
6. DS3231 by Henning Karlsen
7. LiquidCrystal I2C by Frank de Brabander
8. TinyGPS++ by Mikal Hart
9. DHT sensor library by Adafruit
10. SD (built-in)
```

---

## 🚀 INSTALLATION

### 1. Hardware Setup
1. Sambungkan semua komponen sesuai wiring table
2. Pastikan power supply cukup (Arduino + sensors + relays)
3. Pasang SD card kosong ke SD card module
4. Test koneksi I2C (LCD & RTC) menggunakan I2C scanner

### 2. Software Setup
1. Download & install Arduino IDE (1.8.x atau 2.x)
2. Install semua required libraries
3. Buka file `LEAPSBOX_Arduino.ino`
4. Pilih board: **Arduino Mega 2560**
5. Pilih port yang sesuai
6. Upload program

### 3. Initial Configuration
1. Setelah upload, buka Serial Monitor (9600 baud)
2. Tekan tombol MODE untuk masuk ke menu setting
3. Konfigurasi jadwal pompa sesuai kebutuhan
4. Adjust offset ultrasonic jika perlu
5. Simpan settings dengan tombol OK

---

## 🎮 CARA PENGGUNAAN

### Tombol Navigasi
```
MODE   (Pin 33) - Switch antara Display <-> Setting mode
OK     (Pin 34) - Konfirmasi / Lanjut edit
BACK   (Pin 35) - Batal / Kembali
UP     (Pin 36) - Naik / Increment / Ganti slide
DOWN   (Pin 37) - Turun / Decrement / Ganti slide
```

### Display Mode (3 Slides)

**SLIDE 1: Info Umum**
```
TGL: 06/11/2025
Waktu: 14:09:12 (update tiap detik)
GPS: -6.1234,106.5678
Ultrasonic: 405cm
```

**SLIDE 2: Sensor Air**
```
Suhu Air: 25.5C
Sal:2.34 TDS:450
pH: 7.8
DO: 8.45 mg/L
```

**SLIDE 3: Sensor Udara & Pompa**
```
Suhu Udara: 30.2C
Kelembaban: 65.5%
Pompa Laut: OFF
Pompa Bilas: OFF
```

### Setting Mode

**1. Setting Pompa Laut/Bilas**
```
Start: Menit dan detik awal
Durasi: Berapa menit pompa nyala
Repeat jam: Interval repeat (0-23 jam)
Waktu: Waktu mulai operasi
```

**2. Setting Offset Ultrasonic**
```
Offset: +/- adjustment dalam cm
```

### Manual Pump Control (via Serial)
Kirim command via Serial Monitor:
```
POMPA_LAUT_ON    - Nyalakan pompa laut manual
POMPA_LAUT_OFF   - Matikan pompa laut
POMPA_BILAS_ON   - Nyalakan pompa bilas manual
POMPA_BILAS_OFF  - Matikan pompa bilas
RESET_OVERRIDE   - Reset override, kembali ke auto
```

---

## 📁 SD CARD DATA FORMAT

### File Naming
Format: `dd-mm-yy_Data_Log.csv`
Contoh: `06-11-25_Data_Log.csv`

File baru dibuat otomatis setiap ganti hari.

### CSV Header
```csv
timestamp,gps_lat,gps_lng,ultrasonic_cm,water_temp_C,salinity_ppt,conductivity_uS,tds_ppm,pH,do_mgL,air_temp_C,air_humidity_%,pompa_laut,pompa_bilas
```

### Data Example
```csv
14:09:00,-6.123456,106.567890,405,25.5,2.34,1200,450,7.8,8.45,30.2,65.5,OFF,OFF
14:10:00,-6.123456,106.567890,403,25.6,2.35,1205,452,7.8,8.44,30.3,65.4,ON,OFF
```

---

## 🔧 CALIBRATION

### pH Sensor
Edit nilai di `config.h`:
```cpp
const float PH_M_SLOPE = -6.93f;    // Adjust sesuai kalibrasi
const float PH_B_OFFSET = 9.02f;    // Adjust sesuai kalibrasi
```

**Cara Kalibrasi:**
1. Celupkan sensor ke buffer pH 7.0
2. Catat voltage yang terbaca
3. Celupkan sensor ke buffer pH 4.0 atau 10.0
4. Catat voltage yang terbaca
5. Hitung slope (m) dan offset (b) dari 2 titik tersebut
6. Formula: pH = m * voltage + b

### DO Sensor
Edit nilai di `config.h`:
```cpp
const float DO_VZERO_mV = 35.0f;    // Voltage saat 0% saturasi
const float DO_VAIR_mV = 1145.0f;   // Voltage saat 100% saturasi (udara)
```

**Cara Kalibrasi:**
1. Celupkan sensor ke air yang sudah dideoxygenate (sodium sulfite)
2. Catat voltage → ini VZERO
3. Letakkan sensor di udara terbuka, tunggu stabil
4. Catat voltage → ini VAIR

### TDS Sensor
Edit nilai di `config.h`:
```cpp
const float TDS_FACTOR = 0.5f;      // Conversion factor
const float CAL_FACTOR = 1.0f;      // Calibration multiplier
```

### Salinitas Sensor
Konstanta konversi sudah ada di kode, biasanya tidak perlu diubah.
Jika perlu fine-tune, edit di `sensors_water.h`:
```cpp
float conductivity = (0.2142 * sensorValue) + 494.93;
float salinity = conductivity * 0.00064;
```

---

## 🐛 TROUBLESHOOTING

### LCD Tidak Menyala
1. Cek koneksi I2C (SDA=20, SCL=21)
2. Cek alamat I2C LCD (biasanya 0x27 atau 0x3F)
3. Test dengan I2C scanner

### GPS Tidak Fix
1. Pastikan GPS punya view langit terbuka
2. GPS butuh waktu cold start (1-3 menit pertama)
3. Cek koneksi ke Serial2 (pin 16/17)
4. Enable GPS_DEBUG untuk lihat NMEA data

### Sensor Air Tidak Terbaca
1. Pastikan pompa nyala (sensor butuh air mengalir)
2. Atau enable debug mode untuk paksa baca sensor
3. Cek koneksi analog pins

### SD Card Error
1. Pastikan SD card terformat FAT32
2. Cek koneksi SPI (pin 50-53)
3. Pastikan SD card tidak full
4. Lihat Serial Monitor untuk error message

### Salinitas Sensor Tidak Stabil
1. Sensor butuh warm-up 10 detik sebelum baca
2. Biarkan DMS cycle selesai (total ~12 detik)
3. Pastikan sensor terendam 5cm di air

---

## 🔬 DEBUG MODE

Enable debug di `config.h`:
```cpp
bool debugpar = true;  // Master debug

const bool DEBUG_PH = debugpar;
const bool DEBUG_SALINITAS = debugpar;
const bool DEBUG_TDS = debugpar;
const bool DEBUG_DO = debugpar;
const bool DEBUG_USONIC = debugpar;
const bool DEBUG_WATER_TEMP = debugpar;
const bool DEBUG_DHT11 = debugpar;
const bool GPS_DEBUG = false;  // Hati-hati, banyak output!
```

Debug akan print info detail tiap sensor ke Serial Monitor.

---

## 📊 DATA TRANSMISSION TO ESP32

Data dikirim via Serial3 (pin 14/15) melalui level shifter setiap 1 menit.

**Format:**
```
timestamp:14:09:00,gps_lat:-6.123456,gps_lng:106.567890,ultrasonic:405,water_temp:25.5,salinity:2.34,conductivity:1200,tds:450,pH:7.8,do:8.45,air_temp:30.2,air_humidity:65.5,pompa_laut:OFF,pompa_bilas:OFF
```

ESP32 dapat parse data ini dan upload ke cloud/database.

---

## ⚠️ IMPORTANT NOTES

1. **Non-Blocking Design**: Semua sensor menggunakan millis(), tidak ada delay() di loop utama
2. **DMS Control**: Salinitas sensor butuh 10 detik warm-up, jangan ganggu cycle-nya
3. **Water Flow**: Sensor air (TDS, pH, DO) hanya baca saat pompa ON atau 2 menit setelahnya
4. **SD Card**: File baru dibuat otomatis tiap hari, pastikan SD card cukup besar
5. **EEPROM**: Settings pompa dan offset disimpan permanent
6. **Power**: Pastikan power supply cukup untuk semua komponen (minimal 2A @ 5V)

---

## 📝 TODO / FUTURE IMPROVEMENTS

- [ ] Add OTA update capability
- [ ] Implement alarm/notification system
- [ ] Add more sensor types (ORP, Turbidity, etc)
- [ ] Web interface for monitoring
- [ ] Data visualization dashboard
- [ ] Battery backup for RTC
- [ ] Watchdog timer for auto-reset

---

## 👨‍💻 AUTHOR

LEAPSBOX - 2025
Based on Climbox structure with modifications

---

## 📄 LICENSE

Open source - Feel free to modify and distribute

---

## 🆘 SUPPORT

Untuk bantuan dan troubleshooting, hubungi developer atau check:
- Serial Monitor output untuk error messages
- Wiring table untuk koneksi
- Debug mode untuk sensor readings

**Happy Monitoring! 🌊💧**
