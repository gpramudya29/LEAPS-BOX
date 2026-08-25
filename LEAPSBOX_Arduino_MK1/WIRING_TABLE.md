# LEAPSBOX - WIRING TABLE
## Arduino Mega 2560 Pin Configuration

### KOMUNIKASI & INTERFACE
| Arduino Pin | Tujuan Pin | Komponen | Fungsi | Notes |
|------------|-----------|----------|--------|-------|
| 17 (RX2) | TX | GPS Module | GPS Serial RX | Hardware Serial2 |
| 16 (TX2) | RX | GPS Module | GPS Serial TX | Hardware Serial2 |
| 14 (TX3) | HV1 | Level Shifter | Serial to ESP TX | Hardware Serial3 |
| 15 (RX3) | HV2 | Level Shifter | Serial to ESP RX | Hardware Serial3 |
| 20 (SDA) | SDA | LCD 20x4 I2C | I2C Data | Shared I2C Bus |
| 21 (SCL) | SCL | LCD 20x4 I2C | I2C Clock | Shared I2C Bus |
| 20 (SDA) | SDA | RTC DS3231 | I2C Data | Shared I2C Bus |
| 21 (SCL) | SCL | RTC DS3231 | I2C Clock | Shared I2C Bus |

### INPUT - BUTTONS (5 TOMBOL)
| Arduino Pin | Komponen | Fungsi | Pull-up |
|------------|----------|--------|---------|
| 33 | Button | MODE (kuning) | INPUT_PULLUP |
| 34 | Button | OK (ijo) | INPUT_PULLUP |
| 35 | Button | BACK (merah) | INPUT_PULLUP |
| 36 | Button | UP (biru) | INPUT_PULLUP |
| 37 | Button | DOWN (putih) | INPUT_PULLUP |

### SD CARD MODULE
| Arduino Pin | SD Card Pin | Fungsi |
|------------|------------|--------|
| 53 | CS | Chip Select |
| 52 | SCK | SPI Clock |
| 51 | MOSI | SPI Data Out |
| 50 | MISO | SPI Data In |

### OUTPUT - RELAY (2 POMPA)
| Arduino Pin | Relay Pin | Fungsi |
|------------|----------|--------|
| 39 | IN1 | Relay Pompa Air Laut |
| 41 | IN2 | Relay Pompa Air Bilas |

### SENSOR - ULTRASONIC (HC-SR04)
| Arduino Pin | Sensor Pin | Fungsi |
|------------|-----------|--------|
| 8 | TRIG | Trigger Pulse |
| 7 | ECHO | Echo Receive |

### SENSOR - WATER TEMPERATURE (DS18B20 OneWire)
| Arduino Pin | Sensor Pin | Fungsi |
|------------|-----------|--------|
| 2 | DATA | OneWire Data |

### SENSOR - SALINITAS/CONDUCTIVITY
| Arduino Pin | Sensor Pin | Fungsi |
|------------|-----------|--------|
| 6 | DMS Control | Digital Control (Active LOW) |
| A0 | Analog Out | Analog Reading |

### SENSOR - TDS (Total Dissolved Solids)
| Arduino Pin | Sensor Pin | Fungsi |
|------------|-----------|--------|
| A3 | Analog Out | Analog Reading |

### SENSOR - pH
| Arduino Pin | Sensor Pin | Fungsi |
|------------|-----------|--------|
| A1 | Analog Out | Analog Reading |

### SENSOR - DO (Dissolved Oxygen)
| Arduino Pin | Sensor Pin | Fungsi |
|------------|-----------|--------|
| A2 | Analog Out | Analog Reading |

### SENSOR - DHT11 (Temperature & Humidity)
| Arduino Pin | Sensor Pin | Fungsi |
|------------|-----------|--------|
| 3 | DATA | Digital Data |

---

## PIN SUMMARY
### Digital Pins Used: 2, 3, 6, 7, 8, 14, 15, 16, 17, 20, 21, 33, 34, 35, 36, 37, 39, 41, 50, 51, 52, 53
### Analog Pins Used: A0, A1, A2, A3
### Total Pins Used: 26 pins

---

## POWER REQUIREMENTS
- Arduino Mega: 5V via USB/DC Jack
- GPS Module: 3.3V-5V (Check module specs)
- Level Shifter: 5V + 3.3V dual power
- LCD I2C: 5V
- RTC DS3231: 5V
- All Sensors: 5V (except GPS if 3.3V)
- Relays: 5V (with flyback diodes)

---

## NOTES
1. **I2C Bus**: LCD dan RTC berbagi bus I2C (SDA=20, SCL=21). Pastikan alamat I2C berbeda:
   - LCD: 0x27 atau 0x3F
   - RTC DS3231: 0x68
2. **Level Shifter**: Diperlukan untuk komunikasi 5V (Arduino) ke 3.3V (ESP32)
3. **Salinitas Sensor**: DMS pin 6 harus LOW untuk aktifkan sensor, HIGH untuk matikan
4. **SD Card**: Gunakan SPI hardware pada Mega (pin 50-53)
5. **OneWire**: DS18B20 butuh pull-up resistor 4.7kΩ antara DATA dan VCC
6. **DHT11**: Butuh pull-up resistor 10kΩ antara DATA dan VCC (biasanya sudah built-in di module)
7. **Relay**: Gunakan relay modul dengan optocoupler untuk isolasi

---

## CALIBRATION VALUES (TO BE CONFIGURED)
- **Ultrasonic Offset**: Disimpan di EEPROM (adjustable via menu)
- **pH**: m (slope) dan b (offset) perlu kalibrasi
- **Salinitas**: Konstanta konversi sudah di program
- **TDS**: Konstanta kalibrasi sudah di program
- **DO**: V_zero dan V_air perlu kalibrasi

---

Created: 2025-11-06
Version: 1.0
