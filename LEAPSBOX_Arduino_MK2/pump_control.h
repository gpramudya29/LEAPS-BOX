#ifndef PUMP_CONTROL_H
#define PUMP_CONTROL_H

#include "config.h"

// ================== FUNCTION DECLARATIONS ==================
void kontrolPompaTerjadwal();
void prosesPerintahSerialPompa();
void nyalakanPompaLaut();
void matikanPompaLaut();
void nyalakanPompaBilas();
void matikanPompaBilas();
void cekStatusPompa();

// ================== FUNCTION IMPLEMENTATIONS ==================

// Kontrol pompa terjadwal
// Kontrol pompa terjadwal
void kontrolPompaTerjadwal() {
  int m = menit.toInt();
  int h = jam.toInt();
  int d = tanggal.toInt();

  // POMPA LAUT
  if (!overrideLaut) {
    int repHari = settings[IDX_POMPA_LAUT].data[2];
    int repJam = settings[IDX_POMPA_LAUT].data[3];
    int waktuJam = settings[IDX_POMPA_LAUT].data[4];
    int waktuMin = settings[IDX_POMPA_LAUT].data[5];
    int durMin = settings[IDX_POMPA_LAUT].data[0];
    int durSec = settings[IDX_POMPA_LAUT].data[1];
    unsigned long durasi = (durMin * 60000UL) + (durSec * 1000UL);

    bool shouldRun = false;
    if (d % repHari == 0) {
      if (repJam > 0) {
        if (h % repJam == 0 && m == waktuMin) {
          shouldRun = true;
        }
      } else {
        if (h == waktuJam && m == waktuMin) {
          shouldRun = true;
        }
      }
    }

    // Trigger hanya jika belum trigger di menit ini
    if (shouldRun && !pompaLautOn && lastTriggerMinuteLaut != m) {
      nyalakanPompaLaut();
      lastTriggerMinuteLaut = m;  // Catat menit ini
    }

    if (pompaLautOn && (millis() - pompaLautStartMs >= durasi)) {
      matikanPompaLaut();
    }
  }

  // POMPA BILAS
  if (!overrideBilas) {
    int repHari = settings[IDX_POMPA_BILAS].data[2];
    int repJam = settings[IDX_POMPA_BILAS].data[3];
    int waktuJam = settings[IDX_POMPA_BILAS].data[4];
    int waktuMin = settings[IDX_POMPA_BILAS].data[5];
    int durMin = settings[IDX_POMPA_BILAS].data[0];
    int durSec = settings[IDX_POMPA_BILAS].data[1];
    unsigned long durasi = (durMin * 60000UL) + (durSec * 1000UL);

    bool shouldRun = false;
    if (d % repHari == 0) {
      if (repJam > 0) {
        if (h % repJam == 0 && m == waktuMin) {
          shouldRun = true;
        }
      } else {
        if (h == waktuJam && m == waktuMin) {
          shouldRun = true;
        }
      }
    }

    // Trigger hanya jika belum trigger di menit ini
    if (shouldRun && !pompaBilasOn && lastTriggerMinuteBilas != m) {
      nyalakanPompaBilas();
      lastTriggerMinuteBilas = m;  // Catat menit ini
    }

    if (pompaBilasOn && (millis() - pompaBilasStartMs >= durasi)) {
      matikanPompaBilas();
    }
  }
}

// Proses perintah serial untuk pompa
void prosesPerintahSerialPompa() {
  if (!Serial.available()) return;
  String input = Serial.readStringUntil('\n');
  input.trim();

  if (input == "cek") {
    cekStatusPompa();
    return;
  }

  if (input == "ON1") {
    overrideLaut = false;
    Serial.println("Pompa Air Laut Mati (manual override OFF)");
  } else if (input == "ON2") {
    overrideBilas = true;
    nyalakanPompaBilas();
    Serial.println("Pompa Bilas Nyala (manual override ON)");
  } else if (input == "OFF2") {
    matikanPompaBilas();
    overrideBilas = false;
    Serial.println("Pompa Bilas Mati (manual override OFF)");
  }
}

// Nyalakan pompa laut
void nyalakanPompaLaut() {
  digitalWrite(POMPA_AIR_LAUT_PIN, LOW);
  pompaLautOn = true;
  pompaLautStartMs = millis();
  remainingMeasurements = 2;
  Serial.println("Pompa Air Laut NYALA");
}

// Matikan pompa laut
void matikanPompaLaut() {
  digitalWrite(POMPA_AIR_LAUT_PIN, HIGH);
  pompaLautOn = false;
  Serial.println("Pompa Air Laut MATI");
}

// Nyalakan pompa bilas
void nyalakanPompaBilas() {
  digitalWrite(POMPA_AIR_BILAS_PIN, LOW);
  pompaBilasOn = true;
  pompaBilasStartMs = millis();
  remainingMeasurements = 2;
  Serial.println("Pompa Bilas NYALA");
}

// Matikan pompa bilas
void matikanPompaBilas() {
  digitalWrite(POMPA_AIR_BILAS_PIN, HIGH);
  pompaBilasOn = false;
  Serial.println("Pompa Bilas MATI");
}

// Cek status pompa
void cekStatusPompa() {
  Serial.println("Status Pompa:");
  Serial.print("Pompa Air Laut: ");
  Serial.println(pompaLautOn ? "Nyala" : "Mati");
  Serial.print("Pompa Bilas   : ");
  Serial.println(pompaBilasOn ? "Nyala" : "Mati");
  Serial.print("Jam: ");
  Serial.print(jam);
  Serial.print(":");
  Serial.println(menit);
}

#endif // PUMP_CONTROL_H