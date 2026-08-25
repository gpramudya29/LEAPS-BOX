#ifndef SENSORS_OTHER_H
#define SENSORS_OTHER_H

#include "config.h"

// ================== FUNCTION DECLARATIONS ==================
void readUltrasonic();
void readDHT22();
void readGPS();

// ================== DEBUG FUNCTIONS ==================
void debugUltrasonic(long duration, float distance) {
  if (DEBUG_USONIC) {
    Serial.print(F("[ULTRASONIC] Duration="));
    Serial.print(duration);
    Serial.print(F(" us | Distance="));
    if (isnan(distance)) {
      Serial.println(F("no echo"));
    } else {
      Serial.print(distance, 1);
      Serial.println(F(" cm"));
    }
  }
}

void debugDHT22(float temp, float hum) {
  if (DEBUG_DHT22) {
    Serial.print(F("[DHT22] Temp="));
    Serial.print(temp, 1);
    Serial.print(F(" C | Humidity="));
    Serial.print(hum, 1);
    Serial.println(F(" %"));
  }
}

void debugGPS(double lat, double lng, bool fixed) {
  if (GPS_DEBUG) {
    Serial.print(F("[GPS] Lat="));
    Serial.print(lat, 6);
    Serial.print(F(" | Lng="));
    Serial.print(lng, 6);
    Serial.print(F(" | Fixed="));
    Serial.println(fixed ? F("YES") : F("NO"));
  }
}

// ================== SENSOR READING FUNCTIONS ==================

// Baca Ultrasonic HC-SR04 - Non-blocking
void readUltrasonic() {
  static unsigned long lastRead = 0;
  static bool triggerSent = false;
  static unsigned long triggerTime = 0;

  unsigned long now = millis();

  if (now - lastRead >= ultraIntervalMs) {
    if (!triggerSent) {
      // Kirim trigger pulse
      digitalWrite(ultrasonicTrigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(ultrasonicTrigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(ultrasonicTrigPin, LOW);

      triggerSent = true;
      triggerTime = now;
    } else {
      // Baca echo (dengan timeout)
      long duration = pulseIn(ultrasonicEchoPin, HIGH, 30000UL);  // 30ms timeout

      float distanceCm = NAN;
      if (duration > 0) {
        distanceCm = (duration * 0.0343f) / 2.0f;

        // Apply offset dari EEPROM
        // Apply offset dari EEPROM (meter dan cm)
        int offsetMeter = settings[IDX_OFFSET_ULTRASONIC].data[0];
        int offsetCm = settings[IDX_OFFSET_ULTRASONIC].data[1];
        int totalOffsetCm = (offsetMeter * 100) + offsetCm;
        distanceCm += totalOffsetCm;

        // Validasi range
        if (distanceCm > 0 && distanceCm < 1000) {
          distance_cm = (int)distanceCm;
        } else {
          distance_cm = 0;
        }
      } else {
        distance_cm = 0;
      }

      debugUltrasonic(duration, distanceCm);

      triggerSent = false;
      lastRead = now;
    }
  }
}

// Baca dht22 - Non-blocking
void readDHT22() {
  unsigned long now = millis();

  if (now - lastDhtRead >= DHT_READ_INTERVAL) {
    lastDhtRead = now;

    // Baca dht22
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // Cek apakah pembacaan valid
    if (!isnan(h) && !isnan(t)) {
      g_airRH = h;
      g_airTempC = t;
      debugDHT22(t, h);
    } else {
      if (debugDHT22) Serial.println(F("[dht22] Failed to read!"));
    }
  }
}

// Baca GPS - Non-blocking
void readGPS() {
  unsigned long now = millis();

  if (now - lastGpsRead >= GPS_READ_INTERVAL) {
    lastGpsRead = now;

    // Baca data dari GPS serial
    while (Serial2.available() > 0) {
      char c = Serial2.read();
      gps.encode(c);

      if (GPS_DEBUG) Serial.write(c);
    }

    // Update location jika ada fix
    if (gps.location.isValid()) {
      gpsLat = gps.location.lat();
      gpsLng = gps.location.lng();

      if (!gpsEverFixed) {
        gpsEverFixed = true;
        Serial.println(F("[GPS] First fix acquired!"));
      }

      debugGPS(gpsLat, gpsLng, true);
    } else {
      if (GPS_DEBUG && millis() > 5000) {
        Serial.println(F("[GPS] Waiting for fix..."));
      }
    }
  }
}

#endif  // SENSORS_OTHER_H
