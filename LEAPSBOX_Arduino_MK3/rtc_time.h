#ifndef RTC_TIME_H
#define RTC_TIME_H

#include "config.h"

// Fungsi untuk membaca waktu dari RTC
void bacaRTC() {
  Time t = rtc.getTime();
  
  jam = (t.hour < 10) ? ("0" + String(t.hour)) : String(t.hour);
  menit = (t.min < 10) ? ("0" + String(t.min)) : String(t.min);
  detik = (t.sec < 10) ? ("0" + String(t.sec)) : String(t.sec);
  
  tanggal = (t.date < 10) ? ("0" + String(t.date)) : String(t.date);
  bulan = (t.mon < 10) ? ("0" + String(t.mon)) : String(t.mon);
  tahun = String(t.year);
}

#endif // RTC_TIME_H
